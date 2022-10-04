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

        uint8_t init_ = 0;
        uint8_t emit0_ = 0;
        uint8_t emit1_ = 0;
        uint8_t* handle_;
        uint8_t handle_emit_ = 0;
        int handle_size_;
        int handle_count_;

        ~TestNode() {
            delete[] handle_;
        }

        void init(const Yield<uint8_t>& yield) override {
            if (init_ != 0) {
                yield(init_);
            }
        }

        void handle(const uint8_t& event, const Yield<uint8_t>& yield) override {
            if (handle_count_ < handle_size_) {
                handle_[handle_count_] = event;
            }
            if (handle_emit_ != 0) {
                yield(handle_emit_);
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

test(BusTest, Init) {
    TestNode n1 = TestNode(1);
    n1.init_ = 1;

    TestNode n2 = TestNode(1);
    n2.init_ = 2;

    Node<uint8_t>* nodes[] = {&n1, &n2};
    auto bus = Bus<uint8_t>(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.init();

    assertEqual(n1.handle_count_, 1);
    assertEqual(n1.handle_[0], 2);
    assertEqual(n2.handle_count_, 1);
    assertEqual(n2.handle_[0], 1);
}

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

test(BusTest, HandleYield) {
    TestNode n1 = TestNode(1);
    n1.emit0_ = 1;
    n1.handle_emit_ = 11;
    TestNode n2 = TestNode(2);
    n2.emit0_ = 2;

    Node<uint8_t>* nodes[] = {&n1, &n2};
    auto bus = Bus<uint8_t>(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    assertEqual(n1.handle_count_, 1);
    assertEqual(n1.handle_[0], n2.emit0_);
    assertEqual(n2.handle_count_, 2);
    assertEqual(n2.handle_[0], n1.emit0_);
    assertEqual(n2.handle_[1], n1.handle_emit_);
}

test(BusTest, Emit) {
    TestNode n1 = TestNode(1);
    TestNode n2 = TestNode(1);

    Node<uint8_t>* nodes[] = {&n1, &n2};
    auto bus = Bus<uint8_t>(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    bus.emit(4);
    assertEqual(n1.handle_count_, 1);
    assertEqual(n1.handle_[0], 4);
    assertEqual(n2.handle_count_, 1);
    assertEqual(n2.handle_[0], 4);
}

test(BusTest, EmitRecursive) {
    TestNode n1 = TestNode(2);
    n1.handle_emit_ = 11;
    TestNode n2 = TestNode(2);

    Node<uint8_t>* nodes[] = {&n1, &n2};
    auto bus = Bus<uint8_t>(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    bus.emit(4);
    assertEqual(n1.handle_count_, 1);
    assertEqual(n1.handle_[0], 4);
    assertEqual(n2.handle_count_, 2);
    assertEqual(n2.handle_[0], 11);
    assertEqual(n2.handle_[1], 4);
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

