/*
 * AERED.h
 *
 *  Created on: Apr 17, 2020
 *      Author: david
 */

#ifndef SRC_CPU_FLEXCPU_AERED_H_
#define SRC_CPU_FLEXCPU_AERED_H_

#include "../cppSANN/src/Models/include/Autoencoder.h"

#define RISCV_INST_LENGTH 32
#define RISCV_INST_LENGTH_COMPRESSED 16

#define AERED_DEFAULT_WIN_SIZE 5
#define AERED_DEFAULT_NUM_OF_INST_GROUPS 5

#define AERED_LEARNING_RATE_VAL 1e-5

#define DEFAULT_BITS_TO_SKIP {16,17,18,19}

#define EPSILON_PC_DIST 4

class AERED {

private:

	uint num_of_features_;
	uint num_of_inst_groups_;
	uint win_size_;
	std::shared_ptr<SANN::Autoencoder> ae_ptr_;

	double learning_rate_ = AERED_LEARNING_RATE_VAL;

public:

	AERED(uint win_size = AERED_DEFAULT_WIN_SIZE,uint num_of_inst_groups = AERED_DEFAULT_NUM_OF_INST_GROUPS) :
		num_of_inst_groups_(num_of_inst_groups),win_size_(win_size)
	{
		num_of_features_ = RISCV_INST_LENGTH+num_of_inst_groups;

		layer_size_t input_size = win_size_*num_of_features_;
		std::vector<layer_size_t> layers_sizes_vec = {input_size,input_size/2,input_size/4,input_size/2,input_size};
		std::vector<act_t> activations_vec = {act_t::ACT_NONE,act_t::ACT_LEAKY_RELU,act_t::ACT_LEAKY_RELU,act_t::ACT_LEAKY_RELU,act_t::ACT_NONE};

		ae_ptr_ = std::make_shared<SANN::Autoencoder>(layers_sizes_vec,learning_rate_);
		ae_ptr_->set_activations(activations_vec);
		ae_ptr_->set_optimizer(Optimizers::opt_t::OPT_ADAM);
	};

	AERED(std::string model_path,uint num_of_inst_groups = AERED_DEFAULT_NUM_OF_INST_GROUPS) :
		num_of_inst_groups_(num_of_inst_groups)
	{
		SANN::Autoencoder::load_model_from_file(model_path,ae_ptr_,learning_rate_);
		ae_ptr_->set_optimizer(Optimizers::opt_t::OPT_ADAM);
		uint input_layer_size = ae_ptr_->get_list_of_layers().front()->get_layer_size();

		num_of_features_ = RISCV_INST_LENGTH+num_of_inst_groups;
		win_size_ = input_layer_size / num_of_features_;
	}

	virtual ~AERED();


	void generate_ae_sample(uint32_t inst,uint8_t inst_type,uint32_t former_pc, uint32_t pc, uint32_t req_pc);

	void convert_inst_to_vec(uint32_t inst,VectorXd &out_inst,std::vector<uint> bits_to_skip = DEFAULT_BITS_TO_SKIP, bool compressed = false);
	void convert_group_type_to_vec(uint8_t inst_type,VectorXd &out_inst);

	void convert_dpc_dreqpc(uint32_t former_pc,uint32_t pc, uint32_t req_pc,VectorXd &pc_and_req_pc);

	template <class UINT>
	static UINT uabs(UINT a, UINT b) { return std::max(a,b)-std::min(a,b); }
};

#endif /* SRC_CPU_FLEXCPU_AERED_H_ */
