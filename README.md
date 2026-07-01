# ⚡ Order Matching Engine (C++)

A **Limit Order Book Matching Engine** implemented in modern C++17, simulating the core matching logic of an exchange: limit orders, market orders, order cancellation, and price-time priority matching — the same fundamental problem that sits at the heart of every trading venue's infrastructure.

This project was forked and extended from a base skeleton; the core matching algorithms (price-time priority matching, market order sweeping, and lazy deletion for cancelled/filled orders) were designed and implemented from scratch as part of this work.

---

## 🚀 Features

- **Limit Orders** — Add buy/sell limit orders; they either match immediately against crossing orders or rest on the book.
- **Market Orders** — Execute immediately against the best available price(s), sweeping through multiple price levels if needed to fill quantity.
- **Price-Time Priority Matching** — Orders are matched by best price first; ties are resolved in insertion order.
- **Order Cancellation** — Cancel resting orders by ID using a lazy-deletion pattern (no expensive mid-heap removal).
- **Order Book View** — Print the top N bid/ask price levels at any point.
- **Stress Testing** — Benchmarked against 1,000,000 randomly generated orders.

---

## 🏗️ How the Matching Works

The engine uses two `std::priority_queue`s (one max-heap for bids, one min-heap-by-price for asks) backed by an `std::unordered_map<int, Order>` (`activeOrders`) that holds the *authoritative* state of every live order.

`std::priority_queue` in C++ doesn't support efficient removal or update of arbitrary elements — only the top. To work around this without sacrificing performance, the engine uses **lazy deletion**:

1. When an order is partially filled, cancelled, or fully filled, its state is updated (or removed) only in `activeOrders` — the priority queue is left untouched.
2. When the matching loop later pops an entry off the queue, it cross-checks that order's ID against `activeOrders`. If it's stale (cancelled, or already fully filled), it's discarded and the loop moves to the next entry.
3. If it's still live, the queue's copy is reconciled against the authoritative state in the map before matching continues.

This keeps every operation at `O(log n)` amortized, without needing a more complex indexed heap or balanced tree structure.

Matching logic implemented:
- `matchBuy` / `matchSell` — limit order matching, stopping as soon as the best opposing price no longer crosses.
- `matchMarketBuy` / `matchMarketSell` — market order matching, ignoring price and sweeping the book until the order is filled or liquidity runs out.

---

## 📊 Performance Benchmark

```text
Machine: WSL2 (Ubuntu on Windows)
Command: ./matching

Running stress test with 1,000,000 orders...
Processed 1,000,000 orders in ~600-850 ms
```

This is a single-threaded, non-optimized baseline — no lock-free structures, no memory pooling, no CPU pinning. It's meant to demonstrate correct matching logic and algorithmic complexity rather than raw low-latency engineering.

---

## 🛠️ Project Structure

```
OrderMatchingEngine/
├── src/
│   ├── main.cpp              # Entry point (demo + stress test)
│   ├── order.h                # Order struct
│   ├── orderbook.h             # OrderBook class declaration
│   └── matching_engine.cpp     # OrderBook implementation (matching logic)
├── tests/
│   └── test_orderbook.cpp      # Unit tests
├── Makefile                    # Build automation
└── README.md                   # Project documentation
```

---

## ▶️ Getting Started

### Prerequisites
- C++17-compatible compiler (`g++` or `clang++`)
- `make`

### Build & Run
```bash
git clone https://github.com/<your-username>/OrderMatchingEngine.git
cd OrderMatchingEngine
make
./matching
```

### Example Output
```text
----- Order Book (Top 5) -----
Bids:
  101 x 10 (ID 1)
Asks:
  102 x 5 (ID 2)
------------------------------
Running stress test with 1000000 orders...
Processed 1000000 orders in 699 ms
```

---

## 🔮 Future Improvements

- Multi-asset support (multiple symbols/order books simultaneously)
- Advanced order types (Stop-loss, Iceberg, Fill-or-Kill, IOC)
- Multi-threaded / lock-free order ingestion
- Persistent trade log (file or database)
- Basic latency instrumentation (p50/p99 per order)
- TCP-based order entry gateway

---

## 📜 License

This project is licensed under the MIT License — see the `LICENSE` file for details.
