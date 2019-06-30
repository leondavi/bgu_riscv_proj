/*
 * decode.hh
 *
 *  Created on: 30 Jun 2019
 *      Author: david
 */

#ifndef SRC_CPU_RIO_DECODE_HH_
#define SRC_CPU_RIO_DECODE_HH_

namespace Rio {

class Decode
{
protected:
	/** Pointer back to the containing CPU */
	RioCPU &cpu;

	/** Input port carrying branch requests from Execute */
	Latch<int>::Output inp;

	/** Output port carrying read lines to Fetch2 */
	Latch<int>::Input out;
public:
	Decode();
	virtual ~Decode();
};

} /* namespace Rio */

#endif /* SRC_CPU_RIO_DECODE_HH_ */
