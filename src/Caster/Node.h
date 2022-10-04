#ifndef _CASTER_NODE_H_
#define _CASTER_NODE_H_

#include "Yield.h"

namespace Caster {

// Interface for classes which handle bus events.
template <typename Event>
class Node {
    public:
        Node() = default;
        virtual ~Node() = default;

        // Initialize the node. Called by bus.init(). The node may yield
        // messages at this time.
        virtual void init(const Yield<Event>&) {};

        // Handle bus events. This is called on each call to yield() by other
        // nodes. Optionally respond by yielding additional events to be
        // handled by the other nodes.
        //
        // This method is effectively recursive. If another node yields an
        // event in response to this node's yield call then this node will have
        // its handle method called again before the initial call has returned.
        // For this reason it is important that the handle method filter
        // incoming events in such a way that infinite recursion is avoided.
        //
        // Calling yield halts processing in this node until the other nodes
        // return from their handle calls.
        virtual void handle(const Event& event, const Yield<Event>& yield) = 0;

        // Called during each loop iteration to emit events to the bus. The
        // emitter calls the provided yield functor to broadcast an event.
        //
        // Calling yield halts processing in this node until the other nodes
        // return from their handle calls.
        virtual void emit(const Yield<Event>& yield) = 0;
};

}  // namespace Caster

#endif  // _CASTER_NODE_H_
