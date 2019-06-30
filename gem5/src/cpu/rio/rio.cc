#include "cpu/rio/rio.hh"
#include "cpu/rio/pipeline.hh"
#include "arch/utility.hh"

RioCPU::RioCPU(RioCPUParams *params):
BaseCPU(params),
Icache(std::string("icache"),*this),
Dcache(std::string("dcache"),*this)
//pipeline()

{


};

RioCPU *
RioCPUParams::create()
{
    return new RioCPU(this);
}

MasterPort &RioCPU::getInstPort()
{
//    return pipeline->getInstPort();
	return Icache;
}

MasterPort &RioCPU::getDataPort()
{
//    return pipeline->getDataPort();
	return Dcache;
}

Counter RioCPU::totalInsts() const
{
    Counter ret = 0;

//    for (auto i = threads.begin(); i != threads.end(); i ++)
//        ret += (*i)->numInst;

    return ret;
}

Counter RioCPU::totalOps() const
{
    Counter ret = 0;
//
//    for (auto i = threads.begin(); i != threads.end(); i ++)
//        ret += (*i)->numOp;

    return ret;
}

void RioCPU::wakeup(ThreadID tid){}


