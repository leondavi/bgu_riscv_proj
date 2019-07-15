#include "cpu/rio/rio.hh"
#include "cpu/rio/pipeline.hh"

#include "debug/RioCPU.hh"

// Rio parameters
//=============================================================================
RioCPU * RioCPUParams::create()
{
	return new RioCPU(this);
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC
///////////////////////////////////////////////////////////////////////////////

// Rio - constructor
//=============================================================================
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

// Rio destructor
//=============================================================================
RioCPU::~RioCPU(){
	delete pipeline;

    for (ThreadID thread_id = 0; thread_id < threads.size(); thread_id++) {
        delete threads[thread_id];
    }
}

// Init
//=============================================================================
void
RioCPU::init()
{
	DPRINTF(RioCPU,"RIO Init\n");
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

// Startup
//=============================================================================
void
RioCPU::startup()
{
    DPRINTF(RioCPU, "RioCPU startup\n");

    BaseCPU::startup();

    for (ThreadID tid = 0; tid < numThreads; tid++) {
        threads[tid]->startup(); // TODO - debug check if we use it(we saw it empty)
        wakeup(tid);
    }
}

// Wakeup
//=============================================================================
void
RioCPU::wakeup(ThreadID tid)
{
    DPRINTF(RioCPU, "Rio wakeup(%d)\n",tid);
	assert(tid < numThreads);

	if (threads[tid]->status() == ThreadContext::Suspended) {
		threads[tid]->activate();
	}

    pipeline->start();
}

// SwitchOut
//=============================================================================
void
RioCPU::switchOut()
{
    DPRINTF(RioCPU, "Rio swithcOut\n");
	BaseCPU::switchOut();
}

// TakeOverFrom
//=============================================================================
void
RioCPU::takeOverFrom(BaseCPU* cpu)
{
    DPRINTF(RioCPU, "Rio takeOverFrom\n");
	BaseCPU::takeOverFrom(cpu);
}

// ActivateContext
//=============================================================================
void
RioCPU::activateContext(ThreadID tid)
{
	DPRINTF(RioCPU, "activateContext(%d)\n", tid);
    BaseCPU::activateContext(tid);
}

// SuspendContext
//=============================================================================
void
RioCPU::suspendContext(ThreadID tid)
{
	DPRINTF(RioCPU, "suspendContext(%d)\n", tid);
    BaseCPU::suspendContext(tid);
}

// Halt Context
//=============================================================================
void
RioCPU::haltContext(ThreadID tid)
{
	DPRINTF(RioCPU, "haltContext(%d)\n", tid);
    BaseCPU::haltContext(tid);
}

// TotalInsts
//=============================================================================
Counter
RioCPU::totalInsts() const
{
	Counter ret = 0;
	for (auto i = threads.begin(); i != threads.end(); i++)
		ret += (*i)->numInst;

	return ret;
}

// TotalOps
//=============================================================================
Counter
RioCPU::totalOps() const {
	Counter ret = 0;

	for (auto i = threads.begin(); i != threads.end(); i++)
		ret += (*i)->numOp;

	return ret;
}

// RegStats
//=============================================================================
void
RioCPU::regStats()
{
    BaseCPU::regStats();
    //stats.regStats(name(), *this);
    pipeline->regStats();
}

///////////////////////////////////////////////////////////////////////////////
// Protected
///////////////////////////////////////////////////////////////////////////////

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
