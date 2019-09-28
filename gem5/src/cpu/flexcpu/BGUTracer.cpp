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

	//generate headlines
	csv_table_fstr << dummy_package.get_1st_headline_stages_comma_seperated();
	csv_table_fstr << dummy_package.get_2nd_headline_attributes_comma_seperated();
}



/**
 * This function update the tracer with bguinfopacket
 * This function is called by the packet itself when calling packet's send function
 */
bgu_ipckg_status BGUTracer::receive_bgu_info_package(std::weak_ptr<BGUInfoPackage> rcv_pckg,ThreadID tid)
{
	std::shared_ptr<BGUInfoPackage> pckg_inst = rcv_pckg.lock();

	bool continue_cond = (filter_by_thread && filter_which_thread == tid) || !filter_by_thread;

	if (continue_cond)
	{
		// handle the packet
		if(BGUTracer::add_package_to_current_tick_line(pckg_inst))
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

bool BGUTracer::add_package_to_current_tick_line(std::shared_ptr<BGUInfoPackage> rcv_pckg)
{

	return true;//TODO
}






// plan - using the first packet to initialize the BGU tracer
// information of all stages are stored within a single packet so it will be fine






} /* namespace tracer */
