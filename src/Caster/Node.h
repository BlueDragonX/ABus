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

        // Handle a bus event.
        virtual void handle(const Event& event) = 0;

        // Called during each loop iteration to emit events to the bus. The
        // emitter calls the provided yield functor to broadcast an event.
        virtual void emit(const Yield<Event>& yield) = 0;
};

}  // namespace Caster

#endif  // _CASTER_NODE_H_
