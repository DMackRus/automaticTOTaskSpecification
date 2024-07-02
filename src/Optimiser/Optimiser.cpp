
#include "Optimiser/Optimiser.h"

Optimiser::Optimiser(std::shared_ptr<ModelTranslator> _modelTranslator, std::shared_ptr<MuJoCoHelper> _MuJoCo_helper, std::shared_ptr<FileHandler> _yamlReader, std::shared_ptr<Differentiator> _differentiator){
    activeModelTranslator = _modelTranslator;
    MuJoCo_helper = _MuJoCo_helper;
    activeYamlReader = _yamlReader;
    activeDifferentiator = _differentiator;

    // Set up the derivative interpolator from YAML settings
    activeKeyPointMethod.name = activeModelTranslator->keypoint_method;
    activeKeyPointMethod.auto_adjust = activeModelTranslator->auto_adjust;
    activeKeyPointMethod.min_N = activeModelTranslator->min_N;
    activeKeyPointMethod.max_N = activeModelTranslator->max_N;
    activeKeyPointMethod.jerk_thresholds = activeModelTranslator->jerk_thresholds;
    // TODO - fix this - add acell thresholds to yaml
    activeKeyPointMethod.accell_thresholds = activeModelTranslator->jerk_thresholds;
    activeKeyPointMethod.iterative_error_threshold = activeModelTranslator->iterative_error_threshold;
    activeKeyPointMethod.velocity_change_thresholds = activeModelTranslator->velocity_change_thresholds;

    keypoint_generator = std::make_shared<KeypointGenerator>(activeDifferentiator,
                                                             MuJoCo_helper,
                                                             activeModelTranslator->current_state_vector.dof,
                                                             0);

    keypoint_generator->SetKeypointMethod(activeKeyPointMethod);
    keypoint_generator->PrintKeypointMethod();
}

bool Optimiser::CheckForConvergence(double old_cost, double new_cost){
    double costGrad = (old_cost - new_cost) / new_cost;

    if(costGrad < epsConverge){
        return true;
    }
    return false;
}

void Optimiser::Resize(int new_num_dofs, int new_num_ctrl, int new_horizon){

}

void Optimiser::ReturnOptimisationData(double &_optTime, double &_costReduction, double &_avgPercentageDerivs, double &_avgTimeGettingDerivs, int &_numIterations){

    _optTime = opt_time_ms;
    _costReduction = cost_reduction;
    _avgPercentageDerivs = avg_percent_derivs;
    _avgTimeGettingDerivs = avg_time_get_derivs_ms;
    _numIterations = num_iterations;
}

keypoint_method Optimiser::ReturnCurrentKeypointMethod(){
    return keypoint_generator->ReturnCurrentKeypointMethod();
}

void Optimiser::SetCurrentKeypointMethod(keypoint_method _keypoint_method){
    keypoint_generator->SetKeypointMethod(_keypoint_method);
}

void Optimiser::GenerateDerivatives(){
    // STEP 1 - Linearise dynamics and calculate first + second order cost derivatives for current trajectory
    // generate the dynamics evaluation waypoints
    auto start_keypoint_time = high_resolution_clock::now();
    keypoint_generator->GenerateKeyPoints(X_old, A, B);

    keypoint_generator->ResetCache();
//    std::cout << "gen keypoints time: " << duration_cast<microseconds>(high_resolution_clock::now() - start_keypoint_time).count() / 1000.0f << " ms\n";

    // Calculate derivatives via finite differencing / analytically for cost functions if available
    if(activeKeyPointMethod.name != "iterative_error"){
        auto start_fd_time = high_resolution_clock::now();
        ComputeDerivativesAtSpecifiedIndices(keypoint_generator->keypoints);
        auto stop_fd_time = high_resolution_clock::now();
        auto duration_fd_time = duration_cast<microseconds>(stop_fd_time - start_fd_time);

//        std::cout << "fd time: " << duration_fd_time.count() / 1000.0f << " ms\n";
    }
    else{
        ComputeCostDerivatives();
    }

    auto start_interp_time = high_resolution_clock::now();
    keypoint_generator->InterpolateDerivatives(keypoint_generator->keypoints, horizon_length,
                                               A, B, l_x, l_u, l_xx, l_uu, activeYamlReader->costDerivsFD,
                                               activeModelTranslator->current_state_vector.num_ctrl);
//    std::cout <<" interpolate derivs took: " << duration_cast<microseconds>(high_resolution_clock::now() - start_interp_time).count() / 1000.0f << " ms\n";

    std::cout << "------------- residual derivatives wrt State -------------------" << std::endl;
    for(int i = 0; i < activeModelTranslator->num_residual_terms; i++){
        std::cout << "r_x[" << i << "]: " << std::endl;
        std:: cout << r_x[0][i] << std::endl;
    }

    std::cout << "-------------- residual derivatives wrt State ------------------" << std::endl;
    for(int i = 0; i < activeModelTranslator->num_residual_terms; i++){
        std::cout << "r_u[" << i << "]: " << std::endl;
        std:: cout << r_u[0][i] << std::endl;
    }



    // If finite differencing is used for cost derivatives, compute cost derivs from residual derivatives
    if(activeYamlReader->costDerivsFD){
        for(int t = 0; t < horizon_length; t++){
            activeModelTranslator->CostDerivativesFromResiduals(activeModelTranslator->current_state_vector,
                                                                l_x[t], l_xx[t], l_u[t], l_uu[t],
                                                                residuals[t], r_x[t], r_u[t], false);
        }

        activeModelTranslator->CostDerivativesFromResiduals(activeModelTranslator->current_state_vector,
                                                            l_x[horizon_length - 1], l_xx[horizon_length - 1],
                                                            l_u[horizon_length - 1], l_uu[horizon_length - 1],
                                                            residuals[horizon_length - 1], r_x[horizon_length - 1], r_u[horizon_length - 1], true);
    }

    std::cout << "------------------ cost derivatives from residuals ------------------------- \n";
    std::cout << "reisduals[0]: " << residuals[0] << "\n";
    std::cout << "l_x[0]: " << std::endl;
    std:: cout << l_x[0] << std::endl;
    std::cout << "l_xx[0]: " << std::endl;
    std:: cout << l_xx[0] << std::endl;
    std::cout << "l_u[0]: " << std::endl;
    std:: cout << l_u[0] << std::endl;
    std::cout << "l_uu[0]: " << std::endl;
    std:: cout << l_uu[0] << std::endl;

    std::cout << "residuals[horizon_length - 1]: " << residuals[horizon_length - 1] << "\n";
    std::cout << "l_x[horizon-1]: " << std::endl;
    std:: cout << l_x[horizon_length-1] << std::endl;
    std::cout << "l_xx[horizon-1]: " << std::endl;
    std:: cout << l_xx[horizon_length-1] << std::endl;
    std::cout << "l_u[horizon-1]: " << std::endl;
    std:: cout << l_u[horizon_length-1] << std::endl;
    std::cout << "l_uu[horizon-1]: " << std::endl;
    std:: cout << l_uu[horizon_length-1] << std::endl;
    std::cout << "--------------------- Cost derivatives from analytical ------------------------------ \n";

//    ComputeCostDerivatives();
//
//    std::cout << "l_x[0]: " << std::endl;
//    std:: cout << l_x[0] << std::endl;
//    std::cout << "l_xx[0]: " << std::endl;
//    std:: cout << l_xx[0] << std::endl;
//    std::cout << "l_u[0]: " << std::endl;
//    std:: cout << l_u[0] << std::endl;
//    std::cout << "l_uu[0]: " << std::endl;
//    std:: cout << l_uu[0] << std::endl;
//
//    std::cout << "l_x[horizon-1]: " << std::endl;
//    std:: cout << l_x[horizon_length-1] << std::endl;
//    std::cout << "l_xx[horizon-1]: " << std::endl;
//    std:: cout << l_xx[horizon_length-1] << std::endl;
//    std::cout << "l_u[horizon-1]: " << std::endl;
//    std:: cout << l_u[horizon_length-1] << std::endl;
//    std::cout << "l_uu[horizon-1]: " << std::endl;
//    std:: cout << l_uu[horizon_length-1] << std::endl;

    double average_percent_derivs = 0.0;
    for(int i = 0; i < activeModelTranslator->current_state_vector.dof; i++){
        average_percent_derivs += keypoint_generator->last_percentages[i];
    }
    average_percent_derivs /= activeModelTranslator->current_state_vector.dof;

    percentage_derivs_per_iteration.push_back(average_percent_derivs);

    if(filteringMethod != "none"){
        FilterDynamicsMatrices();
    }
}

void Optimiser::ComputeCostDerivatives(){
    #pragma omp parallel for
    for(int i = 0; i < horizon_length; i++){
        activeModelTranslator->CostDerivatives(MuJoCo_helper->saved_systems_state_list[i],
                                               activeModelTranslator->current_state_vector,
                                               l_x[i], l_xx[i], l_u[i], l_uu[i], false);
    }

    activeModelTranslator->CostDerivatives(MuJoCo_helper->saved_systems_state_list[horizon_length - 1],
                                           activeModelTranslator->current_state_vector,
                                           l_x[horizon_length - 1], l_xx[horizon_length - 1],
                                           l_u[horizon_length - 1], l_uu[horizon_length - 1], true);
}

void Optimiser::ComputeDerivativesAtSpecifiedIndices(std::vector<std::vector<int>> keyPoints){

    MuJoCo_helper->InitModelForFiniteDifferencing();

    std::vector<int> timeIndices;
    for(int i = 0; i < keyPoints.size(); i++){
        if(!keyPoints[i].empty()){
            timeIndices.push_back(i);
        }
    }

    // Loop through keypoints and delete any entries that have no keypoints
    for(int i = 0; i < keyPoints.size(); i++){
        if(keyPoints[i].empty()){
            keyPoints.erase(keyPoints.begin() + i);
            i--;
        }
    }

    current_iteration = 0;
    num_threads_iterations = static_cast<int>(keyPoints.size());
    timeIndicesGlobal = timeIndices;

    // TODO - remove this? It is used for WorkerComputeDerivatives function to compute derivatives in parallel
    keypointsGlobal = keyPoints;

    // compute derivs serially
//    for(int i = 0; i < horizon_length; i++){
//        if(!keyPoints[i].empty()){
//            activeDifferentiator->ComputeDerivatives(A[i], B[i], keyPoints[i], l_x[i], l_u[i], l_xx[i], l_uu[i],
//                                                     i, 0, false, activeYamlReader->costDerivsFD, true, 1e-6);
//        }
//    }

    // Setup all the required tasks
    for (int i = 0; i < keyPoints.size(); ++i) {
        tasks.push_back(&Differentiator::ComputeDerivatives);
    }

    // Get the number of threads available
    const int num_threads = std::thread::hardware_concurrency() - 1;  // Get the number of available CPU cores
    std::vector<std::thread> thread_pool;
    for (int i = 0; i < num_threads; ++i) {
        thread_pool.push_back(std::thread(&Optimiser::WorkerComputeDerivatives, this, i));
    }

    for (std::thread& thread : thread_pool) {
        thread.join();
    }
      
    MuJoCo_helper->ResetModelAfterFiniteDifferencing();

    auto time_cost_start = std::chrono::high_resolution_clock::now();

//    if(!activeYamlReader->costDerivsFD){
//        for(int i = 0; i < horizon_length; i++){
//            activeModelTranslator->CostDerivatives(MuJoCo_helper->saved_systems_state_list[i],
//                                                   activeModelTranslator->current_state_vector,
//                                                   l_x[i], l_xx[i], l_u[i], l_uu[i], false);
//        }
//        activeModelTranslator->CostDerivatives(MuJoCo_helper->saved_systems_state_list[horizon_length - 1],
//                                               activeModelTranslator->current_state_vector,
//                                               l_x[horizon_length - 1], l_xx[horizon_length - 1],
//                                               l_u[horizon_length - 1], l_uu[horizon_length - 1], true);
//    }

//    std::cout << "time cost derivs: " << duration_cast<microseconds>(high_resolution_clock::now() - time_cost_start).count() / 1000.0f << " ms\n";
}

void Optimiser::WorkerComputeDerivatives(int threadId) {
    while (true) {
        int iteration = current_iteration.fetch_add(1);
        if (iteration >= num_threads_iterations) {
            break;  // All iterations done
        }

        int timeIndex = timeIndicesGlobal[iteration];
        bool terminal = false;
        if(timeIndex == horizon_length - 1){
            terminal = true;
        }

        std::vector<int> keyPoints;
        (activeDifferentiator.get()->*(tasks[iteration]))(A[timeIndex], B[timeIndex],
                                        keypointsGlobal[iteration], r_x[timeIndex], r_u[timeIndex],
                                        timeIndex, threadId, terminal, activeYamlReader->costDerivsFD, true, 1e-6);
    }
}

void Optimiser::FilterDynamicsMatrices() {

    // Aliases
    int dof = activeModelTranslator->current_state_vector.dof;

    for(int i = dof; i < 2 * dof; i++){
        for(int j = 0; j < 2 * dof; j++){
            std::vector<double> unfiltered;
            std::vector<double> filtered;

            for(int k = 0; k < horizon_length; k++){
                unfiltered.push_back(A[k](i, j));
            }

            if(filteringMethod == "low_pass"){
                filtered = FilterIndValLowPass(unfiltered);
            }
            else if(filteringMethod == "FIR"){
                filtered = FilterIndValFIRFilter(unfiltered, FIRCoefficients);
            }
            else{
                std::cerr << "Filtering method not recognised" << std::endl;
            }


            for(int k = 0; k < horizon_length; k++){
                A[k](i, j) = filtered[k];
            }
        }
    }
}

std::vector<double> Optimiser::FilterIndValLowPass(std::vector<double> unfiltered){
    double yn1 = unfiltered[0];
    double xn1 = unfiltered[0];

    std::vector<double> filtered;
    for(int i = 0; i < unfiltered.size(); i++){
        double xn = unfiltered[i];

        double yn = ((1-lowPassACoefficient)*yn1) + lowPassACoefficient*((xn + xn1)/2);

        xn1 = xn;
        yn1 = yn;

        filtered.push_back(yn);
    }
    return filtered;
}

std::vector<double> Optimiser::FilterIndValFIRFilter(std::vector<double> unfiltered, std::vector<double> filterCoefficients){
    std::vector<double> filtered;

    for(int i = 0; i < unfiltered.size(); i++){
        filtered.push_back(0);
    }

    for(int i = 0; i < unfiltered.size(); i++){
        for(int j = 0; j < filterCoefficients.size(); j++){
            if(i - j >= 0){
                filtered[i] += unfiltered[i - j] * filterCoefficients[j];
            }
        }

    }
    return filtered;
}

void Optimiser::setFIRFilter(std::vector<double> _FIRCoefficients){
    FIRCoefficients.clear();

    for(int i = 0; i < _FIRCoefficients.size(); i++){
        FIRCoefficients.push_back(_FIRCoefficients[i]);
    }
}

void Optimiser::SaveSystemStateToRolloutData(mjData *d, int thread_id, int data_index){

    rollout_data[thread_id][data_index].time = d->time;

    for(int i = 0; i < MuJoCo_helper->model->nq; i++){
        rollout_data[thread_id][data_index].q_pos[i] = d->qpos[i];
    }

    for(int i = 0; i < MuJoCo_helper->model->nv; i++){
        rollout_data[thread_id][data_index].q_vel[i] = d->qvel[i];
        rollout_data[thread_id][data_index].q_acc[i] = d->qacc[i];
        rollout_data[thread_id][data_index].q_acc_warmstart[i] = d->qacc_warmstart[i];
        rollout_data[thread_id][data_index].qfrc_applied[i] = d->qfrc_applied[i];
    }

    for(int i = 0; i < MuJoCo_helper->model->nu; i++){
        rollout_data[thread_id][data_index].ctrl[i] = d->ctrl[i];
    }

    for(int i = 0; i < 6*MuJoCo_helper->model->nbody; i++){
        rollout_data[thread_id][data_index].xfrc_applied[i] = d->xfrc_applied[i];
    }
}

void Optimiser::SaveBestRollout(int thread_id){
    for(int t = 0; t < horizon_length; t++){

        MuJoCo_helper->saved_systems_state_list[t+1]->time = rollout_data[thread_id][t].time;

        for(int i = 0; i < MuJoCo_helper->model->nq; i++){
            MuJoCo_helper->saved_systems_state_list[t+1]->qpos[i] = rollout_data[thread_id][t].q_pos[i];
        }

        for(int i = 0; i < MuJoCo_helper->model->nv; i++){
            MuJoCo_helper->saved_systems_state_list[t+1]->qvel[i]           = rollout_data[thread_id][t].q_vel[i];
            MuJoCo_helper->saved_systems_state_list[t+1]->qacc[i]           = rollout_data[thread_id][t].q_acc[i];
            MuJoCo_helper->saved_systems_state_list[t+1]->qacc_warmstart[i] = rollout_data[thread_id][t].q_acc_warmstart[i];
            MuJoCo_helper->saved_systems_state_list[t+1]->qfrc_applied[i]   = rollout_data[thread_id][t].qfrc_applied[i];
        }

        for(int i = 0; i < MuJoCo_helper->model->nu; i++){
            MuJoCo_helper->saved_systems_state_list[t+1]->ctrl[i] = rollout_data[thread_id][t].ctrl[i];
        }

        for(int i = 0; i < 6*MuJoCo_helper->model->nbody; i++){
            MuJoCo_helper->saved_systems_state_list[t+1]->xfrc_applied[i] = rollout_data[thread_id][t].xfrc_applied[i];
        }

        // Update the residuals of the nominal trajectory
        residuals[t] = activeModelTranslator->Residuals(MuJoCo_helper->saved_systems_state_list[t+1]);
    }
}