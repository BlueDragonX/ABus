#include <Arduino.h>
#include <AUnit.h>
#include <ABus.h>

using namespace aunit;
using namespace ABus;

// Test boilerplate.
void setup() {
#ifdef ARDUINO
    delay(1000);
#endif
    SERIAL_PORT_MONITOR.begin(115200);
    while(!SERIAL_PORT_MONITOR);
}

void loop() {
    TestRunner::run();
    delay(1);
}

// A mock bus node for testing.
class MockNode : public Node<uint8_t> {
    public:
        MockNode(int buffer_size) :
            send_(new uint8_t[buffer_size]),
            send_size_(buffer_size),
            send_count_(0) {
            for (int i = 0; i < send_size_; i++) {
                send_[i] = 0;
            }
        }

        uint8_t receive_ = 0;
        uint8_t receive_extra_ = 0;
        uint8_t* send_;
        int send_size_;
        int send_count_;
        int filter1_ = 0;
        int filter2_ = 0;

        ~MockNode() {
            delete send_;
        }

        void receive(const Broadcast<uint8_t>& broadcast) override {
            if (receive_ != 0) {
                broadcast(receive_);
            }
            if (receive_extra_ != 0) {
                broadcast(receive_extra_);
            }
        }

        void send(const uint8_t& event) override {
            if (send_count_ < send_size_) {
                send_[send_count_] = event;
            }
            ++send_count_;
        }

        bool filter(Node<uint8_t>* sender, const uint8_t& id) const override {
            return filter1_ == -1 || filter1_ == id || filter2_ == id;
        }
};

test(BusTest, SingleBroadcast) {
    MockNode n1 = MockNode(1);
    n1.receive_ = 1;

    MockNode n2 = MockNode(1);
    n2.receive_ = 0;
    n2.filter1_ = 1;

    MockNode n3 = MockNode(1);
    n3.receive_ = 0;
    n3.filter1_ = 1;

    Node<uint8_t>* nodes[] = {&n1, &n2, &n3};

    auto bus = Bus<uint8_t>(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    assertEqual(n1.send_count_, 0);
    assertEqual(n2.send_count_, 1);
    assertEqual(n1.receive_, n2.send_[0]);
    assertEqual(n3.send_count_, 1);
    assertEqual(n1.receive_, n3.send_[0]);
}

test(BusTest, MultiBroadcast) {
    MockNode n1 = MockNode(1);
    n1.receive_ = 1;
    n1.filter1_ = 2;

    MockNode n2 = MockNode(1);
    n2.receive_ = 2;
    n2.filter1_ = 1;

    MockNode n3 = MockNode(2);
    n3.receive_ = 0;
    n3.filter1_ = 1;
    n3.filter2_ = 2;

    Node<uint8_t>* nodes[] = {&n1, &n2, &n3};

    auto bus = Bus<uint8_t>(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    assertEqual(n1.send_count_, 1);
    assertEqual(n2.receive_, n1.send_[0]);
    assertEqual(n2.send_count_, 1);
    assertEqual(n1.receive_, n2.send_[0]);
    assertEqual(n3.send_count_, 2);
    assertEqual(n1.receive_, n3.send_[0]);
    assertEqual(n2.receive_, n3.send_[1]);
}

test(BusTest, MultiReceive) {
    MockNode n1 = MockNode(1);
    n1.receive_ = 1;
    n1.receive_extra_ = 2;

    MockNode n2 = MockNode(2);
    n2.receive_ = 0;
    n2.filter1_ = 1;
    n2.filter2_ = 2;

    Node<uint8_t>* nodes[] = {&n1, &n2};

    auto bus = Bus<uint8_t>(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    assertEqual(n1.send_count_, 0);
    assertEqual(n2.send_count_, 2);
    assertEqual(n1.receive_, n2.send_[0]);
    assertEqual(n1.receive_extra_, n2.send_[1]);
}

test(BusTest, MultiLoop) {
    MockNode n1 = MockNode(1);
    n1.receive_ = 1;

    MockNode n2 = MockNode(1);
    n2.receive_ = 0;

    MockNode n3 = MockNode(3);
    n3.receive_ = 0;
    n3.filter1_ = 1;
    n3.filter2_ = 2;

    Node<uint8_t>* nodes[] = {&n1, &n2, &n3};

    auto bus = Bus<uint8_t>(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    assertEqual(n1.send_count_, 0);
    assertEqual(n2.send_count_, 0);
    assertEqual(n3.send_count_, 1);
    assertEqual(n1.receive_, n3.send_[0]);

    n2.receive_ = 2;

    bus.loop();

    assertEqual(n1.send_count_, 0);
    assertEqual(n2.send_count_, 0);
    assertEqual(n3.send_count_, 3);
    assertEqual(n1.receive_, n3.send_[0]);
    assertEqual(n1.receive_, n3.send_[1]);
    assertEqual(n2.receive_, n3.send_[2]);
}
