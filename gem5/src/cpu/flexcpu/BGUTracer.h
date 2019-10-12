/*
 * BGUTracer.h
 *
 *  Created on: 01 Sep 2019
 *      Author: david leon
 */

#ifndef SRC_CPU_FLEXCPU_BGUTRACER_H_
#define SRC_CPU_FLEXCPU_BGUTRACER_H_

enum PIPE_STAGES{E_FETCH,E_ISSUE,E_EXECUTE,TOTAL_PIPELINE_STAGES,NULL_STAGE};

#include "inflight_inst.hh"
#include "cpu/simple_thread.hh"
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include <sstream>
#include <iostream>
#include <fstream>
#include <unistd.h>

#include <sstream>

inline std::string get_cwd()
{
	char cwd[256];
	if (getcwd(cwd,sizeof(cwd))== NULL)
	{
		perror("getcwd() error");
	}
	else
	{
		return std::string(cwd);
	}
	return std::string();
}

namespace tracer {

#define DEFAULT_CSV_FILE get_cwd()+"/output.csv"
#define X_VAL "x"

typedef enum {BGUI_PCKG_FILTERED,BGUI_PCKG_ERR,BGUI_PCKG_ADDED} bgu_ipckg_status;

class BGUInfoPackage;
/**
 * Singlton class
 * BGUTracer - generates the trace by receiving BGUInfoPackages
 */
class BGUTracer {
	using buffer_attr = std::pair<Tick,std::vector<std::string>>; //tick and what to print
private:
	BGUTracer(std::string CsvFileFullPath = DEFAULT_CSV_FILE ,bool FilterByThread = false ,ThreadID FilterWhichThread = 0);

	//------------- flags ---------------//

	//------------- private variables -------------//

	std::fstream csv_table_fstr;
	std::string full_file_path;
	bool initialized = false;
	bool filter_by_thread;
	ThreadID filter_which_thread;

	Tick last_tick;
	Tick curr_tick;

	//lines buffers:
	std::unordered_map<ThreadID,buffer_attr> tid_buffer_strings;

	uint16_t total_stages;


	std::string generate_comma_seperated_from_vec_of_string(std::vector<std::string> &in_vec);
	bool add_package_to_string_buffer(std::shared_ptr<BGUInfoPackage> rcv_pckg);
	void start_a_new_line(Tick newTick);

	void deploy_string_buffers_to_table();
	bool check_if_empty(std::vector<std::string> &in_vec);

	std::string generate_comma_seperated_lines(std::vector<std::vector<std::string>> lines_matrix);

	bool thread_string_buffer_exit_check(ThreadID tid);
	void add_and_resize_string_buffer(ThreadID tid,Tick curr_tick);
	void reset_string_buffers_all_threads(Tick new_tick);

public:
	BGUTracer(const BGUTracer&) = delete;
	BGUTracer& operator=(const BGUTracer &) = delete;
	BGUTracer(BGUTracer &&) = delete; //rvalue reference is forbidden
	BGUTracer & operator=(BGUTracer &&) = delete;

	//getters

	static BGUTracer& get_inst()
	{
		static BGUTracer tracer;
		return tracer;
	}

	bgu_ipckg_status receive_bgu_info_package(std::shared_ptr<BGUInfoPackage> rcv_pckg,ThreadID tid);

	//setters

};

/*********************************************************************************************************************/
/*********************************************************************************************************************/
/*********************************************************************************************************************/

class BGUInfoPackage : public std::enable_shared_from_this<BGUInfoPackage>
{
	enum PCKG_STATE{PCKG_UNREGISTERED,PCKG_REGISTERED,PCKG_SENT};

private:

	std::vector<std::string> status_strings = { //TODO we have to add fetch to status somehow
			        "Inv" //0,
			        "Empt", //1 Dynamic instruction slot has been created but not yet filled. This is fetch
			        "DE", //2 A StaticInst has been provided.
			        // Perhaps an independent rename stage may be useful. Most
			        // functionality conventionally called rename is packaged with issue
			        // right now.
			        "IS", //3 Dependencies and results have been identified and recorded.
			        "EX", //4 Request for execution sent, but waiting for results.
			        "EfAd", //5 Effective address calculated, but not yet sent to memory.
			        "Memo", //6 Request for memory sent, but waiting for results.
			        "Comp", //7 Results have been received, but not yet committed.
			        "Cmtd", //8 Values have been committed, but this object might be
			                   // alive for a little longer due to the shared_ptr being in
			                   // the call-stack.
					};

	std::unordered_map <std::string,std::string> pckg_attributes;

	uint pipe_stage = NULL_STAGE;
	bool pckg_state = PCKG_UNREGISTERED;
	ThreadID tid;
	std::vector<std::string> data;

	std::vector<std::string> Pstatus_strings = {"FE","DE","IS","EX"};

	enum e_stages
	{
		//Note: the order of attributes is important!
		STG_FE,
		STG_DE,
		STG_IS,
		STG_EX,
		STG_TOTAL,
		PST_NOT_INITIALIZED
	};

	typedef e_stages Pstatus;

	Pstatus packet_status;



	const std::string tid_str = "tid", pc_str = "pc", status_str = "status";

	std::weak_ptr<InflightInst> wk_ptr_inst;


	inline std::string dec_to_hex_str(unsigned dec)
	{
		std::stringstream stream;
		stream << std::hex << dec;
		return stream.str();
	}

	void generate_attributes();

	std::vector<std::vector<std::string>> attributes;


	std::vector<std::string> inflightinst_to_string(); // works on pckg_attributes - needs wk_ptr_inst
	std::vector<std::string> decode_to_string(std::shared_ptr<InflightInst> inst);//TODO  // works on pckg_attributes - needs wk_ptr_inst
	std::vector<std::string> issue_to_string(std::shared_ptr<InflightInst> inst);//TODO  // works on pckg_attributes - needs wk_ptr_inst
	std::vector<std::string> execute_to_string(std::shared_ptr<InflightInst> inst);//TODO // works on pckg_attributes - needs wk_ptr_inst
	std::vector<std::string> fetch_to_string(std::shared_ptr<InflightInst> inst);//TODO // works on pckg_attributes - needs wk_ptr_inst

public:
	BGUInfoPackage(ThreadID tid,std::weak_ptr<InflightInst> wk_ptr_inst);
	BGUInfoPackage(); // [used by Tracer]

	std::shared_ptr <BGUInfoPackage> get_a_BGUInfoPackage()
	{
		return shared_from_this();
	}

	void update_package_attributes(); // works on pckg_attributes - needs wk_ptr_inst
	void send_packet_to_tracer();//TODO

	inline Pstatus get_packet_status() { return this->packet_status; }
	inline std::vector<std::string> get_data() { return this->data; }
	inline ThreadID get_ThreadID() { return this->tid; }

	std::string get_1st_headline_stages_comma_seperated(); // [used by Tracer]
	std::string get_2nd_headline_attributes_comma_seperated(); // [used_by Tracer]

	inline uint16_t get_total_stages_count() { return STG_TOTAL;}


};

} /* namespace tracer */

#endif /* SRC_CPU_FLEXCPU_BGUTRACER_H_ */
