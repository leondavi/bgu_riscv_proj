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


void AERED::convert_inst_to_vec(uint32_t inst,VectorXd &out_inst,std::vector<uint> bits_to_skip, bool compressed)
{
	uint bits = compressed ? RISCV_INST_LENGTH_COMPRESSED : RISCV_INST_LENGTH;
	bits -= bits_to_skip.size();

	for (uint i = 0, it=0 ; i<bits; i++)
	{
		bool bit_val = (inst >> i) & 0x1;
		if (std::find(bits_to_skip.begin(), bits_to_skip.end(), i) == bits_to_skip.end())
		{
			out_inst(it++) = bit_val;
		}
	}
}

void AERED::convert_group_type_to_vec(uint8_t inst_type,VectorXd &out_inst)
{
	out_inst = VectorXd::Zero(1,this->num_of_inst_groups_);
	if(inst_type < this->num_of_inst_groups_)
	{
		out_inst(inst_type);
	}
	else
	{
		out_inst (0) = 1;
	}
}

