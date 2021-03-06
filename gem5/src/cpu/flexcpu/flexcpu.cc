/*
 * Copyright (c) 2018 The Regents of The University of California
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Bradley Wang
 */

#include "cpu/flexcpu/flexcpu.hh"

#include "arch/locked_mem.hh"
#include "arch/mmapped_ipr.hh"
#include "base/compiler.hh"
#include "debug/FlexCPUCoreEvent.hh"
#include "cpu/flexcpu/BGUTracer.h"

using namespace std;
using namespace TheISA;


FlexCPU::FlexCPU(FlexCPUParams* params):
    BaseCPU(params),
    cacheBlockMask(~(cacheLineSize() - 1)),
    executionLatency(params->execution_latency),
	zeroTimeMicroopExecution(params->zero_time_microop_execution),
	dataAddrTranslationUnit(this, Cycles(0), 0, name() + ".dtbUnit"),
    executionUnit(this, params->execution_latency,
                  params->execution_bandwidth, name() + ".executionUnit"),
    instFetchUnit(this, params->fetch_decision_latency ,params->fetch_bandwidth, name() + ".iFetchUnit"),
    instAddrTranslationUnit(this, Cycles(0),
                            0, name() + ".itbUnit"),
    issueUnit(this, params->issue_latency,
              params->issue_bandwidth, name() + ".issueUnit"),
    memoryUnit(this, params->mem_bandwidth, //Cycles(0),
               name() + ".memoryUnit"),
    issueThreadUnit(this, params->thread_manged_latency,
              params->thread_manged_bandwidth, name() + ".issueThreadUnit",
			  params->issuePolicy),
    _dataPort(name() + "._dataPort", this),
    _instPort(name() + "._instPort", this),
    _branchPred(params->branchPred)
{

	if(!params->BGUTrace)
	{
		tracer::BGUTracer::get_inst().set_tracer_shut_down();
	}

    fatal_if(FullSystem, "FullSystem not implemented for FlexCPU");

    warn_if(!params->branch_pred_max_depth,
        "Infinite branch predictor depth may play poorly with unconstrained "
        "fetching and decoding.");

    for (ThreadID i = 0; i < numThreads; i++) {
        // Move constructor + vector should allow us to avoid memory leaks like
        // the SimpleCPU implementations have.
        threads.push_back(m5::make_unique<FlexCPUThread>(this, i,
            params->system, params->workload[i], params->itb, params->dtb,
            params->isa[i],
            params->branch_pred_max_depth,
            params->fetch_buffer_size,
            params->in_order_begin_execute,
            params->in_order_execute,
            params->instruction_buffer_size,
            params->strict_serialization));

        threadContexts.push_back(threads[i].get());
    }
}

void
FlexCPU::activateContext(ThreadID tid)
{
    DPRINTF(FlexCPUCoreEvent, "activateContext(%d)\n", tid);

    BaseCPU::activateContext(tid);
}

void
FlexCPU::completeMemAccess(PacketPtr orig_pkt, StaticInstPtr inst,
                                     std::weak_ptr<ExecContext> context,
                                     Trace::InstRecord* trace_data,
                                     MemCallback callback,
                                     Tick send_time,
                                     SplitAccCtrlBlk* split)
{
    PacketPtr pkt;

    memLatency.sample(curTick() - send_time);

    if (split) {
        if (orig_pkt == split->low) {
            split->lowReceived = true;
        } else if (orig_pkt == split->high) {
            split->highReceived = true;
        } else {
            panic("Malformed split MemAccessReq received!");
        }

        // If the other half of the request was killed somehow, we need to
        // clean up to avoid memory leaks. Only one of the conditions here will
        // be true, because the other one will match orig_pkt, and be deleted
        // via the recvTimingResp() function.
        if (!split->low || !split->high) {
            delete split->main;
            delete split;
            return;
        }

        if (!(split->lowReceived && split->highReceived))
            return; // We need to wait for the other to complete as well

        pkt = split->main;
        assert(pkt);
        pkt->makeResponse();
    } else {
        pkt = orig_pkt;
    }

    const shared_ptr<ExecContext> ctxt = context.lock();

    Fault fault = ctxt && !inst->isStore() ?
        inst->completeAcc(pkt, ctxt.get(), trace_data) : NoFault;

    if (split) {
        // The packets we actually sent to memory are deleted by recvTimingResp
        delete split->main;
        delete split;
    }

    callback(fault);
}

BPredUnit*
FlexCPU::getBranchPredictor()
{
    return _branchPred;
}

MasterPort&
FlexCPU::getDataPort()
{
    return _dataPort;
}

MasterPort&
FlexCPU::getInstPort()
{
    return _instPort;
}

bool
FlexCPU::hasBranchPredictor() const
{
    // Explicit conversion from pointer to bool.
    return static_cast<bool>(_branchPred);
}

void
FlexCPU::init()
{
    BaseCPU::init();

    if (!params()->switched_out &&
        system->getMemoryMode() != Enums::timing)
    {
        fatal("The Dataflow CPU requires the memory system to be in "
              "'timing' mode.\n");
    }

    for (auto& thread : threads) {
        thread->initMemProxies(thread.get());

        // TODO if fullsystem, we need to check if we need to call initCPU on
        // the ISA
    }
    // TODO add post-construction initialization code
}

void
FlexCPU::markActiveCycle()
{
    if (curTick() != lastActiveTick) {
        numCycles++;
        lastActiveTick = curTick();
        for (auto& thread : threads) {
            thread->recordCycleStats();
        }
    }
}

void
FlexCPU::requestBranchPredictor(
    std::function<void(BPredUnit* pred)> callback_func)
{
    schedule(new EventFunctionWrapper([this, callback_func]
                                      { callback_func(_branchPred); },
             name() + ".bpredret", true), curTick());
}

void
FlexCPU::requestDataAddrTranslation(const RequestPtr& req,
    ThreadContext* tc, bool write, std::weak_ptr<ExecContext> context,
    TranslationCallback callback_func)
{
    DPRINTF(FlexCPUCoreEvent, "requestDataAddrTranslation()\n");
    // delay if we're out of slots for simultaneous dtb translations

    Tick queue_time = curTick();

    dataAddrTranslationUnit.addRequest([this, req, tc, write, context,
                                        callback_func, queue_time]
    {
        if (context.expired()) return false; // Don't request for squashed inst

        Tick now = curTick();

        waitingForDataXlation.sample(now - queue_time);
        // The event callback for when the translation completed comes through
        // a BaseTLB::Translation object, so we pass that callback straight to
        // the requestor's callback function by subclassing it.
        BaseTLB::Translation* handler = new CallbackTransHandler(
                                            this, callback_func, now);

        tc->getDTBPtr()->translateTiming(req, tc, handler,
                                    write ? BaseTLB::Write : BaseTLB::Read);

        return true;
    });

    dataAddrTranslationUnit.schedule();
}

void
FlexCPU::requestExecution(StaticInstPtr inst,
                                    weak_ptr<ExecContext> context,
                                    Trace::InstRecord* trace_data,
                                    ExecCallback callback_func)
{
    DPRINTF(FlexCPUCoreEvent, "requestExecution()\n");

    Tick queue_time = curTick();

    // Do this when we *actually* execute the instruction
    executionUnit.addRequest([this, inst, context, trace_data, callback_func,
                              queue_time]
    {
        if (context.expired()) return false; // Don't execute squashed inst

        waitingForExecution.sample(curTick() - queue_time);

        Event *e = new EventFunctionWrapper([context, inst, trace_data,
                                             callback_func]{
            const shared_ptr<ExecContext> ctxt = context.lock();
            if (!ctxt) return;
            // If the context expired (a result of instruction squash), then
            // a callback is unnecessary, at least for existing FlexCPUThread
            // implementation.

            Fault fault = inst->isMemRef() ?
                          inst->initiateAcc(ctxt.get(), trace_data) :
                          inst->execute(ctxt.get(), trace_data);

            callback_func(fault);
        }, name() + ".delayedCall", true);

        const Tick execution_time = zeroTimeMicroopExecution
                                 && inst->isMicroop()
                                 && !inst->isLastMicroop() ?
                                     0 : 0; // Changed to zero - since we work with RISC and
        									// We don't want the extra latency add here.
        							// cyclesToTicks(executionLatency);
        schedule(e, curTick() + execution_time);

        return true;
    });

    executionUnit.schedule();
}

void
FlexCPU::requestInstAddrTranslation(const RequestPtr& req,
    ThreadContext* tc,
    TranslationCallback callback_func,
    std::function<bool()> is_squashed)
{
    DPRINTF(FlexCPUCoreEvent, "requestInstAddrTranslation()\n");

    Tick queue_time = curTick();

    instAddrTranslationUnit.addRequest([this, req, tc, callback_func,
                                        queue_time, is_squashed]
    {
        if (is_squashed()) return false;
        Tick now = curTick();

        waitingForInstXlation.sample(now - queue_time);
        // The event callback for when the translation completed comes through
        // a BaseTLB::Translation object, so we pass that callback straight to
        // the requestor's callback function by subclassing it.
        BaseTLB::Translation* handler = new CallbackTransHandler(
                                            this, callback_func, now);

        tc->getITBPtr()->translateTiming(req, tc, handler, BaseTLB::Execute);

        return true;
    });

    instAddrTranslationUnit.schedule();
}

void
FlexCPU::requestInstructionData(const RequestPtr& req,
    FetchCallback callback_func, std::function<bool()> is_squashed,ThreadID tid,std::weak_ptr<InflightInst> inst)
{
    DPRINTF(FlexCPUCoreEvent, "requestInstructionData()\n");

    // Allocate some memory for objects we're about to send through the Port
    // system
    PacketPtr pkt = new Packet(req, MemCmd::ReadReq);
    pkt->allocate();

    // Note: pkt is deleted in InstPort::recvTimingResp();

    Tick queue_time = curTick();

    shared_ptr<InflightInst> inst_locked = inst.lock();//TODO inst of request should be weak

    instFetchUnit.addRequest(tid,inst_locked,[this, req, callback_func, pkt, queue_time,
                              is_squashed]
    {
        if (is_squashed()) {
            delete pkt;
            return false;
        }

        waitingForInstData.sample(curTick() - queue_time);

        //  Don't send any requests while we're waiting for recvReqRetry
        _instPort.sendPacket(pkt);

        DPRINTF(FlexCPUCoreEvent, "Sent a fetch request(pa: %#x)\n",
                                pkt->req->getPaddr());

        // We mark the control flow so that we know what to call when memory
        // responds
        outstandingFetches.emplace(pkt, callback_func);

        return true;
    });

    instFetchUnit.schedule();
}

void
FlexCPU::requestIssue(function<void()> callback_func,
		std::function<bool()> is_squashed)
{
    DPRINTF(FlexCPUCoreEvent, "requestIssue()\n");

    issueUnit.addRequest([callback_func, is_squashed] {
        if (is_squashed()) return false;
        callback_func();
        return true;
    });

    issueUnit.schedule();

}

void
FlexCPU::requestIssueTid(function<void()> callback_func,
		std::function<bool()> is_squashed,
		std::shared_ptr<InflightInst> inst, ThreadID tid)
{
	issueThreadUnit.addRequest(tid,inst,[callback_func, is_squashed] {
		if (is_squashed()) return false;
		callback_func();
		return true;
	});


	for (ThreadID ii = 0; ii < threads.size(); ii++)
	{
		std::cout<<"TID : "<< ii <<" size :"<< issueThreadUnit.map_requests[ii].size()<<"\n";
	}

	issueThreadUnit.schedule();
}

bool
FlexCPU::requestMemRead(const RequestPtr& req, ThreadContext* tc,
                                  StaticInstPtr inst,
                                  weak_ptr<ExecContext> context,
                                  Trace::InstRecord* trace_data,
                                  MemCallback callback_func)
{
    DPRINTF(FlexCPUCoreEvent, "requestMemRead()\n");

    // Request can be handled immediately
    if (req->getFlags().isSet(Request::NO_ACCESS)) {
        return false; // TODO handle now.
    }

    PacketPtr pkt = Packet::createRead(req);
    pkt->allocate();
    // Note: pkt is deleted in DataPort::recvTimingResp(), except in the case
    //       of mmappedIpr where it is deleted in the internally scheduled
    //       event.

    // Despite mmappedIpr accessing registers, we do not explicitly check any
    // register dependencies under the assumption that the misc regs will not
    // be accessed through other means, and that memory dependence resolution
    // will be sufficient to ensure correct out-of-order behavior.
    if (req->isMmappedIpr()) {
        // TODO think about limiting access to mmapped IPR, either alongside
        //      other memory requests, or as its own constraint

        Tick now = curTick();
        Tick delay(cyclesToTicks(TheISA::handleIprRead(tc, pkt)));

        schedule(
            new EventFunctionWrapper(
                [this, pkt, inst, context, trace_data, callback_func, now]
                {
                    completeMemAccess(pkt, inst, context, trace_data,
                                      callback_func, now);
                    delete pkt;
                },
                name() + ".mmappedIprEvent",
                true),
            now + delay);

        return true;
    }

    Tick queue_time = curTick();

    memoryUnit.addRequest([this, req, tc, inst, context, trace_data,
                           callback_func, pkt, queue_time]
    {
        if (context.expired()) {
            // No need to send req if squashed
            delete pkt;
            return false;
        }

        Tick now = curTick();
        waitingForMem.sample(now - queue_time);

        _dataPort.sendPacket(pkt);

        if (req->isLLSC()) {
            TheISA::handleLockedRead(tc, req);
        }
        DPRINTF(FlexCPUCoreEvent, "Sent a memory request(pa: %#x)\n",
                                req->getPaddr());

        // When the result comes back from memory, call completeMemAccess
        outstandingMemReqs.emplace(pkt, [this, pkt, inst, context, trace_data,
                                         callback_func, now]
        {
            completeMemAccess(pkt, inst, context, trace_data, callback_func,
                              now);
        });

        return true;
    });

    memoryUnit.schedule();

    return true;
}

bool
FlexCPU::requestMemWrite(const RequestPtr& req, ThreadContext* tc,
                                   StaticInstPtr inst,
                                   weak_ptr<ExecContext> context,
                                   Trace::InstRecord* trace_data,
                                   uint8_t* data, MemCallback callback_func)
{
    DPRINTF(FlexCPUCoreEvent, "requestMemWrite()\n");

    // Request can be handled immediately
    if (req->getFlags().isSet(Request::NO_ACCESS)) {
        return false; // TODO handle now.
    }

    PacketPtr pkt = Packet::createWrite(req);
    // Note: pkt is deleted in DataPort::recvTimingResp(), except in the case
    //       of mmappedIpr where it is deleted in the internally scheduled
    //       event.

    if (data) {
        const unsigned req_size = req->getSize();

        pkt->allocate();
        memcpy(pkt->getPtr<uint8_t>(), data, req_size);
    } else { // Assume that no pointer given means we're filling with zero
        assert(req->getFlags() & Request::STORE_NO_DATA);
        // Maybe this line is important? Not all CPUs make this check, so I'm
        // leaving this commented out to see if it causes problems...
        // memset(mailbox, 0, req_size);
        panic("Should be unreachable...");
    }

    if (req->isMmappedIpr()) {
        // TODO think about limiting access to mmapped IPR, either alongside
        //      other memory requests, or as its own constraint

        Tick now = curTick();
        Tick delay(cyclesToTicks(TheISA::handleIprWrite(tc, pkt)));

        schedule(
            new EventFunctionWrapper(
                [this, pkt, inst, context, trace_data, callback_func, now]
                {
                    completeMemAccess(pkt, inst, context, trace_data,
                                      callback_func, now);
                    delete pkt;
                },
                name() + ".mmappedIprEvent",
                true),
            now + delay);

        return true;
    }

    Tick queue_time = curTick();

    memoryUnit.addRequest([this, req, tc, inst, context, trace_data,
                           callback_func, pkt, queue_time]
    {
        if (context.expired()) {
            // No need to send req if squashed
            delete pkt;
            return false;
        }

        Tick now = curTick();
        waitingForMem.sample(now - queue_time);

        if (req->isLLSC()) {
            if (!TheISA::handleLockedWrite(tc, req, cacheBlockMask)) {
                panic("Haven't quite decided how to do this yet");
                // TODO kill the request
            }
        }

        _dataPort.sendPacket(pkt);

        DPRINTF(FlexCPUCoreEvent, "Sent a memory request(pa: %#x)\n",
                                req->getPaddr());

        // When the result comes back from memory, call completeMemAccess
        outstandingMemReqs.emplace(pkt, [this, pkt, inst, context, trace_data,
                                         callback_func, now]
        {
            completeMemAccess(pkt, inst, context, trace_data, callback_func,
                              now);
        });

        return true;
    });

    memoryUnit.schedule();

    return true;
}

bool
FlexCPU::requestSplitMemRead(const RequestPtr& main,
                                       const RequestPtr& low,
                                       const RequestPtr& high,
                                       ThreadContext* tc, StaticInstPtr inst,
                                       weak_ptr<ExecContext> context,
                                       Trace::InstRecord* trace_data,
                                       MemCallback callback_func)
{
    // Request can be handled immediately
    if (main->getFlags().isSet(Request::NO_ACCESS)) {
        return false; // TODO figure out if this is even possible on split
                      //      requests?
    }

    panic_if(main->isMmappedIpr(), "SDCPU does not yet support split "
                                   "mmappedIpr.");

    SplitAccCtrlBlk* split_acc = new SplitAccCtrlBlk;

    split_acc->low = Packet::createRead(low);
    split_acc->high = Packet::createRead(high);

    split_acc->main = Packet::createRead(main);
    split_acc->main->allocate();
    split_acc->low->dataStatic(split_acc->main->getPtr<uint8_t>());
    split_acc->high->dataStatic(split_acc->main->getPtr<uint8_t>()
                                + low->getSize());

    Tick queue_time = curTick();

    memoryUnit.addRequest([this, low, tc, inst, context, trace_data,
                           callback_func, split_acc, queue_time]
    {
        if (context.expired()) {
            // No need to send req if squashed
            delete split_acc->low;
            split_acc->low = nullptr;
            if (!split_acc->high || split_acc->highReceived) {
                delete split_acc->main;
                delete split_acc;
            }
            return false;
        }

        Tick now = curTick();
        waitingForMem.sample(now - queue_time);

        _dataPort.sendPacket(split_acc->low);

        if (low->isLLSC()) {
            TheISA::handleLockedRead(tc, low);
        }
        DPRINTF(FlexCPUCoreEvent, "Sent a memory request(pa: %#x)\n",
                                low->getPaddr());

        // When the result comes back from memory, call completeMemAccess
        outstandingMemReqs.emplace(split_acc->low, [this, inst, context,
                                                   trace_data, callback_func,
                                                   split_acc, now]
        {
            completeMemAccess(split_acc->low, inst, context, trace_data,
                              callback_func, now, split_acc);
        });

        return false;
    });

    memoryUnit.addRequest([this, high, tc, inst, context, trace_data,
                           callback_func, split_acc, queue_time]
    {
        if (context.expired()) {
            // No need to send req if squashed
            delete split_acc->high;
            split_acc->high = nullptr;
            if (!split_acc->low || split_acc->lowReceived) {
                delete split_acc->main;
                delete split_acc;
            }
            return false;
        }

        Tick now = curTick();
        waitingForMem.sample(now - queue_time);

        _dataPort.sendPacket(split_acc->high);

        if (high->isLLSC()) {
            TheISA::handleLockedRead(tc, high);
        }
        DPRINTF(FlexCPUCoreEvent, "Sent a memory request(pa: %#x)\n",
                                high->getPaddr());

        // When the result comes back from memory, call completeMemAccess
        outstandingMemReqs.emplace(split_acc->high, [this, inst, context,
                                                     trace_data, callback_func,
                                                     split_acc, now]
        {
            completeMemAccess(split_acc->high, inst, context, trace_data,
                              callback_func, now, split_acc);
        });
        return true;
    });

    memoryUnit.schedule();

    return true;
}

bool
FlexCPU::requestSplitMemWrite(const RequestPtr& main,
                                        const RequestPtr& low,
                                        const RequestPtr& high,
                                        ThreadContext* tc, StaticInstPtr inst,
                                        weak_ptr<ExecContext> context,
                                        Trace::InstRecord* trace_data,
                                        uint8_t* data,
                                        MemCallback callback_func)
{
    // Request can be handled immediately
    if (main->getFlags().isSet(Request::NO_ACCESS)) {
        return false; // TODO figure out if this is even possible on split
                      //      requests?
    }

    panic_if(main->isMmappedIpr(), "SDCPU does not yet support split "
                                   "mmappedIpr.");

    SplitAccCtrlBlk* split_acc = new SplitAccCtrlBlk;

    // This pointer may be used in completeAcc. It probably doesn't need data.
    // But, we give it data, just to be safe.
    split_acc->main = Packet::createWrite(main);
    split_acc->main->allocate();

    split_acc->low = Packet::createWrite(low);
    split_acc->high = Packet::createWrite(high);

    if (data) {
        split_acc->main->setData(data);
        uint8_t* main_ptr = split_acc->main->getPtr<uint8_t>();
        split_acc->low->dataStatic(main_ptr);
        split_acc->high->dataStatic(main_ptr + low->getSize());
    } else { // Assume that no pointer given means we're filling with zero
        assert(main->getFlags() & Request::STORE_NO_DATA);
        // Maybe this line is important? Not all CPUs make this check, so I'm
        // leaving this commented out to see if it causes problems...
        // memset(mailbox, 0, req_size);
        panic("Should be unreachable...");
    }

    Tick queue_time = curTick();

    memoryUnit.addRequest([this, low, tc, inst, context, trace_data,
                           callback_func, split_acc, queue_time]
    {
        if (context.expired()) {
            // No need to send req if squashed
            delete split_acc->low;
            split_acc->low = nullptr;
            if (!split_acc->high || split_acc->highReceived) {
                delete split_acc->main;
                delete split_acc;
            }
            return false;
        }

        Tick now = curTick();
        waitingForMem.sample(now - queue_time);

        if (low->isLLSC()) {
            if (!TheISA::handleLockedWrite(tc, low, cacheBlockMask)) {
                panic("Haven't quite decided how to do this yet");
                // TODO kill the request
            }
        }

        _dataPort.sendPacket(split_acc->low);

        DPRINTF(FlexCPUCoreEvent, "Sent a memory request(pa: %#x)\n",
                                low->getPaddr());

        // When the result comes back from memory, call completeMemAccess
        outstandingMemReqs.emplace(split_acc->low, [this, inst, context,
                                                    trace_data, callback_func,
                                                    split_acc, now]
        {
            completeMemAccess(split_acc->low, inst, context, trace_data,
                              callback_func, now, split_acc);
        });

        return true;
    });

    memoryUnit.addRequest([this, high, tc, inst, context, trace_data,
                           callback_func, split_acc, queue_time]
    {
        if (context.expired()) {
            // No need to send req if squashed
            delete split_acc->high;
            split_acc->high = nullptr;
            if (!split_acc->low || split_acc->lowReceived) {
                delete split_acc->main;
                delete split_acc;
            }
            return false;
        }

        Tick now = curTick();
        waitingForMem.sample(now - queue_time);

        if (high->isLLSC()) {
            if (!TheISA::handleLockedWrite(tc, high, cacheBlockMask)) {
                panic("Haven't quite decided how to do this yet");
                // TODO kill the request
            }
        }

        _dataPort.sendPacket(split_acc->high);

        DPRINTF(FlexCPUCoreEvent, "Sent a memory request(pa: %#x)\n",
                                high->getPaddr());

        // When the result comes back from memory, call completeMemAccess
        outstandingMemReqs.emplace(split_acc->high, [this, inst, context,
                                                     trace_data, callback_func,
                                                     split_acc, now]
        {
            completeMemAccess(split_acc->high, inst, context, trace_data,
                              callback_func, now, split_acc);
        });

        return true;
    });

    memoryUnit.schedule();

    return true;
}

void
FlexCPU::startup()
{
    BaseCPU::startup();

    for (auto& thread : threads) {
        thread->startup();
    }
}

void
FlexCPU::suspendContext(ThreadID tid)
{
    // Need to de-schedule any instructions in the pipeline?
    BaseCPU::suspendContext(tid);
}

void
FlexCPU::haltContext(ThreadID tid)
{
    // Need to de-schedule any instructions in the pipeline?
    BaseCPU::haltContext(tid);
}

void
FlexCPU::switchOut()
{
    // TODO flush speculative state.
    // TODO clear any outstanding memory requests.

    BaseCPU::switchOut();
}

void
FlexCPU::takeOverFrom(BaseCPU* cpu)
{
    // TODO may want to ensure clean speculative state prior to attempting?

    BaseCPU::takeOverFrom(cpu); // BaseCPU::takeOverFrom() already handles the
                                // calls to ThreadContext::takeOverFrom() via
                                // the internal ThreadContext* table.

    // TODO load any other state.
}

Counter
FlexCPU::totalInsts() const
{
    Counter insts = 0;
    for (auto &thread : threads) {
        insts += thread->getNumInsts();
    }
    return insts;
}

Counter
FlexCPU::totalOps() const
{
    Counter ops = 0;
    for (auto &thread : threads) {
        ops += thread->getNumOps();
    }
    return ops;
}

void
FlexCPU::wakeup(ThreadID tid)
{
    assert(tid < numThreads);

    // Get the ThreadContext interface pointer from BaseCPU::getContext(), for
    // flexibility in how the ThreadContext is retrieved (in case we change
    // from using SimpleThread).
    ThreadContext* tc = getContext(tid);
    if (tc->status() == ThreadContext::Suspended) {
        tc->activate();
    }
}

bool
FlexCPU::DataPort::recvTimingResp(PacketPtr pkt)
{
    panic_if(!cpu->outstandingMemReqs.count(pkt),
        "Received a data port response we don't remember sending!");

    auto& callback = cpu->outstandingMemReqs[pkt];

    callback();

    cpu->outstandingMemReqs.erase(pkt);
    delete pkt;

    return true;
}

void
FlexCPU::DataPort::recvReqRetry()
{
    assert(blockedPkt);
    DPRINTF(FlexCPUCoreEvent, "data retrying %s\n", blockedPkt->print());

    PacketPtr pkt = blockedPkt;
    blockedPkt = nullptr;
    sendPacket(pkt);

    if (!blockedPkt) {
        DPRINTF(FlexCPUCoreEvent, "Sent successfully!\n");
        // If we're now unblocked, try to schedule other things from memoryunit
        cpu->memoryUnit.schedule();
    }
}

void
FlexCPU::DataPort::sendPacket(PacketPtr pkt)
{
    DPRINTF(FlexCPUCoreEvent, "Data sending %s\n", pkt->print());
    assert(!blockedPkt);
    if (!sendTimingReq(pkt)) {
        DPRINTF(FlexCPUCoreEvent, "Data failed to send %s\n",
                 pkt->print());
        blockedPkt = pkt;
    }
}

bool
FlexCPU::InstPort::recvTimingResp(PacketPtr pkt)
{
    DPRINTF(FlexCPUCoreEvent, "Received _instPort response (pa: %#x)\n",
                            pkt->req->getPaddr());

    panic_if(!cpu->outstandingFetches.count(pkt),
        "Received an instruction port response we don't remember sending!");

    const FetchCallback& callback = cpu->outstandingFetches[pkt];

    // Right now, this function is assuming that the request function will set
    // up the callback in a way that cleans up after itself. Since this
    // function never calls new, we don't explicitly manage pointer lifespan
    // here.
    callback(pkt->getPtr<uint8_t>());

    cpu->outstandingFetches.erase(pkt);
    delete pkt;

    return true;
}

void
FlexCPU::InstPort::recvReqRetry()
{
    assert(blockedPkt);
    DPRINTF(FlexCPUCoreEvent, "inst retrying %s\n", blockedPkt->print());

    PacketPtr pkt = blockedPkt;
    blockedPkt = nullptr;
    sendPacket(pkt);

    if (!blockedPkt) {
        DPRINTF(FlexCPUCoreEvent,  "Sent successfully!\n");
        // If we're now unblocked, try to schedule other things from memoryunit
        cpu->instFetchUnit.schedule();
    }
}

void
FlexCPU::InstPort::sendPacket(PacketPtr pkt)
{
    DPRINTF(FlexCPUCoreEvent, "Inst sending %s\n", pkt->print());
    assert(!blockedPkt);
    if (!sendTimingReq(pkt)) {
        DPRINTF(FlexCPUCoreEvent, "Inst failed to send %s\n",
                pkt->print());
        blockedPkt = pkt;
    }
}

FlexCPU::Resource::Resource(FlexCPU *cpu,
    Cycles latency, int bandwidth, std::string _name, bool run_last) :
    cpu(cpu), latency(latency), bandwidth(bandwidth),
    _name(_name),
    attemptAllEvent(EventFunctionWrapper([this]{ attemptAllRequests(); },
                    name() + ".attemptAll", false,
                    run_last ? Event::CPU_Tick_Pri : Event::Default_Pri))
{
    //
}

void
FlexCPU::Resource::addRequest(
    const std::function<bool ()>& run_function)
{
    DPRINTF(FlexCPUCoreEvent, "Adding request. %d on queue\n",
                              requests.size());
    requests.push_back(run_function);
}

void
FlexCPU::Resource::attemptAllRequests()
{
//	Cycles tmpLatency; // [YE] - moved back
    DPRINTF(FlexCPUCoreEvent, "Attempting all requests. %d on queue\n",
            requests.size());

    if (curTick() != lastActiveTick) {
        assert(!bandwidth || usedBandwidth <= bandwidth);

        // Stats
        activeCycles++;
        bandwidthPerCycle.sample(usedBandwidth);

        // Reset the bandwidth since we're on a new cycle.
        DPRINTF(FlexCPUCoreEvent,
                "resetting the bandwidth. Used %d last cycle\n",
                usedBandwidth);
        usedBandwidth = 0;
        lastActiveTick = curTick();

        cpu->markActiveCycle();
    }

    if (requests.empty()) {
        return; // This happens with 0 latency events if a request enqueues
                // another request.
    }

    while (!requests.empty() && resourceAvailable()) {
        DPRINTF(FlexCPUCoreEvent, "Running request. %d left in queue. "
                "%d this cycle\n", requests.size(), usedBandwidth);
        auto& req = requests.front();

        DPRINTF(FlexCPUCoreEvent, "Executing request directly\n");
        if (req()) usedBandwidth++;

        requests.pop_front();
    }

    if (!requests.empty()) {
        // There's more thing to execute so reschedule the event for next time
        DPRINTF(FlexCPUCoreEvent, "Rescheduling resource\n");
        Tick next_time;
//        if (latency == 0) {
//            next_time = cpu->nextCycle();
//        } else {
//		 next_time = cpu->clockEdge((latency));
//
//
////        	tmpLatency = latency + reqCycle.front() - cpu->curCycle(); // [YE] - moved back
////        	tmpLatency = tmpLatency < Cycles(1) ? Cycles(1) : tmpLatency;
////        	//std::cout << "YE latency " << tmpLatency << "\n";
////        	//std::cout << "YE req" << reqCycle.front() << "\n";
////            next_time = cpu->clockEdge(Cycles(tmpLatency));
//        }
        next_time = cpu->nextCycle(); // YE

        assert(next_time != curTick());
        // Note: it could be scheduled if one of the requests above schedules
        // This "always" reschedules since it may or may not be on the queue
        // right now.
        cpu->reschedule(&attemptAllEvent, next_time, true);
    }
}

bool
FlexCPU::Resource::resourceAvailable()
{
    // Bandwidth of 0 implies infinite
    if (!bandwidth || usedBandwidth < bandwidth) {
        return true;
    } else {
        return false;
    }
}

void
FlexCPU::Resource::schedule()
{
    // Note: on retries from memory we could try to schedule this even though
    //       the list of requests is empty. Revist this after implementing an
    //       LSQ.
    DPRINTF(FlexCPUCoreEvent, "Trying to schedule resource\n");
    if (!requests.empty() && !attemptAllEvent.scheduled()) {
        DPRINTF(FlexCPUCoreEvent, "Scheduling attempt all\n");
        // [YE] - add latency between stages
        cpu->schedule(&attemptAllEvent, cpu->clockEdge(latency));
    }
}


FlexCPU::MemoryResource::MemoryResource(FlexCPU *cpu,
    int bandwidth, std::string _name) :
    Resource(cpu, Cycles(0), bandwidth, _name)
{ }


bool
FlexCPU::MemoryResource::resourceAvailable()
{
    if (cpu->_dataPort.blocked()) {
        return false;
    } else {
        return Resource::resourceAvailable();
    }
}

void
FlexCPU::regStats()
{
    dataAddrTranslationUnit.regStats();
    executionUnit.regStats();
    instFetchUnit.regStats();
    instAddrTranslationUnit.regStats();
    issueUnit.regStats();
    memoryUnit.regStats();

    issueThreadUnit.regStats(); // YE - need to be used

    memLatency
        .name(name() + ".memLatency")
        .init(16)
        .desc("Latency for memory requests from send to receive")
        ;
    waitingForDataXlation
        .name(name() + ".waitingForDataXlation")
        .init(16)
        .desc("Time waiting for resource to be free")
        ;
    waitingForExecution
        .name(name() + ".waitingForExecution")
        .init(16)
        .desc("Time waiting for resource to be free")
        ;
    waitingForInstXlation
        .name(name() + ".waitingForInstXlation")
        .init(16)
        .desc("Time waiting for resource to be free")
        ;
    waitingForInstData
        .name(name() + ".waitingForInstData")
        .init(16)
        .desc("Time waiting for resource to be free")
        ;
    waitingForMem
        .name(name() + ".waitingForMem")
        .init(16)
        .desc("Time waiting for resource to be free")
        ;

    BaseCPU::regStats();
}

void
FlexCPU::Resource::regStats()
{
    activeCycles
        .name(name() + ".activeCycles")
        .desc("Number of cycles this resource was active")
        ;

    bandwidthPerCycle
        .name(name() + ".bandwidthPerCycle")
        .desc("For each active cycles, the number of times the resource was"
              "used")
        .init(8)
    ;
}

FlexCPU*
FlexCPUParams::create()
{
    return new FlexCPU(this);
}
