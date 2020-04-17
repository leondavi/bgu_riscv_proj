/*
 * AERED.h
 *
 *  Created on: Apr 17, 2020
 *      Author: david
 */

#ifndef SRC_CPU_FLEXCPU_AERED_H_
#define SRC_CPU_FLEXCPU_AERED_H_

#include "../cppSANN/src/include/Model.h"

#define RISCV_INST_LENGTH 32
#define RISCV_INST_LENGTH_COMPRESSED 16

#define AERED_DEFAULT_WIN_SIZE 5
#define AERED_DEFAULT_NUM_OF_INST_GROUPS 5

typedef Eigen::Matrix<int,1,RISCV_INST_LENGTH> VectorInst;

class AERED {

private:

	uint8_t win_size_;
	uint8_t num_of_inst_groups_;
	uint8_t num_of_features_;

public:

	AERED(uint8_t win_size = AERED_DEFAULT_WIN_SIZE,uint8_t num_of_inst_groups = AERED_DEFAULT_NUM_OF_INST_GROUPS) :
		win_size_(win_size),num_of_inst_groups_(num_of_inst_groups)
	{
		num_of_features_ = win_size*(RISCV_INST_LENGTH+num_of_inst_groups);
	};
	virtual ~AERED();

	 static void convert_inst_to_vec(uint32_t inst,VectorInst &out_inst, bool compressed = false);
	 void convert_group_type_to_vec(uint8_t inst_type,VectorXi &out_inst);

};

#endif /* SRC_CPU_FLEXCPU_AERED_H_ */
