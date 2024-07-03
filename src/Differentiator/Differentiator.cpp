#include "Differentiator.h"

Differentiator::Differentiator(std::shared_ptr<ModelTranslator> model_translator, std::shared_ptr<MuJoCoHelper> MuJoCo_helper){
    this->model_translator = model_translator;
    this->MuJoCo_helper = MuJoCo_helper;
}

void Differentiator::ComputeDerivatives(MatrixXd &A, MatrixXd &B, const std::vector<int> &cols,
                                        vector<MatrixXd> &r_x, vector<MatrixXd> &r_u,
                                        int data_index, int tid, bool terminal, bool cost_derivs,
                                        bool central_diff, double eps){

    // Aliases
    dof = model_translator->current_state_vector.dof;
    num_ctrl = model_translator->current_state_vector.num_ctrl;
    dim_state = 2 * dof;

    // Aliases
    int nq = MuJoCo_helper->model->nq, nv = MuJoCo_helper->model->nv,
        na = MuJoCo_helper->model->na;

    // Reset some debugging timing variables
    time_mj_forwards = 0.0f;
    count_integrations = 0;
    auto start = std::chrono::high_resolution_clock::now();
    auto diff_start = std::chrono::high_resolution_clock::now();

    // Memory allocation for next states, whether perturbed or not
    MatrixXd next_state(dim_state, 1);
    MatrixXd next_state_plus(dim_state, 1);
    MatrixXd next_state_minus(dim_state, 1);

    // Memory allocation for current state and controls
    MatrixXd unperturbed_controls(num_ctrl, 1);
    MatrixXd unperturbed_positions(dof, 1);
    MatrixXd unperturbed_velocities(dof, 1);

    // Memory allocation for perturbed state and controls
    MatrixXd perturbed_controls(num_ctrl, 1);
    MatrixXd perturbed_positions(dof, 1);
    MatrixXd perturbed_velocities(dof, 1);

    // Initialise sub matrices of A and B matrix
    // ------------ dof x dof ----------------
    MatrixXd dstatedqpos(dim_state, dof);
    MatrixXd dstatedqvel(dim_state, dof);

    // ------------ dof x ctrl --------------
    MatrixXd dstatedctrl(dim_state, num_ctrl);

    // How the cost changes
//    MatrixXd dcostdctrl(num_ctrl, 1);
//    MatrixXd dcostdpos(dof, 1);
//    MatrixXd dcostdvel(dof, 1);

//    double cost;
//    double cost_inc = 0.0f;
//    double cost_dec = 0.0f;

    MatrixXd residuals(2*dof + num_ctrl, 1);
    MatrixXd residuals_inc(2*dof + num_ctrl, 1);
    MatrixXd residuals_dec(2*dof + num_ctrl, 1);

    // Mark stack for stack allocation of some dynamic variables
    mj_markStack(MuJoCo_helper->fd_data[tid]);

    mjtNum *dpos  = mj_stackAllocNum(MuJoCo_helper->fd_data[tid], nv);
    mjtNum *vel_diff = mj_stackAllocNum(MuJoCo_helper->fd_data[tid], nv);

    mjtNum *next_full_state = mj_stackAllocNum(MuJoCo_helper->fd_data[tid], nq + nv + na);
    mjtNum *next_full_state_pos = mj_stackAllocNum(MuJoCo_helper->fd_data[tid], nq + nv + na);
    mjtNum *next_full_state_minus = mj_stackAllocNum(MuJoCo_helper->fd_data[tid], nq + nv + na);
    mju_zero(next_full_state, nq + nv + na);
    mju_zero(next_full_state_pos, nq + nv + na);
    mju_zero(next_full_state_minus, nq + nv + na);

    // Copy data we wish to finite-difference into finite differencing data (for multi threading)
    MuJoCo_helper->CpMjData(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid], MuJoCo_helper->saved_systems_state_list[data_index]);

    residuals = model_translator->Residuals(MuJoCo_helper->fd_data[tid]);

    // Compute next state with no perturbations
    mj_step(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid]);
    next_state = model_translator->ReturnStateVector(MuJoCo_helper->fd_data[tid], model_translator->current_state_vector);
    mj_getState(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid], next_full_state, mjSTATE_PHYSICS);
//    cost = model_translator->CostFunction(MuJoCo_helper->fd_data[tid], model_translator->full_state_vector, terminal);

    // Reset the simulator to the initial state
    MuJoCo_helper->CopySystemState(MuJoCo_helper->fd_data[tid], MuJoCo_helper->saved_systems_state_list[data_index]);

    unperturbed_controls = model_translator->ReturnControlVector(MuJoCo_helper->fd_data[tid], model_translator->current_state_vector);
    unperturbed_velocities = model_translator->ReturnVelocityVector(MuJoCo_helper->fd_data[tid], model_translator->current_state_vector);

    // --------------------------------------------- FD for controls ---------------------------------------------
    MatrixXd control_limits = model_translator->ReturnControlLimits(model_translator->current_state_vector);
    for(int i = 0; i < num_ctrl; i++){
        bool compute_column = false;
        for(int col : cols){
            if(i == col){
                compute_column = true;
            }
        }

        // Skip this column if it is not in the list of columns to compute
        if(!compute_column){
            continue;
        }

        // perturb control vector positively
        perturbed_controls = unperturbed_controls.replicate(1,1);
        perturbed_controls(i) += eps;

        // Check if the perturbed control is within the control limits
        int nudge_forward = 1;
        if(perturbed_controls(i) > control_limits(2*i + 1)){
            nudge_forward = 0;
        }

        if(nudge_forward){
            // count how many calls to step_skip we make
            count_integrations++;

            // Set perturbed control vector
            model_translator->SetControlVector(perturbed_controls, MuJoCo_helper->fd_data[tid], model_translator->current_state_vector);

            // If computing cost derivatives
            if(cost_derivs){
                residuals_inc = model_translator->Residuals(MuJoCo_helper->fd_data[tid]);
            }

            // Integrate the simulator
            start = std::chrono::high_resolution_clock::now();
            mj_stepSkip(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid], mjSTAGE_VEL, 1);
            time_mj_forwards += static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count());

            // return the new state vector
            next_state_plus = model_translator->ReturnStateVector(MuJoCo_helper->fd_data[tid], model_translator->current_state_vector);

            // Undo the perturbation
            MuJoCo_helper->CopySystemState(MuJoCo_helper->fd_data[tid], MuJoCo_helper->saved_systems_state_list[data_index]);
        }

        int nudge_back;

        // perturb control vector in opposite direction
        perturbed_controls = unperturbed_controls.replicate(1, 1);
        perturbed_controls(i) -= eps;

        // If we use central difference or we didnt nudge forward, due to control limits
        if(central_diff || !nudge_forward){
            nudge_back = 1;
            if(perturbed_controls(i) < control_limits(2*i)){
                nudge_back = 0;
            }
        } else {
            nudge_back = 0;
        }

        if(nudge_back){
            model_translator->SetControlVector(perturbed_controls, MuJoCo_helper->fd_data[tid], model_translator->current_state_vector);

            // If calculating cost derivatives via finite-differencing
            // TODO - I am unsure whether we should compute residuals before or after the integration
            if(cost_derivs){
                residuals_dec = model_translator->Residuals(MuJoCo_helper->fd_data[tid]);
            }

            // integrate simulator
            start = std::chrono::high_resolution_clock::now();
            mj_stepSkip(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid], mjSTAGE_VEL, 1);
            time_mj_forwards += static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count());

            // return the new state vector
            next_state_minus = model_translator->ReturnStateVector(MuJoCo_helper->fd_data[tid], model_translator->current_state_vector);

            // Undo perturbation
            MuJoCo_helper->CopySystemState(MuJoCo_helper->fd_data[tid], MuJoCo_helper->saved_systems_state_list[data_index]);
        }

        // Compute finite differences, depending on what perturbations were made
        if(nudge_forward && nudge_back){
            for(int j = 0; j < dim_state; j++){
                dstatedctrl(j, i) = (next_state_plus(j) - next_state_minus(j))/(2*eps);
            }

            if(cost_derivs){
                // Loop through number of residuals, compute dr0/dx, dr1/dx ... drn/dx
                for(int j = 0; j < model_translator->residual_list.size(); j++){
                    r_u[j](i, 0) = (residuals_inc(j) - residuals_dec(j)) / (2 * eps);
                }
            }
        }
        else if(nudge_forward){
            for(int j = 0; j < dim_state; j++){
                dstatedctrl(j, i) = (next_state_plus(j) - next_state(j))/(eps);
            }

            if(cost_derivs){
                for(int j = 0; j < model_translator->residual_list.size(); j++){
                    r_u[j](i, 0) = (residuals_inc(j) - residuals(j)) / (eps);
                }
            }
        }
        else if(nudge_back){
            for(int j = 0; j < dim_state; j++){
                dstatedctrl(j, i) = (next_state(j) - next_state_minus(j))/(eps);
            }

            if(cost_derivs){
                for(int j = 0; j < model_translator->residual_list.size(); j++){
                    r_u[j](i, 0) = (residuals(j) - residuals_dec(j)) / (eps);
                }
            }
        }
    }

    // ----------------------------------------------- FD for velocities ---------------------------------------------
    for(int i = 0; i < dof; i++){
        bool compute_column = false;

        for(int col : cols){
            if(i == col){
                compute_column = true;
            }
        }

        // Skip this column if it is not in the list of columns to compute
        if(!compute_column){
            continue;
        }

        count_integrations++;
        // Perturb velocity vector positively
        perturbed_velocities = unperturbed_velocities.replicate(1, 1);
        perturbed_velocities(i) += eps;
        model_translator->SetVelocityVector(perturbed_velocities, MuJoCo_helper->fd_data[tid], model_translator->current_state_vector);

        // If calculating cost derivs via finite-differencing
        if(cost_derivs){
            residuals_inc = model_translator->Residuals(MuJoCo_helper->fd_data[tid]);
        }

        // Integrate the simulator
        start = std::chrono::high_resolution_clock::now();
        mj_stepSkip(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid], mjSTAGE_POS, 1);
        time_mj_forwards += static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count());

        // return the new velocity vector
        mj_getState(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid], next_full_state_pos, mjSTATE_PHYSICS);
        next_state_plus = model_translator->ReturnStateVector(MuJoCo_helper->fd_data[tid], model_translator->current_state_vector);

        if(central_diff){
            // reset the data state back to initial data state
            MuJoCo_helper->CopySystemState(MuJoCo_helper->fd_data[tid], MuJoCo_helper->saved_systems_state_list[data_index]);

            // perturb velocity vector negatively
            perturbed_velocities = unperturbed_velocities.replicate(1, 1);
            perturbed_velocities(i) -= eps;
            model_translator->SetVelocityVector(perturbed_velocities, MuJoCo_helper->fd_data[tid], model_translator->current_state_vector);

            // If calculating cost derivs via finite-differencing
            if(cost_derivs){
                residuals_dec = model_translator->Residuals(MuJoCo_helper->fd_data[tid]);
            }

            // Integrate the simulator
            start = std::chrono::high_resolution_clock::now();
            mj_stepSkip(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid], mjSTAGE_POS, 1);
            time_mj_forwards += static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count());

            // Return the new velocity vector
            mj_getState(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid], next_full_state_minus, mjSTATE_PHYSICS);
            next_state_minus = model_translator->ReturnStateVector(MuJoCo_helper->fd_data[tid], model_translator->current_state_vector);

        }

        // TODO - this could be refactored as its fairly repetitive code
        if(central_diff){
            // Compute one column of the A matrix
            mj_differentiatePos(MuJoCo_helper->model, vel_diff, (2 * eps), next_full_state_minus, next_full_state_pos);
            for(int j = 0; j < dim_state / 2; j++){
                int q_index = model_translator->StateIndexToQposIndex(j, model_translator->current_state_vector);
                dstatedqvel(j, i) = vel_diff[q_index];
            }

            for(int j = dim_state / 2; j < dim_state; j++){
                dstatedqvel(j, i) = (next_state_plus(j) - next_state_minus(j))/(2*eps);
            }

            if(cost_derivs) {
                for(int j = 0; j < model_translator->residual_list.size(); j++){
                    r_x[j](i + dof, 0) = (residuals_inc(j) - residuals_dec(j)) / (2 * eps);
                }
            }

        }
        else{
            mj_differentiatePos(MuJoCo_helper->model, vel_diff, eps, next_full_state, next_full_state_pos);
            for(int j = 0; j < dim_state / 2; j++){
                int q_index = model_translator->StateIndexToQposIndex(j, model_translator->current_state_vector);
                dstatedqvel(j, i) = vel_diff[q_index];
            }

            for(int j = dim_state / 2; j < dim_state; j++){
                dstatedqvel(j, i) = (next_state_plus(j) - next_state(j))/(eps);
            }

            if(cost_derivs) {
                for(int j = 0; j < model_translator->residual_list.size(); j++){
                    r_x[j](i + dof, 0) = (residuals_inc(j) - residuals(j)) / (eps);
                }
            }
        }

        // Undo perturbation
        MuJoCo_helper->CopySystemState(MuJoCo_helper->fd_data[tid], MuJoCo_helper->saved_systems_state_list[data_index]);
    }

    // ----------------------------------------------- FD for positions ---------------------------------------------
    for(int i = 0; i < dof; i++){
        bool compute_column = false;
        for(int col : cols) {
            if (i == col) {
                compute_column = true;
            }
        }

        // Skip this column if it is not in the list of columns to compute
        if(!compute_column){
            continue;
        }

        // Compute the index of the position vector in MuJoCo that corresponds to the index of the state vector
        int dpos_index = model_translator->StateIndexToQposIndex(i, model_translator->current_state_vector);

        count_integrations++;

        // Perturb position vector positively
        mju_zero(dpos, nv);
        dpos[dpos_index] = 1;
        mj_integratePos(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid]->qpos, dpos, eps);

        if(cost_derivs){
            residuals_inc = model_translator->Residuals(MuJoCo_helper->fd_data[tid]);
        }

        // Integrate the simulator
        start = std::chrono::high_resolution_clock::now();
        mj_stepSkip(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid], mjSTAGE_NONE, 1);
        time_mj_forwards += static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count());

        // return the positive perturbed next state vector
        mj_getState(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid], next_full_state_pos, mjSTATE_PHYSICS);
        next_state_plus = model_translator->ReturnStateVector(MuJoCo_helper->fd_data[tid], model_translator->current_state_vector);

        if(central_diff){
            // reset the data state back to initial data statedataIndex
            MuJoCo_helper->CopySystemState(MuJoCo_helper->fd_data[tid], MuJoCo_helper->saved_systems_state_list[data_index]);

            // perturb position vector negatively
            mj_integratePos(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid]->qpos, dpos, -eps);

            if(cost_derivs){
                residuals_dec = model_translator->Residuals(MuJoCo_helper->fd_data[tid]);
            }

            // Integrate the simulator
            start = std::chrono::high_resolution_clock::now();
            mj_stepSkip(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid], mjSTAGE_NONE, 1);
            time_mj_forwards += static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count());

            // Return the decremented state vector
            mj_getState(MuJoCo_helper->model, MuJoCo_helper->fd_data[tid], next_full_state_minus, mjSTATE_PHYSICS);
            next_state_minus = model_translator->ReturnStateVector(MuJoCo_helper->fd_data[tid], model_translator->current_state_vector);

        }

        if(central_diff){
            // Compute one column of the A matrix
            mj_differentiatePos(MuJoCo_helper->model, vel_diff, (2 * eps), next_full_state_minus, next_full_state_pos);

            for(int j = 0; j < dim_state / 2; j++){
                int q_index = model_translator->StateIndexToQposIndex(j, model_translator->current_state_vector);
                dstatedqpos(j, i) = vel_diff[q_index];
            }

            for(int j = dim_state / 2; j < dim_state; j++){
                dstatedqpos(j, i) = (next_state_plus(j) - next_state_minus(j))/(2*eps);
            }

            if(cost_derivs) {
                for(int j = 0; j < model_translator->residual_list.size(); j++){
                    r_x[j](i, 0) = (residuals_inc(j) - residuals_dec(j)) / (2 * eps);
                }
            }
        }
        else{
            mj_differentiatePos(MuJoCo_helper->model, vel_diff, eps, next_full_state, next_full_state_pos);

            for(int j = 0; j < dim_state / 2; j++){
                int q_index = model_translator->StateIndexToQposIndex(j, model_translator->current_state_vector);
                dstatedqpos(j, i) = vel_diff[q_index];
            }

            for(int j = dim_state / 2; j < dim_state; j++){
                dstatedqpos(j, i) = (next_state_plus(j) - next_state(j))/eps;
            }


            if(cost_derivs) {
                for(int j = 0; j < model_translator->residual_list.size(); j++){
                    r_x[j](i, 0) = (residuals_inc(j) - residuals(j)) / (eps);
                }
            }
        }

        // Undo perturbation
        MuJoCo_helper->CopySystemState(MuJoCo_helper->fd_data[tid], MuJoCo_helper->saved_systems_state_list[data_index]);

    }

    // free the stack allocated variables
    mj_freeStack(MuJoCo_helper->fd_data[tid]);

//    std::cout << "statedqvel: \n";
//    std::cout << dstatedqvel << "\n";

    // ------------ A -----------------
    // dqposdqpos       dqposdqvel
    //
    // dqveldqpos       dqveldqvel
    // --------------------------------
    for(int col : cols){
        A.block(0, col, dim_state, 1) =
                dstatedqpos.block(0, col, dim_state, 1);

        A.block(0, col + dof, dim_state, 1) =
                dstatedqvel.block(0, col, dim_state, 1);
    }

    // ------------- B -------------------
    //          dqposdctrl
    //          dqveldctrl
    // ----------------------------------
    for(int col : cols){
        if(col < num_ctrl){
            B.block(0, col, dim_state, 1) = dstatedctrl.block(0, col, dim_state, 1);
        }
    }

//    if(cost_derivs){
//        std::cout << "warning - fd for cost derivatives is currently untested \n";
//        l_x.block(0, 0, dof, 1) = dcostdpos;
//        l_x.block(dof, 0, dof, 1) = dcostdvel;
//        l_u.block(0, 0, num_ctrl, 1) = dcostdctrl;
//
//        l_uu = l_u.transpose() * l_u;
//        l_xx = l_x.transpose() * l_x;
//    }

//    std::cout << "time of sim integration: " << time_mj_forwards / 1000.0f << "\n";
//    std::cout << "num of sim integration: " << count_integrations << "\n";
//    std::cout << "diff time: "  << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - diff_start).count() / 1000.0 << std::endl;
}