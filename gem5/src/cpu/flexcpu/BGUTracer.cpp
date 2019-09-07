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


void BGUTracer::get_package(std::weak_ptr<BGUInfoPackage> rcv_pckg)
{
	std::shared_ptr<BGUInfoPackage> pckg_inst = rcv_pckg.lock();
}

// plan - using the first packet to initialize the BGU tracer
// information of all stages are stored within a single packet so it will be fine









//------------------------------------//
//		BGUInfoPackage 				  //
//------------------------------------//

BGUInfoPackage::BGUInfoPackage(ThreadID tid,std::weak_ptr<InflightInst> wk_ptr_inst):
		tid(tid),
		wk_ptr_inst(wk_ptr_inst)
{
	update_package_attributes();
}

void BGUInfoPackage::update_package_attributes()
{
	data = inflightinst_to_string();
}

std::vector<std::string> BGUInfoPackage::inflightinst_to_string()
{
	std::vector<std::string> res_string;

	//-------------- status ---------------//
	std::shared_ptr<InflightInst> inst = wk_ptr_inst.lock();
	res_string.push_back("status"); res_string.push_back(status_strings[inst->status()]);

	//-------------- pc ------------------//
	res_string.push_back("pc"); res_string.push_back("0x"+dec_to_hex_str(inst->pcState().instAddr()));

	switch (inst->status())
	{
	case InflightInst::Status::Decoded :
		{
			//std::vector<std::string> vec_str = decode_to_string(inst);
			//res_string.insert(res_string.end(),vec_str.begin(),vec_str.end());
			break;
		}
	case InflightInst::Status::Executing :
		{
			break;
		}
	case InflightInst::Status::Issued :
		{
			break;
		}
	case InflightInst::Status::Memorying :
		{
			break;
		}
	case InflightInst::Status::Committed :
		{
			break;
		}
	case InflightInst::Status::Complete :
		{
			break;
		}
	default:
	{
		break;
	}

	}

	return res_string;
}




} /* namespace tracer */
