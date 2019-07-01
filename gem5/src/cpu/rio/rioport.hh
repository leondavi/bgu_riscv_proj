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

/** Thread Scheduling Policy (RoundRobin, Random, etc) */
// Enums::ThreadPolicy threadPolicy; - TODO - we have separate in each relevant place
/** Provide a non-protected base class for Minor's Ports as derived
 *  classes are created by Fetch1 and Execute */
class RioCPUPort: public MasterPort {
public:
	/** The enclosing cpu */
	RioCPU &cpu;

	/*
	     MasterPort(const std::string& name, MemObject* owner,
	               PortID id = InvalidPortID);
	 */

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


/*	class IcachePort : public MinorCPU::MinorCPUPort
	{
	protected:

		Fetch1 &fetch;

	public:
		IcachePort(std::string name, Fetch1 &fetch_, MinorCPU &cpu) :
			MinorCPU::MinorCPUPort(name, cpu), fetch(fetch_)
		{ }

	protected:
		bool recvTimingResp(PacketPtr pkt)
		{ return fetch.recvTimingResp(pkt); }

		void recvReqRetry() { fetch.recvReqRetry(); }
	};*/

/*
class MinorCPUPort : public MasterPort
    {
      public:
        MinorCPU &cpu;

      public:
        MinorCPUPort(const std::string& name_, MinorCPU &cpu_)
            : MasterPort(name_, &cpu_), cpu(cpu_)
        { }

    };*/



#endif /* SRC_CPU_RIO_RIOPORT_HH_ */
