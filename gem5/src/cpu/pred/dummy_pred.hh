#ifndef DUMMY_PRED_H
#define DUMMY_PRED_H

#include "base/types.hh"
#include "cpu/pred/bpred_unit.hh"
#include "cpu/pred/sat_counter.hh"
#include "params/DummyBP.hh"

enum {BP_NOT_TAKEN,BP_TAKEN};

class DummyBP : public BPredUnit
{
public:

    DummyBP(const DummyBPParams *params);

/***********************************************/
/*   pure virtual derived from bpred_unit      */
/***********************************************/
    /**
     * Restores the global branch history on a squash.
     * @param bp_history Pointer to the BPHistory object that has the
     * previous global branch history in it.
     */
    void squash(ThreadID tid, void *bp_history);
    void uncondBranch(ThreadID tid, Addr pc, void * &bp_history);
    bool lookup(ThreadID tid, Addr instPC, void * &bp_history);
    void btbUpdate(ThreadID tid, Addr instPC, void * &bp_history);
    void update(ThreadID tid, Addr instPC, bool taken, void *bp_history, bool squashed);

private:
    //private variables:
    unsigned prediction;

    inline bool getPrediction();
};

#endif // DUMMY_PRED_H
