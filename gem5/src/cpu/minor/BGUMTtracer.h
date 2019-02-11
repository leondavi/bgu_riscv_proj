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

#define STRING_VAR(name) var_to_string((char*) #name)

inline std::string var_to_string(char* name)
{
	std::stringstream ss;
	ss<<name;
    return ss.str();
}

namespace bgu
{

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
	virtual std::vector<var_attr_t> get_vars() = 0;
};


//--------------------------------//
class BGUMtTracer {
private:
	std::vector<std::string> pipe_trace_line;

	std::fstream logfile;
	std::fstream tablefile;

	bool first_time_print_header;
	bool generate_table;

	void clear_line();

public:

	BGUMtTracer(bool generate_table = false);
	~BGUMtTracer();

	void update_stage(BguInfo* bgu_info);
	void update_pipe_tick();


};
//--------------------------------//

}//end of namespace

#endif /* SRC_CPU_MINOR_BGUMTTRACER_H_ */
