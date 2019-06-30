
#ifndef __CPU_RIO_BUFFER_HH__
#define __CPU_RIO_BUFFER_HH__

#include "cpu/timebuf.hh"

namespace Rio
{

class RioBufferComply
{
public:
	virtual bool isBubble() const = 0;
};

/** TimeBuffer with MinorTrace and Named interfaces */
template <typename ElemType>
class RioBuffer : public Named, public TimeBuffer<ElemType>
{
  protected:
    /** The range of elements that should appear in trace lines */
    int reportLeft, reportRight;

  public:
    RioBuffer(const std::string &name,
        int num_past, int num_future,
        int report_left = -1, int report_right = -1) :
        Named(name), TimeBuffer<ElemType>(num_past, num_future),
        reportLeft(report_left), reportRight(report_right)
    { }

  public:

    /* Is this buffer full of only bubbles */
    bool empty() const
    {
        bool ret = true;

        for (int i = -this->past; i <= this->future; i++) {
            //if (!BubbleTraits::isBubble((*this)[i]))
            if (!((*this)[i]).isBubble())
                ret = false;
        }

        return ret;
    }

};

/** Wraps a MinorBuffer with Input/Output interfaces to ensure that units
 *  within the model can only see the right end of buffers between them. */
template <typename Data>
class RioLatch
{
  public:
    typedef RioBuffer<Data> Buffer;

  protected:
    /** Delays, in cycles, writing data into the latch and seeing it on the
     *  latched wires */
    Cycles delay;

    Buffer buffer;

  public:
    /** forward/backwardDelay specify the delay from input to output in each
     *  direction.  These arguments *must* be >= 1 */
    RioLatch(const std::string &name,
        Cycles delay_ = Cycles(1),
        bool report_backwards = false) :
        delay(delay_),
        buffer(name, delay_, 0, (report_backwards ? -delay_ : 0),
            (report_backwards ? 0 : -delay_))
    { }

  public:
    /** Encapsulate wires on either input or output of the latch.
     *  forward/backward correspond to data direction relative to the
     *  pipeline.  Latched and Immediate specify delay for backward data.
     *  Immediate data is available to earlier stages *during* the cycle it
     *  is written */
    class Input
    {
      public:
        typename Buffer::wire inputWire;

      public:
        Input(typename Buffer::wire input_wire) :
            inputWire(input_wire)
        { }
    };

    class Output
    {
      public:
        typename Buffer::wire outputWire;

      public:
        Output(typename Buffer::wire output_wire) :
            outputWire(output_wire)
        { }
    };

    bool empty() const { return buffer.empty(); }

    /** An interface to just the input of the buffer */
    Input input() { return Input(buffer.getWire(0)); }

    /** An interface to just the output of the buffer */
    Output output() { return Output(buffer.getWire(-delay)); }


    void evaluate() { buffer.advance(); }
};

}

#endif // __CPU_RIO_BUFFER_HH__
