/*
 * pipeline.cpp
 *
 *  Created on: 30 Jun 2019
 *      Author: yossi
 */

#include "cpu/rio/pipeline.hh"
#include "cpu/rio/fetch.hh"

#include <algorithm>


namespace Rio {

Pipeline::Pipeline(RioCPU &cpu_, RioCPUParams &params) :
		Ticked(cpu_, &(cpu_.BaseCPU::numCycles)),
		cpu(cpu_),
	    allow_idling(params.enableIdling),
		fe_to_de(cpu.name()+".fe_to_de",Cycles(1)), // TODO- change to parameter
		de_to_ex(cpu.name()+".de_to_ex",Cycles(1)), // TODO- change to parameter
		ex_to_fe(cpu.name()+".ex_to_fe",Cycles(1)), // TODO- change to parameter
		fetch_(cpu_,ex_to_fe.output(), fe_to_de.input()),
		decode_(cpu_, fe_to_de.output(), de_to_ex.input()),
		execute_(cpu_, de_to_ex.output(), ex_to_fe.input()),
		activityRecorder(cpu.name()+".activity", Num_StageId , 3, 0) // TODO - fix the const 3
{
	// TODO Auto-generated constructor stub

}

void Pipeline::evaluate()
{
    /* Note that it's important to evaluate the stages in order to allow
     *  'immediate', 0-time-offset TimeBuffer activity to be visible from
     *  later stages to earlier ones in the same cycle */

//    execute.evaluate();
//    decode.evaluate();
//    fetch2.evaluate();
	fetch_.evaluate();
//
//    if (DTRACE(BGUTrace))
//    	bguTrace();

    /* Update the time buffers after the stages */
//    f1ToF2.evaluate();
//    f2ToF1.evaluate();
//    f2ToD.evaluate();
//    dToE.evaluate();
//    eToF1.evaluate();

    /* The activity recorder must be be called after all the stages and
     *  before the idler (which acts on the advice of the activity recorder */
    activityRecorder.advance();

//    if (allow_idling) {
//        /* Become idle if we can but are not draining */
//        if (!activityRecorder.active() && !needToSignalDrained) {
//            DPRINTF(Quiesce, "Suspending as the processor is idle\n");
//            stop();
//        }
//
//        /* Deactivate all stages.  Note that the stages *could*
//         *  activate and deactivate themselves but that's fraught
//         *  with additional difficulty.
//         *  As organised herre */
//        activityRecorder.deactivateStage(Pipeline::CPUStageId);
//        activityRecorder.deactivateStage(Pipeline::Fetch1StageId);
//        activityRecorder.deactivateStage(Pipeline::Fetch2StageId);
//        activityRecorder.deactivateStage(Pipeline::DecodeStageId);
//        activityRecorder.deactivateStage(Pipeline::ExecuteStageId);
//    }
}


void Pipeline::regStats()
{
	Ticked::regStats();

//        fetch2.regStats();
}


}/* namespace Rio */
