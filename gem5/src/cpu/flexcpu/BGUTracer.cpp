/*
 * BGUTracer.cpp
 *
 *  Created on: 01 Sep 2019
 *      Author: david
 */

#include "BGUTracer.h"

namespace tracer {

BGUTracer::BGUTracer(std::string CsvFileFullPath ,bool FilterByThread,ThreadID FilterWhichThread):
		full_file_path(CsvFileFullPath),
		filter_by_thread(FilterByThread),
        filter_which_thread(FilterWhichThread),
		last_tick(curTick()),
		curr_tick(curTick())
{
	if(file_exists(full_file_path))
	{
		std::remove(full_file_path.c_str());
	}
	// TODO Auto-generated constructor stub
	csv_table_fstr.open(full_file_path.c_str(),std::ofstream::out | std::ofstream::app);

	BGUInfoPackage dummy_package; //using this to initialize the table

	total_stages = dummy_package.get_total_stages_count();

	reset_string_buffers();
	//generates headlines
	csv_table_fstr << "Sim Time,"<< dummy_package.get_1st_headline_stages_comma_seperated() << std::endl;
	csv_table_fstr << " ,"<< dummy_package.get_2nd_headline_attributes_comma_seperated() << std::endl;

}



/**
 * This function update the tracer with bguinfopacket
 * This function is called by the packet itself when calling packet's send function
 * TODO add functionality of filters to filter out whatever user wants
 */
bgu_ipckg_status BGUTracer::receive_bgu_info_package(std::shared_ptr<BGUInfoPackage> rcv_pckg,ThreadID tid)
{
	curr_tick = curTick();

	Tick diff_tick = curr_tick - last_tick;
	if (diff_tick > 0)
	{
		deploy_string_buffers_to_table(last_tick);
		reset_string_buffers();
		if(diff_tick < CYCLE_ROUND_VAL)
		{
			last_tick += CYCLE_ROUND_VAL;
		}
		else
		{
			last_tick = curr_tick;
		}
	}


	bool continue_cond = (filter_by_thread && filter_which_thread == tid) || !filter_by_thread;

	if (continue_cond)
	{
		// handle the packet
		if(BGUTracer::add_package_to_string_buffer(rcv_pckg))
		{
			return bgu_ipckg_status::BGUI_PCKG_ADDED;
		}
	}
	else
	{
		return bgu_ipckg_status::BGUI_PCKG_FILTERED;
	}
	return bgu_ipckg_status::BGUI_PCKG_ERR;
}

bool BGUTracer::add_package_to_string_buffer(std::shared_ptr<BGUInfoPackage> rcv_pckg)
{
	uint16_t curr_status = rcv_pckg->get_packet_status();

	if (curr_status < total_stages) //update relevant stage
	{
		std::vector<std::string> data = rcv_pckg->get_data();
		//std::cout<<"tid: "<<rcv_pckg->get_ThreadID()<<" cur st"<<curr_status<<" stringD: "<<generate_comma_seperated_from_vec_of_string(data)<<std::endl;
		if(this->tid_buffer_strings[curr_status] == X_VAL)
		{
			this->tid_buffer_strings[curr_status] = generate_comma_seperated_from_vec_of_string(data);
		}
		else
		{
			this->tid_buffer_strings[curr_status] = add_to_comma_seperated_from_vec_of_string(this->tid_buffer_strings[curr_status],data);
		}
		return true;
	}

	return false;
}



std::string BGUTracer::add_to_comma_seperated_from_vec_of_string(std::string old_str,std::vector<std::string> &in_vec)
{
	std::vector <std::string> previous_components;
	break_string_to_its_components(old_str,previous_components);//update previous compnents vector with previous data

	std::stringstream res;
		res<<"\"";
		for(int i=0; i<in_vec.size(); i++)
		{
			if(i<previous_components.size())
			{
				res<<previous_components[i]<<" ";
			}
			res<<in_vec[i];
			if(i<in_vec.size()-1)
			{
				res<<",";
			}
		}
		res<<"\"";
		return res.str();
}

std::string BGUTracer::generate_comma_seperated_from_vec_of_string(std::vector<std::string> &in_vec)
{
	std::stringstream res;
	res<<"\"";
	for(int i=0; i<in_vec.size(); i++)
	{
		res<<in_vec[i];
		if(i<in_vec.size()-1)
		{
			res<<",";
		}
	}
	res<<"\"";
	return res.str();

}

void BGUTracer::break_string_to_its_components(std::string status_string, std::vector<std::string> &output)
{
	if(status_string.find(",") == std::string::npos)
	{
		return ;
	}
	output.clear();

	boost::erase_all(status_string,"\"");

	std::string delimiter = ",";
	size_t pos = 0;
	while((pos = status_string.find(delimiter)) != std::string::npos)
	{
		std::string token = status_string.substr(0,pos);
		output.push_back(token);
		status_string.erase(0,pos+delimiter.length());
	}
}



/**
 * This function writes the content of string buffers to csv table
 */
void BGUTracer::deploy_string_buffers_to_table(Tick tick_to_print)
{
	std::stringstream res_vec;

	res_vec<<tick_to_print/CYCLE_ROUND_VAL<<",";
	for (int i=0; i<this->tid_buffer_strings.size(); i++)
	{
		res_vec<<this->tid_buffer_strings[i];
		if (i < this->tid_buffer_strings.size()-1)
		{
			res_vec<<",";
		}
	}
	res_vec<<"\n"; //start a new line at the end of each line
//	if (tick_to_print>=415000 && tick_to_print<=420000)
//		{
//			std::cout<<"Deploying curr_tick: "<<curr_tick<<"\n";
//			std::cout<<res_vec.str()<<"line new line"<<std::endl;
//			int stop = 0;
//			stop++;
//		}

	csv_table_fstr<<res_vec.str();
}

/**
 * returns false if one of the element isn't empty
 */
bool BGUTracer::check_if_empty(std::vector<std::string> &in_vec)
{

	for (auto &elem : in_vec)
	{
		if(elem != X_VAL)
		{
			return false;
		}
	}

	return true;
}



void BGUTracer::reset_string_buffers()
{
	this->tid_buffer_strings.assign(total_stages,X_VAL);
}






// plan - using the first packet to initialize the BGU tracer
// information of all stages are stored within a single packet so it will be fine






} /* namespace tracer */
