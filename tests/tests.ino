#include <Arduino.h>
#include <AUnit.h>
#include <Caster.h>

using namespace aunit;

namespace Caster {

// A mock bus node for testing.
class TestNode : public Node<uint8_t> {
    public:
        TestNode(int buffer_size) :
            handle_(new uint8_t[buffer_size]),
            handle_size_(buffer_size),
            handle_count_(0) {
            for (int i = 0; i < handle_size_; i++) {
                handle_[i] = 0;
            }
        }

        uint8_t emit0_ = 0;
        uint8_t emit1_ = 0;
        uint8_t* handle_;
        int handle_size_;
        int handle_count_;

        ~TestNode() {
            delete[] handle_;
        }

        void handle(const uint8_t& event) override {
            if (handle_count_ < handle_size_) {
                handle_[handle_count_] = event;
            }
            ++handle_count_;
        }

        void emit(const Yield<uint8_t>& yield) override {
            if (emit0_ != 0) {
                yield(emit0_);
            }
            if (emit1_ != 0) {
                yield(emit1_);
            }
        }
};

test(BusTest, SingleBroadcast) {
    TestNode n1 = TestNode(1);
    n1.emit0_ = 1;

    TestNode n2 = TestNode(1);
    n2.emit0_ = 0;

    TestNode n3 = TestNode(1);
    n3.emit0_ = 0;

    Node<uint8_t>* nodes[] = {&n1, &n2, &n3};
    auto bus = Bus<uint8_t>(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    assertEqual(n1.handle_count_, 0);
    assertEqual(n2.handle_count_, 1);
    assertEqual(n1.emit0_, n2.handle_[0]);
    assertEqual(n3.handle_count_, 1);
    assertEqual(n1.emit0_, n3.handle_[0]);
}

test(BusTest, MultiBroadcast) {
    TestNode n1 = TestNode(1);
    n1.emit0_ = 1;

    TestNode n2 = TestNode(1);
    n2.emit0_ = 2;

    TestNode n3 = TestNode(2);
    n3.emit0_ = 0;

    Node<uint8_t>* nodes[] = {&n1, &n2, &n3};
    auto bus = Bus<uint8_t>(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    assertEqual(n1.handle_count_, 1);
    assertEqual(n2.emit0_, n1.handle_[0]);
    assertEqual(n2.handle_count_, 1);
    assertEqual(n1.emit0_, n2.handle_[0]);
    assertEqual(n3.handle_count_, 2);
    assertEqual(n1.emit0_, n3.handle_[0]);
    assertEqual(n2.emit0_, n3.handle_[1]);
}

test(BusTest, MultiReceive) {
    TestNode n1 = TestNode(1);
    n1.emit0_ = 1;
    n1.emit1_ = 2;

    TestNode n2 = TestNode(2);
    n2.emit0_ = 0;

    Node<uint8_t>* nodes[] = {&n1, &n2};
    auto bus = Bus<uint8_t>(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    assertEqual(n1.handle_count_, 0);
    assertEqual(n2.handle_count_, 2);
    assertEqual(n1.emit0_, n2.handle_[0]);
    assertEqual(n1.emit1_, n2.handle_[1]);
}

test(BusTest, MultiLoop) {
    TestNode n1 = TestNode(1);
    n1.emit0_ = 1;

    TestNode n2 = TestNode(1);
    n2.emit0_ = 0;

    TestNode n3 = TestNode(3);
    n3.emit0_ = 0;

    Node<uint8_t>* nodes[] = {&n1, &n2, &n3};
    auto bus = Bus<uint8_t>(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    assertEqual(n1.handle_count_, 0);
    assertEqual(n2.handle_count_, 1);
    assertEqual(n1.emit0_, n2.handle_[0]);
    assertEqual(n3.handle_count_, 1);
    assertEqual(n1.emit0_, n3.handle_[0]);

    n2.emit0_ = 2;

    bus.loop();

    assertEqual(n1.handle_count_, 1);
    assertEqual(n2.emit0_, n1.handle_[0]);
    assertEqual(n2.handle_count_, 2);
    assertEqual(n1.emit0_, n2.handle_[0]);
    assertEqual(n1.emit0_, n2.handle_[0]);
    assertEqual(n3.handle_count_, 3);
    assertEqual(n1.emit0_, n3.handle_[0]);
    assertEqual(n1.emit0_, n3.handle_[1]);
    assertEqual(n2.emit0_, n3.handle_[2]);
}

}  // namespace Caster

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

