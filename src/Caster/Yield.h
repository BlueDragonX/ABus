#ifndef _CASTER_YIELD_H_
#define _CASTER_YIELD_H_

namespace Caster {

// Callable used to yield events to a bus.
template <typename Event>
class Yield {
    public:
        Yield() = default;
        virtual ~Yield() = default;

        // Yield an event.
        virtual void operator()(const Event& event) const = 0;
};

}  // namespace Caster

#endif  // _CASTER_YIELD_H_
