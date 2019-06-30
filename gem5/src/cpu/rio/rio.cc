#include "cpu/rio/rio.hh"
#include "cpu/rio/pipeline.hh"
#include "arch/utility.hh"

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

	Icache = new Rio::CachePort(std::string("icache"), &this);
	Dcache = new Rio::CachePort(std::string("dcache"), &this);

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
	return Icache;
}

//=============================================================================
MasterPort &RioCPU::getDataPort() {
//    return pipeline->getDataPort();
	return Dcache;
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

//=============================================================================
void RioCPU::wakeup(ThreadID tid) {
	assert(tid < numThreads);

	if (threads[tid]->status() == ThreadContext::Suspended) {
		threads[tid]->activate();
	}
}
