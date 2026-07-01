#include "orderbook.h"

// Comparator implementations
bool BuyCompare::operator()(const Order& a, const Order& b) {
    return a.price < b.price;
}
bool SellCompare::operator()(const Order& a, const Order& b) {
    return a.price > b.price;
}

// OrderBook member functions
int OrderBook::addOrder(const std::string& side, const std::string& type, double price, int quantity, bool log) {
    Order order{++orderId, side, type, price, quantity, orderId};
    activeOrders[order.id] = order;

    if (order.type == "MARKET") {
        if (side == "BUY") matchMarketBuy(order, log);
        else matchMarketSell(order, log);
    } else {
        if (side == "BUY") {
            matchBuy(order, log);
            if (order.quantity > 0) buyOrders.push(order);
        } else {
            matchSell(order, log);
            if (order.quantity > 0) sellOrders.push(order);
        }
    }
    return order.id;
}

void OrderBook::cancelOrder(int id) {
    if (activeOrders.find(id) != activeOrders.end()) {
        activeOrders.erase(id);
        std::cout << "CANCELLED: OrderID " << id << std::endl;
    } else {
        std::cout << "OrderID " << id << " not found." << std::endl;
    }
}

void OrderBook::printOrderBook(int depth) {
    std::cout << "----- Order Book (Top " << depth << ") -----" << std::endl;

    auto buys = buyOrders;
    std::cout << "Bids:" << std::endl;
    int count = 0;
    while (!buys.empty() && count < depth) {
        auto o = buys.top(); buys.pop();
        if (activeOrders.find(o.id) != activeOrders.end() && o.quantity > 0) {
            std::cout << "  " << o.price << " x " << o.quantity << " (ID " << o.id << ")" << std::endl;
            count++;
        }
    }

    auto sells = sellOrders;
    std::cout << "Asks:" << std::endl;
    count = 0;
    while (!sells.empty() && count < depth) {
        auto o = sells.top(); sells.pop();
        if (activeOrders.find(o.id) != activeOrders.end() && o.quantity > 0) {
            std::cout << "  " << o.price << " x " << o.quantity << " (ID " << o.id << ")" << std::endl;
            count++;
        }
    }
    std::cout << "------------------------------" << std::endl;
}

void OrderBook::runStressTest(int N) {
    std::cout << "Running stress test with " << N << " orders..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_real_distribution<double> priceDist(95.0, 105.0);
    std::uniform_int_distribution<int> qtyDist(1, 10);

    for (int i = 0; i < N; i++) {
        std::string side = sideDist(rng) ? "BUY" : "SELL";
        double price = priceDist(rng);
        int qty = qtyDist(rng);
        addOrder(side, "LIMIT", price, qty, false);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Processed " << N << " orders in "
              << duration.count() << " ms" << std::endl;
}

// --- Private matching functions (implementations) ---
// --- Private matching functions (implementations) ---

// Incoming BUY limit order tries to match against resting SELL orders.
// Only matches while the best ask price is <= our buy price (price crosses).
void OrderBook::matchBuy(Order& order, bool log) {
    while (order.quantity > 0 && !sellOrders.empty()) {
        Order top = sellOrders.top(); // stale copy, just used to peek

        // Lazy deletion: skip entries that were cancelled or fully filled already
        auto it = activeOrders.find(top.id);
        if (it == activeOrders.end() || it->second.quantity <= 0) {
            sellOrders.pop();
            continue;
        }

        Order& resting = it->second; // authoritative live state of the resting order
        if (resting.price > order.price) break; // best ask too expensive, no cross possible

        int tradedQty = std::min(order.quantity, resting.quantity);
        order.quantity -= tradedQty;
        resting.quantity -= tradedQty;

        if (log) {
            std::cout << "TRADE: " << tradedQty << " @ " << resting.price
                      << " (Buy ID " << order.id << " <-> Sell ID " << resting.id << ")" << std::endl;
        }

        sellOrders.pop();
        if (resting.quantity > 0) {
            sellOrders.push(resting); // push updated remaining quantity back
        } else {
            activeOrders.erase(resting.id); // fully filled, remove from book
        }
    }
    activeOrders[order.id].quantity = order.quantity; // sync incoming order's remaining qty
}

// Mirror of matchBuy: incoming SELL limit order matches against resting BUY orders.
void OrderBook::matchSell(Order& order, bool log) {
    while (order.quantity > 0 && !buyOrders.empty()) {
        Order top = buyOrders.top();

        auto it = activeOrders.find(top.id);
        if (it == activeOrders.end() || it->second.quantity <= 0) {
            buyOrders.pop();
            continue;
        }

        Order& resting = it->second;
        if (resting.price < order.price) break; // best bid too low, no cross possible

        int tradedQty = std::min(order.quantity, resting.quantity);
        order.quantity -= tradedQty;
        resting.quantity -= tradedQty;

        if (log) {
            std::cout << "TRADE: " << tradedQty << " @ " << resting.price
                      << " (Sell ID " << order.id << " <-> Buy ID " << resting.id << ")" << std::endl;
        }

        buyOrders.pop();
        if (resting.quantity > 0) {
            buyOrders.push(resting);
        } else {
            activeOrders.erase(resting.id);
        }
    }
    activeOrders[order.id].quantity = order.quantity;
}

// Market BUY: fills against best available asks regardless of price, until
// quantity is exhausted or the book runs out of liquidity.
void OrderBook::matchMarketBuy(Order& order, bool log) {
    while (order.quantity > 0 && !sellOrders.empty()) {
        Order top = sellOrders.top();

        auto it = activeOrders.find(top.id);
        if (it == activeOrders.end() || it->second.quantity <= 0) {
            sellOrders.pop();
            continue;
        }

        Order& resting = it->second;
        int tradedQty = std::min(order.quantity, resting.quantity);
        order.quantity -= tradedQty;
        resting.quantity -= tradedQty;

        if (log) {
            std::cout << "TRADE: " << tradedQty << " @ " << resting.price
                      << " (Market Buy ID " << order.id << " <-> Sell ID " << resting.id << ")" << std::endl;
        }

        sellOrders.pop();
        if (resting.quantity > 0) {
            sellOrders.push(resting);
        } else {
            activeOrders.erase(resting.id);
        }
    }
    activeOrders[order.id].quantity = order.quantity;
    if (log && order.quantity > 0) {
        std::cout << "Market Buy ID " << order.id << " partially filled, "
                  << order.quantity << " unfilled (no liquidity)." << std::endl;
    }
}

// Market SELL: mirror of matchMarketBuy against resting buy orders.
void OrderBook::matchMarketSell(Order& order, bool log) {
    while (order.quantity > 0 && !buyOrders.empty()) {
        Order top = buyOrders.top();

        auto it = activeOrders.find(top.id);
        if (it == activeOrders.end() || it->second.quantity <= 0) {
            buyOrders.pop();
            continue;
        }

        Order& resting = it->second;
        int tradedQty = std::min(order.quantity, resting.quantity);
        order.quantity -= tradedQty;
        resting.quantity -= tradedQty;

        if (log) {
            std::cout << "TRADE: " << tradedQty << " @ " << resting.price
                      << " (Market Sell ID " << order.id << " <-> Buy ID " << resting.id << ")" << std::endl;
        }

        buyOrders.pop();
        if (resting.quantity > 0) {
            buyOrders.push(resting);
        } else {
            activeOrders.erase(resting.id);
        }
    }
    activeOrders[order.id].quantity = order.quantity;
    if (log && order.quantity > 0) {
        std::cout << "Market Sell ID " << order.id << " partially filled, "
                  << order.quantity << " unfilled (no liquidity)." << std::endl;
    }
}