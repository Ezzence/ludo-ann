#ifndef AC_PLAYER_H
#define AC_PLAYER_H
#include <QObject>
#include <iostream>
#include "positions_and_dice.h"

#include "floatfann.h"
#include <eigen3/Eigen/Core>
//#include "fann.h"

class acPlayer : public QObject {
    Q_OBJECT
private:
    std::vector<int> posStart;
    //std::vector<int> posStartPrev;
    std::vector<int> posEnd;
    int dice_roll;

    struct fann* actor[4];

    fann_type inputStart[4];

    fann_type inputProgress[4];

    //fann_type inputDistChange[4];
    fann_type inputFinishFail[4];

    fann_type inputSafetyChange[4];

    fann_type inputDangerChange[4];

    /// \brief move will result in figure being hit
    fann_type inputDirectDanger[4];

    struct fann* critic;

    std::vector<std::vector<float> > offsets;
    std::vector<float> devs;  ///< devations are the same for a specific input regardless of offset

    float reward[4];


    int make_decision();
    void runCritic();


public:
    acPlayer();

    bool isEnemyOnZone(int position);

    bool newGame;
signals:
    void select_piece(int);
    void turn_complete(bool);
public slots:
    void start_turn(positions_and_dice relative);
    void post_game_analysis(std::vector<int> relative_pos);
};

#endif // AC_PLAYER_H
