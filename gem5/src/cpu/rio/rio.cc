#include "cpu/rio/rio.hh"
#include "cpu/rio/pipeline.hh"

RioCPU::RioCPU(RioCPUParams *params) :
		BaseCPU(params)
{
	/* create threads */
	Rio::RioThread *thread;

	for (ThreadID i = 0; i < numThreads; i++) {
		if (FullSystem) {
			fatal("The Rio Model does not support FullSystem yet\n");
		} else {
			thread = new Rio::RioThread(this, i, params->system,
					params->workload[i], params->itb, params->dtb,
					params->isa[i]);
		}

		threads.push_back(thread);
		ThreadContext *tc = thread->getTC();
		threadContexts.push_back(tc);
	}

	// TODO - chechk DynInst
	//  Minor::MinorDynInst::init();

    pipeline = new Rio::Pipeline(*this, *params);
	activityRecorder = pipeline->getActivityRecorder();

	Icache = new Rio::RioCachePort(std::string("icache"), *this);
	Dcache = new Rio::RioCachePort(std::string("dcache"), *this);

}
//=============================================================================
RioCPU::~RioCPU(){
	delete pipeline;

    for (ThreadID thread_id = 0; thread_id < threads.size(); thread_id++) {
        delete threads[thread_id];
    }
}

//=============================================================================
RioCPU * RioCPUParams::create() {
	return new RioCPU(this);
}

//=============================================================================
MasterPort &RioCPU::getInstPort() {
//    return pipeline->getInstPort();
	return *Icache;
}

//=============================================================================
MasterPort &RioCPU::getDataPort() {
//    return pipeline->getDataPort();
	return *Dcache;
}

//=============================================================================
Counter RioCPU::totalInsts() const {
	Counter ret = 0;
	for (auto i = threads.begin(); i != threads.end(); i++)
		ret += (*i)->numInst;

	return ret;
}

//=============================================================================
Counter RioCPU::totalOps() const {
	Counter ret = 0;

	for (auto i = threads.begin(); i != threads.end(); i++)
		ret += (*i)->numOp;

	return ret;
}

/** Stats interface from SimObject (by way of BaseCPU) */
//=============================================================================
void RioCPU::regStats()
{
    BaseCPU::regStats();
    //stats.regStats(name(), *this);
    pipeline->regStats();
}


//=============================================================================
void RioCPU::init()
{
	std::cout<<"RIO Init\n";
    BaseCPU::init();

    if (!params()->switched_out &&
        system->getMemoryMode() != Enums::timing)
    {
        fatal("The Minor CPU requires the memory system to be in "
            "'timing' mode.\n");
    }

    /* Initialise the ThreadContext's memory proxies */
    for (ThreadID thread_id = 0; thread_id < threads.size(); thread_id++) {
        ThreadContext *tc = getContext(thread_id);

        tc->initMemProxies(tc);
    }

    /* Initialise CPUs (== threads in the ISA) */
    if (FullSystem && !params()->switched_out) {
		fatal("The Rio Model does not support FullSystem yet\n");
    }
}

//=============================================================================
void RioCPU::startup()
{
	std::cout<<"RIO startup\n";
    //DPRINTF(MinorCPU, "MinorCPU startup\n"); //TODO open it

    BaseCPU::startup();

    for (ThreadID tid = 0; tid < numThreads; tid++) {
        threads[tid]->startup(); // TODO - debug check if we use it(we saw it empty)
//        pipeline->wakeupFetch(tid);
        wakeup(tid);
    }

}

//=============================================================================
void RioCPU::wakeup(ThreadID tid) {
	assert(tid < numThreads);

	if (threads[tid]->status() == ThreadContext::Suspended) {
		threads[tid]->activate();
	}

    pipeline->start();
}

void
RioCPU::activateContext(ThreadID tid)
{
//    DPRINTF(FlexCPUCoreEvent, "activateContext(%d)\n", tid);

    BaseCPU::activateContext(tid);
}

void
RioCPU::suspendContext(ThreadID tid)
{
    // Need to de-schedule any instructions in the pipeline?
    BaseCPU::suspendContext(tid);
}

void
RioCPU::haltContext(ThreadID tid)
{
    // Need to de-schedule any instructions in the pipeline?
    BaseCPU::haltContext(tid);
}
