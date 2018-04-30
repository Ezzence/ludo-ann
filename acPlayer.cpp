#include "acPlayer.h"

acPlayer::acPlayer() : newGame(true){


    // ACTORS
    const unsigned int num_input = 5;
    const unsigned int num_output = 1;
    const unsigned int num_layers = 2;

    for(int i = 0; i < 4; ++i)
    {
        actor[i] = fann_create_standard(num_layers, num_input, num_output);
        fann_set_activation_function_output(actor[i], FANN_LINEAR);

        fann_set_weight(actor[i], 0, 6, 200.f); // inputStart
        fann_set_weight(actor[i], 1, 6, 2.f); // inputProgess
        fann_set_weight(actor[i], 2, 6, -1000.f); // inputFinishFail
        fann_set_weight(actor[i], 3, 6, -60.f); // inputDangerChange
        fann_set_weight(actor[i], 4, 6, -1000.f); // inputDirectDanger
        fann_set_weight(actor[i], 5, 6, 0);

    }


    fann_print_connections(actor[0]);

    // ---------------------------------------------------------------------

    // CRITIC



    // ----------------------------------------------------------------------

    //fann_get_weights(ann, weights);



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
        if(!(posStart[i] == 99 || (posStart[i] == -1 && dice_roll != 6)))
        {
            eligible.push_back(i);
        }
    }
    //printf("%i: ", dice_roll);

    fann_type bestOutput = -9999.f;
    size_t bestIndex = -1;
    for(size_t i = 0; i < eligible.size(); ++i)
    {
        int myPos = posStart[eligible[i]];
        // initialize inputs
        inputProgress[eligible[i]] = myPos;
        //inputDistChange[eligible[i]] = dice_roll;
        if(myPos == -1)
        {
            inputStart[eligible[i]] = 1;
        }
        else
        {
            inputStart[eligible[i]] = 0;
        }
        if(myPos > 50 && ((myPos + dice_roll) != 56))
        {
            inputFinishFail[eligible[i]] = 1.f;
        }
        else
        {
            inputFinishFail[eligible[i]] = 0;
        }

        // CHECK DANGER OF NEARBY ENEMY FIGURES
        fann_type danger = 0;
        for(size_t j = 4; j < 16; ++j)
        {
            // AND NOT ON SAFE
            int pos = posStart[j];
            bool safe = (myPos == 0);
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
                danger = 1;
            }
        }
        fann_type nextDanger = 0;
        for(size_t j = 4; j < 16; ++j)
        {
            int pos = posStart[j];
            bool safe = (myPos == 0);
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
            else if((pos < (myPos + dice_roll)) && (pos > (myPos + dice_roll - 5)))
            {
                nextDanger = 1;
            }
        }
        inputDangerChange[eligible[i]] = nextDanger - danger;
        // -----------------------------------------------------

        // CHECK DIRECT DANGER
        inputDirectDanger[eligible[i]] = 0;
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
        }


        // -------------------------------------

        fann_type inputs[5] = {inputStart[eligible[i]], inputProgress[eligible[i]], inputFinishFail[eligible[i]], inputDangerChange[eligible[i]], inputDirectDanger[eligible[i]]};
        fann_type* output = fann_run(actor[eligible[i]], inputs);
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

    /*for(int i = 0; i < 4; ++i)
    {
        if(disqualified[i] == true)
        {
            continue;
        }
        inputGoalDist[i] = pos_start_of_turn[i] + 1;
        if(pos_start_of_turn[i] == -1)
        {
            inputStart[i] = 1;
        }
        else
        {
            inputStart[i] = 0;
        }
    }


    printf("\n %i: %f %f %f %f \n", dice_roll, inputStart[0], inputStart[1], inputStart[2], inputStart[3]);
    fflush(stdout);

    fann_type input[4] = {1.f, 1.f, 0, 0};
    //fann_type* output = fann_run(actor[0], input);
    //printf("\n %f", output[0]);
    //fflush(stdout);*/




    /*std::vector<int> valid_moves;
    if(dice_roll == 6){
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i]<0){
                valid_moves.push_back(i);
            }
        }
    }
    for(int i = 0; i < 4; ++i){
        if(pos_start_of_turn[i]>=0 && pos_start_of_turn[i] != 99){
            valid_moves.push_back(i);
        }
    }
    if(valid_moves.size()==0){
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i] != 99){
                valid_moves.push_back(i);
            }
        }
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> piece(0, valid_moves.size()-1);
    int select = piece(gen);
    return valid_moves[select];*/

}

void acPlayer::runActor()
{
    /*Eigen::Vector4f vec;
    vec << 1.1f, 2.f, 3.f, 4.f;
    Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
    std::cout << std::endl << vec.format(CleanFmt) << std::endl;
    fflush(stdin);*/

}

void acPlayer::start_turn(positions_and_dice relative){

    // if new game, posStart and posStartPrev should be the same, POSSIBLE ISSUE WITH RESETTING GAME
    if(newGame)
    {
        posStart = relative.pos;
        newGame = false;
    }

    // save previous state
    std::vector<int> posStartPrev(posStart);
    posStart = relative.pos;

    // punish if a figure was sent home since last state
    for(int i = 0; i < 4; ++i)
    {
        // first, reset in every turn
        reward[i] = 0;
        if(posStart[i] == -1 && posStartPrev[i] != -1)
        {
            reward[i] += -1.f;
        }
    }
    std::cout << std::endl << reward[0] << " " << reward[1] << " " << reward[2] << " " << reward[3] << " ";
    fflush(stdin);
    runActor();

    dice_roll = relative.dice;
    int decision = make_decision();
    emit select_piece(decision);
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
