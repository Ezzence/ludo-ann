#include "acPlayer.h"

#define V_MAX 1.f
#define V_MIN -1.f
#define LEARNING_RATE 0.5f

acPlayer::acPlayer() : lastDiceRoll(0), gaussDistribution(0, 1.f), prevV(0), newGame(true)
{


    // ACTORS
    const unsigned int num_input = 4;
    const unsigned int num_output = 1;
    const unsigned int num_layers = 2;

    for(int i = 0; i < 4; ++i)
    {
        actor[i] = fann_create_standard(num_layers, num_input, num_output);
        fann_set_activation_function_output(actor[i], FANN_LINEAR);

        /*fann_set_weight(actor[i], 0, 6, 200.f); // inputStart
        fann_set_weight(actor[i], 1, 6, 2.f); // inputProgess
        fann_set_weight(actor[i], 2, 6, -1000.f); // inputFinishFail
        fann_set_weight(actor[i], 3, 6, -60.f); // inputDangerChange
        fann_set_weight(actor[i], 4, 6, -1000.f); // inputDirectDanger
        fann_set_weight(actor[i], 5, 6, 0);
        */
        fann_set_weight(actor[i], 0, 5, 0.f); // inputStart
        fann_set_weight(actor[i], 1, 5, 0.f); // inputProgess
        fann_set_weight(actor[i], 2, 5, 0.f); // inputDangerChange
        fann_set_weight(actor[i], 3, 5, 0.f); // inputFinishFail
        fann_set_weight(actor[i], 4, 5, 0.f);
    }

    inputWeightVec = {0, 0, 0, 0};



    fann_print_connections(actor[0]);

    // ---------------------------------------------------------------------

    // CRITIC



    /*for(auto offset : offsets)
    {
        tmp += offset.size();
    }
    numInputs = tmp;*/



    /*critic = fann_create_sparse(1, numLayers, numInputs, numMid, numOutput);
    fann_randomize_weights(critic, 0, 0);

    fann_print_connections(critic);*/

    /*int inputNeuronsPassed = 0;
    size_t midNeuronSkipCounter = 1;
    for(size_t i = 0; i < offsets.size(); ++i)
    {
        for(size_t j = 0; j < offsets[i].size(); ++j)
        {
            int iter = j + offsets[i].size(); // + offsets[i].size() jut to make sure number is higher or equal to modulo
            for(size_t k = numInputs; k < numInputs + numMid; k += midNeuronSkipCounter)
            {
                if(iter % offsets[i].size() == 0)
                {
                    for(size_t l = 0; l < midNeuronSkipCounter; ++l)
                    {
                        fann_set_weight(critic, inputNeuronsPassed, k + 1 + l, 1.f);
                    }
                }
                ++iter;
            }
            ++inputNeuronsPassed;
        }
        midNeuronSkipCounter *= offsets[i].size();
    }

    fann_print_connections(critic);*/


    offsets = { {0.f, 1.f}, {0.f, 50.f}, {-6.f, 0.f, 6.f}, {0.f, 1.f} }; //inputStart, inputProgress, inputDangerChange, inputFinishFail
    devs = {0.2f, 5.f, 1.f, 0.2f};

    S = Eigen::MatrixXf::Zero(devs.size(), devs.size());
    for(size_t i = 0; i < devs.size(); ++i)
    {
        S(i, i) = devs[i]*devs[i];  // ?????????????????
    }
    S = S.inverse();
    std::cout << S << std::endl;
    fflush(stdin);

    prevV = 0;

    numInputs = offsets.size();

    for(size_t i = 0; i < 4; ++i)
    {
        for(size_t j = 0; j < numInputs; ++j)
        {
            lastInputVec[i].push_back(0);
            prevInputVec[i].push_back(0);
        }
    }

    int tmp = 1;

    for(auto offset : offsets)
    {
        tmp *= offset.size();
    }

    numMid = tmp;
    const unsigned int numOutput = 1;
    const unsigned int numLayers = 3;


    for(size_t i = 0; i < numMid; ++i)
    {
        midNeurons.push_back(MidNeuron());
        midNeurons.back().offsetVec.resize(offsets.size());
        midNeurons.back().weight = 0;
    }

    int inputNeuronsPassed = 0;
    size_t midNeuronSkipCounter = 1;
    for(size_t i = 0; i < offsets.size(); ++i)
    {
        for(size_t j = 0; j < offsets[i].size(); ++j)
        {
            int iter = j + offsets[i].size(); // + offsets[i].size() jut to make sure number is higher or equal to modulo
            for(size_t k = 0; k < numMid; k += midNeuronSkipCounter)
            {
                if(iter % offsets[i].size() == 0)
                {
                    for(size_t l = 0; l < midNeuronSkipCounter; ++l)
                    {
                        midNeurons[k + l].offsetVec[i] = offsets[i][j];
                    }
                }
                ++iter;
            }
            ++inputNeuronsPassed;
        }
        midNeuronSkipCounter *= offsets[i].size();
    }

    printMidNeurons();

    // ----------------------------------------------------------------------

    //fann_get_weights(ann, weights);



}

void acPlayer::printMidNeurons()
{
    std::cout << "\nNum: " << midNeurons.size();

    for(size_t i = 0; i < midNeurons.size(); ++i)
    {
        std::cout << std::endl << "weight: " << midNeurons[i].weight << "   ";
        for(size_t j = 0; j < midNeurons[i].offsetVec.size(); ++j)
        {
            std::cout << " " << midNeurons[i].offsetVec[j];
        }
    }
}

bool acPlayer::isEnemyOnZone(int position)
{
    bool val = false;
    for(int j = 4; j < 16; ++j)
    {
        if(posStart[j] == position)
        {
            val = true;
        }
    }
    return val;
}

int acPlayer::make_decision(){

    std::vector<size_t> eligible;

    // set which figures are eligible to move with the current dice roll
    for(int i = 0; i < 4; ++i)
    {
        if(!(posStart[i] == 99 || (posStart[i] == -1 && diceRoll != 6)))
        {
            eligible.push_back(i);
        }
    }

    // save and reset lastInputVec
    for(size_t i = 0; i < 4; ++i)
    {
        for(size_t j = 0; j < lastInputVec[i].size(); ++j)
        {
            prevInputVec[i][j] = lastInputVec[i][j];
            lastInputVec[i][j] = 0;
        }
    }

    fann_type bestOutput = -99999.f;
    size_t bestIndex = -1;
    for(size_t i = 0; i < eligible.size(); ++i)
    {
        int myPos = posStart[eligible[i]];
        // initialize inputs
        inputProgress[eligible[i]] = myPos;
        if(myPos == -1)
        {
            inputStart[eligible[i]] = 1.f;
        }
        else
        {
            inputStart[eligible[i]] = 0;
        }
        if(myPos > 50 && ((myPos + diceRoll) > 56))
        {
            inputFinishFail[eligible[i]] = 1.f;
        }
        else
        {
            inputFinishFail[eligible[i]] = 0;
        }

        // CHECK DANGER OF NEARBY ENEMY FIGURES
        fann_type danger = 0;
        for(size_t j = 4; j < 16; ++j) // current danger
        {
            // AND NOT ON SAFE
            int pos = posStart[j];
            bool safe = (myPos == -1 || myPos == 0 || myPos > 50);
            if(safe)
            {
                //no danger
            }
            else if(pos < 1)
            {
                // disregard enemy figure, no danger
            }
            else if(myPos < 10 && pos > 40)
            {
                // wrap around enemy position
                pos -= 52;
            }
            else if((pos < myPos) && (pos > (myPos - 5)))
            {
                danger += 1; // 1 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            }
        }
        fann_type nextDanger = 0;
        for(size_t j = 4; j < 16; ++j) // would-be danger
        {
            int pos = posStart[j];
            bool safe = (myPos == -1 || myPos == 0 || myPos > 50);
            if(safe)
            {
                // no danger
            }
            else if(pos < 1)
            {
                // disregard enemy figure, no danger
            }
            else if(myPos < 10 && pos > 40)
            {
                // wrap around enemy position
                pos -= 52;
            }
            else if((pos < (myPos + diceRoll)) && (pos > (myPos + diceRoll - 5)))
            {
                nextDanger += 1;
            }
        }
        // CHECK DIRECT DANGER
        std::vector<int> allSafeZones = {13, 26, 39};
        for(size_t j = 0; j < allSafeZones.size(); ++j)
        {
            if(myPos + diceRoll == allSafeZones[j])
            {
                if(isEnemyOnZone(allSafeZones[j]))
                {
                    nextDanger += 6;
                }
            }
        }
        inputDangerChange[eligible[i]] = nextDanger - danger;
        // -----------------------------------------------------

        // CHECK DIRECT DANGER (EDIT: copied to dangerChange)
        /*inputDirectDanger[eligible[i]] = 0;
        std::vector<int> allSafeZones = {13, 26, 39};
        for(size_t j = 0; j < allSafeZones.size(); ++j)
        {
            if(myPos + dice_roll == allSafeZones[j])
            {
                if(isEnemyOnZone(allSafeZones[j]))
                {
                    inputDirectDanger[eligible[i]] = 1;
                }
            }
        }*/


        // -------------------------------------

        //fann_type inputs[5] = {inputStart[eligible[i]], inputProgress[eligible[i]], inputFinishFail[eligible[i]], inputDangerChange[eligible[i]], inputDirectDanger[eligible[i]]};
        lastInputVec[eligible[i]] = {inputStart[eligible[i]], 0, inputDangerChange[eligible[i]], 0};
        fann_type inputs[4] = {inputStart[eligible[i]], inputProgress[eligible[i]], inputDangerChange[eligible[i]], inputFinishFail[eligible[i]]};
        fann_type* output = fann_run(actor[eligible[i]], inputs);
        if(eligible[i] == 0)
        {
            output[0] += epsilon;   // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        }
        if(output[0] > bestOutput)
        {
            bestOutput= output[0];
            bestIndex = eligible[i];
        }


        //printf("%i %i: %f \n", eligible[i], posStart[eligible[i]], output[0]);
        //fflush(stdout);
    }
    /*for(size_t j = 4; j < 16; ++j)
    {
        printf("%i ", pos_start_of_turn[j]);
    }*/
    //printf("\n \n");
    //fflush(stdout);

    return bestIndex;

}

void acPlayer::runCritic()
{
    /*Eigen::Vector4f vec;
    vec << 1.1f, 2.f, 3.f, 4.f;
    Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
    std::cout << std::endl << vec.format(CleanFmt) << std::endl;
    fflush(stdin);*/


    // PREVIOUS INPUTS ------------------------------------------------------------

    Eigen::VectorXf prevX(numInputs);
    for(size_t i = 0; i < numInputs; ++i)
    {
        prevX[i] = prevInputVec[0][i];
    }

    std::vector<float> prevY_j;
    std::vector<float> prevA_j;

    for(size_t i = 0; i < midNeurons.size(); ++i)
    {
        Eigen::VectorXf c(numInputs);
        for(size_t j = 0; j < midNeurons[i].offsetVec.size(); ++j)
        {
            c[j] = midNeurons[i].offsetVec[j];
        }
        Eigen::VectorXf tmp = S*(prevX - c);
        prevA_j.push_back(std::exp(-tmp.squaredNorm()));
    }

    float sum_prevA_j = 0;
    for(auto a : prevA_j)
    {
        sum_prevA_j += a;
    }

    for(size_t i = 0; i < midNeurons.size(); ++i)
    {
        if(sum_prevA_j > 0)
        {
            prevY_j.push_back(prevA_j[i]/sum_prevA_j);
        }
        else
        {
            prevY_j.push_back(0);
        }
    }


    // NEW INPUTS ---------------------------------------------------------------

    Eigen::VectorXf x(numInputs);
    for(size_t i = 0; i < numInputs; ++i)
    {
        x[i] = lastInputVec[0][i];
    }

    std::vector<float> y_j;
    std::vector<float> a_j;

    for(size_t i = 0; i < midNeurons.size(); ++i)
    {
        Eigen::VectorXf c(numInputs);
        for(size_t j = 0; j < midNeurons[i].offsetVec.size(); ++j)
        {
            c[j] = midNeurons[i].offsetVec[j];
        }
        Eigen::VectorXf tmp = S*(x - c);
        a_j.push_back(std::exp(-tmp.squaredNorm()));
    }

    float sum_a_j = 0;
    for(auto a : a_j)
    {
        sum_a_j += a;
    }

    for(size_t i = 0; i < midNeurons.size(); ++i)
    {
        if(sum_a_j > 0)
        {
            y_j.push_back(a_j[i]/sum_a_j);
        }
        else
        {
            y_j.push_back(0);
        }
    }

    std::vector<float> v_j;
    float V = 0;

    for(size_t i = 0; i < midNeurons.size(); ++i)
    {
        v_j.push_back(midNeurons[i].weight);
        V += v_j[i]*y_j[i];
    }

    float gaussNoise = gaussDistribution(generator);
    epsilon = 10*gaussNoise*std::min(1.f, std::max(0.f, (V_MAX - prevV)/(V_MAX - V_MIN)));

    float delta = reward[0] + 0.1f*V - prevV;
    prevV = V;

    std::vector<float> dw;
    for(size_t i = 0; i < numInputs; ++i)
    {
        dw.push_back(LEARNING_RATE*epsilon*delta*prevX[i]);
    }
    for(size_t i = 0; i < inputWeightVec.size(); ++i)
    {
        inputWeightVec[i] += dw[i];
    }

    for(int i = 0; i < 4; ++i)
    {

        fann_set_weight(actor[i], 0, 5, inputWeightVec[0]); // inputStart
        fann_set_weight(actor[i], 1, 5, inputWeightVec[1]); // inputProgess
        fann_set_weight(actor[i], 2, 5, inputWeightVec[2]); // inputDangerChange
        fann_set_weight(actor[i], 3, 5, inputWeightVec[3]); // inputFinishFail
    }
    std::cout << std::endl << inputWeightVec[0] << " " << inputWeightVec[1] << " " << inputWeightVec[2] << " " << inputWeightVec[3] << "    " << delta << std::endl;
    fflush(stdin);

    std::vector<float> dv;
    for(size_t i = 0; i < midNeurons.size(); ++i)
    {
        dv.push_back(LEARNING_RATE*delta*prevY_j[i]);
        midNeurons[i].weight += dv[i];
    }

}

void acPlayer::start_turn(positions_and_dice relative){

    // if new game, posStart and posStartPrev should be the same, POSSIBLE ISSUE WITH RESETTING GAME
    if(newGame)
    {
        posStart = relative.pos;
    }

    // save previous state
    std::vector<int> posStartPrev(posStart);
    posStart = relative.pos;

    // REWARDS
    for(size_t i = 0; i < 4; ++i)
    {
        // first, reset in every turn
        reward[i] = 0;

        // punish if a figure was sent home and there could have been a better choice
        if(posStart[i] == -1 && posStartPrev[i] != -1 && lastDecision == i)
        {
            bool missedBetterChoice = false;
            for(size_t j = 0; j < 4; ++j)
            {
                if(j == i)
                {
                    continue;
                }
                if( (diceRoll == 6 && posStartPrev[j] == -1) || (posStartPrev[j] > -1 && posStartPrev[j] < 99 && inputDangerChange[j] < inputDangerChange[i]))     // INDEX !!!!!!!!!! (danger input)
                {
                    // means another figure was available without danger
                    missedBetterChoice = true;
                }
            }

            if(missedBetterChoice)
            {
                reward[i] += -1.f;
            }
        }

        // punish for missing finish
        //if(posStartPrev[i] > 50 && (posStart[i] < posStartPrev[i] + diceRoll) && lastDecision == i)
        /*if(inputFinishFail[i] > 0.1f && lastDecision == i)
        {
            bool missedBetterChoice = false;
            for(size_t j = 0; j < 4; ++j)
            {
                if(j == i)
                {
                    continue;
                }
                //if( (posStartPrev[j] == -1 && diceRoll == 6) || (posStartPrev[j] > -1 && posStartPrev[j] < 51) )
                if( (diceRoll == 6 && posStartPrev[j] == -1) || (posStartPrev[j] > -1 && posStartPrev[j] < 50 && inputDangerChange[j] < 0.1f) )
                {
                    missedBetterChoice = true;
                }

                if(missedBetterChoice)
                {
                    reward[i] += -0.1f;
                //}
            //}
        }*/

        if(posStartPrev[i] == -1 && diceRoll == 6 && lastDecision == i)
        {
            reward[i] += 1.f;
        }

        /*if(posStart[i] == 99 && posStartPrev[i] != 99)
        {
            reward[i] += 1.f;
            //std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
        }*/
    }
    //std::cout << std::endl << reward[0] << " " << reward[1] << " " << reward[2] << " " << reward[3] << " ";
    //fflush(stdin);
    if(!newGame)
    {
        make_decision();
        runCritic();
    }
    else
    {
        newGame = false;
    }

    //lastDiceRoll = diceRoll;
    diceRoll = relative.dice;
    lastDecision = make_decision();
    emit select_piece(lastDecision);
}

void acPlayer::post_game_analysis(std::vector<int> relative_pos){
    posEnd = relative_pos;
    bool game_complete = true;
    for(int i = 0; i < 4; ++i){
        if(posEnd[i] < 99){
            game_complete = false;
        }
    }
    if(game_complete)
    {
        newGame = true;
    }
    emit turn_complete(game_complete);
}
