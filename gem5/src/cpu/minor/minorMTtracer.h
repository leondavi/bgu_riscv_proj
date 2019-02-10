/*
 * MinorMTLogger.h
 *
 *  Created on: 05 Feb 2019
 *      Author: david
 */

#ifndef SRC_CPU_MINOR_MINORMTTRACER_H_
#define SRC_CPU_MINOR_MINORMTTRACER_H_

#include "fort.hpp"
#include "cpu/minor/cpu.hh"


#define TRACE_WORKSPACE_DIR "/home/david/workspace/bgu_riscv_proj"

namespace tracer
{

typedef enum pipe_stages {FE1,FE2,DE,EX} pipe_stages;

class MinorMtTracer {
public:

	Tick A = curTick();

	MinorMtTracer();
	~MinorMtTracer();


};
}

#endif /* SRC_CPU_MINOR_MINORMTTRACER_H_ */
