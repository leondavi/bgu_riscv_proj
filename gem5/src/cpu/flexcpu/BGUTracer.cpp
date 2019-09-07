/*
 * BGUTracer.cpp
 *
 *  Created on: 01 Sep 2019
 *      Author: david
 */

#include "BGUTracer.h"

namespace tracer {

BGUTracer::BGUTracer(std::string CsvFilePath,bool FilterByThread,ThreadID FilterWhichThread) :
		filepath(CsvFilePath),
		filter_by_thread(FilterByThread),
        filter_which_thread(FilterWhichThread)
{
	// TODO Auto-generated constructor stub
	//csvfile.open(path_csv.str(),std::ofstream::out | std::ofstream::app);

}

/**
 * This function update the tracer with bguinfopacket
 * This function is called by the packet itself when calling packet's send function
 */
bgu_ipckg_status BGUTracer::get_bgu_info_package(std::weak_ptr<BGUInfoPackage> rcv_pckg,ThreadID tid)
{
	bgu_ipckg_status return_status;

	std::shared_ptr<BGUInfoPackage> pckg_inst = rcv_pckg.lock();

	bool continue_cond = (filter_by_thread && filter_which_thread == tid) || !filter_by_thread;

	if (continue_cond)
	{
		// handle the packet
		if(BGUTracer::add_package_to_current_tick_line(pckg_inst,tid))
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

bool BGUTracer::add_package_to_current_tick_line(std::shared_ptr<BGUInfoPackage> rcv_pckg,ThreadID tid)
{
	return true;//TODO
}




// plan - using the first packet to initialize the BGU tracer
// information of all stages are stored within a single packet so it will be fine






} /* namespace tracer */
