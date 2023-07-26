#include "stdInclude.h"
#include "ros/ros.h"
#include "fileHandler.h"

// --------------------- different scenes -----------------------
#include "doublePendulum.h"
#include "reaching.h"
#include "twoDPushing.h"
#include "boxFlick.h"
#include "locomotion.h"

#include "visualizer.h"
#include "MuJoCoHelper.h"

#include "interpolated_iLQR.h"
#include "stomp.h"
#include "gradDescent.h"

// ------------ MODES OF OEPRATION -------------------------------
#define SHOW_INIT_CONTROLS          0
#define ILQR_ONCE                   1
#define MPC_CONTINOUS               2
#define MPC_UNTIL_COMPLETE          3
#define GENERATE_TEST_SCENES        4
#define GENERATE_TESTING_DATA       5
#define GENERATE_FILTERING_DATA     6
#define DEFAULT_KEYBOARD_CONTROL    7
#define GENERIC_TESTING             9

enum scenes{
    pendulum = 0,
    reaching = 1,
    cylinderPushing = 2,
    cylinderPushingMildClutter = 3,
    cylinderPushingHeavyClutter = 4,
    cylinderPushingMldClutterConstrained = 5,
    boxPushingToppling = 6,
    boxFlicking = 7,
    boxFlickingMildClutter = 8,
    boxFlickingHeavyClutter = 9,
    anymal_locomotion = 10
};

// --------------------- Global class instances --------------------------------
std::shared_ptr<modelTranslator> activeModelTranslator;
std::shared_ptr<differentiator> activeDifferentiator;
std::shared_ptr<optimiser> activeOptimiser;
std::shared_ptr<interpolatediLQR> iLQROptimiser;
std::shared_ptr<stomp> stompOptimiser;
std::shared_ptr<gradDescent> gradDescentOptimiser;
std::shared_ptr<visualizer> activeVisualiser;
std::shared_ptr<fileHandler> yamlReader;

int interpolationMethod = linear;
int keyPointMethod = setInterval;
//int keyPointMethod = setInterval;

bool mpcVisualise = false;

void showInitControls();
void optimiseOnceandShow();
void MPCUntilComplete(bool &sucess, double &finalDist, double &totalOptimisationTime, double &totalExecutionTime, double &avgTimeGettingDerivs, double &avgPercentDerivs, double &avgTimeBP, double &avgTimeFP,
                      int MAX_TASK_TIME, int REPLAN_TIME, int OPT_HORIZON);
void MPCContinous();
void generateTestScenes();
void keyboardControl();

void generateTestingData_MPC();
void generateTestingData();
void generateFilteringData();

void genericTesting();

int main(int argc, char **argv) {
    cout << "program started \n";
    std::string optimiser;
    int mode;
    int task;
    std::string taskInitMode;

    yamlReader = std::make_shared<fileHandler>();
    yamlReader->readSettingsFile("/generalConfig.yaml");
    optimiser = yamlReader->optimiser;
    mode = yamlReader->project_display_mode;
    task = yamlReader->taskNumber;
    taskInitMode = yamlReader->taskInitMode;

    MatrixXd startStateVector(1, 1);

    if(mode == GENERATE_TESTING_DATA){
//        generateTestingData_MPC();
        generateTestingData();
        return 1;
    }

    if(task == pendulum){
        std::shared_ptr<doublePendulum> myDoublePendulum = std::make_shared<doublePendulum>();
        activeModelTranslator = myDoublePendulum;
    }
    else if(task == reaching){
        std::shared_ptr<pandaReaching> myReaching = std::make_shared<pandaReaching>();
        activeModelTranslator = myReaching;
    }
    else if(task == cylinderPushing){
        std::shared_ptr<twoDPushing> myTwoDPushing = std::make_shared<twoDPushing>(noClutter);
        activeModelTranslator = myTwoDPushing;

    }
    else if(task == cylinderPushingMildClutter){
        std::shared_ptr<twoDPushing> myTwoDPushing = std::make_shared<twoDPushing>(lowClutter);
        activeModelTranslator = myTwoDPushing;

    }
    else if(task == cylinderPushingHeavyClutter){
        std::shared_ptr<twoDPushing> myTwoDPushing = std::make_shared<twoDPushing>(heavyClutter);
        activeModelTranslator = myTwoDPushing;

    }
    else if(task == cylinderPushingMldClutterConstrained){
        std::shared_ptr<twoDPushing> myTwoDPushing = std::make_shared<twoDPushing>(constrainedClutter);
        activeModelTranslator = myTwoDPushing;
    }
    else if(task == boxPushingToppling){
        cout << "not implemented task yet " << endl;
        return -1;
    }
    else if(task == boxFlicking){
        std::shared_ptr<boxFlick> myBoxFlick = std::make_shared<boxFlick>(noClutter);
        activeModelTranslator = myBoxFlick;
    }
    else if(task == boxFlickingMildClutter){
        std::shared_ptr<boxFlick> myBoxFlick = std::make_shared<boxFlick>(lowClutter);
        activeModelTranslator = myBoxFlick;
    }
    else if(task == boxFlickingHeavyClutter){
        std::shared_ptr<boxFlick> myBoxFlick = std::make_shared<boxFlick>(heavyClutter);
        activeModelTranslator = myBoxFlick;
    }
    else if(task == anymal_locomotion){
        std::shared_ptr<locomotion_anymal> myLocomotion = std::make_shared<locomotion_anymal>();
        activeModelTranslator = myLocomotion;
    }
    else{
        std::cout << "invalid scene selected, exiting" << std::endl;
    }

    startStateVector.resize(activeModelTranslator->stateVectorSize, 1);
    startStateVector = activeModelTranslator->X_start;

    // random start and goal state
    std::string taskPrefix = activeModelTranslator->modelName;
    if(taskInitMode == "random"){
        startStateVector = activeModelTranslator->returnRandomStartState();
        activeModelTranslator->X_start = startStateVector;
        activeModelTranslator->X_desired = activeModelTranslator->returnRandomGoalState(startStateVector);

        yamlReader->saveTaskToFile(taskPrefix, 0, activeModelTranslator->X_start, activeModelTranslator->X_desired);
    }
    else if(taskInitMode == "fromCSV"){
        yamlReader->loadTaskFromFile(taskPrefix, yamlReader->csvRow, startStateVector, activeModelTranslator->X_desired);
        activeModelTranslator->X_start = startStateVector;
        cout << "start state " << activeModelTranslator->X_start << endl;
        cout << "desired state " << activeModelTranslator->X_desired << endl;
    }

    activeDifferentiator = std::make_shared<differentiator>(activeModelTranslator, activeModelTranslator->myHelper);
    activeModelTranslator->setStateVector(startStateVector, MASTER_RESET_DATA);
    activeModelTranslator->activePhysicsSimulator->stepSimulator(5, MASTER_RESET_DATA);
    activeModelTranslator->activePhysicsSimulator->appendSystemStateToEnd(MASTER_RESET_DATA);

//    double cost = activeModelTranslator->costFunction(MASTER_RESET_DATA, false);

    //Instantiate my optimiser
    activeVisualiser = std::make_shared<visualizer>(activeModelTranslator);
//    activeVisualiser->render("test");
//
//    activeModelTranslator->setStateVector(activeModelTranslator->X_desired, MAIN_DATA_STATE);
//    activeModelTranslator->activePhysicsSimulator->stepSimulator(1, MAIN_DATA_STATE);
//    activeVisualiser->render("test");

    if(optimiser == "interpolated_iLQR"){
        yamlReader->readOptimisationSettingsFile(opt_iLQR);
        iLQROptimiser = std::make_shared<interpolatediLQR>(activeModelTranslator, activeModelTranslator->activePhysicsSimulator, activeDifferentiator, yamlReader->maxHorizon, activeVisualiser, yamlReader);
        activeOptimiser = iLQROptimiser;
    }
    else if(optimiser == "stomp"){
        yamlReader->readOptimisationSettingsFile(opt_stomp);
        stompOptimiser = std::make_shared<stomp>(activeModelTranslator, activeModelTranslator->activePhysicsSimulator, yamlReader, activeDifferentiator, yamlReader->maxHorizon, 50);
        activeOptimiser = stompOptimiser;
    }
    else if(optimiser == "gradDescent"){
        yamlReader->readOptimisationSettingsFile(opt_gradDescent);
        gradDescentOptimiser = std::make_shared<gradDescent>(activeModelTranslator, activeModelTranslator->activePhysicsSimulator, activeDifferentiator, activeVisualiser, yamlReader->maxHorizon, yamlReader);
        activeOptimiser = gradDescentOptimiser;
    }
    else{
        cout << "invalid optimiser selected, exiting" << endl;
        return -1;
    }

    if(mode == SHOW_INIT_CONTROLS){
        cout << "SHOWING INIT CONTROLS MODE \n";
        showInitControls();
    }
    else if(mode == ILQR_ONCE){
        cout << "OPTIMISE TRAJECTORY ONCE AND DISPLAY MODE \n";
        activeOptimiser->verboseOutput = true;
        optimiseOnceandShow();
    }
    else if(mode == MPC_CONTINOUS){
        cout << "CONTINOUS MPC MODE \n";
        cout << "mode is disabled for now \n";
        return 0;
//        MPCContinous();
    }
    else if(mode == MPC_UNTIL_COMPLETE){
        cout << "MPC UNTIL TASK COMPLETE MODE \n";
        bool _;
        double __, ___, ____, _____, ______, _7;
        double finalDist;
        activeOptimiser->setupTestingExtras(1000, interpolationMethod, keyPointMethod, activeOptimiser->min_interval, activeOptimiser->approximate_backwardsPass);
        std::vector<MatrixXd> initSetupControls = activeModelTranslator->createInitSetupControls(1000);
        activeModelTranslator->activePhysicsSimulator->copySystemState(MASTER_RESET_DATA, MAIN_DATA_STATE);
        activeOptimiser->verboseOutput = true;
        mpcVisualise = true;

        // No clutter - 1800 - 500 - 1800
        MPCUntilComplete(_, finalDist, __, ___, ____, _____, ______, _7, 2500, 5, 100);
    }
    else if(mode == GENERATE_TEST_SCENES){
        cout << "TASK INIT MODE \n";
        generateTestScenes();
    }
    else if(mode == GENERATE_TESTING_DATA){
        cout << "GENERATE TESTING DATA MODE \n";
        generateTestingData();
    }
    else if(mode == GENERATE_FILTERING_DATA){
        cout << "GENERATE FILTERING DATA MODE \n";
        generateFilteringData();
    }
    else if(mode == DEFAULT_KEYBOARD_CONTROL){
        cout << "KEYBOARD TESTING MODE \n";
        keyboardControl();
    }
    else if(mode == GENERIC_TESTING){
        genericTesting();
    }
    else{
        cout << "INVALID MODE OF OPERATION OF PROGRAM \n";
    }
    return 0;
}

void genericTesting(){
    // Initialise trajectory from csv and genrate A, B, steta and controls
    std::string taskPrefix = activeModelTranslator->modelName;
    MatrixXd startStateVector;
    startStateVector.resize(activeModelTranslator->stateVectorSize, 1);

    for(int i = 0; i < 100; i++){

        yamlReader->loadTaskFromFile(taskPrefix, i, startStateVector, activeModelTranslator->X_desired);
        activeModelTranslator->X_start = startStateVector;

        cout << "start state " << activeModelTranslator->X_start << endl;

        activeModelTranslator->setStateVector(startStateVector, MASTER_RESET_DATA);
        activeModelTranslator->activePhysicsSimulator->stepSimulator(5, MASTER_RESET_DATA);
        if(activeModelTranslator->activePhysicsSimulator->checkIfDataIndexExists(0)){
            activeModelTranslator->activePhysicsSimulator->copySystemState(0, MASTER_RESET_DATA);
        }
        else{

            activeModelTranslator->activePhysicsSimulator->appendSystemStateToEnd(MASTER_RESET_DATA);
        }

        int setupHorizon = 1000;
        int optHorizon = 2980;
        std::vector<MatrixXd> initSetupControls = activeModelTranslator->createInitSetupControls(setupHorizon);
        activeModelTranslator->activePhysicsSimulator->copySystemState(MASTER_RESET_DATA, MAIN_DATA_STATE);
        std::vector<MatrixXd> initOptimisationControls = activeModelTranslator->createInitOptimisationControls(optHorizon);

        activeOptimiser->horizonLength = optHorizon;
        activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);

        activeOptimiser->rolloutTrajectory(0, true, initOptimisationControls);
        activeOptimiser->generateDerivatives();

        yamlReader->saveTrajecInfomation(activeOptimiser->A, activeOptimiser->B, activeOptimiser->X_old, activeOptimiser->U_old, activeModelTranslator->modelName, i, optHorizon);



    }



}

void onetaskGenerateTestingData(){
    int setupHorizon = 1000;
    int optHorizon = 2800;

    MatrixXd startStateVector;
    startStateVector.resize(activeModelTranslator->stateVectorSize, 1);

    std::vector<std::vector<double>> optTimes;
    std::vector<double> optTimesRow;

    std::vector<std::vector<double>> costReductions;
    std::vector<double> costReductionsRow;

    std::vector<std::vector<double>> avgPercentageDerivs;
    std::vector<double> avgPercentageDerivsRow;

    std::vector<std::vector<double>> avgTimeForDerivs;
    std::vector<double> avgTimeForDerivsRow;

    std::vector<std::vector<int>> numIterations;
    std::vector<int> numIterationsRow;

//    std::vector<std::string> methodNames = {"baseline", "setInterval100", "setInterval200", "adaptive_jerk", "adaptive_accel", "iterative_error",
//                                            "setInterval20_bpp", "setInterval100_bpp", "adaptive_jerk_bpp", "adaptive_accel_bpp", "iterative_error_bpp"};
//    int numMethods = methodNames.size();
//    int keyPointMethods[11] = {setInterval, setInterval, setInterval, adaptive_jerk, adaptive_accel,iterative_error,
//                               setInterval, setInterval, adaptive_jerk, adaptive_accel, iterative_error};
//    int interpMethod[11] = {linear, linear, linear, linear, linear, linear,
//                           linear, linear, linear, linear, linear};
//    int minN[11] = {1, 100, 1000, 50, 50, 50,
//                   100, 200, 50, 50, 50};
//    bool approxBackwardsPass[11] = {false, false, false, false, false, false,
//                                    true, true, true, true, true};

//    std::vector<std::string> methodNames = {"baseline", "setInterval100", "setInterval1000", "adaptive_jerk_50", "adaptive_accel_50", "iterative_error_50"};
//    int numMethods = methodNames.size();
//    int keyPointMethods[6] = {setInterval, setInterval, setInterval, adaptive_jerk, adaptive_accel,iterative_error};
//    int interpMethod[6] = {linear, linear, linear, linear, linear, linear};
//    int minN[6] = {1, 100, 1000, 50, 50, 50};
//    bool approxBackwardsPass[6] = {false, false, false, false, false, false};

    std::vector<std::string> methodNames = {"baseline", "setInterval_5", "setInterval_100", "setInterval_1000", "adaptive_jerk_5"};
    int numMethods = methodNames.size();
    std::vector<int> keyPointMethods = {setInterval, setInterval, setInterval, setInterval, adaptive_jerk};
    std::vector<int> interpMethod = {linear, linear, linear, linear, linear};
    std::vector<int> minN = {1, 5, 100, 1000, 5};
    std::vector<bool> approxBackwardsPass = {false, false, false, false, false};

    // Loop through saved trajectories
    for(int i = 0; i < 100; i++){
        cout << "------------------------------------ Trajec " << i << " ------------------------------------\n";

        // Loop through our interpolating derivatives methods
        optTimesRow.clear();
        costReductionsRow.clear();
        avgPercentageDerivsRow.clear();
        avgTimeForDerivsRow.clear();
        numIterationsRow.clear();

        yamlReader->loadTaskFromFile(activeModelTranslator->modelName, i, startStateVector, activeModelTranslator->X_desired);
        activeModelTranslator->X_start = startStateVector;
        activeModelTranslator->setStateVector(startStateVector, MASTER_RESET_DATA);
        activeModelTranslator->activePhysicsSimulator->stepSimulator(5, MASTER_RESET_DATA);

        if(activeModelTranslator->activePhysicsSimulator->checkIfDataIndexExists(0) == false){
            activeModelTranslator->activePhysicsSimulator->appendSystemStateToEnd(MASTER_RESET_DATA);
        }

        // Move the end-effector to a decent starting position
        activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);
        std::vector<MatrixXd> setupControls = activeModelTranslator->createInitSetupControls(1000);
        activeModelTranslator->activePhysicsSimulator->copySystemState(MASTER_RESET_DATA, MAIN_DATA_STATE);

        std::vector<MatrixXd> initOptimisationControls = activeModelTranslator->createInitOptimisationControls(optHorizon);

        // Load a task from saved tasks

        for(int j = 0; j < numMethods; j++){
            double optTime;
            double costReduction;
            double avgPercentageDerivs;
            double avgTimeForDerivs;
            int numIterationsForConvergence;

            activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);
            activeModelTranslator->activePhysicsSimulator->copySystemState(0, MASTER_RESET_DATA);



            // Setup interpolation method
            activeOptimiser->setupTestingExtras(i, interpMethod[j], keyPointMethods[j], minN[j], approxBackwardsPass[j]);
            // Setup initial state of the problem

//            activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, 0);
            std::vector<MatrixXd> optimisedControls = activeOptimiser->optimise(0, initOptimisationControls, 6, 6, optHorizon);

            // Return testing data and append appropriately
            activeOptimiser->returnOptimisationData(optTime, costReduction, avgPercentageDerivs, avgTimeForDerivs, numIterationsForConvergence);
            optTimesRow.push_back(optTime);
            costReductionsRow.push_back(costReduction);
            avgPercentageDerivsRow.push_back(avgPercentageDerivs);
            avgTimeForDerivsRow.push_back(avgTimeForDerivs);
            numIterationsRow.push_back(numIterationsForConvergence);

//            activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, 0);

//            int controlCounter = 0;
//            int visualCounter = 0;
//            cout << "final controls size: " << optimisedControls.size() << endl;
//
//            while(controlCounter < initOptimisationControls.size()){
//
//                activeModelTranslator->setControlVector(initOptimisationControls[controlCounter], MAIN_DATA_STATE);
//
//                activeModelTranslator->activePhysicsSimulator->stepSimulator(1, MAIN_DATA_STATE);
//
//                controlCounter++;
//                visualCounter++;
//
//                if(visualCounter > 5){
//
//                    activeVisualiser->render("show init controls");
//                    visualCounter = 0;
//                }
//            }

        }
        optTimes.push_back(optTimesRow);
        costReductions.push_back(costReductionsRow);
        avgPercentageDerivs.push_back(avgPercentageDerivsRow);
        avgTimeForDerivs.push_back(avgTimeForDerivsRow);
        numIterations.push_back(numIterationsRow);

        cout << "average percent derivs: " << avgPercentageDerivsRow[0] << endl;
        cout << "row " << i << " done \n";
    }

    // Save data to file
    cout << "save data to file \n";
    yamlReader->saveResultsDataForMethods(activeModelTranslator->modelName, methodNames,optTimes, costReductions, avgPercentageDerivs, avgTimeForDerivs, numIterations);
}

void generateTestingData(){

//    int configs[3] = {noClutter, lowClutter, heavyClutter};
    int configs[1] = {heavyClutter};


    // start timer here
    auto startTime = std::chrono::high_resolution_clock::now();

//    for(int i = 0; i < 1; i ++){
////        pandaReaching* myPandaReaching = new pandaReaching();
////        activeModelTranslator = myPandaReaching;
//        doublePendulum* myDoublePendulum = new doublePendulum();
//        activeModelTranslator = myDoublePendulum;
//        activeDifferentiator = new differentiator(activeModelTranslator, activeModelTranslator->myHelper);
//
//        MatrixXd startStateVector;
//        startStateVector.resize(activeModelTranslator->stateVectorSize, 1);
//        startStateVector = activeModelTranslator->X_start;
//        activeModelTranslator->setStateVector(startStateVector, MASTER_RESET_DATA);
//        activeModelTranslator->activePhysicsSimulator->stepSimulator(1, MAIN_DATA_STATE);
//        activeModelTranslator->activePhysicsSimulator->appendSystemStateToEnd(MAIN_DATA_STATE);
//
//        activeVisualiser = new visualizer(activeModelTranslator);
//        yamlReader->readOptimisationSettingsFile(opt_iLQR);
//        iLQROptimiser = new interpolatediLQR(activeModelTranslator, activeModelTranslator->activePhysicsSimulator, activeDifferentiator, yamlReader->maxHorizon, activeVisualiser, yamlReader);
//        activeOptimiser = iLQROptimiser;
//
//        onetaskGenerateTestingData();
//
//    }

    for(int i = 0; i < 1; i ++){
//        twoDPushing *myTwoDPushing = new twoDPushing(heavyClutter);
        std::shared_ptr<twoDPushing> myTwoDPushing = std::make_shared<twoDPushing>(heavyClutter);
        activeModelTranslator = myTwoDPushing;
        activeDifferentiator = std::make_shared<differentiator>(activeModelTranslator, activeModelTranslator->myHelper);

        MatrixXd startStateVector;
        startStateVector.resize(activeModelTranslator->stateVectorSize, 1);
        startStateVector = activeModelTranslator->X_start;
        activeModelTranslator->setStateVector(startStateVector, MAIN_DATA_STATE);
        activeModelTranslator->activePhysicsSimulator->stepSimulator(1, MAIN_DATA_STATE);
        activeModelTranslator->activePhysicsSimulator->appendSystemStateToEnd(MAIN_DATA_STATE);

        activeVisualiser = std::make_shared<visualizer>(activeModelTranslator);
        yamlReader->readOptimisationSettingsFile(opt_iLQR);
        iLQROptimiser = std::make_shared<interpolatediLQR>(activeModelTranslator, activeModelTranslator->activePhysicsSimulator, activeDifferentiator, yamlReader->maxHorizon, activeVisualiser, yamlReader);
        activeOptimiser = iLQROptimiser;

        onetaskGenerateTestingData();

    }

//    for(int i = 0; i < 3; i ++){
//        boxFlick *myBoxFlicking = new boxFlick(configs[i]);
//        activeModelTranslator = myBoxFlicking;
//        activeDifferentiator = new differentiator(activeModelTranslator, activeModelTranslator->myHelper);
//
//        MatrixXd startStateVector;
//        startStateVector.resize(activeModelTranslator->stateVectorSize, 1);
//        startStateVector = activeModelTranslator->X_start;
//        activeModelTranslator->setStateVector(startStateVector, MAIN_DATA_STATE);
//        activeModelTranslator->activePhysicsSimulator->stepSimulator(1, MAIN_DATA_STATE);
//        activeModelTranslator->activePhysicsSimulator->appendSystemStateToEnd(MAIN_DATA_STATE);
//
//        activeVisualiser = new visualizer(activeModelTranslator);
//        yamlReader->readOptimisationSettingsFile(opt_iLQR);
//        iLQROptimiser = new interpolatediLQR(activeModelTranslator, activeModelTranslator->activePhysicsSimulator, activeDifferentiator, yamlReader->maxHorizon, activeVisualiser, yamlReader);
//        activeOptimiser = iLQROptimiser;
//
//        onetaskGenerateTestingData();
//
//    }

    auto stopTimer = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTimer - startTime);
    double timeForTesting = duration.count() / 1000.0;
    timeForTesting = timeForTesting / 3600.0;
    cout << "time taken: " << timeForTesting << " h" << endl;
//    double predictedTimeFor100Scenes = timeForTesting * (100.0 / 3600.0);
//    cout << "predicted time taken: " << predictedTimeFor100Scenes << " h" << endl;
}

void generateFilteringData(){
    int setupHorizon = 1000;
    int optHorizon = 2200;

    MatrixXd startStateVector;
    startStateVector.resize(activeModelTranslator->stateVectorSize, 1);

    for(int i = 0; i < 100; i++) {
        // Load a task from saved tasks

        yamlReader->loadTaskFromFile(activeModelTranslator->modelName, i, startStateVector,
                                     activeModelTranslator->X_desired);
        activeModelTranslator->X_start = startStateVector;
        cout << "starting state: " << startStateVector << endl;
        cout << "desired state: " << activeModelTranslator->X_desired << endl;
        activeModelTranslator->setStateVector(startStateVector, MAIN_DATA_STATE);
        activeModelTranslator->activePhysicsSimulator->stepSimulator(1, MAIN_DATA_STATE);

        // Load optimiser

        // Reset optimisers variables as required
        activeOptimiser->setupTestingExtras(i, interpolationMethod, keyPointMethod, activeOptimiser->min_interval, false);

        // Generate init controls
        std::vector<MatrixXd> initControls;
        std::vector<MatrixXd> finalControls;

        activeModelTranslator->activePhysicsSimulator->copySystemState(MASTER_RESET_DATA, MAIN_DATA_STATE);
        std::vector<MatrixXd> initSetupControls = activeModelTranslator->createInitSetupControls(setupHorizon);
        activeModelTranslator->activePhysicsSimulator->copySystemState(0, MAIN_DATA_STATE);
        std::vector<MatrixXd> initOptimisationControls = activeModelTranslator->createInitOptimisationControls(
                optHorizon);
        activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, 0);

        auto start = high_resolution_clock::now();
        std::vector<MatrixXd> optimisedControls = activeOptimiser->optimise(MAIN_DATA_STATE, initOptimisationControls,
                                                                            yamlReader->maxIter, yamlReader->minIter,
                                                                            optHorizon);
        auto stop = high_resolution_clock::now();
        auto linDuration = duration_cast<microseconds>(stop - start);
        cout << "iLQR once took: " << linDuration.count() / 1000000.0f << " s\n";

        initControls.insert(initControls.end(), initSetupControls.begin(), initSetupControls.end());
        initControls.insert(initControls.end(), initOptimisationControls.begin(), initOptimisationControls.end());
        finalControls.insert(finalControls.end(), initSetupControls.begin(), initSetupControls.end());
        finalControls.insert(finalControls.end(), optimisedControls.begin(), optimisedControls.end());

        activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);

        int controlCounter = 0;
        int visualCounter = 0;
        cout << "final controls size: " << finalControls.size() << endl;

//        while(controlCounter < finalControls.size()){
//
//            activeModelTranslator->setControlVector(initControls[controlCounter], MAIN_DATA_STATE);
//
//            activeModelTranslator->activePhysicsSimulator->stepSimulator(1, MAIN_DATA_STATE);
//
//            controlCounter++;
//            visualCounter++;
//
//            if(visualCounter > 5){
//
//                activeVisualiser->render("show init controls");
//                visualCounter = 0;
//            }
//        }
    }
}

void generateTestScenes(){
    for(int i = 0; i < 100; i++){
        MatrixXd startStateVector = activeModelTranslator->returnRandomStartState();
        activeModelTranslator->X_start = startStateVector;
        activeModelTranslator->X_desired = activeModelTranslator->returnRandomGoalState(startStateVector);
        activeModelTranslator->setStateVector(startStateVector, MAIN_DATA_STATE);
        activeVisualiser->render("init state");
        cout << "starting state: " << startStateVector << endl;


        cout << "model name: " << activeModelTranslator->modelName << endl;
        yamlReader->saveTaskToFile(activeModelTranslator->modelName, i, activeModelTranslator->X_start, activeModelTranslator->X_desired);
    }
}

void showInitControls(){
    int setupHorizon = 1000;
    int optHorizon = 2500;
    int controlCounter = 0;
    int visualCounter = 0;

    std::vector<MatrixXd> initControls;

//    activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);
    std::vector<MatrixXd> initSetupControls = activeModelTranslator->createInitSetupControls(setupHorizon);
    activeModelTranslator->activePhysicsSimulator->copySystemState(MASTER_RESET_DATA, MAIN_DATA_STATE);
    std::vector<MatrixXd> initOptimisationControls = activeModelTranslator->createInitOptimisationControls(optHorizon);
    activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);

    double cost = activeModelTranslator->costFunction(MAIN_DATA_STATE, false);
    cout << "cost: " << cost << endl;


    //Stitch setup and optimisation controls together
//    initControls.insert(initControls.end(), initSetupControls.begin(), initSetupControls.end());
    initControls.insert(initControls.end(), initOptimisationControls.begin(), initOptimisationControls.end());

    while(activeVisualiser->windowOpen()){

        activeModelTranslator->setControlVector(initControls[controlCounter], MAIN_DATA_STATE);
        activeModelTranslator->activePhysicsSimulator->stepSimulator(1, MAIN_DATA_STATE);


        controlCounter++;
        visualCounter++;

        if(controlCounter == setupHorizon){
            int a = 1;

        }

        if(controlCounter == setupHorizon + optHorizon/2){
            int a = 1;

        }

        if(controlCounter == setupHorizon + optHorizon){
            int a = 1;

        }

        if(controlCounter >= initControls.size()){
            controlCounter = 0;
            activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);
        }

        if(visualCounter > 5){
            visualCounter = 0;
            activeVisualiser->render("show init controls");
        }
    }
}

void optimiseOnceandShow(){
//    int setupHorizon = 1000;
    int optHorizon = 1500;
    int controlCounter = 0;
    int visualCounter = 0;
    bool showFinalControls = true;
    char* label = "Final controls";

    std::vector<MatrixXd> initControls;
    std::vector<MatrixXd> finalControls;

//    activeModelTranslator->activePhysicsSimulator->copySystemState(0, MAIN_DATA_STATE);
//    MatrixXd test = activeModelTranslator->returnStateVector(MAIN_DATA_STATE);
//    cout << "test: " << test << endl;

    std::vector<MatrixXd> initSetupControls = activeModelTranslator->createInitSetupControls(1000);
    activeModelTranslator->activePhysicsSimulator->copySystemState(MASTER_RESET_DATA, MAIN_DATA_STATE);

    std::vector<MatrixXd> initOptimisationControls = activeModelTranslator->createInitOptimisationControls(optHorizon);
    activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);
    activeModelTranslator->activePhysicsSimulator->copySystemState(0, MASTER_RESET_DATA);
//    test = activeModelTranslator->returnStateVector(MAIN_DATA_STATE);
//    cout << "test 2: " << test << endl;
    activeOptimiser->setupTestingExtras(1000, interpolationMethod, keyPointMethod, activeOptimiser->min_interval, activeOptimiser->approximate_backwardsPass);

    auto start = high_resolution_clock::now();
    std::vector<MatrixXd> optimisedControls = activeOptimiser->optimise(0, initOptimisationControls, yamlReader->maxIter, yamlReader->minIter, optHorizon);
    auto stop = high_resolution_clock::now();
    auto linDuration = duration_cast<microseconds>(stop - start);
    cout << "optimisation took: " << linDuration.count() / 1000000.0f << " ms\n";

    // Stitch together setup controls with init control + optimised controls
//    initControls.insert(initControls.end(), initSetupControls.begin(), initSetupControls.end());
    initControls.insert(initControls.end(), initOptimisationControls.begin(), initOptimisationControls.end());
//    finalControls.insert(finalControls.end(), initSetupControls.begin(), initSetupControls.end());
    finalControls.insert(finalControls.end(), optimisedControls.begin(), optimisedControls.end());

    activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);

    while(activeVisualiser->windowOpen()){

        if(showFinalControls){
            activeModelTranslator->setControlVector(finalControls[controlCounter], MAIN_DATA_STATE);
        }
        else{
            activeModelTranslator->setControlVector(initControls[controlCounter], MAIN_DATA_STATE);
        }

        activeModelTranslator->activePhysicsSimulator->stepSimulator(1, MAIN_DATA_STATE);

        controlCounter++;
        visualCounter++;

        if(controlCounter >= finalControls.size()){
            controlCounter = 0;
            activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);
            showFinalControls = !showFinalControls;
            if(showFinalControls){
                label = "Final controls";
            }
            else{
                label = "Init Controls";
            }
        }

        if(visualCounter > 5){
            visualCounter = 0;
            activeVisualiser->render(label);
        }
    }
}

void MPCContinous(){
    int horizon = 200;
    bool taskComplete = false;
    int currentControlCounter = 0;
    int visualCounter = 0;
    int overallTaskCounter = 0;
    int reInitialiseCounter = 0;
    const char* label = "MPC Continous";

    // Instantiate init controls
    std::vector<MatrixXd> initControls;
    initControls = activeModelTranslator->createInitOptimisationControls(horizon);
    activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, 0);

    std::vector<MatrixXd> optimisedControls = activeOptimiser->optimise(0, initControls, yamlReader->maxIter, yamlReader->minIter, horizon);
    MatrixXd initState = activeModelTranslator->returnStateVector(MAIN_DATA_STATE);
    cout << "init state in MPC continous: " << initState << endl;

    while(!taskComplete){
        MatrixXd nextControl = optimisedControls[0].replicate(1, 1);

        optimisedControls.erase(optimisedControls.begin());

        optimisedControls.push_back(optimisedControls.at(optimisedControls.size() - 1));

        activeModelTranslator->setControlVector(nextControl, MAIN_DATA_STATE);

        activeModelTranslator->activePhysicsSimulator->stepSimulator(1, MAIN_DATA_STATE);

        reInitialiseCounter++;
        visualCounter++;

        if(reInitialiseCounter > 10){
            //initControls = activeModelTranslator->createInitControls(horizon);
            optimisedControls = activeOptimiser->optimise(MAIN_DATA_STATE, optimisedControls, yamlReader->maxIter, yamlReader->minIter, horizon);
            //initState = activeModelTranslator->returnStateVector(MAIN_DATA_STATE);
            //cout << "init state in MPC continous: " << initState << endl;
            reInitialiseCounter = 0;
        }

        if(visualCounter > 5){
            activeVisualiser->render(label);
            visualCounter = 0;
        }
    }
}

// Before calling this function, we should setup the activeModelTranslator with the correct initial state and the
// optimiser settings. This function can then return necessary testing data for us to store
void MPCUntilComplete(bool &sucess, double &finalDist, double &totalExecutionTime, double &totalOptimisationTime, double &avgTimeGettingDerivs, double &avgPercentDerivs, double &avgTimeBP, double &avgTimeFP,
                      int MAX_TASK_TIME, int REPLAN_TIME, int OPT_HORIZON){
    bool taskComplete = false;
    int visualCounter = 0;
    int overallTaskCounter = 0;
    int reInitialiseCounter = 0;
    const char* label = "MPC until complete";

    activeVisualiser->replayControls.clear();

    totalOptimisationTime = 0.0f;
    totalExecutionTime = 0.0f;
    finalDist = 0.0f;
    std::vector<double> timeGettingDerivs;
    std::vector<double> timeBackwardsPass;
    std::vector<double> timeForwardsPass;
    std::vector<double> percentagesDerivsCalculated;

    std::vector<MatrixXd> optimisedControls;

    // Instantiate init controls
    std::vector<MatrixXd> initOptimisationControls;

    int maxHorizon = MAX_TASK_TIME - overallTaskCounter;
    int horizon = OPT_HORIZON;
    if(maxHorizon < OPT_HORIZON){
        horizon = maxHorizon;
    }

    initOptimisationControls = activeModelTranslator->createInitOptimisationControls(horizon);
    activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);
    activeModelTranslator->activePhysicsSimulator->copySystemState(0, MASTER_RESET_DATA);
//    activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, 0);

//    cout << "first control: " << initOptimisationControls[1] << endl;
//    MatrixXd testStartState = activeModelTranslator->returnStateVector(0);
//    cout << "test start state: " << testStartState << endl;
//    cout << "desired state: " << activeModelTranslator->X_desired << endl;

    optimisedControls = activeOptimiser->optimise(0, initOptimisationControls, 3, 2, OPT_HORIZON);

    while(!taskComplete){
        MatrixXd nextControl = optimisedControls[0].replicate(1, 1);
        activeVisualiser->replayControls.push_back(nextControl.replicate(1, 1));

        optimisedControls.erase(optimisedControls.begin());

        optimisedControls.push_back(optimisedControls.at(optimisedControls.size() - 1));

        activeModelTranslator->setControlVector(nextControl, MAIN_DATA_STATE);

        activeModelTranslator->activePhysicsSimulator->stepSimulator(1, MAIN_DATA_STATE);

        reInitialiseCounter++;
        visualCounter++;

        if(activeModelTranslator->taskComplete(MAIN_DATA_STATE, finalDist)){
            taskComplete = true;
            sucess = true;
        }
        else{
            if(reInitialiseCounter > REPLAN_TIME){
                activeModelTranslator->activePhysicsSimulator->copySystemState(0, MAIN_DATA_STATE);
//                initOptimisationControls = activeModelTranslator->createInitOptimisationControls(optHorizon);
//                activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, 0);
//
//                optimisedControls = initOptimisationControls;

                maxHorizon = MAX_TASK_TIME - overallTaskCounter;
                horizon = OPT_HORIZON;
                if(maxHorizon < OPT_HORIZON){
                    horizon = maxHorizon;
                }

                optimisedControls = activeOptimiser->optimise(0, optimisedControls, 5, 1, horizon);
                reInitialiseCounter = 0;

                totalOptimisationTime += activeOptimiser->optTime / 1000.0f;
                timeGettingDerivs.push_back(activeOptimiser->avgTime_getDerivs_ms);
                timeBackwardsPass.push_back(activeOptimiser->avgTime_backwardsPass_ms);
                timeForwardsPass.push_back(activeOptimiser->avgTime_forwardsPass_ms);
                percentagesDerivsCalculated.push_back(activeOptimiser->avgPercentDerivs);

            }
        }

        if(mpcVisualise){
            if(visualCounter > 10){
                activeVisualiser->render(label);
                visualCounter = 0;
            }
        }

        overallTaskCounter++;

        if(overallTaskCounter > MAX_TASK_TIME){
            taskComplete = true;
            sucess = false;
        }
    }

    // Print some useful information about the task
//    avgTimeGettingDerivs = 0.0f;
//    avgTimeBP = 0.0f;
//    avgTimeFP = 0.0f;
//    avgPercentDerivs = 0.0f;
//    totalExecutionTime = activeVisualiser->replayControls.size() * MUJOCO_DT;
//
//    for(int i = 0; i < timeGettingDerivs.size(); i++){
//        avgTimeGettingDerivs += timeGettingDerivs[i];
//        avgTimeBP += timeBackwardsPass[i];
//        avgTimeFP += timeForwardsPass[i];
//        avgPercentDerivs += percentagesDerivsCalculated[i];
//    }
//    avgTimeGettingDerivs /= timeGettingDerivs.size();
//    avgTimeBP /= timeBackwardsPass.size();
//    avgTimeFP /= timeForwardsPass.size();
//    avgPercentDerivs /= percentagesDerivsCalculated.size();
//
//    if(sucess){
//        cout << "| Task sucessful |\n";
//    }
//    else{
//        cout << "| Task failed |\n";
//    }
//    cout << "| Distance to goal: " << finalDist << " |\n";
//    cout << "| Execution time: " << totalExecutionTime << " seconds |\n";
//    cout << "| Optimisation time: " << totalOptimisationTime << " seconds |\n";
//    cout << "| Avg percentage of derivatives calculated: " << avgPercentDerivs << "\n";
//    cout << "| avg time derivs: " << avgTimeGettingDerivs << " bp: " << avgTimeBP << " fp: " << avgTimeFP << " ms |\n";

    if(mpcVisualise){
        while(activeVisualiser->windowOpen()){
            if(activeVisualiser->replayTriggered){
                activeVisualiser->replayTriggered = false;

                activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);
                int controlCounter = 0;
                while(controlCounter < activeVisualiser->replayControls.size()){
                    MatrixXd nextControl = activeVisualiser->replayControls[controlCounter].replicate(1, 1);

                    activeModelTranslator->setControlVector(nextControl, MAIN_DATA_STATE);

                    activeModelTranslator->activePhysicsSimulator->stepSimulator(1, MAIN_DATA_STATE);

                    controlCounter++;

                    if(controlCounter % 5 == 0){
                        activeVisualiser->render("replaying");
                    }
                }
            }
            activeVisualiser->render("replay_mode");
        }
    }
}

void generateTestingData_MPC(){
    //    int configs[3] = {noClutter, lowClutter, heavyClutter};
    int configs[1] = {heavyClutter};


    // start timer here
    auto startTime = std::chrono::high_resolution_clock::now();

    for(int k = 0; k < 1; k ++) {
        std::shared_ptr<twoDPushing> myBoxPushing = std::make_shared<twoDPushing>(heavyClutter);
        activeModelTranslator = myBoxPushing;
        activeDifferentiator = std::make_shared<differentiator>(activeModelTranslator, activeModelTranslator->myHelper);

        activeVisualiser = std::make_shared<visualizer>(activeModelTranslator);
        yamlReader->readOptimisationSettingsFile(opt_iLQR);
        iLQROptimiser = std::make_shared<interpolatediLQR>(activeModelTranslator, activeModelTranslator->activePhysicsSimulator,
                                             activeDifferentiator, yamlReader->maxHorizon, activeVisualiser,
                                             yamlReader);
        activeOptimiser = iLQROptimiser;

        // ------------------------- data storage -------------------------------------
        std::vector<std::vector<bool>> sucesses;
        std::vector<bool> sucessesRow;

        std::vector<std::vector<double>> finalDistances;
        std::vector<double> finalDistancesRow;

        std::vector<std::vector<double>> executionTime;
        std::vector<double> executionTimeRow;

        std::vector<std::vector<double>> optimisationTime;
        std::vector<double> optimisationTimeRow;

        std::vector<std::vector<double>> avgTimeForDerivs;
        std::vector<double> avgTimeForDerivsRow;

        std::vector<std::vector<double>> avgTimeBP;
        std::vector<double> avgTimeBPRow;

        std::vector<std::vector<double>> avgTimeFP;
        std::vector<double> avgTimeFPRow;

        std::vector<std::vector<double>> avgPercentDerivs;
        std::vector<double> avgPercentDerivsRow;

        std::vector<std::string> methodNames = {"baseline", "setInterval10", "setInterval200", "adaptive_jerk_10",
                                                "adaptive_accel_10", "iterative_error_10"};
        int numMethods = methodNames.size();
        int keyPointMethods[6] = {setInterval, setInterval, setInterval, adaptive_jerk, adaptive_accel,
                                  iterative_error};
        int interpMethod[6] = {linear, linear, linear, linear, linear, linear};
        int minN[6] = {1, 10, 200, 10, 10, 10};
        bool approxBackwardsPass[6] = {false, false, false, false, false, false};

//        std::vector<std::string> methodNames = {"setInterval10"};
//        int numMethods = methodNames.size();
//        int keyPointMethods[1] = {setInterval};
//        int interpMethod[1] = {linear};
//        int minN[1] = {10};
//        bool approxBackwardsPass[1] = {false};

        auto startTimer = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 100; i++) {
            cout << "------------------------------------ Trajec " << i << " ------------------------------------\n";

            // Loop through our interpolating derivatives methods
            sucessesRow.clear();
            finalDistancesRow.clear();
            executionTimeRow.clear();
            optimisationTimeRow.clear();
            avgTimeForDerivsRow.clear();
            avgTimeBPRow.clear();
            avgTimeFPRow.clear();
            avgPercentDerivsRow.clear();

            MatrixXd startStateVector;
            startStateVector.resize(activeModelTranslator->stateVectorSize, 1);

            yamlReader->loadTaskFromFile(activeModelTranslator->modelName, i, startStateVector,
                                         activeModelTranslator->X_desired);

            activeModelTranslator->X_start = startStateVector;
            activeModelTranslator->setStateVector(startStateVector, MASTER_RESET_DATA);
            activeModelTranslator->activePhysicsSimulator->stepSimulator(5, MASTER_RESET_DATA);

            if(activeModelTranslator->activePhysicsSimulator->checkIfDataIndexExists(0) == false){
                activeModelTranslator->activePhysicsSimulator->appendSystemStateToEnd(MASTER_RESET_DATA);
            }

            // Move the end-effector to a decent starting position
            activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);
            std::vector<MatrixXd> setupControls = activeModelTranslator->createInitSetupControls(1000);
            activeModelTranslator->activePhysicsSimulator->copySystemState(MASTER_RESET_DATA, MAIN_DATA_STATE);

//            activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);
//            activeModelTranslator->activePhysicsSimulator->copySystemState(0, MASTER_RESET_DATA);

            for (int j = 0; j < numMethods; j++) {
                bool sucess;
                double finalDistance = 0.0f;
                double executionTime = 0.0f;
                double optimisationTime = 0.0f;
                double avgPercentageDerivs = 0.0f;
                double avgTimeForDerivs = 0.0f;
                double avgTimeBP = 0.0f;
                double avgTimeFP = 0.0f;
                cout << "--------------------------------------------------------------------------------\n";
                cout << "current method: " << methodNames[j] << "\n";

                activeOptimiser->setupTestingExtras(i, interpMethod[j], keyPointMethods[j], minN[j],
                                                    approxBackwardsPass[j]);
                activeModelTranslator->activePhysicsSimulator->copySystemState( MAIN_DATA_STATE, MASTER_RESET_DATA);
                MPCUntilComplete(sucess, finalDistance, executionTime, optimisationTime, avgTimeForDerivs, avgPercentageDerivs,
                                 avgTimeBP, avgTimeFP, 2500, 500, 1800);

                sucessesRow.push_back(sucess);
                finalDistancesRow.push_back(finalDistance);
                executionTimeRow.push_back(executionTime);
                optimisationTimeRow.push_back(optimisationTime);
                avgTimeForDerivsRow.push_back(avgTimeForDerivs);
                avgTimeBPRow.push_back(avgTimeBP);
                avgTimeFPRow.push_back(avgTimeFP);
                avgPercentDerivsRow.push_back(avgPercentageDerivs);

            }
            // New row of data added
            sucesses.push_back(sucessesRow);
            finalDistances.push_back(finalDistancesRow);
            executionTime.push_back(executionTimeRow);
            optimisationTime.push_back(optimisationTimeRow);
            avgTimeForDerivs.push_back(avgTimeForDerivsRow);
            avgTimeBP.push_back(avgTimeBPRow);
            avgTimeFP.push_back(avgTimeFPRow);
            avgPercentDerivs.push_back(avgPercentDerivsRow);

            auto currentTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTimer).count();


            cout << "Time taken so far: " << duration/ 1000.0f << " s" << endl;
        }
        // Save data to csv
        yamlReader->saveResultsData_MPC(activeModelTranslator->modelName, methodNames, sucesses, finalDistances, executionTime,
                                        optimisationTime, avgTimeForDerivs, avgTimeBP, avgTimeFP, avgPercentDerivs);
    }
}

void keyboardControl(){

    MatrixXd startStateVector;
    startStateVector.resize(activeModelTranslator->stateVectorSize, 1);
    int dof = activeModelTranslator->stateVectorSize / 2;

    startStateVector << 0.3, 0, -0.3, -0.3, 0, 0.3,
            0, 0, 0, 0, 0, 0.5;



    activeModelTranslator->X_start = startStateVector;
    activeModelTranslator->setStateVector(startStateVector, MASTER_RESET_DATA);
    activeModelTranslator->activePhysicsSimulator->forwardSimulator(MASTER_RESET_DATA);

//    double cost = activeModelTranslator->costFunction(MASTER_RESET_DATA, false);
//
//    startStateVector << 0, 0, 0, 0, 0, 0,
//            0, 0, 0, 0, 0, 0;
//
//    activeModelTranslator->setStateVector(startStateVector, MASTER_RESET_DATA);
//
//    activeModelTranslator->activePhysicsSimulator->forwardSimulator(MASTER_RESET_DATA);
//
//    double cost2 = activeModelTranslator->costFunction(MASTER_RESET_DATA, false);

    activeModelTranslator->activePhysicsSimulator->stepSimulator(5, MASTER_RESET_DATA);
    activeModelTranslator->activePhysicsSimulator->copySystemState(MAIN_DATA_STATE, MASTER_RESET_DATA);

    while(activeVisualiser->windowOpen()){
        vector<double> gravCompensation;
//        activeModelTranslator->activePhysicsSimulator->getRobotJointsGravityCompensaionControls("panda", gravCompensation, MAIN_DATA_STATE);

        MatrixXd control(activeModelTranslator->num_ctrl, 1);
        int testIndex = 0;
        startStateVector = activeModelTranslator->returnStateVector(MAIN_DATA_STATE);
        cout << "state: " << startStateVector << endl;
//
//        for(int i = 0; i < dof; i++){
//            if ( i != testIndex){
//                startStateVector(i) = 0.0f;
//            }
//        }

        control(0) = 0.1;
        control(1) = -0.1;
        control(2) = 0.1;

        control(3) = 0.2;
        control(4) = -0.2;
        control(5) = 0.3;

//        control(testIndex) = 0.1;
//        activeModelTranslator->setStateVector(startStateVector, MAIN_DATA_STATE);


//        cout << "control: " << control << endl;
//        activeModelTranslator->setControlVector(control, MAIN_DATA_STATE);
//        MatrixXd returncontrol = activeModelTranslator->returnControlVector(MAIN_DATA_STATE);
//        cout << "return control: " << returncontrol << endl;
        activeModelTranslator->activePhysicsSimulator->stepSimulator(1, MAIN_DATA_STATE);

        activeVisualiser->render("keyboard control");
    }
}