/*
 * MinorMTLogger.h
 *
 *  Created on: 05 Feb 2019
 *      Author: david
 */

#ifndef SRC_CPU_MINOR_BGUMTTRACER_H_
#define SRC_CPU_MINOR_BGUMTTRACER_H_

#include "fort.hpp"
#include "cpu/minor/cpu.hh"
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <typeinfo>



#define TOTAL_NUM_OF_PIPE_STAGES 4
#define TRACE_WORKSPACE_DIR "/home/david/workspace/bgu_riscv_proj"
#define FILE_NAME "output"
#define X_ATTRIBUTE " x "


#define STRING_VAR(name) var_to_string((char*) #name)

inline std::string var_to_string(char* name)
{
	std::stringstream ss;
	ss<<name;
    return ss.str();
}

namespace bgu
{

#define PIPE_STAGES_VEC {"Fetch1","Fetch2","Decode","Execute"}
typedef enum pipe_stage_t {FE1,FE2,DE,EX} pipe_stage;

//--------------------------------//
typedef std::pair <std::string,std::string> var_attr_t;//first is varname second is str(var)

class BguInfo
{
protected:
	pipe_stage stage;
public:
	BguInfo(pipe_stage stage) {this->stage = stage;}
	virtual ~BguInfo() {}
	/**
	 * returns strings of headers and params of BguInfo class
	 */

	inline pipe_stage get_stage() {return this->stage;}
	virtual std::vector<var_attr_t> get_vars() = 0; //pure virtual - derived classes have to implement
};


enum file_t {E_TABLE,E_CSV,E_LOGFILE};

//--------------------------------//
/**
 * Singleton class
 */
class BGUMtTracer {
private:
	std::vector<std::string> pipe_trace_line;

	std::vector <std::string> pipe_stages_str_vec;

	std::fstream logfile;
	std::fstream csvfile;
	std::fstream tablefile;
	std::string diff_str;
	std::stringstream path_log,path_table,path_csv;

	const int summary_table_headers_row = 0;
	const int simtime_col = 0;
	fort::table summary_table;

	bool first_time_print_header;
	int file_to_generate;

	void clear_line();
	void generate_table_headers();
	void generate_csv_headers();

	static BGUMtTracer *inst_;

	BGUMtTracer(int file_to_generate = E_CSV);


public:

	BGUMtTracer* get_instance(int file_to_generate = E_CSV)
	{
		if(this->inst_ == NULL)
		{
			this->inst_ = new BGUMtTracer(file_to_generate);
		}
		return(inst_);
	}
	~BGUMtTracer();


	void update_stage(BguInfo* bgu_info);
	void update_row_in_table(std::string sim_time,std::vector <std::string> &pipe_trace_line);
	void update_row_in_csv_file(std::string sim_time,std::vector <std::string> &pipe_trace_line);
	void end_pipe_tick();
	void save_table_to_file();


};
//--------------------------------//


}//end of namespace

#endif /* SRC_CPU_MINOR_BGUMTTRACER_H_ */
