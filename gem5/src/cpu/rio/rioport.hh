/*
 * rioport.hh
 *
 *  Created on: Jul 1, 2019
 *      Author: david
 */

#ifndef SRC_CPU_RIO_RIOPORT_HH_
#define SRC_CPU_RIO_RIOPORT_HH_

#include "cpu/base.hh"
#include "cpu/rio/rio.hh"


namespace Rio
{

class RioCPUPort: public MasterPort {
public:
	/** The enclosing cpu */
	RioCPU &cpu;

	RioCPUPort(const std::string& name_, RioCPU &cpu_) 	: MasterPort(name_, (MemObject*)&cpu_),cpu(cpu_)
	{		}

};

class RioCachePort : public RioCPUPort
{
public:

	RioCachePort(std::string name, RioCPU &cpu) :
		RioCPUPort(name, cpu)
		{ }

protected:
	bool recvTimingResp(PacketPtr pkt)
	{ return false; }

	void recvReqRetry() {}
};

}//end of namespace


#endif /* SRC_CPU_RIO_RIOPORT_HH_ */
