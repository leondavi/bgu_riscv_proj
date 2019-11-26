/*
 * ResourceThreadsManaged.cc
 *
 *  Created on: 05 Nov 2019
 *      Author: david
 */


#include "cpu/flexcpu/flexcpu.hh"

#include "debug/FlexCPUBranchPred.hh"
#include "debug/FlexCPUBufferDump.hh"
#include "debug/FlexCPUDeps.hh"
#include "debug/FlexCPUInstEvent.hh"
#include "debug/FlexCPUThreadEvent.hh"
#include "debug/FlexCPUCoreEvent.hh"

using namespace std;
using namespace TheISA;


void FlexCPU::ResourceThreadsManaged::addRequest(ThreadID tid,
		std::shared_ptr<InflightInst> inst,
		const std::function<bool()>& run_function)
{
	thread_attr new_attr(inst,run_function);
    map_requests[tid].push_back(new_attr);
    DPRINTF(FlexCPUCoreEvent, "Adding request on thread %d queue size: %d\n",
                                 tid,map_requests[tid].size());
}

void FlexCPU::ResourceThreadsManaged::attemptAllRequests()
{
	//	Cycles tmpLatency; // [YE] - moved back
	    DPRINTF(FlexCPUCoreEvent, "Attempting all requests. %d on queue\n",
	            map_requests[0].size());

	    if (curTick() != lastActiveTick) {
	        assert(!bandwidth || usedBandwidth <= bandwidth);

	        // Stats
	        activeCycles++;
	        bandwidthPerCycle.sample(usedBandwidth);

	        // Reset the bandwidth since we're on a new cycle.
	        DPRINTF(FlexCPUCoreEvent,
	                "resetting the bandwidth. Used %d last cycle\n",
	                usedBandwidth);
	        usedBandwidth = 0;
	        lastActiveTick = curTick();

	        cpu->markActiveCycle();
	    }

	    clean_squashed();

	    bool q_empty = there_is_no_any_request();

	    if (q_empty)
	    {
	        return; // This happens with 0 latency events if a request enqueues
	                // another request.
	    }

	    ThreadID chosen_tid;

	    chosen_tid = qid_select();

	    while ((chosen_tid != -1) && !map_requests[chosen_tid].empty() && resourceAvailable())
	    {
	    	DPRINTF(FlexCPUCoreEvent, "Running request. %d left in queue. "
	    			"%d this cycle\n", map_requests[chosen_tid].size(), usedBandwidth);
	    	//	        auto& req = requests.front();
	    	thread_attr &req = map_requests[chosen_tid].front();
	    	DPRINTF(FlexCPUCoreEvent, "Executing request directly tid %d\n",chosen_tid);
	    	if (req.func()) usedBandwidth++;

	    	map_requests[chosen_tid].pop_front();
	    	//        reqCycle.pop_front(); // [YE] -moved back
	    }

	    q_empty = there_is_no_any_request();

	    if (!q_empty)
	    {
	        // There's more thing to execute so reschedule the event for next time
	        DPRINTF(FlexCPUCoreEvent, "Rescheduling resource\n");
	        Tick next_time;
	        next_time = cpu->nextCycle(); // YE

	        assert(next_time != curTick());
	        // Note: it could be scheduled if one of the requests above schedules
	        // This "always" reschedules since it may or may not be on the queue
	        // right now.
	        cpu->reschedule(&attemptAllEvent, next_time, true);
	    }
}

void FlexCPU::ResourceThreadsManaged::schedule()
{
	bool q_empty = there_is_no_any_request();
    // Note: on retries from memory we could try to schedule this even though
    //       the list of requests is empty. Revist this after implementing an
    //       LSQ.
    DPRINTF(FlexCPUCoreEvent, "Trying to schedule resource\n");
    if (!q_empty && !attemptAllEvent.scheduled()) {
        DPRINTF(FlexCPUCoreEvent, "Scheduling attempt all\n");
        // [YE] - add latency between stages
        cpu->schedule(&attemptAllEvent, cpu->clockEdge(latency));
    }
}

bool FlexCPU::ResourceThreadsManaged::there_is_no_any_request()
{
	for(ThreadID tid = 0 ; tid < cpu->threads.size(); tid++)
	{
		if(!map_requests[tid].empty())
		{
			return false;
		}
	}
	return true;
}


ThreadID FlexCPU::ResourceThreadsManaged::qid_select()
{
	ThreadID chosen_tid=-1;


	switch (this->threadPolicy) {
	case Enums::FlxSingleThreaded:
		chosen_tid = 0;
		break;
	case Enums::FlxRoundRobin:
		chosen_tid = roundRobinPriority();
		break;
	case Enums::FlxRandom:
		chosen_tid =  randomPriority();
		break;
	case Enums::FlxMax:
		chosen_tid = maxPriority();
		break;
	case Enums::FlxEvent:
		chosen_tid = eventPriority();
		break;
	case Enums::FlxCorse:
		chosen_tid = corsePriority();
		break;
	default:
		panic("Unknown fetch policy");
	}
	return chosen_tid;
}

ThreadID FlexCPU::ResourceThreadsManaged::roundRobinPriority()
{
    ThreadID chosen_tid;

    for (ThreadID i = 1; i <= cpu->threads.size(); i++) {
    	chosen_tid = (priority + i) % cpu->threads.size();
    	thread_attr &req = map_requests[chosen_tid].front();
    	if(map_requests[chosen_tid].size() && req.inst->isReady())
    	{
    		priority = chosen_tid;
    		return chosen_tid;
    	}
    }
    chosen_tid = -1; // didnt found
    return chosen_tid;
}

ThreadID FlexCPU::ResourceThreadsManaged::randomPriority()
{
    ThreadID chosen_tid=std::rand();

    for (ThreadID i = 0; i < cpu->threads.size(); i++) {
    	chosen_tid = (chosen_tid + 1) % cpu->threads.size();
    	thread_attr &req = map_requests[chosen_tid].front();
    	if(map_requests[chosen_tid].size() && req.inst->isReady())
    		return chosen_tid;
    }
    return chosen_tid;
}

ThreadID FlexCPU::ResourceThreadsManaged::maxPriority()
{
    int curr = 0;
    int max = 0;

	ThreadID chosen_tid=-1;
    for(ThreadID tid = 0; (tid < cpu->threads.size()); tid++)
    {
    	curr = map_requests[tid].size();
    	thread_attr &req = map_requests[tid].front();
    	if(curr > max && req.inst->isReady())
    	{
    		chosen_tid = tid;
    		max = curr;
    	}
    }

    return chosen_tid;
}

ThreadID FlexCPU::ResourceThreadsManaged::corsePriority()
{
	ThreadID chosen_tid=0;

	// In this case we start from zero
	for (ThreadID i = 0; i < cpu->threads.size(); i++) {
		chosen_tid = (priority+  i) % cpu->threads.size();
    	thread_attr &req = map_requests[chosen_tid].front();
    	if(map_requests[chosen_tid].size() && req.inst->isReady())
    	{
    		priority = chosen_tid;
    		return chosen_tid;
    	}
	}
	chosen_tid = -1;
	return chosen_tid;
}

// TODO
ThreadID FlexCPU::ResourceThreadsManaged::eventPriority()
{

    ThreadID chosen_tid;
//    int is_mem_load;
//    int is_mem_store;

    for (ThreadID i = 1; i <= cpu->threads.size(); i++) {
    	chosen_tid = (priority + i) % cpu->threads.size();
    	thread_attr &req = map_requests[chosen_tid].front();
    	int is_control = 0;
    	int is_mem_ref = 0;
    	if(map_requests[chosen_tid].size())
    	{
    	is_control = req.inst->staticInst()->isControl();
    	is_mem_ref = req.inst->staticInst()->isMemRef();
//    	is_mem_load = req.inst->staticInst()->isLoad();
//    	is_mem_store = req.inst->staticInst()->isStore();
//        DPRINTF(FlexCPUThreadEvent,
//                "YEEE  %s is cntrl %d ref %d load %d store %d\n",
//                req.inst->staticInst()->disassemble(
//                		req.inst->pcState().pc()).c_str() , is_control,
//						is_mem_ref,is_mem_load,is_mem_store);
    	}
    	if(map_requests[chosen_tid].size() && req.inst->isReady() &&
    			(is_control || is_mem_ref))
    	{
    		priority = chosen_tid;
    		return chosen_tid;
    	}
    }

//    chosen_tid = -i;
    chosen_tid = roundRobinPriority();
    return chosen_tid;
}

int
FlexCPU::ResourceThreadsManaged::totalInstInQueues()
{
	int count =0;
	for (ThreadID i = 0; i < cpu->threads.size(); i++) {
		count += map_requests[i].size();
	}

	return count;
}

void
FlexCPU::ResourceThreadsManaged::clean_squashed()
{
	for (ThreadID i = 0; i < cpu->threads.size(); i++) {
		//loop over all queue
		while(map_requests[i].size())
		{
			thread_attr &req = map_requests[i].front();
			if(!(req.inst->isSquashed())) break;
			map_requests[i].pop_front();
		}

	}
}

