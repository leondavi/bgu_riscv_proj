/*
 * Copyright (c) 2012-2013,2015 ARM Limited
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2002-2005 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Steve Reinhardt
 */

#ifndef __CPU_RIO_CPU_HH__
#define __CPU_RIO_CPU_HH__

#include "cpu/rio/pipeline.hh"
#include "cpu/activity.hh"
#include "cpu/simple_thread.hh"
#include "cpu/base.hh"
#include "params/RioCPU.hh"
#include "cpu/rio/rioport.hh"


namespace Rio
{
///** Forward declared to break the cyclic inclusion dependencies between
// *  pipeline and cpu */
class Pipeline;
//
///** Rio will use the SimpleThread state for now */
typedef SimpleThread RioThread;
};


class RioCPU: public BaseCPU {
protected:
	Rio::Pipeline *pipeline;

public:
	RioCPU(RioCPUParams *params);

	~RioCPU();

	/** Activity recording for pipeline.  This belongs to Pipeline but
	 *  stages will access it through the CPU as the MinorCPU object
	 *  actually mediates idling behaviour */
	ActivityRecorder *activityRecorder; //TODO- We need to call advance function

	/** These are thread state-representing objects for this CPU.  If
	 *  you need a ThreadContext for *any* reason, use
	 *  threads[threadId]->getTC() */
	std::vector<Rio::RioThread *> threads;

	/** Thread Scheduling Policy (RoundRobin, Random, etc) */
	// Enums::ThreadPolicy threadPolicy; - TODO - we have separate in each relevant place


protected:
	/** Return a reference to the data port. */
	MasterPort &getDataPort() override;

	/** Return a reference to the instruction port. */
	MasterPort &getInstPort() override;

public:
//	void init() override;	// Used at the constructor
//  void startup() override;
    void wakeup(ThreadID tid) override;

    /** Simple inst count interface from BaseCPU */
    Counter totalInsts() const override;
    Counter totalOps() const override;


    ///////////////////////////////////////////////////////////////////
    // TODO remove below

	Rio::RioCachePort *Icache;
	Rio::RioCachePort *Dcache;


};

#endif // __CPU_RIO_CPU_HH__
