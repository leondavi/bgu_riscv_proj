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
#include "base/output.hh"
#include "cpu/base.hh"
//#include "cpu/minor/cpu.hh"
#include "cpu/minor/dyn_inst.hh"

extern OutputDirectory simout;

#define TRACE_WORKSPACE_DIR "/home/david/workspace/bgu_riscv_proj"
#define FILE_NAME "output"
#define X_ATTRIBUTE " x "

static const std::vector<std::string> branch_insts_vec {"beq"/*branch if equal*/,
														"bne"/*branch if not equal*/,
														"blt"/*branch if less than*/,
														"ble"/*branch if less than or equal*/,
														"bltu"/*branch if less than (unsigned)*/,
														"bleu"/*branch if less than or equal (unsigned)*/,
														"bgt"/*branch if greater than*/,
														"bge"/*branch if greater than or equal*/,
														"bgtu"/*branch if greater than (unsigned)*/,
														"bgeu"/*branch if greater than or equal (unsigned)*/,
														"jal"/*jump and link (with imm for short distance jump)"*/,
														"jalr"/*jump and link register*/,
														"jr"/*jumpr register */};


#define STRING_VAR(name) var_to_string((char*) #name)

inline std::string var_to_string(char* name)
{
	std::stringstream ss;
	ss<<name;
    return ss.str();
}

inline std::string dec_to_hex_str(unsigned dec)
{
	std::stringstream stream;
	stream << std::hex << dec;
	return stream.str();
}

namespace bgu
{

#define PIPE_STAGES_VEC {"Fetch1_rqt","Fetch1_rsp","Fetch2","Decode","Execute_Issued","Execute_Commited"}
typedef enum pipe_stage_t {FE1_RQT,FE1_RSP,FE2,DE,EX_IS,EX_COM,TOTAL_NUM_OF_PIPE_STAGES} pipe_stage;

//--------------------------------//
typedef std::pair <std::string,std::string> var_attr_t;//first is varname second is str(var)

class BguInfo
{
protected:
	bool vld;//valid flag - if operation has been done
	Minor::InstId id;
//	ThreadID Tid;
	TheISA::PCState Pc;
	pipe_stage stage;
	std::vector<bgu::var_attr_t> vars_pairs;
public:
	BguInfo(pipe_stage stage) {this->stage = stage; vld = false;}
	virtual ~BguInfo() {}
	/**
	 * returns strings of headers and params of BguInfo class
	 */

	inline pipe_stage get_stage() {return this->stage;}

	virtual std::vector<var_attr_t> get_vars() = 0; //pure virtual - derived classes have to implement

	inline std::vector<bgu::var_attr_t> clear_and_add_default_vars()
	{
		bgu::var_attr_t tmp_attr;
		vars_pairs.clear();
		//valid create attributes
		tmp_attr.first = STRING_VAR(vld);
		tmp_attr.second = std::to_string(vld);
		vars_pairs.push_back(tmp_attr);
		//Tid create attributes
		tmp_attr.first = STRING_VAR(Tid);
		tmp_attr.second = std::to_string(id.threadId);
		vars_pairs.push_back(tmp_attr);
		//Pc create attributes
		tmp_attr.first = STRING_VAR(Pc);
		tmp_attr.second = "0x"+dec_to_hex_str(Pc.instAddr());
		vars_pairs.push_back(tmp_attr);


//		tmp_attr.first = STRING_VAR(S);
//		tmp_attr.second = std::to_string(id.streamSeqNum);
//		vars_pairs.push_back(tmp_attr);
//
//
//		tmp_attr.first = STRING_VAR(P);
//		tmp_attr.second = std::to_string(id.predictionSeqNum);
//		vars_pairs.push_back(tmp_attr);
//
//		tmp_attr.first = STRING_VAR(L);
//		tmp_attr.second = std::to_string(id.lineSeqNum);
//		vars_pairs.push_back(tmp_attr);
//
//
//		tmp_attr.first = STRING_VAR(F);
//		tmp_attr.second = std::to_string(id.fetchSeqNum);
//		vars_pairs.push_back(tmp_attr);
//
//		tmp_attr.first = STRING_VAR(E);
//		tmp_attr.second = std::to_string(id.execSeqNum);
//		vars_pairs.push_back(tmp_attr);

		return vars_pairs;
	}


	//setters
	inline bool set_valid_value(bool value=false) { this->vld = value; return this->vld;}
	inline void set_pc(TheISA::PCState new_pc) {this->Pc = new_pc;}
//	inline void set_tid(ThreadID tid_val) { this->Tid = tid_val;}
	inline void set_id(Minor::InstId new_id) {this->id = new_id;}

	//getters
	inline bool is_valid() { return this->vld;}
//	inline ThreadID get_tid() { return this->Tid;}
	inline TheISA::PCState get_pc() {return this->Pc;}
	inline pipe_stage get_bgu_info_stage(){ return this->stage;}
	inline Minor::InstId get_id() {return this->id;}

};


enum file_t {E_TABLE,E_CSV,E_LOGFILE};

//--------------------------------//
/**
 * Singleton class
 */
class BGUMtTracer {
private:
	std::vector<std::string> pipe_trace_line;
	std::vector<BguInfo*> pipe_info_trace;

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
	bool after_sim_initialized;
	bool print_csv_tuple_explanation_row;
	int file_to_generate;

	std::string output_dir;

	bool output_flag = false;

	void clear_line();
	void generate_table_headers();
	void generate_csv_headers();
	void init_after_simulation_start();

	void update_stage_csv(BguInfo *bgu_info);

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
	void update_row_in_csv_file(std::string sim_time);
	void end_pipe_tick();
	void save_table_to_file();
	inline void set_output(bool state = true) { this->output_flag = state; }


};
//--------------------------------//


}//end of namespace

#endif /* SRC_CPU_MINOR_BGUMTTRACER_H_ */
