# Metrics

Mercury exports metrics in a Prometheus-friendly text format.

## Core counters

- `mercury_messages_total`
- `mercury_invalid_messages_total`
- `mercury_duplicate_messages_total`
- `mercury_gap_events_total`
- `mercury_latency_samples_total`

## Gauges

- `mercury_throughput_msgs_per_sec`
- `mercury_last_latency_ns`
- `mercury_order_books_tracked`

## Histograms

Latency buckets are emitted in nanoseconds and intended to be scraped by an external metrics endpoint wrapper or written to disk during offline replay.
