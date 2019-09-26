
#include "BGUTracer.h"


namespace tracer {


//------------------------------------//
//		BGUInfoPackage 				  //
//------------------------------------//

BGUInfoPackage::BGUInfoPackage(ThreadID tid,std::weak_ptr<InflightInst> wk_ptr_inst):
		tid(tid),
		wk_ptr_inst(wk_ptr_inst)
{
	//generate attributes vector:
	attributes.resize(ATTR_TOTAL);
	attributes[ATTR_DEFAULT] = {"tid","status","pc"};
	attributes[ATTR_FE] = {};//TODO
	attributes[ATTR_DE] = {};//TODO
	attributes[ATTR_EX] = {};//TODO
	attributes[ATTR_IS] = {};//TODO

	update_package_attributes();
}

void BGUInfoPackage::update_package_attributes()
{
	data = inflightinst_to_string();
}

std::vector<std::string> BGUInfoPackage::inflightinst_to_string()
{
	std::vector<std::string> res_string;
	//******************************************//
	//			Default attributes 				//
	//*****************************************//
	std::shared_ptr<InflightInst> inst = wk_ptr_inst.lock();
	//-------------- tid ---------------//
	res_string.push_back("tid"); res_string.push_back(std::to_string(this->tid));
	//-------------- status ---------------//
	res_string.push_back("status"); res_string.push_back(status_strings[inst->status()]);
	//-------------- pc ------------------//
	res_string.push_back("pc"); res_string.push_back("0x"+dec_to_hex_str(inst->pcState().instAddr()));

	switch (inst->status())
	{
	case InflightInst::Status::Decoded :
		{
			this->packet_status = PST_DE;
			res_string = decode_to_string(inst);
			break;
		}
	case InflightInst::Status::Executing :
		{
			this->packet_status = PST_EX;
			res_string = execute_to_string(inst);
			break;
		}
	case InflightInst::Status::Issued :
		{
			this->packet_status = PST_IS;
			res_string = issue_to_string(inst);
			break;
		}
	case InflightInst::Status::Empty : //equivalent to fetch
		{
			this->packet_status = PST_FE;
			res_string = fetch_to_string(inst);
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
void BGUInfoPackage::send_packet_to_tracer()
{
	// BGUTracer* tracer_p = BGUTracer::get_inst();
	// tracer_p->add_package_to_current_tick_line(std::make_shared<BGUInfoPackage>(this),this->tid);
}

std::vector<std::string> BGUInfoPackage::decode_to_string(std::shared_ptr<InflightInst> inst)
{
	std::vector<std::string> res;
	return res;
}
std::vector<std::string> BGUInfoPackage::issue_to_string(std::shared_ptr<InflightInst> inst)
{
	std::vector<std::string> res;
		return res;
}
std::vector<std::string> BGUInfoPackage::execute_to_string(std::shared_ptr<InflightInst> inst)
{
	std::vector<std::string> res;
		return res;
}
std::vector<std::string> BGUInfoPackage::fetch_to_string(std::shared_ptr<InflightInst> inst)
{
	std::vector<std::string> res;
		return res;
}

std::string BGUInfoPackage::get_all_attributes_comma_seperated()
	{
		std::stringstream res;
		for (auto &att_vec : this->attributes)
		{
			for (int i=0; i < att_vec.size(); i++)
			{
				res<<att_vec[i];
				if(i<att_vec.size()-1)
				{
					res<<"\",";//put comma to string
				}
			}
			res<<",";
		}

		return res.str();
	}

std::string BGUInfoPackage::get_Pstatus_headers()
{
	std::stringstream res;
	for (int i=0; i<Pstatus_strings.size(); i++)
	{
		res<<Pstatus_strings[i];
		if(i<Pstatus_strings.size()-1)
		{
			res<<","; //put the comma separator
		}
	}

	return res.str();
}

}//end of namespace


