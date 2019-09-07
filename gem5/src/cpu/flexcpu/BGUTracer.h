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

namespace tracer {

#define DEFAULT_CSV_FILE "output.csv"

class BGUInfoPackage;
/**
 * Singlton class
 * BGUTracer - generates the trace by receiving BGUInfoPackages
 */
class BGUTracer {
private:

	~BGUTracer() {}
	BGUTracer();

	//------------- flags ---------------//
	bool filter_by_thread;
	ThreadID filter_which_thread;


public:
	BGUTracer(const BGUTracer&) = delete;
	BGUTracer& operator=(const BGUTracer &) = delete;
	BGUTracer(BGUTracer &&) = delete; //rvalue reference is forbidden
	BGUTracer & operator=(BGUTracer &&) = delete;

	static BGUTracer& get_inst()
	{
		static BGUTracer tracer;
		return tracer;
	}

	void get_package(std::weak_ptr<BGUInfoPackage> rcv_pckg);
};

class BGUInfoPackage : public std::enable_shared_from_this<BGUInfoPackage>
{
	enum PCKG_STATE{PCKG_UNREGISTERED,PCKG_REGISTERED,PCKG_SENT};

private:

	std::vector<std::string> status_strings = { //TODO we have to add fetch to status somehow
			        "Inv" //0,
			        "Empt", //1 Dynamic instruction slot has been created but not yet filled.
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

	std::weak_ptr<InflightInst> wk_ptr_inst;


	inline std::string dec_to_hex_str(unsigned dec)
	{
		std::stringstream stream;
		stream << std::hex << dec;
		return stream.str();
	}

	std::vector<std::string> default_attributes = {"pc","status"};
	std::vector<std::string> decode_attributes = {};//TODO
	std::vector<std::string> issue_attribtues = {};//TODO
	std::vector<std::string> execute_attributes = {};//TODO
	std::vector<std::string> fetch_attributes = {};//TODO


	std::vector<std::string> inflightinst_to_string(); // works on pckg_attributes - needs wk_ptr_inst
	std::vector<std::string> decode_to_string(std::shared_ptr<InflightInst> inst);//TODO  // works on pckg_attributes - needs wk_ptr_inst
	std::vector<std::string> issue_to_string(std::shared_ptr<InflightInst> inst);//TODO  // works on pckg_attributes - needs wk_ptr_inst
	std::vector<std::string> execute_to_string(std::shared_ptr<InflightInst> inst);//TODO // works on pckg_attributes - needs wk_ptr_inst
	std::vector<std::string> fetch_to_string(std::shared_ptr<InflightInst> inst);//TODO // works on pckg_attributes - needs wk_ptr_inst




public:
	BGUInfoPackage(ThreadID tid,std::weak_ptr<InflightInst> wk_ptr_inst);

	std::shared_ptr <BGUInfoPackage> get_a_BGUInfoPackage()
	{
		return shared_from_this();
	}

	void update_package_attributes(); // works on pckg_attributes - needs wk_ptr_inst
	void send_packet_to_tracer();//TODO

};

} /* namespace tracer */

#endif /* SRC_CPU_FLEXCPU_BGUTRACER_H_ */
