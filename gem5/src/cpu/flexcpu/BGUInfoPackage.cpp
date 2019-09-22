
#include "BGUTracer.h"


namespace tracer {


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
	res_string.push_back("tid"); res_string.push_back(std::to_string(this->tid));
	res_string.push_back("status"); res_string.push_back(status_strings[inst->status()]);

	//-------------- pc ------------------//
	res_string.push_back("pc"); res_string.push_back("0x"+dec_to_hex_str(inst->pcState().instAddr()));

	switch (inst->status())
	{
	case InflightInst::Status::Decoded :
		{
			res_string = decode_to_string(inst);
			break;
		}
	case InflightInst::Status::Executing :
		{
			res_string = execute_to_string();
			break;
		}
	case InflightInst::Status::Issued :
		{
			res_string = issue_to_string();
			break;
		}
	case InflightInst::Status::Empty : //equivalent to fetch
		{
			res_string = fetch_to_string();
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
	BGUTracer* tracer_p = BGUTracer::get_inst();
	tracer_p->add_package_to_current_tick_line(std::make_shared<BGUInfoPackage>(this),this->tid);
}

}//end of namespace


