# Latency Budget

| Stage | Budget | Notes |
|---|---:|---|
| Receiver header strip | 50ns | L2/L3/L4 removal only |
| Parse + normalize | 120ns | endian conversion + field extraction |
| Validation | 80ns | type/length/duplicate/gap checks |
| Routing | 60ns | symbol hash and worker dispatch |
| Consumer work | 150ns | logger/book/metrics |
| Total | 460ns | leaves headroom for jitter |

The budget intentionally focuses on tail latency. Any optimization that worsens P99.9 to improve average throughput should be treated with suspicion.
