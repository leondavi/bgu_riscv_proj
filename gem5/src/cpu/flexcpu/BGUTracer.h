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
#include <vector>

namespace tracer {

class BGUInfoPackage;
/**
 * Singlton class
 * BGUTracer - generates the trace by receiving BGUInfoPackages
 */
class BGUTracer {
private:
	std::vector<std::shared_ptr<BGUInfoPackage>> packages;
	BGUTracer();
public:
	BGUTracer(const BGUTracer&) = delete;
	BGUTracer& operator=(const BGUTracer &) = delete;
	BGUTracer(BGUTracer &&) = delete; //rvalue reference is forbidden
	BGUTracer & operator=(BGUTracer &&) = delete;

	int32_t register_package_event(std::shared_ptr<BGUInfoPackage> info_pckg)
	{
		packages.push_back(info_pckg);
	}


	static auto& get_inst()
	{
		static BGUTracer tracer;
		return tracer;
	}
};

class BGUInfoPackage : public std::enable_shared_from_this<BGUInfoPackage>
{
private:
	uint state = NULL_STATE;
	bool was_send = false;
	ThreadID tid;
	int32_t package_id_in_tracer = -1;

public:
	BGUInfoPackage(ThreadID tid);

	std::shared_ptr <BGUInfoPackage> get_a_BGUInfoPackage()
	{
		return shared_from_this();
	}


	bool was_send() { return this->was_send; }

};

} /* namespace tracer */

#endif /* SRC_CPU_FLEXCPU_BGUTRACER_H_ */
