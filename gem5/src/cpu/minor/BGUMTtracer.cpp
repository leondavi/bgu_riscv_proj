/*
 * MinorMTLogger.cpp
 *
 *  Created on: 05 Feb 2019
 *      Author: david
 */

#include "BGUMTtracer.h"

namespace bgu
{

BGUMtTracer::BGUMtTracer(bool generate_table)
{
	// TODO Auto-generated constructor stub
	this->first_time_print_header = false;
	this->generate_table = generate_table;
	pipe_trace_line.resize(TOTAL_NUM_OF_PIPE_STAGES);

	std::stringstream path_log,path_table;
	path_log<<TRACE_WORKSPACE_DIR<<"/"<<FILE_NAME<<"_log.txt";
	path_table<<TRACE_WORKSPACE_DIR<<"/"<<FILE_NAME<<"_table.txt";
	//remove if exists
	std::remove(path_log.str().c_str());
	std::remove(path_table.str().c_str());

	logfile.open(path_log.str());
	tablefile.open(path_table.str());

	clear_line();
}

BGUMtTracer::~BGUMtTracer()
{
	logfile.close();
	tablefile.close();
	// TODO Auto-generated destructor stub
}

void BGUMtTracer::clear_line()
{
	for(auto &element: this->pipe_trace_line)
	{
		element = X_ATTRIBUTE;
	}
}

void BGUMtTracer::update_stage(BguInfo *bgu_info)
{
	this->pipe_trace_line[bgu_info->get_stage()] = "";
	std::vector<var_attr_t> vars = bgu_info->get_vars();
	for(var_attr_t &var : vars)
	{
		this->pipe_trace_line[bgu_info->get_stage()] = this->pipe_trace_line[bgu_info->get_stage()] + var.first+"="+var.second+" ";
	}
	if(this->pipe_trace_line[bgu_info->get_stage()].empty())
	{
		this->pipe_trace_line[bgu_info->get_stage()] = X_ATTRIBUTE;
	}
}

void BGUMtTracer::end_pipe_tick()
{
	std::stringstream pipe_tick_line;
	std::string tick_str = std::to_string(curTick());
	pipe_tick_line<<tick_str<<" ";
	for(int l=0; l<pipe_trace_line.size(); l++)
	{
		pipe_tick_line<<pipe_trace_line[l];
	}
	pipe_tick_line<<std::endl;

	std::string line = pipe_tick_line.str();
	logfile<<line;
	std::cout<<line;

	clear_line();
}

}
