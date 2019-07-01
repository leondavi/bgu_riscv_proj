/*
 * decode.cc
 *
 *  Created on: 30 Jun 2019
 *      Author: david
 */

#include "cpu/rio/decode.hh"

namespace Rio {

Decode::Decode(RioCPU &cpu_ ,RioLatch<int>::Output inp_,RioLatch<int>::Input out_) :
				cpu(cpu_),
				inp(inp_),
				out(out_)
{
	// TODO Auto-generated constructor stub

}

Decode::~Decode() {
	// TODO Auto-generated destructor stub
}

} /* namespace Rio */
