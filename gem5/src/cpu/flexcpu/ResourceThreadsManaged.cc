/*
 * ResourceThreadsManaged.cc
 *
 *  Created on: 05 Nov 2019
 *      Author: david
 */


#include "cpu/flexcpu/flexcpu.hh"


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

	    bool q_empty = there_is_no_any_request();

	    if (q_empty)
	    {
	        return; // This happens with 0 latency events if a request enqueues
	                // another request.
	    }

	    ThreadID chosen_tid;

	    int curr = 0;
	    int max = 0;
	    for(ThreadID tid = 0; (tid < cpu->threads.size()); tid++)
	    {
	    	 DPRINTF(FlexCPUCoreEvent, "Attempting all requests. %d on queue\n",
	    		            map_requests[tid].size());
	    	curr = map_requests[tid].size();
	    	if(curr > max)
	    	{
	    		chosen_tid = tid;
	    		max = curr;
	    	}
	    }



	    while (!map_requests[chosen_tid].empty() && resourceAvailable())
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
