import sys
import argparse

import m5
from m5.objects import *
from Caches import *

##############################simple cache module initilization###################################
#=================================================================================================
def initCacheModule(args, cache_init, sys1Bus, sys2Bus):
    cache_init.size = args.size
    cache_init.assoc = args.assoc
    cache_init.data_latency = args.latency
    cache_init.tag_latency = args.latency
    cache_init.response_latency = args.latency
    cache_init.cpu_side = sys1Bus.master
    cache_init.mem_side = sys2Bus.slave
    return cache_init
#=================================================================================================

############################Banked cache module initilization#####################################
#=================================================================================================
def initBankedCacheModule(args, cache_init, sys1Bus, sys2Bus, bankBits, intlvbits, match):
    cache_init.size = args.size
    cache_init.assoc = args.assoc
    cache_init.data_latency = args.latency
    cache_init.tag_latency = args.latency
    cache_init.response_latency = args.latency
    cache_init.cpu_side = sys1Bus.master
    cache_init.mem_side = sys2Bus.slave
    cache_init.addr_ranges = AddrRange(start=0, end=18446744073709551615,intlvHighBit = bankBits, intlvBits=intlvbits,intlvMatch=match)
    return cache_init
#=================================================================================================

##################################argument parsing################################################
#=================================================================================================
def getparseOptions():
    parser = argparse.ArgumentParser(description='RISCV multicore config options')
    group = parser.add_mutually_exclusive_group()
    parser.add_argument('-t'    ,'--num-threads',type=int   ,default=1          ,help='number of riscv threads (DUMMY for g5gui)')
    parser.add_argument('-n'    ,'--num_proc'   ,type=int   ,default=1          ,help='number of riscv cores')
    parser.add_argument('-s'    ,'--size'       ,type=str   ,default='64 kB'    ,help='l1 i cache size')
    parser.add_argument('-l'    ,'--latency'    ,type=int   ,default=2          ,help='l1 i cache latency')
    parser.add_argument('-a'    ,'--assoc'      ,type=int   ,default=4          ,help='l1 i cache association')
    parser.add_argument('-f'    ,'--file'       ,type=str   ,default="fft_4096" ,help='file to run')
    group.add_argument('-ps'    ,'--l1Config'   ,type=int   ,default=0          ,help='L1 configuration (0 = private, 1 = shared, else = no L1)')
    group.add_argument('-l0'    ,'--L0lineWidth',type=int   ,default=4          ,help='Change line width of L0 (4, 8, 16)')
    group.add_argument('-ba'    ,'--banks'      ,type=int   ,default=1          ,help='Number of Banks in L1 cache')
    group.add_argument('-bb'    ,'--bankBits'   ,type=int   ,default=14         ,help='Bit Number choice in Banked L1 cache')
    group.add_argument('-c'     ,'--clusters'   ,type=int   ,default=1          ,help='Number of Clusters in system')
    group.add_argument('-b'     ,'--binary'     ,type=str   ,default="~/workspace/bgu_riscv_proj/gem5/AAI/benchmark-bin",help='Execute code')
    args = parser.parse_args()
    return args
#=================================================================================================

############################################system config##########################################
#=================================================================================================
def systemInit(args):
    system = System(cpu = [MinorCPU(cpu_id=i) for i in xrange(args.num_proc)])
    
    system.clk_domain = SrcClockDomain()
    system.clk_domain.clock = '1GHz'
    system.clk_domain.voltage_domain = VoltageDomain()
    
    system.mem_mode = 'timing'
    system.mem_ranges = [AddrRange('512MB')]
    
    system.cpu_voltage_domain = VoltageDomain()
    system.cpu_clk_domain = SrcClockDomain(clock = '1GHz',voltage_domain= system.cpu_voltage_domain)
    multiprocess = [Process(cmd='AAI/benchmark-bin/' + args.file,pid=100+i) for i in xrange(args.num_proc)]
#    multiprocess = [Process(cmd='AAI/benchmark-bin/' + args.file,pid=100+i) for i in xrange(args.num_proc)]
    system.l2bus = L2XBar(clk_domain = system.cpu_clk_domain)
    system.membus = SystemXBar()
    system.multi_thread = False
    
    #cpu and dcache config
    for i in xrange(args.num_proc):
    #core Configuration
        system.cpu[i].fetch1LineSnapWidth = args.L0lineWidth
        system.cpu[i].fetch1LineWidth = args.L0lineWidth
        system.cpu[i].fetch2InputBufferSize = 1
        system.cpu[i].decodeInputBufferSize = 1
        system.cpu[i].decodeInputWidth = 4
        system.cpu[i].executeInputWidth = 1
        system.cpu[i].executeIssueLimit = 1
        system.cpu[i].executeCommitLimit = 1
        system.cpu[i].executeInputBufferSize = 2
    #dcache config
        system.cpu[i].dcache = L1DCache()
        system.cpu[i].dcache.connectCPU(system.cpu[i])
        system.cpu[i].dcache.connectBus(system.l2bus)
        #system.cpu[i].dcache_port = system.membus.slave
        system.cpu[i].createInterruptController()
        system.cpu[i].workload = multiprocess[i] 
        system.cpu[i].createThreads()
    
    #L2 setup
    system.l2cache = L2Cache(clk_domain=system.cpu_clk_domain)
    system.l2cache.connectCPUSideBus(system.l2bus)
    system.l2cache.connectMemSideBus(system.membus)
    system.system_port = system.membus.slave
    
    #DDR controller setup
    system.mem_ctrl = DDR3_1600_8x8()
    system.mem_ctrl.range = system.mem_ranges[0]
    system.mem_ctrl.port = system.membus.master

    return system
###############################################cache config#######################################
#=================================================================================================
def configCache(args, system):
    if args.banks == 2:
        intBits = 1
    elif args.banks == 4:
        intBits = 2
    elif args.banks == 8:
        intBits = 3
    
    coreInClusters = args.num_proc/args.clusters
    #SHARED L1 CONFIGURATION
    if args.l1Config == 1: #sharedl1
        if args.clusters == 1:          #1 cluster: all cores share a single L1I$ 
            system.l1bus = L2XBar()
            if args.banks == 1:
                system.l1cache = initCacheModule(args, L1ICache(), system.l1bus, system.l2bus)
            elif args.banks == 2:
                system.l1cache1b = initBankedCacheModule(args, L1ICacheBank(),system.l1bus, system.l2bus, args.bankBits,intBits,0)
                system.l1cache2b = initBankedCacheModule(args, L1ICacheBank(),system.l1bus, system.l2bus, args.bankBits,intBits,1)
            elif args.banks == 4:
                system.l1cache1b = initBankedCacheModule(args, L1ICacheBank(),system.l1bus, system.l2bus, args.bankBits,intBits,0)
                system.l1cache2b = initBankedCacheModule(args, L1ICacheBank(),system.l1bus, system.l2bus, args.bankBits,intBits,1)
                system.l1cache3b = initBankedCacheModule(args, L1ICacheBank(),system.l1bus, system.l2bus, args.bankBits,intBits,2)
                system.l1cache4b = initBankedCacheModule(args, L1ICacheBank(),system.l1bus, system.l2bus, args.bankBits,intBits,3)
            elif args.banks == 8:
                system.l1cache1b = initBankedCacheModule(args, L1ICacheBank(),system.l1bus, system.l2bus, args.bankBits,intBits,0)
                system.l1cache2b = initBankedCacheModule(args, L1ICacheBank(),system.l1bus, system.l2bus, args.bankBits,intBits,1)
                system.l1cache3b = initBankedCacheModule(args, L1ICacheBank(),system.l1bus, system.l2bus, args.bankBits,intBits,2)
                system.l1cache4b = initBankedCacheModule(args, L1ICacheBank(),system.l1bus, system.l2bus, args.bankBits,intBits,3)
                system.l1cache5b = initBankedCacheModule(args, L1ICacheBank(),system.l1bus, system.l2bus, args.bankBits,intBits,4)
                system.l1cache6b = initBankedCacheModule(args, L1ICacheBank(),system.l1bus, system.l2bus, args.bankBits,intBits,5)
                system.l1cache7b = initBankedCacheModule(args, L1ICacheBank(),system.l1bus, system.l2bus, args.bankBits,intBits,6)
            for i in xrange(args.num_proc):
                system.cpu[i].icache_port = system.l1bus.slave
    
        elif args.clusters == 2:        #2 clusters: every 2(4 cores total)/4(8 cores total) share an L1I$ instance. 2 L1I$ instances.
            system.l1bus1 = L2XBar()
            system.l1bus2 = L2XBar()
            if args.banks == 1:
                system.l1cache1 = initCacheModule(args, L1ICache(), system.l1bus1, system.l2bus)
                system.l1cache2 = initCacheModule(args, L1ICache(), system.l1bus2, system.l2bus)
            elif args.banks == 2:
                system.l1cache1b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,0)
                system.l1cache1b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,1)
                system.l1cache2b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,0)
                system.l1cache2b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,1)
            elif args.banks == 4:
                system.l1cache1b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,0)
                system.l1cache1b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,1)
                system.l1cache1b3 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,2)
                system.l1cache1b4 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,3)
                system.l1cache2b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,0)
                system.l1cache2b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,1)
                system.l1cache2b3 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,2)
                system.l1cache2b4 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,3)
            elif args.banks == 8:
                system.l1cache1b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,0)
                system.l1cache1b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,1)
                system.l1cache1b3 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,2)
                system.l1cache1b4 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,3)
                system.l1cache1b5 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,4)
                system.l1cache1b6 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,5)
                system.l1cache1b7 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,6)
                system.l1cache1b8 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,7)
                system.l1cache2b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,0)
                system.l1cache2b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,1)
                system.l1cache2b3 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,2)
                system.l1cache2b4 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,3)
                system.l1cache2b5 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,4)
                system.l1cache2b6 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,6)
                system.l1cache2b7 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,7)
                system.l1cache2b8 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,8)
    
            for i in xrange(coreInClusters):
                system.cpu[i].icache_port                   = system.l1bus1.slave
                system.cpu[i+coreInClusters].icache_port    = system.l1bus2.slave
        elif args.clusters == 4:        #4 clusters: every 2(8 cores total) share an L1I$ instance. 2 L1I$ instance.
            system.l1bus1 = L2XBar()
            system.l1bus2 = L2XBar()
            system.l1bus3 = L2XBar()
            system.l1bus4 = L2XBar()
            if args.banks == 1:
                system.l1cache1 = initCacheModule(args, L1ICache(), system.l1bus1, system.l2bus)
                system.l1cache2 = initCacheModule(args, L1ICache(), system.l1bus2, system.l2bus)
                system.l1cache3 = initCacheModule(args, L1ICache(), system.l1bus3, system.l2bus)
                system.l1cache4 = initCacheModule(args, L1ICache(), system.l1bus4, system.l2bus)
            elif args.banks == 2:
                system.l1cache1b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,0)
                system.l1cache1b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,1)
                system.l1cache2b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,0)
                system.l1cache2b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,1)
                system.l1cache3b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,0)
                system.l1cache3b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,1)
                system.l1cache4b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,0)
                system.l1cache4b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,1)
            elif args.banks == 4:
                system.l1cache1b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,0)
                system.l1cache1b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,1)
                system.l1cache1b3 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,2)
                system.l1cache1b4 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,3)
                system.l1cache2b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,0)
                system.l1cache2b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,1)
                system.l1cache2b3 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,2)
                system.l1cache2b4 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,3)
                system.l1cache3b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,0)
                system.l1cache3b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,1)
                system.l1cache3b3 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,2)
                system.l1cache3b4 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,3)
                system.l1cache4b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,0)
                system.l1cache4b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,1)
                system.l1cache4b3 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,2)
                system.l1cache4b4 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,3)
            elif args.banks == 8:
                system.l1cache1b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,0)
                system.l1cache1b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,1)
                system.l1cache1b3 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,2)
                system.l1cache1b4 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,3)
                system.l1cache1b5 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,4)
                system.l1cache1b6 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,5)
                system.l1cache1b7 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,6)
                system.l1cache1b8 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus1, system.l2bus, args.bankBits,intBits,7)
                system.l1cache2b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,0)
                system.l1cache2b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,1)
                system.l1cache2b3 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,2)
                system.l1cache2b4 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,3)
                system.l1cache2b5 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,4)
                system.l1cache2b6 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,5)
                system.l1cache2b7 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,6)
                system.l1cache2b8 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus2, system.l2bus, args.bankBits,intBits,7)
                system.l1cache3b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,0)
                system.l1cache3b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,1)
                system.l1cache3b3 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,2)
                system.l1cache3b4 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,3)
                system.l1cache3b5 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,4)
                system.l1cache3b6 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,5)
                system.l1cache3b7 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,6)
                system.l1cache3b8 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus3, system.l2bus, args.bankBits,intBits,7)
                system.l1cache4b1 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,0)
                system.l1cache4b2 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,1)
                system.l1cache4b3 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,2)
                system.l1cache4b4 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,3)
                system.l1cache4b5 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,4)
                system.l1cache4b6 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,5)
                system.l1cache4b7 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,6)
                system.l1cache4b8 = initBankedCacheModule(args, L1ICacheBank(),system.l1bus4, system.l2bus, args.bankBits,intBits,7)
            for i in xrange(coreInClusters):
                system.cpu[i].icache_port                   = system.l1bus1.slave
                system.cpu[i+coreInClusters].icache_port    = system.l1bus2.slave
                system.cpu[i+coreInClusters+2].icache_port  = system.l1bus3.slave
                system.cpu[i+coreInClusters+4].icache_port  = system.l1bus4.slave

    #PRIVATE L1 CONFIGURATION
    elif args.l1Config == 0: # privatel1:
        for i in xrange(args.num_proc):
            system.cpu[i].icache = L1ICache()
            system.cpu[i].icache.size = args.size
            system.cpu[i].icache.assoc = args.assoc
            system.cpu[i].icache.data_latency = args.latency
            system.cpu[i].icache.tag_latency = args.latency
            system.cpu[i].icache.response_latency = args.latency
            system.cpu[i].icache.connectCPU(system.cpu[i])
            system.cpu[i].icache.connectBus(system.l2bus)
    
    #NO L1 CACHE CONFIGURATION
    else:   #no L1
        for i in xrange(args.num_proc):
            system.cpu[i].icache_port=system.l2bus.slave

    return system

######################################### simulate ###############################################
#=================================================================================================
def StartSim(args, system):

    print ("#######################Begining Simulation!#########################################")
    print ("with Argumnet:")
    print ("Work Load: {}:".format(args.file))
    print ("Processors: {}:".format(args.num_proc))
    print ("Cache Size: {}:".format(args.size))
    print ("L1 config Private/Shared/No: {}:".format(args.l1Config))
    print ("L0 Width: {}:".format(args.L0lineWidth))
    print ("Banks: {}, Bank Bit: {}".format(args.banks, args.bankBits))
    print ("Clusters: {}:".format(args.clusters))

    root = Root(full_system = False , system = system)

    m5.instantiate()

    exit_event = m5.simulate()

    print ('Exiting @ tick {} because {}'.format(m5.curTick() , exit_event.getCause()))

    print ("#######################Simulation Done!#########################################")

############################################# main ###############################################
#=================================================================================================
def main():

    args = getparseOptions()
    # initialize system arguments:
    system = systemInit(args)
    # configure cache params:
    system = configCache(args, system)
    # start Simulation: 
    StartSim(args, system)
#=================================================================================================
main()
  
