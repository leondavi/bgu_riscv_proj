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
	out_inst(bits);

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

void AERED::generate_ae_sample(uint32_t inst,uint8_t inst_type,uint32_t former_pc, uint32_t pc, uint32_t req_pc)
{
	VectorXd inst_v,inst_type_v,pc_deltas_v;
	convert_inst_to_vec(inst,inst_v);
	convert_group_type_to_vec(inst_type,inst_type_v);
	convert_dpc_dreqpc(former_pc,pc,req_pc,pc_deltas_v);

	VectorXd sample_row(1,inst_v.size(),inst_type_v.size(),pc_deltas_v.size());
	sample_row<<inst_v,inst_type_v,pc_deltas_v;
}

void AERED::convert_dpc_dreqpc(uint32_t former_pc,uint32_t pc, uint32_t req_pc,VectorXd &pc_and_req_pc)
{
	pc_and_req_pc(2) << abs(former_pc-pc)>EPSILON_PC_DIST,abs(pc-req_pc) > (this->win_size_*EPSILON_PC_DIST);
}

