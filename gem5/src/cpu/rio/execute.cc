/*
 * Execute.cc
 *
 *  Created on: 30 Jun 2019
 *      Author: david
 */

#include "cpu/rio/execute.hh"

namespace Rio {


Execute::Execute(RioCPU &cpu_ ,RioLatch<int>::Output inp_,RioLatch<int>::Input out_) :
		cpu(cpu_),
		inp(inp_),
		out(out_)
{
	// TODO Auto-generated constructor stub

}

Execute::~Execute() {
	// TODO Auto-generated destructor stub
}




void Execute::evaluate()
{
	int &count_in  = *inp.outputWire;
	int &count_out = *out.inputWire;

	count_out = count_in;
	std::cout << std::to_string(curTick()/1000) <<" Execute : " << count_out << "|";
}
} /* namespace Rio */
