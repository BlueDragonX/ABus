#ifndef _ABUS_H_
#define _ABUS_H_

#include <Arduino.h>

namespace Caster {

// Callable used by nodes to broadcast events to the bus.
template <typename T>
class Broadcast {
    public:
        Broadcast() = default;
        virtual ~Broadcast() = default;

        // Broadcast an event.
        virtual void operator()(const T& event) const = 0;
};

// A bus node. The bus receives events from nodes. Events received from nodes
// are broadcast to all nodes on the bus.
template <typename T>
class Node {
    public:
        Node() = default;
        virtual ~Node() = default;

        // Receive events from a node. The node calls broadcast() to put events
        // on the bus. Events are sent to all other nodes whose filters matches
        // the broadcast event. The event is not sent back to the broadcasting
        // node.
        virtual void receive(const Broadcast<T>& broadcast) = 0;

        // Send an event to the node. Every received event is sent to every node
        // on the bus whose filter method returns true. A node is not required
        // to process an event.
        virtual void send(const T& event) = 0;

        // Filter events to this node. Return true for events that should be
        // sent to this node.
        virtual bool filter(const T& event) const { return true; }
};

// Bus connects a series of nodes which send and receive events. Events are
// received sequentially from the connected nodes. Any time an event is
// received it is broadcast to all connected nodes, including the originator of
// the event.
template <typename T>
class Bus {
    public:
        // Construct a bus that connects the provided set of nodes. Count is
        // the number of nodes in the array.
        Bus(Node<T>** nodes, uint8_t count) : nodes_(nodes), count_(count), broadcast_(this) {}

        // Called on each main loop iteration. Calls receive on each node and
        // broadcasts any received events.
        void loop();

    private:
        class BroadcastImpl : public Broadcast<T> {
            public:
                void operator()(const T& event) const override;

            private:
                Bus* bus_;
                uint8_t sender_;
                BroadcastImpl(Bus* bus) : bus_(bus) {}
                friend class Bus;
        };


        Node<T>** nodes_;
        uint8_t count_;
        BroadcastImpl broadcast_;
};

template <typename T>
void Bus<T>::loop() {
    for (uint8_t i = 0; i < count_; i++) {
        broadcast_.sender_ = i;
        nodes_[i]->receive(broadcast_);
    }
}

template <typename T>
void Bus<T>::BroadcastImpl::operator()(const T& event) const {
    for (uint8_t i = 0; i < bus_->count_; i++) {
        if (sender_ != i && bus_->nodes_[i]->filter(event)) {
            bus_->nodes_[i]->send(event);
        }
    }
}

}  // namespace Caster

#endif  // _ABUS_H_
