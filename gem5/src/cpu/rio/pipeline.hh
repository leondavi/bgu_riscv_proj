/*
 * pipeline.h
 *
 *  Created on: 30 Jun 2019
 *      Author: yossi
 */

#ifndef __CPU_RIO_PIPELINE_HH_
#define __CPU_RIO_PIPELINE_HH_

#include "cpu/activity.hh"
#include "sim/ticked_object.hh"
#include "cpu/rio/rio.hh"
#include "params/RioCPU.hh"

namespace Rio {

class Pipeline: public Ticked {
protected:
	RioCPU &cpu;

	/** Allow cycles to be skipped when the pipeline is idle */
	bool allow_idling;

	ActivityRecorder activityRecorder;

public:
	/** Enumerated ids of the 'stages' for the activity recorder */
	enum StageId {
		/* A stage representing wakeup of the whole processor */
		CPUStageId = 0,
		/* Real pipeline stages */
		FetchStageId, DecodeStageId, ExecuteStageId, Num_StageId /* Stage count */
	};

	Pipeline(RioCPU &cpu_, RioCPUParams &params);

    /** A custom evaluate allows report in the right place (between
     *  stages and pipeline advance) */
    void evaluate() override;

    /** To give the activity recorder to the CPU */
    ActivityRecorder *getActivityRecorder() { return &activityRecorder; }

	// bgu::BGUMtTracer* bgu_pipeline_tracer;
    // void bguTrace(); 						// TODO - copy this

}; // End of class

} /* namespace Rio */

#endif /* __CPU_RIO_PIPELINE_HH_ */
