/*
 * decode.hh
 *
 *  Created on: 30 Jun 2019
 *      Author: david
 */

#ifndef SRC_CPU_RIO_DECODE_HH_
#define SRC_CPU_RIO_DECODE_HH_

#include "cpu/rio/buffer.hh"
#include "cpu/rio/rio.hh"

class RioCPU;

namespace Rio {

class Decode
{
protected:
	/** Pointer back to the containing CPU */
	RioCPU &cpu;

	/** Input port carrying branch requests from Execute */
	RioLatch<int>::Output inp;

	/** Output port carrying read lines to Fetch2 */
	RioLatch<int>::Input out;
public:
	Decode(RioCPU &cpu_ ,RioLatch<int>::Output inp_,RioLatch<int>::Input out_);

	~Decode();
};

} /* namespace Rio */

#endif /* SRC_CPU_RIO_DECODE_HH_ */
