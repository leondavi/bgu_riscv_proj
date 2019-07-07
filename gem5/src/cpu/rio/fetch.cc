/*
 * fetch.cpp
 *
 *  Created on: 30 Jun 2019
 *      Author: david
 */

#include "cpu/rio/fetch.hh"
#include "debug/RioCPU.hh"
namespace Rio {


// Fetch - constructor
//=============================================================================
Fetch::Fetch(
		RioCPU &cpu_,
		RioLatch<int>::Output inp_, RioLatch<int>::Input out_)  :
		cpu(cpu_),
		inp(inp_),
		out(out_)
{
	temp_counter = 0;
}

Fetch::~Fetch() {

}

// Fetch - destructor
//=============================================================================
void
Fetch::evaluate()
{
	int &count_out = *out.inputWire;

	count_out = ++temp_counter;
	DPRINTF(RioCPU,"Fetch : (%d)\n",count_out);

	if(temp_counter <10)
	{
		cpu.activityRecorder->activity();
	}
	// First we need to check per thread if there is a stream change from
	// execute stage or from branch prediction unit

	// Check which thread can send fetch request

	// Try To send the request

	// Check if a response is exists, if so update state and move forward

	// In case no action, turn stage off?
}



} /* namespace Rio */
