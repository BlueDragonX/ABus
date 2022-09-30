#ifndef _CASTER_BUS_H_
#define _CASTER_BUS_H_

#include <Arduino.h>
#include "Node.h"
#include "Yield.h"

namespace Caster {

// Bus connects a series of nodes which send and receive events. Events are
// received sequentially from the connected nodes. Any time an event is
// received it is broadcast to all connected nodes, including the originator of
// the event.
template <typename Event>
class Bus {
    public:
        // Construct a bus that connects the provided set of nodes. Count is
        // the number of nodes in the array.
        Bus(Node<Event>** nodes, uint8_t count) :
            nodes_(nodes), count_(count), yield_(this) {}

        // Called on each main loop iteration. Calls receive on each node and
        // broadcasts any received events.
        void loop();

        // Emit an external event directly to the bus.
        void emit(const Event& event);

    private:
        class YieldImpl : public Yield<Event> {
            public:
                void operator()(const Event& event) const override;

            private:
                Bus* bus_;
                uint8_t emitter_;
                YieldImpl(Bus* bus) : bus_(bus) {}
                friend class Bus;
        };

        Node<Event>** nodes_;
        uint8_t count_;
        YieldImpl yield_;
};

}  // namespace Caster

#include "Bus.tpp"

#endif  // _CASTER_BUS_H_
