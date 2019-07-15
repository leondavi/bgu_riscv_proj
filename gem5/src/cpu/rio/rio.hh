#ifndef __CPU_RIO_CPU_HH__
#define __CPU_RIO_CPU_HH__

#include "cpu/activity.hh"
#include "cpu/simple_thread.hh"
//#include "cpu/rio/rio_thread.hh"
#include "cpu/base.hh"
#include "params/RioCPU.hh"
#include "cpu/rio/rioport.hh"
#include "arch/utility.hh"


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

	Rio::RioCachePort *Icache;
	Rio::RioCachePort *Dcache;

	/** Return a reference to the data port. */
	MasterPort &getDataPort() override;

	/** Return a reference to the instruction port. */
	MasterPort &getInstPort() override;
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

	/**
     * Called after constructor but before running. (Make sure to call
     * BaseCPU::init() in this function to let it do its job)
     */
	void init() override;	// Used at the constructor

	/**
     * Called after init, during the first simulate() event.
     */
	void startup() override;

    /**
     * Called to resume execution of a particular thread on this CPU.
     */
    void wakeup(ThreadID tid) override;

    /**
     * Called before this CPU instance is swapped with another. Must ensure
     * that by the time this function returns, internal state is flushed.
     */
    void switchOut() override;

    /**
     * Called before a running CPU instance is swapped for this. We are
     * expected to initialize our state using the old CPU's state, and take
     * over any connections (ports) the previous CPU held.
     *
     * @param cpu CPU to load state from.
     */
    void takeOverFrom(BaseCPU* cpu) override;

     /**
     * Called to set up the state to run a particular thread context. This also
     * kicks off the first simulation
     *
     * @param tid The ID of the thread that was activated
     */
    void activateContext(ThreadID tid) override;

    void suspendContext(ThreadID tid) override;

    void haltContext(ThreadID tid) override;

    /** Simple inst count interface from BaseCPU */
    Counter totalInsts() const override;
    Counter totalOps() const override;

    /** Stats interface from SimObject (by way of BaseCPU) */
    void regStats() override;


};

#endif // __CPU_RIO_CPU_HH__
