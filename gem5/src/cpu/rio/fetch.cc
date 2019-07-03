/*
 * fetch.cpp
 *
 *  Created on: 30 Jun 2019
 *      Author: david
 */

#include "cpu/rio/fetch.hh"

namespace Rio {

Fetch::Fetch(
		RioCPU &cpu_,
		RioLatch<int>::Output inp_, RioLatch<int>::Input out_)  :
		cpu(cpu_),
		inp(inp_),
		out(out_)
{
	// TODO Auto-generated constructor stub
	temp_counter = 0;
}

Fetch::~Fetch() {
	// TODO Auto-generated destructor stub
}

void
Fetch::evaluate()
{
	int &count_out = *out.inputWire;

	count_out = ++temp_counter;
	std::cout <<std::to_string(curTick()/1000) << " Fetch : " << temp_counter<<std::endl;

	if(temp_counter <10)
	{
		cpu.activityRecorder->activity();
	}
}

} /* namespace Rio */
