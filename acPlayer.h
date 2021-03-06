#ifndef AC_PLAYER_H
#define AC_PLAYER_H
#include <QObject>
#include <iostream>
#include <random>
#include <fstream>
#include "positions_and_dice.h"

#include "floatfann.h"
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
//#include "fann.h"


// manual implementation of Radial Basis Function neurons (input variances are shared among all midNeurons)
struct MidNeuron
{
    float weight;
    std::vector<float> offsetVec;
};


class acPlayer : public QObject {
    Q_OBJECT
private:
    std::vector<int> posStart;
    //std::vector<int> posStartPrev;
    std::vector<int> posEnd;
    int diceRoll;
    size_t lastDecision;

    struct fann* actor[4];

    std::vector<size_t> eligible;

    fann_type inputStart[4];

    fann_type inputSpecialStep[4];

    fann_type inputDangerChange[4];

    fann_type inputFinishFail[4];

    fann_type inputProgress[4];

    /// \brief move will result in figure being hit
    fann_type inputDirectDanger[4];

    std::vector<float> lastInputVec[4];
    std::vector<float> prevInputVec[4];

    struct fann* critic;

    std::vector<std::vector<float> > offsets;
    std::vector<float> devs;  ///< devations are the same for a specific input regardless of offset
    Eigen::MatrixXf S;          ///< diagonal matrix of variances
    std::vector<MidNeuron> midNeurons;

    std::default_random_engine generator;
    std::normal_distribution<float> gaussDistribution;
    float epsilon[4]; ///< result of V and random noise
    float prevEpsilon[4];

    float reward[4];

    unsigned int numInputs;
    unsigned int numMid;

    int make_decision();
    void runCritic();
    void runICO(std::vector<int>& prevPosStart);


public:
    acPlayer();

    void printMidNeurons();
    bool isEnemyOnZone(int position);

    std::vector<float> inputWeightVec;

    bool newGame;
    int numRuns;
    std::ofstream logFile;
signals:
    void select_piece(int);
    void turn_complete(bool);
public slots:
    void start_turn(positions_and_dice relative);
    void post_game_analysis(std::vector<int> relative_pos);
};

#endif // AC_PLAYER_H
