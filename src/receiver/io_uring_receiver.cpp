#include "mercury/receiver/io_uring_receiver.hpp"

namespace mercury::receiver {

bool IoUringReceiver::start() {
    running_ = true;
    return false;
}

void IoUringReceiver::stop() { running_ = false; }

} // namespace mercury::receiver
