/*
 * AERED.cpp
 *
 *  Created on: Apr 17, 2020
 *      Author: david
 */

#include "AERED.h"


AERED::~AERED() {
	// TODO Auto-generated destructor stub
}


void AERED::convert_inst_to_vec(uint32_t inst,VectorInst &out_inst, bool compressed)
{
	uint8_t bits = compressed ? RISCV_INST_LENGTH_COMPRESSED : RISCV_INST_LENGTH;

	for (uint8_t i = 0 ; i<bits; i++)
	{
		bool bit_val = (inst >> i) & 0x1;
		out_inst(i) = bit_val;
	}
}

void AERED::convert_group_type_to_vec(uint8_t inst_type,VectorXi &out_inst)
{
	out_inst = VectorXi::Zero(1,this->num_of_inst_groups_);
	if(inst_type < this->num_of_inst_groups_)
	{
		out_inst(inst_type);
	}
	else
	{
		out_inst (0) = 1;
	}
}

