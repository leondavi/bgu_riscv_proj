

#include "cpu/flexcpu/flexcpu.hh"

#include "debug/FlexCPUBranchPred.hh"
#include "debug/FlexCPUBufferDump.hh"
#include "debug/FlexCPUDeps.hh"
#include "debug/FlexCPUInstEvent.hh"
#include "debug/FlexCPUThreadEvent.hh"
#include "debug/FlexCPUCoreEvent.hh"

using namespace std;
using namespace TheISA;


bool FlexCPU::ResourceFetchDecision::resourceAvailable(ThreadID tid)
{
    if (cpu->_instPort.blocked() || (cpu->issueThreadUnit.get_queue_size_by_threadid(tid)>20)) {
        return false;
    } else {
        return Resource::resourceAvailable();
    }
}

void FlexCPU::ResourceFetchDecision::addRequest(ThreadID tid,
		std::shared_ptr<InflightInst> inst,	const std::function<bool()>& run_function)
{
	thread_attr new_attr(inst,run_function);
    map_requests[tid].push_back(new_attr);
    DPRINTF(FlexCPUCoreEvent, "Adding request on thread %d queue size: %d\n",
                                 tid,map_requests[tid].size());
}

void FlexCPU::ResourceFetchDecision::attemptAllRequests()
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

//	    std::cout<<"Queues Status \n ~~~~~~~~~~~~~~~~~~\n";
//		for (int i=0; i < cpu->numThreads; i++)
//		{
//			std::cout<<"tid: "<<i<<" Q size: "<< map_requests[i].size()<<"  ||  ";
//		}
//		std::cout<<std::endl;
//
//	    std::cout<<"before!!!"<<std::endl;

	    ThreadID chosen_tid = threadid_by_autoencoder();//rand() % cpu->numThreads; //Here add the autoencoder
//	    std::cout<<"min qid: "<<chosen_tid<<std::endl;



	    while ((chosen_tid != InvalidThreadID) && !map_requests[chosen_tid].empty() && resourceAvailable(chosen_tid))
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

bool FlexCPU::ResourceFetchDecision::update_from_execution_unit(ThreadID tid,std::shared_ptr<InflightInst> inst_ptr, bool control,bool branch_state)
{

	//--- history table update ---//

	Addr req_addr = inst_ptr->pcState().instAddr();
	if(!map_requests[tid].empty())
	{
		thread_attr tid_attr = map_requests[tid].front();
		req_addr = tid_attr.inst->pcState().instAddr();
	}
	hist_attr inst_attr(inst_ptr,req_addr);


	//------- Stats update -------//
	if(control && branch_state)
	{
		inst_attr.set_branch_taken();
		numBranchesTaken++;
	}
	else //not taken branch
	{
		switch(inst_attr.get_inst_type())
		{
			case inst_attr.INST_TYPE_LOAD: {this->numOfLoadInsts++; break;}
			case inst_attr.INST_TYPE_STORE: {this->numOfStoreInsts++; break;}
		}
	}
	hist_tables[tid].push(inst_attr);
	numOfCompleted++;

	if(dump_table_flag && (tid==0) && (table_counter[tid] < 3000))
	{
		if (this->dumping_counter[tid] <= 0)
		{
			hist_tables[tid].dump_to_csv(simout.directory()+"/histtab_tid_"+to_string(tid)+"_ctr_"+to_string(table_counter[tid])+".csv");
			future_tables[tid].dump_to_csv(simout.directory()+"/futuretab_tid_"+to_string(tid)+"_ctr_"+to_string(table_counter[tid])+".csv");

			dumping_counter[tid] = dump_interval;
			table_counter[tid]++;
		}
		dumping_counter[tid]--;
	}

	//----------------------------//


	TheISA::PCState pc = inst_ptr->pcState();
	StaticInstPtr cur_inst_ptr = inst_ptr->staticInst();

	Addr pc_val = inst_ptr->pcState().instAddr();
	ExtMachInst machine_inst_val = inst_ptr->staticInst()->machInst;
	std::string inst_name = inst_ptr->staticInst()->getName();
	//Tick time_from_creation = inst_ptr->getTimingRecord().creationTick;

	std::cout<<"[ResourceFetchDecision] update received from exec unit: pc: "<<pc_val<<
			" inst mach: "<<machine_inst_val<<
			" taken: "<<branch_state<<
			" inst name: "<<inst_name<<std::endl;
	return true;
}


void FlexCPU::ResourceFetchDecision::regStats()
{
	Resource::regStats();
	this->numBranchesTaken
	        .name(name() + ".numBranchesTaken")
	        .desc("Number of control instructions that were taken as counted in FetchDecisionUnit")
	        ;

	this->numOfCompleted.name(name() + ".numOfCompleted")
	    	        .desc("Number of completed instructions")
	    	        ;
	this->numOfLoadInsts.name(name() + ".numOfLoadInsts")
	    	        .desc("Number of completed Load instructions")
	    	        ;
	this->numOfStoreInsts.name(name() + ".numOfStoreInsts")
	    	        .desc("Number of completed store instructions")
	    	        ;

}



//------------------------ Fetch Policies ------------------/


ThreadID FlexCPU::ResourceFetchDecision::get_min_qid()
{
	ThreadID res = InvalidThreadID;

	std::unordered_map<ThreadID, std::list<thread_attr>>* map_requests_ptr = this->IssueUnit_ptr->get_map_requests();

	int min_size = std::numeric_limits<int>::max();

	for(ThreadID tid = 0; tid < this->cpu->numThreads; tid++ )
	{
        //TODO don't use cout std::cout<<"tid: "<<tid<<" Issue queue: "<<(*map_requests_ptr)[tid].size()<<std::endl;

        if(min_size > (*map_requests_ptr)[tid].size() && !this->map_requests[tid].empty())
		{
			res = tid;
            min_size = (*map_requests_ptr)[tid].size();
		}
	}

	return res;
}

ThreadID FlexCPU::ResourceFetchDecision::threadid_by_autoencoder()
{
	ThreadID res = InvalidThreadID;

	std::unordered_map<ThreadID, std::list<thread_attr>>* map_requests_ptr = this->IssueUnit_ptr->get_map_requests();

	int min_size = std::numeric_limits<int>::max();

	for(ThreadID tid = 0; tid < this->cpu->numThreads; tid++ )
	{
        //TODO don't use cout std::cout<<"tid: "<<tid<<" Issue queue: "<<(*map_requests_ptr)[tid].size()<<std::endl;

		std::list<thread_attr>::iterator it;
		for(it = (*map_requests_ptr)[tid].begin(); it != (*map_requests_ptr)[tid].end(); ++it)
		{
			if(it->inst->isDecoded())
			{
				Addr req_addr = it->inst->pcState().instAddr();
				if(!map_requests[tid].empty())
				{
					thread_attr tid_attr = map_requests[tid].front();
					req_addr = tid_attr.inst->pcState().instAddr();
				}
				future_tables[tid].push(hist_attr(it->inst,req_addr));
			}
		}

        if(min_size > (*map_requests_ptr)[tid].size() && !this->map_requests[tid].empty())
		{
			res = tid;
            min_size = (*map_requests_ptr)[tid].size();
		}

    	if (!future_tables[tid].empty())
    	{
    		AERED aered_test();
    		if (!future_tables[tid].get_history_table_ptr()->empty())
    		{
    		 //hist_attr former_attr = future_tables[tid].get_history_table_ptr()->at(1);
    		 hist_attr attr = future_tables[tid].get_history_table_ptr()->at(0);
    		 aered_inst.generate_ae_sample(attr.machine_inst_,attr.get_inst_type(),0,attr.pc_,attr.pc_req_);
    		}
//    		VectorXd inst_vec;
//    		std::string inst_name = future_tables[tid].get_history_table_ptr()->front().inst_name_;
//    		AERED::convert_inst_to_vec((uint32_t)future_tables[tid].get_history_table_ptr()->front().machine_inst_,inst_vec);
//    		std::cout<<"name: "<<inst_name<<std::endl;
//    		std::cout<<inst_vec<<std::endl;
    	}


	}


	return res;
}

void FlexCPU::ResourceFetchDecision::generate_aered_win(ThreadID tid, uint win_size,std::vector<AERED::aered_input> &out_input_to_ae)
{
	uint pushed_items = 0;
	std::deque<hist_attr> *current_inst_table = future_tables[tid].get_history_table_ptr();
	std::deque<hist_attr> *hist_inst_table = hist_tables[tid].get_history_table_ptr();
	std::vector<AERED::aered_input> inputs_to_ae;
	inputs_to_ae.resize(win_size);


	for (uint i=0; (i<current_inst_table->size()) && (pushed_items<win_size); i++)
	{
		hist_attr *attr = &current_inst_table->at(i);
		uint32_t former_pc = 0;
		if(i+1<current_inst_table->size())
		{
			former_pc = current_inst_table->at(i+1).pc_;
		}
		else if (!hist_inst_table->empty())
		{
			former_pc = hist_inst_table->front().pc_;
		}


		inputs_to_ae[pushed_items] = AERED::aered_input(attr->machine_inst_,attr->get_inst_type(),former_pc,attr->pc_,attr->pc_req_);
		pushed_items++;
	}

	for (uint i=0; (i<hist_inst_table->size()) && (pushed_items<win_size); i++)
	{
		hist_attr *attr = &hist_inst_table->at(i);
		uint32_t former_pc = 0;
		if(i+1<current_inst_table->size())
		{
			former_pc = hist_inst_table->at(i+1).pc_;
		}

		inputs_to_ae[pushed_items] = AERED::aered_input(attr->machine_inst_,attr->get_inst_type(),former_pc,attr->pc_,attr->pc_req_);
		pushed_items++;
	}
}

