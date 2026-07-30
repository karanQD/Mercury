#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <pthread.h>
#include <sched.h>

#include "mercury/consumer/latency_logger.hpp"
#include "mercury/consumer/metrics_consumer.hpp"
#include "mercury/consumer/order_book.hpp"
#include "mercury/parser/itch50_parser.hpp"
#include "mercury/parser/message_validator.hpp"
#include "mercury/receiver/pcap_receiver.hpp"
#include "mercury/router/symbol_router.hpp"
#include "mercury/timestamp/timestamp_engine.hpp"

namespace {

void pin_this_thread(const int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    (void)pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

std::vector<std::uint8_t> build_demo_add_message() {
    std::vector<std::uint8_t> msg(36U, 0U);
    msg[0] = 'A';
    msg[4] = 1U;
    msg[10] = 1U;
    msg[18] = 'B';
    msg[22] = 10U;
    std::memcpy(msg.data() + 23U, "AAPL    ", 8U);
    msg[34] = 0x27U;
    msg[35] = 0x10U;
    return msg;
}

} // namespace

int main(int argc, char** argv) {
    pin_this_thread(0);

    mercury::timestamp::TimestampEngine ts;
    ts.calibrate();

    mercury::parser::Itch50Parser parser;
    mercury::parser::MessageValidator validator;
    mercury::router::SymbolRouter router;
    mercury::consumer::LatencyLogger latency_logger;
    mercury::consumer::OrderBook order_book;
    mercury::consumer::MetricsConsumer metrics;

    for (std::size_t worker = 0; worker < mercury::router::SymbolRouter::kWorkerCount; ++worker) {
        router.set_worker_callback(worker, [&](const mercury::normalizer::MarketEvent& event) {
            auto published = event;
            published.published_ns = ts.now_ns();
            latency_logger.record(published);
            order_book.on_event(published);
            metrics.on_event(published);
        });
    }

    auto process_payload = [&](std::span<const std::uint8_t> payload) {
        const auto arrival = ts.now_ns();
        if (payload.empty()) {
            return;
        }
        const auto raw = validator.validate_raw(static_cast<char>(payload[0]), payload.size());
        if (!raw.ok) {
            metrics.on_invalid();
            return;
        }
        if (!parser.parse(payload)) {
            metrics.on_invalid();
            return;
        }
        auto event = parser.get_last_event();
        event.packet_arrival_ns = arrival;
        event.parser_complete_ns = ts.now_ns();
        event.validator_complete_ns = event.parser_complete_ns;
        router.route(event);
    };

    if (argc == 3 && std::string(argv[1]) == "--pcap") {
        mercury::receiver::PcapReplayReceiver receiver(argv[2]);
        receiver.set_on_data(process_payload);
        if (!receiver.start()) {
            std::cerr << "failed to replay pcap\n";
            return 1;
        }
    } else {
        const auto msg = build_demo_add_message();
        process_payload(msg);
    }

    std::cout << metrics.render_prometheus();
    return 0;
}
