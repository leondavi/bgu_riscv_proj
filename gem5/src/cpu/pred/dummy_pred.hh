#ifndef DUMMY_PRED_H
#define DUMMY_PRED_H

#include "base/types.hh"
#include "cpu/pred/bpred_unit.hh"
#include "cpu/pred/sat_counter.hh"
#include "params/DummyBP.hh"

class DummyBP : public BPredUnit
{
public:

    DummyBP(const DummyBPParams *params);
};

#endif // DUMMY_PRED_H
