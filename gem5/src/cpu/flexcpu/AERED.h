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

typedef Eigen::Matrix<int,1,RISCV_INST_LENGTH> VectorInst;

class AERED {

private:

	uint8_t win_size_;

public:

	AERED(uint8_t win_size) : win_size_(win_size) {};
	virtual ~AERED();

	 static void convert_inst_to_vec(uint32_t inst,VectorInst &out_inst, bool compressed = false);
};

#endif /* SRC_CPU_FLEXCPU_AERED_H_ */
