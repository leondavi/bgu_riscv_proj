#include "dummy_pred.hh"

DummyBP::DummyBP(const DummyBPParams *params) : BPredUnit(params),prediction(params->prediction)
{
	
}


/**
 * @brief DummyBP::create
 * Createing an instance of BP
 * @return
 */
DummyBP* DummyBPParams::create()
{
    return new DummyBP(this);
}

void DummyBP::squash(ThreadID tid, void *bp_history)
{

}

/**
 * @brief DummyBP::btbUpdate
 * @param tid
 * @param branch_addr
 * @param bp_history
 * Update the branch target buffers
 */
void DummyBP::btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history)
{
// Place holder for a function that is called to update predictor history when
// a BTB entry is invalid or not found.
}


bool DummyBP::lookup(ThreadID tid, Addr branch_addr, void * &bp_history)
{
    //std::cout<<"dummy lookup called"<<std::endl;
    return getPrediction();

    /*
    bool taken;
    uint8_t counter_val;
    unsigned local_predictor_idx = getLocalIndex(branch_addr);

    DPRINTF(Fetch, "Looking up index %#x\n",
            local_predictor_idx);

    counter_val = localCtrs[local_predictor_idx].read();

    DPRINTF(Fetch, "prediction is %i.\n",
            (int)counter_val);

    taken = getPrediction(counter_val);

#if 0
    // Speculative update.
    if (taken) {
        DPRINTF(Fetch, "Branch updated as taken.\n");
        localCtrs[local_predictor_idx].increment();
    } else {
        DPRINTF(Fetch, "Branch updated as not taken.\n");
        localCtrs[local_predictor_idx].decrement();
    }
#endif

    return taken;*/
}

/**
 * Updates the branch predictor with the actual result of a branch.
 * @param branch_addr The address of the branch to update.
 * @param taken Whether or not the branch was taken.
 * @param bp_history Pointer to the BPHistory object that was created
 * when the branch was predicted.
 * @param squashed is set when this function is called during a squash
 * operation.
 */
void DummyBP::update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
                bool squashed)
{
    /*
    assert(bp_history == NULL);
    unsigned local_predictor_idx;

    // No state to restore, and we do not update on the wrong
    // path.
    if (squashed) {
        return;
    }

    // Update the local predictor.
    local_predictor_idx = getLocalIndex(branch_addr);

    DPRINTF(Fetch, "Looking up index %#x\n", local_predictor_idx);

    if (taken) {
        DPRINTF(Fetch, "Branch updated as taken.\n");
        localCtrs[local_predictor_idx].increment();
    } else {
        DPRINTF(Fetch, "Branch updated as not taken.\n");
        localCtrs[local_predictor_idx].decrement();
    }
    */
}

inline bool DummyBP::getPrediction()
{
    // Get the MSB of the count
   // return (count >> (localCtrBits - 1));
    return BP_TAKEN;
}

void DummyBP::uncondBranch(ThreadID tid, Addr pc, void *&bp_history)
{

}
