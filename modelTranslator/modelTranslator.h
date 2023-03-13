//
// Created by dave on 03/03/23.
//

#ifndef PHYSICSSIMSWITCHING_MODELTRANSLATOR_H
#define PHYSICSSIMSWITCHING_MODELTRANSLATOR_H

#include "../stdInclude/stdInclude.h"
#include "../physicsSimulators/MuJoCoHelper.h"

struct bodyStateVec{
    string name;
    bool activeLinearDOF[3];
    bool activeAngularDOF[3];
};

struct stateVectorList{
    vector<robot> robots;
    vector<bodyStateVec> bodiesStates;
};

class modelTranslator {
public:
    modelTranslator();
    ~modelTranslator(){

    }

    void initModelTranslator(const char* filePath, int _num_ctrl, vector<robot> _robots, vector<string> bodies);

    MatrixXd returnStateVector();
    bool setStateVector(MatrixXd _stateVector);
    MatrixXd returnControlVector();
    bool setControlVector(MatrixXd _controlVector);
    
    
    virtual double costFunction(MatrixXd Xt, MatrixXd Ut, MatrixXd X_last, MatrixXd U_last) = 0;

    MatrixXd Q;
    MatrixXd R;
    MatrixXd J;

    // int dof;
    int num_ctrl;
    int stateVectorSize;
    struct stateVectorList myStateVector;

    physicsSimulator *activePhysicsSimulator;

protected:
    MatrixXd X_desired;

private:

};

#endif //PHYSICSSIMSWITCHING_MODELTRANSLATOR_H
