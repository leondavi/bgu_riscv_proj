# Copyright (c) 2012-2014 ARM Limited
# All rights reserved.
#
# The license below extends only to copyright in the software and shall
# not be construed as granting a license to any other intellectual
# property including but not limited to intellectual property relating
# to a hardware implementation of the functionality of the software
# licensed hereunder.  You may use the software subject to the license
# terms below provided that you ensure that this notice is replicated
# unmodified and in its entirety in all distributions of the software,
# modified or unmodified, in source code or in binary form.
#
# Copyright (c) 2007 The Regents of The University of Michigan
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Gabe Black
#          Nathan Binkert
#          Andrew Bardsley

from __future__ import print_function

from m5.defines import buildEnv
from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject
from BaseCPU import BaseCPU
from MinorCPU import MinorCPU
from DummyChecker import DummyChecker
from BranchPredictor import *
from TimingExpr import TimingExpr

from FuncUnit import OpClass

class MthreadsCPUOpClass(SimObject):
    """Boxing of OpClass to get around build problems and provide a hook for
    future additions to OpClass checks"""

    type = 'MthreadsCPUOpClass'
    cxx_header = "cpu/MthreadsCPU/func_unit.hh"

    opClass = Param.OpClass("op class to match")

class MthreadsCPUOpClassSet(SimObject):
    """A set of matchable op classes"""

    type = 'MthreadsCPUOpClassSet'
    cxx_header = "cpu/MthreadsCPU/func_unit.hh"

    opClasses = VectorParam.MthreadsCPUOpClass([], "op classes to be matched."
        "  An empty list means any class")

class MthreadsCPUFUTiming(SimObject):
    type = 'MthreadsCPUFUTiming'
    cxx_header = "cpu/MthreadsCPU/func_unit.hh"

    mask = Param.UInt64(0, "mask for testing ExtMachInst")
    match = Param.UInt64(0, "match value for testing ExtMachInst:"
        " (ext_mach_inst & mask) == match")
    suppress = Param.Bool(False, "if true, this inst. is not executed by"
        " this FU")
    extraCommitLat = Param.Cycles(0, "extra cycles to stall commit for"
        " this inst.")
    extraCommitLatExpr = Param.TimingExpr(NULL, "extra cycles as a"
        " run-time evaluated expression")
    extraAssumedLat = Param.Cycles(0, "extra cycles to add to scoreboard"
        " retire time for this insts dest registers once it leaves the"
        " functional unit.  For mem refs, if this is 0, the result's time"
        " is marked as unpredictable and no forwarding can take place.")
    srcRegsRelativeLats = VectorParam.Cycles("the maximum number of cycles"
        " after inst. issue that each src reg can be available for this"
        " inst. to issue")
    opClasses = Param.MthreadsCPUOpClassSet(MthreadsCPUOpClassSet(),
        "op classes to be considered for this decode.  An empty set means any"
        " class")
    description = Param.String('', "description string of the decoding/inst."
        " class")

def MthreadsCPUMakeOpClassSet(op_classes):
    """Make a MthreadsCPUOpClassSet from a list of OpClass enum value strings"""
    def boxOpClass(op_class):
        return MthreadsCPUOpClass(opClass=op_class)

    return MthreadsCPUOpClassSet(opClasses=map(boxOpClass, op_classes))

class MthreadsCPUFU(SimObject):
    type = 'MthreadsCPUFU'
    cxx_header = "cpu/MthreadsCPU/func_unit.hh"

    opClasses = Param.MthreadsCPUOpClassSet(MthreadsCPUOpClassSet(), "type of operations"
        " allowed on this functional unit")
    opLat = Param.Cycles(1, "latency in cycles")
    issueLat = Param.Cycles(1, "cycles until another instruction can be"
        " issued")
    timings = VectorParam.MthreadsCPUFUTiming([], "extra decoding rules")

    cantForwardFromFUIndices = VectorParam.Unsigned([],
        "list of FU indices from which this FU can't receive and early"
        " (forwarded) result")

class MthreadsCPUFUPool(SimObject):
    type = 'MthreadsCPUFUPool'
    cxx_header = "cpu/MthreadsCPU/func_unit.hh"

    funcUnits = VectorParam.MthreadsCPUFU("functional units")

class MthreadsCPUDefaultIntFU(MthreadsCPUFU):
    opClasses = MthreadsCPUMakeOpClassSet(['IntAlu'])
    timings = [MthreadsCPUFUTiming(description="Int",
        srcRegsRelativeLats=[2])]
    opLat = 3

class MthreadsCPUDefaultIntMulFU(MthreadsCPUFU):
    opClasses = MthreadsCPUMakeOpClassSet(['IntMult'])
    timings = [MthreadsCPUFUTiming(description='Mul',
        srcRegsRelativeLats=[0])]
    opLat = 3

class MthreadsCPUDefaultIntDivFU(MthreadsCPUFU):
    opClasses = MthreadsCPUMakeOpClassSet(['IntDiv'])
    issueLat = 9
    opLat = 9

class MthreadsCPUDefaultFloatSimdFU(MthreadsCPUFU):
    opClasses = MthreadsCPUMakeOpClassSet([
        'FloatAdd', 'FloatCmp', 'FloatCvt', 'FloatMisc', 'FloatMult',
        'FloatMultAcc', 'FloatDiv', 'FloatSqrt',
        'SimdAdd', 'SimdAddAcc', 'SimdAlu', 'SimdCmp', 'SimdCvt',
        'SimdMisc', 'SimdMult', 'SimdMultAcc', 'SimdShift', 'SimdShiftAcc',
        'SimdSqrt', 'SimdFloatAdd', 'SimdFloatAlu', 'SimdFloatCmp',
        'SimdFloatCvt', 'SimdFloatDiv', 'SimdFloatMisc', 'SimdFloatMult',
        'SimdFloatMultAcc', 'SimdFloatSqrt'])
    timings = [MthreadsCPUFUTiming(description='FloatSimd',
        srcRegsRelativeLats=[2])]
    opLat = 6

class MthreadsCPUDefaultMemFU(MthreadsCPUFU):
    opClasses = MthreadsCPUMakeOpClassSet(['MemRead', 'MemWrite', 'FloatMemRead',
                                     'FloatMemWrite'])
    timings = [MthreadsCPUFUTiming(description='Mem',
        srcRegsRelativeLats=[1], extraAssumedLat=2)]
    opLat = 1

class MthreadsCPUDefaultMiscFU(MthreadsCPUFU):
    opClasses = MthreadsCPUMakeOpClassSet(['IprAccess', 'InstPrefetch'])
    opLat = 1

class MthreadsCPUDefaultFUPool(MthreadsCPUFUPool):
    funcUnits = [MthreadsCPUDefaultIntFU(), MthreadsCPUDefaultIntFU(),
        MthreadsCPUDefaultIntMulFU(), MthreadsCPUDefaultIntDivFU(),
        MthreadsCPUDefaultFloatSimdFU(), MthreadsCPUDefaultMemFU(),
        MthreadsCPUDefaultMiscFU()]

#class ThreadPolicy(Enum): vals = ['SingleThreaded', 'RoundRobin', 'Random'] Check why not passing compile

class MthreadsCPU(MinorCPU):
    type = 'MthreadsCPU'
    cxx_header = "cpu/MthreadsCPU/cpu.hh"


