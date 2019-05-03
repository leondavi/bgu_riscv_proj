# -*- coding: utf-8 -*-
from __future__ import print_function

import optparse
import sys

# import the m5 (gem5) library created when gem5 is built
import m5
# import all of the SimObjects
from m5.objects import *
from caches import *

###############################################################################
###############################################################################
###############################################################################

#==============================================================================
def getOptions():
    parser = optparse.OptionParser()
    # SYSTEM
    parser.add_option("--sys-voltage", 
                      action="store", 
                      type="string",
                      default='1.0V',
                      help = """Top-level voltage for blocks running at system
                      power supply""")

    parser.add_option("--sys-clock",
                      action="store", 
                      type="string",
                      default='1GHz',
                      help = """Top-level clock for blocks running at system
                      speed""")

    parser.add_option("--cpu-clock",
                      action="store",
                      type="string",
                      default='2GHz',
                      help="Clock for blocks running at CPU speed")

    parser.add_option("--mem-size",
                      action="store",
                      type="string",
                      default="512MB",
                      help="Specify the physical memory size (single memory)")
    #CPU
    parser.add_option("-t",
                      "--num-threads",
                      type="int",
                      default=1,
                      help = "Number of threads that running on the CPU")

    # Cachce
    parser.add_option("-c", 
                      "--cache-enable",
                      type="int",
                      default=1,
                      help = "NOT supprted - by deault connected to cache")

    
    # Execute Code
    parser.add_option("-b", "--binary",
                      action = "store",
                      type = "string",
                      default = '../mibench/sum.o',
                      help = "Execute code")

    parser.add_option('--l1i_size',
                      action = "store",
                      type="string",
                      default = '16kB',
                      help="L1 instruction cache size. Default" 
                      )
                    
    parser.add_option('--l1d_size',
                      action = "store",
                      type="string",
                      default = '64kB',
                      help="L1 instruction cache size. Default" 
                      )

    parser.add_option('--l2_size',
                      action = "store",
                      type="string",
                      default = '256kB',
                      help="L1 instruction cache size. Default" 
                      )                      
                    
    (options, args) = parser.parse_args()    
    return options


# Build System
#==============================================================================
# Create the system we are going to simulate
def buildSystem(options):
    system = System()

    # Create a top-level voltage domain
    system.voltage_domain = VoltageDomain(voltage = options.sys_voltage)
    
    # Create a source clock for the system and set the clock period
    system.clk_domain = SrcClockDomain(clock =  options.sys_clock,
                                       voltage_domain = system.voltage_domain)

    # Create a CPU voltage domain
    system.cpu_voltage_domain = VoltageDomain()

    # Create a separate clock domain for the CPUs
    system.cpu_clk_domain = SrcClockDomain(clock = options.cpu_clock,
                                           voltage_domain =
                                           system.cpu_voltage_domain)
    # Setup memory
    system.mem_mode = 'timing'
#    system.mem_mode.latency = '2ns'
    system.mem_ranges = [AddrRange(options.mem_size)]
    system.mem_ctrl = DDR4_2400_8x8() #DR3_1600_8x8()
    system.mem_ctrl.range = system.mem_ranges[0]
#    system.mem_ctrl.tRCD = '1ns'
#    system.mem_ctrl.tCL = '1ns'

    return system

# Build CPU
#==============================================================================
# Based on the parameters build the CPU model
def buildCPU(options,system):
    # TODO - currently not supporting multicore and the cpu modle is minor
    system.cpu = MinorCPU()
    
    
    system.multi_thread = True
    system.cpu.numThreads = options.num_threads
    system.cpu.createThreads()
    
    system.cpu.createInterruptController()

    # Pipeline params - cann't be set as 0, it makes issues
    system.cpu.fetch1ToFetch2ForwardDelay = 1
    system.cpu.fetch2ToDecodeForwardDelay = 1 
    system.cpu.decodeToExecuteForwardDelay = 1
    system.cpu.executeBranchDelay = 1

    # Fetch 1 params
    system.cpu.fetch1LineSnapWidth = 16
    system.cpu.fetch1LineWidth = 16
    system.cpu.fetch1FetchLimit = 2*options.num_threads

    # Fetch 2 params
    system.cpu.decodeInputWidth = 8  
    system.cpu.fetch2CycleInput = 1
    system.cpu.fetch2InputBufferSize = 1 # can be changed to 1, since no delay expected here.

    # Decode params
    system.cpu.executeInputWidth = 8
    system.cpu.decodeCycleInput = 1    
    system.cpu.decodeInputBufferSize = 8

    # Exexute
    system.cpu.executeIssueLimit = 1
    system.cpu.executeMemoryIssueLimit = options.num_threads
    system.cpu.executeCommitLimit = 1 
    system.cpu.executeMemoryCommitLimit = 1
    system.cpu.executeCycleInput = 1
#    system.cpu.executeFuncUnits 0x55f45d6ea8c0
    system.cpu.executeAllowEarlyMemoryIssue = 1
    system.cpu.executeMaxAccessesInMemory = options.num_threads 
    system.cpu.executeMemoryWidth = 0
    system.cpu.executeLSQRequestsQueueSize = options.num_threads
    system.cpu.executeLSQTransfersQueueSize = options.num_threads 
    system.cpu.executeLSQStoreBufferSize = 5
    system.cpu.executeLSQMaxStoreBufferStoresPerCycle = options.num_threads 

    for i in range(0,options.num_threads):
        process = Process()
        process.cmd = [options.binary]
        process.pid = 100+i
        system.cpu.workload.append(process)

    return system

 # Build Mem
#==============================================================================
def buildMem(options,system):
    # Create a memory bus, a system crossbar, in this case
    system.membus = SystemXBar()
    system.membus.frontend_latency = 0 #3
    system.membus.forward_latency = 0 #4
    system.membus.response_latency = 0 #2
    system.membus.snoop_response_latency = 0 #4
    system.membus.width = 64

    if (options.cache_enable):
        system.cpu.icache = L1ICache(options)
        system.cpu.dcache = L1DCache(options)
        system.cpu.icache.connectCPU(system.cpu)
        system.cpu.dcache.connectCPU(system.cpu)

        system.cpu.icache.connectBus(system.membus) #.slave)
        system.cpu.dcache.connectBus(system.membus) #.slave)
    else :
        # Hook the CPU ports up to the membus
        system.cpu.icache_port = system.membus.slave
        system.cpu.dcache_port = system.membus.slave
    
    # create the interrupt controller for the CPU and connect to the membus
    
    # Create a DDR3 memory controller and connect it to the membus
    system.mem_ctrl.port = system.membus.master
    
    # Connect the system up to the membus
    system.system_port = system.membus.slave
    return system

# Start Sim
#==============================================================================
def startSim(options,system):
    # set up the root SimObject and start the simulation
    root = Root(full_system = False, system = system)
    #instantiate all of the objects we've created above
    m5.instantiate()
 
    print("Beginning simulation!")
    exit_event = m5.simulate()
    print('Exiting @ tick %i because %s' %
        (m5.curTick(), exit_event.getCause()))

#==============================================================================
def main():
    # Extract default balues and command line options
    options = getOptions()
    
    # System
    system = buildSystem(options)

    # CPU
    system = buildCPU(options,system)
    
    # Memory hierarchy  connection
    syestm = buildMem(options,system)

    # Run simultion
    startSim(options,system)
#==============================================================================
main()

