namespace Caster {

template <typename Event>
void Bus<Event>::loop() {
    for (uint8_t i = 0; i < count_; i++) {
        yield_.emitter_ = i;
        nodes_[i]->emit(yield_);
    }
}

template <typename Event>
void Bus<Event>::YieldImpl::operator()(const Event& event) const {
    for (uint8_t i = 0; i < bus_->count_; i++) {
        if (emitter_ != i) {
            bus_->nodes_[i]->handle(event);
        }
    }
}

}  // namespace Caster
