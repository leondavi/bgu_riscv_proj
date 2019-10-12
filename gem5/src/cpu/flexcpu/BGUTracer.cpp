/*
 * BGUTracer.cpp
 *
 *  Created on: 01 Sep 2019
 *      Author: david
 */

#include "BGUTracer.h"

namespace tracer {

BGUTracer::BGUTracer(std::string CsvFileFullPath,bool FilterByThread,ThreadID FilterWhichThread) :
		full_file_path(CsvFileFullPath),
		filter_by_thread(FilterByThread),
        filter_which_thread(FilterWhichThread),
		last_tick(curTick()),
		curr_tick(curTick())
{
	// TODO Auto-generated constructor stub
	csv_table_fstr.open(full_file_path.c_str(),std::ofstream::out | std::ofstream::app);

	BGUInfoPackage dummy_package; //using this to initialize the table

	total_stages = dummy_package.get_total_stages_count();
	//generates headlines
	csv_table_fstr << "Sim Time,"<< dummy_package.get_1st_headline_stages_comma_seperated() << std::endl;
	csv_table_fstr << " ,"<< dummy_package.get_2nd_headline_attributes_comma_seperated() << std::endl;

}



/**
 * This function update the tracer with bguinfopacket
 * This function is called by the packet itself when calling packet's send function
 */
bgu_ipckg_status BGUTracer::receive_bgu_info_package(std::weak_ptr<BGUInfoPackage> rcv_pckg,ThreadID tid)
{
	std::shared_ptr<BGUInfoPackage> pckg_inst = rcv_pckg.lock();

	curTick = curTick();
	if (curr_tick - last_tick > 0)
	{
		last_tick = curr_tick;
	}

	bool continue_cond = (filter_by_thread && filter_which_thread == tid) || !filter_by_thread;

	if (continue_cond)
	{
		// handle the packet
		if(BGUTracer::add_package_to_string_buffer(pckg_inst))
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
	ThreadID curr_tid = rcv_pckg->get_ThreadID();



	return true;//TODO
}



void BGUTracer::deploy_string_buffers_to_table()
{
	std::vector<std::vector<std::string>> lines_matrix;
	std::unordered_map<ThreadID,buffer_attr>::iterator it_over_buffers = this->tid_buffer_strings.begin();
	while (it_over_buffers != this->tid_buffer_strings.end())
	{
		if(!check_if_empty(it_over_buffers->second.second)) //if not empty
		{
			lines_matrix.push_back(it_over_buffers->second.second);
		}
		it_over_buffers++;
	}

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
}

std::string BGUTracer::generate_comma_seperated_lines(std::vector<std::vector<std::string>> lines_matrix)
{
	std::vector<std::stringstream> res_vec;
	for (int l=0; l<lines_matrix.size(); l++) //scanning line
	{
		res_vec<<curr_tick<<",";
		for (int cell=0 ; cell < lines_matrix[l].size(); cell++) //scanning cells
		{
			res_vec<<lines_matrix[l][cell]<<",";
		}
	}
}

std::vector<std::vector<std::string>> BGUTracer::merge_rows(std::vector<std::vector<std::string>> &lines_matrix)
{
	//TODO
//	for (int col = 0; col < lines_matrix.front().size(); col++)
//	{
//		for (int row = lines_matrix.size()-1; row >= 0 ; row--) //iterate from last to first row
//		{
//
//		}
//	}

}
/**
 * return true if there is such trhead
 */
bool BGUTracer::thread_string_buffer_exit_check(ThreadID tid)
{
	std::unordered_map<ThreadID,buffer_attr>::iterator it_over_buffers = this->tid_buffer_strings.find(tid);
	if (it_over_buffers != this->tid_buffer_strings.end())
	{
		return true;
	}
	return false;
}

void BGUTracer::add_and_resize_string_buffer(ThreadID tid,Tick curr_tick)
{
	buffer_attr new_attr;
	new_attr.first = curr_tick;
	new_attr.second.assign(total_stages,X_VAL);
	this->tid_buffer_strings[tid] = new_attr;
}

void BGUTracer::reset_string_buffers_all_threads(Tick new_tick)
{
	std::unordered_map<ThreadID,buffer_attr>::iterator it_over_buffers = this->tid_buffer_strings.begin();
	while (it_over_buffers != this->tid_buffer_strings.end())
	{
		it_over_buffers->second.first = new_tick;
		for (int stage = 0 ; stage < it_over_buffers->second.second; stage++)
		{
			stage < it_over_buffers->second.second[stage] = X_VAL;
		}
		it_over_buffers++;
	}
}






// plan - using the first packet to initialize the BGU tracer
// information of all stages are stored within a single packet so it will be fine






} /* namespace tracer */
