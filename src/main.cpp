#include "orderbook.h"

int main() {
    OrderBook ob;

    // Small demo
    ob.addOrder("BUY", "LIMIT", 101, 10);
    ob.addOrder("SELL", "LIMIT", 102, 5);
    ob.printOrderBook();

    // Stress test
    ob.runStressTest(1000000);

    return 0;
}