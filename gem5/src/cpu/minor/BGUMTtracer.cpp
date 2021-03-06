/*
 * MinorMTLogger.cpp
 *
 *  Created on: 05 Feb 2019
 *      Author: david
 */

#include "BGUMTtracer.h"

namespace bgu
{
BGUMtTracer *BGUMtTracer::inst_ = 0; //Singlton pointer definition

BGUMtTracer::BGUMtTracer(int file_to_generate)
{
	// TODO Auto-generated constructor stub
	this->diff_str = "";
	this->first_time_print_header = false;
	this->pipe_trace_line.resize(TOTAL_NUM_OF_PIPE_STAGES);
	this->pipe_info_trace.resize(TOTAL_NUM_OF_PIPE_STAGES);
	this->pipe_stages_str_vec = PIPE_STAGES_VEC;
	this->file_to_generate = file_to_generate;
	this->after_sim_initialized = false;
	this->print_csv_tuple_explanation_row = false;
	for(auto &p : pipe_info_trace)//initialize values to NULL
	{
		p = NULL;
	}
}

void BGUMtTracer::init_after_simulation_start()
{
	if(!after_sim_initialized)
	{
		this->output_dir = simout.directory();

		path_log<<this->output_dir<<"/"<<FILE_NAME<<"_log.txt";
		path_table<<this->output_dir<<"/"<<FILE_NAME<<"_table.txt";
		path_csv<<this->output_dir<<"/"<<FILE_NAME<<"_table.csv";
		//remove if exists
		std::remove(path_log.str().c_str());
		std::remove(path_table.str().c_str());
		std::remove(path_csv.str().c_str());

		//generating headers
		switch (file_to_generate)
		{
			case E_CSV: {csvfile.open(path_csv.str(),std::ofstream::out | std::ofstream::app);
						 generate_csv_headers(); break;}
			case E_TABLE: {tablefile.open(path_table.str(),std::ofstream::out | std::ofstream::app);
							generate_table_headers(); break;}
			case E_LOGFILE: {logfile.open(path_log.str(),std::ofstream::out | std::ofstream::app); break;}
		}

		clear_line();
	}
	this->after_sim_initialized = true;
}

BGUMtTracer::~BGUMtTracer()
{
	//std::cout<<"Destructor called ******";
	//tablefile<<summary_table.to_string()<<std::endl;

//	logfile.close();
//	tablefile.close();
	// TODO Auto-generated destructor stub
}

void BGUMtTracer::save_table_to_file()
{
	//tablefile<<summary_table.to_string()<<std::endl;
	//std::cout<<summary_table.to_string()<<std::endl;
	switch (file_to_generate)
	{
		case E_CSV: {csvfile.close(); break;}
		case E_TABLE: {tablefile<<summary_table.to_string()<<std::endl; tablefile.close(); break;}
		case E_LOGFILE: {logfile.close(); break;}
	}
}

void BGUMtTracer::clear_line()
{
	for(auto &element: this->pipe_trace_line)
	{
		element = X_ATTRIBUTE;
	}
}

void BGUMtTracer::update_stage_csv(BguInfo *bgu_info)
{
	this->pipe_info_trace[bgu_info->get_bgu_info_stage()] = bgu_info;
}

void BGUMtTracer::update_stage(BguInfo *bgu_info)
{
	update_stage_csv(bgu_info);
	this->pipe_info_trace[bgu_info->get_bgu_info_stage()];

	init_after_simulation_start();//first initialization
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

void BGUMtTracer::update_row_in_csv_file(std::string sim_time)
{
	if(!this->print_csv_tuple_explanation_row)
	{
		this->csvfile<<" ";//for simtime print empty in this case
		for (int s = 0 ; s < this->pipe_info_trace.size(); s++)
		{
			this->csvfile<<",";//New row
			this->csvfile<<"\"";//in order to print the comma in tuple
			std::vector<var_attr_t> vars = this->pipe_info_trace[s]->get_vars();
			for(int i=0 ; i<vars.size(); i++)
			{
				if(vars[i].first.find("vld") == std::string::npos)
				{
					this->csvfile<<vars[i].first;
					if(i<vars.size()-1)
					{
						//print comma separator in csv
						this->csvfile<<",";
					}
				}
			}
			this->csvfile<<"\"";//in order to print the comma in tuple

		}
		this->print_csv_tuple_explanation_row = true;
		this->csvfile<<std::endl;
	}

	this->csvfile<<sim_time;
	for (int s=0; s<this->pipe_info_trace.size(); s++)
	{
		this->csvfile<<",";//New row
		this->csvfile<<"\"";//in order to print the comma in tuple
		if(this->pipe_info_trace[s] == NULL || this->pipe_info_trace[s]->is_valid())
		{
			std::vector<var_attr_t> vars = this->pipe_info_trace[s]->get_vars();
			for(int i=0 ; i<vars.size(); i++)
			{
				if(vars[i].first.find("vld") == std::string::npos)
				{
					this->csvfile<<vars[i].second;
					if(i<vars.size()-1)
					{
						//print comma separator in csv
						this->csvfile<<",";
					}
				}
			}
		}
		else
		{
			this->csvfile<<"x";
		}
		this->csvfile<<"\"";
	}
	this->csvfile<<std::endl;
}

void BGUMtTracer::generate_csv_headers()
{
	this->csvfile<<"simtime"<<",";
	for(int vec_idx=0; vec_idx < this->pipe_stages_str_vec.size(); vec_idx++)
	{
		this->csvfile<<pipe_stages_str_vec[vec_idx];
		if(vec_idx < this->pipe_stages_str_vec.size()-1)
		{
			this->csvfile<<",";//adding the delimeter
		}
	}
	this->csvfile<<std::endl;
}

void BGUMtTracer::generate_table_headers()
{
	summary_table << fort::header;
	summary_table[summary_table_headers_row][simtime_col] = "simtime";

	for(int col = simtime_col+1,vec_idx=0; vec_idx < this->pipe_stages_str_vec.size(); vec_idx++,col++)
	{
		summary_table[summary_table_headers_row][col] = pipe_stages_str_vec[vec_idx];
	}
	summary_table<<fort::endr;
}

void BGUMtTracer::update_row_in_table(std::string sim_time,std::vector <std::string> &pipe_trace_line)
{
	summary_table<<sim_time;
	for(auto &cur_string : pipe_trace_line)
	{
		summary_table<<cur_string;
	}
	summary_table<<fort::endr;
}

void BGUMtTracer::end_pipe_tick()
{
	std::stringstream pipe_tick_line;
	std::string tick_str = std::to_string(curTick()/1000);
	pipe_tick_line<<tick_str<<" ";
	for(int l=0; l<pipe_trace_line.size(); l++)
	{
		pipe_tick_line<<pipe_trace_line[l];
	}
	pipe_tick_line<<std::endl;

	if(output_flag)
	{
	   std::cout<<pipe_tick_line.str();
	}

	switch (file_to_generate)
	{
		case E_CSV: {update_row_in_csv_file(tick_str); break;}
		case E_TABLE: {update_row_in_table(tick_str, pipe_trace_line); break;}
		case E_LOGFILE: {std::string line = pipe_tick_line.str(); logfile<<line; break;}
	}
	clear_line();
}

}
