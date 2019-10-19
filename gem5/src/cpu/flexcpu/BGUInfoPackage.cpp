
#include "BGUTracer.h"


namespace tracer {


//------------------------------------//
//		BGUInfoPackage 				  //
//------------------------------------//

void BGUInfoPackage::generate_attributes()
{
		attributes.resize(STG_TOTAL);
		attributes[STG_FE] = {};//TODO
		attributes[STG_DE] = {};//TODO
		attributes[STG_EX] = {opcode};//TODO
		attributes[STG_IS] = {opcode};//TODO

		std::vector<std::string> default_vec = {tid_str,status_str,pc_str};

		for (int i=0; i<attributes.size(); i++) //adding the default attributes
		{
			attributes[i].insert(attributes[i].begin(),default_vec.begin(),default_vec.end());
		}
}


BGUInfoPackage::BGUInfoPackage(ThreadID tid,std::weak_ptr<InflightInst> wk_ptr_inst) :
		tid(tid),
		wk_ptr_inst(wk_ptr_inst)
{
    packet_status = PST_NOT_INITIALIZED;
    generate_attributes();
	update_package_attributes();
}


/**
 * This constructor is used by tracer to create a dummy package to generate the
 * headlines
 */
BGUInfoPackage::BGUInfoPackage()
{
    packet_status = PST_NOT_INITIALIZED;
    tid = 0;
    generate_attributes();
}

void BGUInfoPackage::update_package_attributes()
{
	data = inflightinst_to_string();
}

std::vector<std::string> BGUInfoPackage::inflightinst_to_string()
{
	std::vector<std::string> res_string;
	std::vector<std::string> stage_vec_strings;
	//******************************************//
	//			Default attributes 				//
	//*****************************************//
	std::shared_ptr<InflightInst> inst = wk_ptr_inst.lock();
	//-------------- tid ---------------//
	res_string.push_back(std::to_string(this->tid));
	//-------------- status ---------------//
	res_string.push_back(status_strings[inst->status()]);
	//-------------- pc ------------------//
	res_string.push_back("0x"+dec_to_hex_str(inst->pcState().instAddr()));

	switch (inst->status())
	{
	case InflightInst::Status::Decoded :
		{
			this->packet_status = STG_DE;
			stage_vec_strings = decode_to_string(inst);
			break;
		}
	case InflightInst::Status::Executing :
		{
			this->packet_status = STG_EX;
			stage_vec_strings = execute_to_string(inst);
			break;
		}
	case InflightInst::Status::Issued :
		{
			this->packet_status = STG_IS;
			stage_vec_strings = issue_to_string(inst);
			break;
		}
	case InflightInst::Status::Empty : //equivalent to fetch
		{
			this->packet_status = STG_FE;
			stage_vec_strings = fetch_to_string(inst);
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

	res_string.insert(res_string.end(),stage_vec_strings.begin(),stage_vec_strings.end());

	return res_string;
}
void BGUInfoPackage::send_packet_to_tracer()
{
	if(!this->wk_ptr_inst.expired())
	{
		update_package_attributes();
		BGUTracer::get_inst().receive_bgu_info_package(shared_from_this(),this->tid);
	}
}

std::vector<std::string> BGUInfoPackage::decode_to_string(std::shared_ptr<InflightInst> inst)
{
	std::vector<std::string> res;
	return res;
}
std::vector<std::string> BGUInfoPackage::issue_to_string(std::shared_ptr<InflightInst> inst)
{
	std::vector<std::string> res;
	//--------------opcode ---------------//
	res.push_back(inst->staticInst()->disassemble(inst->pcState().instAddr()));

	return res;
}
std::vector<std::string> BGUInfoPackage::execute_to_string(std::shared_ptr<InflightInst> inst)
{
	std::vector<std::string> res;
	//--------------opcode ---------------//
		res.push_back(inst->staticInst()->disassemble(inst->pcState().instAddr()));
		return res;
}
std::vector<std::string> BGUInfoPackage::fetch_to_string(std::shared_ptr<InflightInst> inst)
{
	std::vector<std::string> res;
		return res;
}

/**
 * This function is used by Tracer instance in order to generate the headline of the table
 */
std::string BGUInfoPackage::get_1st_headline_stages_comma_seperated()
{
	std::string res = "";
	for (int i=0; i < Pstatus_strings.size(); i++)
	{
		res += Pstatus_strings[i];
		if(i < Pstatus_strings.size()-1)
		{
			res += ",";
		}
	}
	return res;
}

/**
 * This function is used by Tracer instance in order to generate the second headline of the table
 */
std::string BGUInfoPackage::get_2nd_headline_attributes_comma_seperated()
{
	std::stringstream res;
	for (int i=0; i<this->attributes.size(); i++)
	{
		res<<"\"";
		for (int j=0; j < this->attributes[i].size(); j++) //att_vec is vector of strings
		{
			res<<this->attributes[i][j];
			if(j<this->attributes[i].size()-1)
			{
				res<<",";//put comma to string
			}
		}
		res<<"\"";
		if(i < this->attributes.size()-1) //put commas
		{
			res<<",";
		}
	}

	return res.str();
}

}//end of namespace


