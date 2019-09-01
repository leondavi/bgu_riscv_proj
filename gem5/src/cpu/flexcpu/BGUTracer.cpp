/*
 * BGUTracer.cpp
 *
 *  Created on: 01 Sep 2019
 *      Author: david
 */

#include "BGUTracer.h"

namespace tracer {

BGUTracer::BGUTracer() {
	// TODO Auto-generated constructor stub

}

//------------------------------------//
//		BGUInfoPackage 				  //
//------------------------------------//

BGUInfoPackage::BGUInfoPackage(uint pipe_stage,ThreadID tid,std::weak_ptr<InflightInst> wk_ptr_inst):
		pipe_stage(pipe_stage),
		tid(tid),
		wk_ptr_inst(wk_ptr_inst)
{

}

std::vector<std::string> BGUInfoPackage::inflightinst_to_string(std::weak_ptr<InflightInst> wk_ptr_inst)
{
	std::vector<std::string> res_string;

	//-------------- status ---------------//
	std::shared_ptr<InflightInst> inst = wk_ptr_inst.lock();
	res_string.push_back("status"); res_string.push_back(status_strings[inst->status()]);

	//-------------- pc ------------------//
	res_string.push_back("pc"); res_string.push_back("0x"+dec_to_hex_str(inst->pcState().instAddr()));



	return res_string;

	/*
	 * inline std::vector<bgu::var_attr_t> clear_and_add_default_vars()
	{
		bgu::var_attr_t tmp_attr;
		vars_pairs.clear();
		//valid create attributes
		tmp_attr.first = STRING_VAR(vld);
		tmp_attr.second = std::to_string(vld);
		vars_pairs.push_back(tmp_attr);
		//Tid create attributes
		tmp_attr.first = STRING_VAR(Tid);
		tmp_attr.second = std::to_string(id.threadId);
		vars_pairs.push_back(tmp_attr);
		//Pc create attributes
		tmp_attr.first = STRING_VAR(Pc);
		tmp_attr.second = "0x"+dec_to_hex_str(Pc.instAddr());
		vars_pairs.push_back(tmp_attr);
	 */
}


} /* namespace tracer */
