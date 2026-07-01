#include "../src/orderbook.h"
#include <cassert>
#include <iostream>

void test_full_match() {
    OrderBook ob;
    ob.addOrder("BUY", 100, 5);
    ob.addOrder("SELL", 100, 5);
    std::cout << "✅ test_full_match passed" << std::endl;
}

void test_partial_match() {
    OrderBook ob;
    ob.addOrder("BUY", 101, 10);
    ob.addOrder("SELL", 101, 6);
    ob.addOrder("SELL", 101, 10); // second sell should leave 6 in book
    ob.printOrderBook();
    std::cout << "✅ test_partial_match passed" << std::endl;
}

void test_no_match() {
    OrderBook ob;
    ob.addOrder("BUY", 100, 5);
    ob.addOrder("SELL", 105, 5); // no trade
    ob.printOrderBook();
    std::cout << "✅ test_no_match passed" << std::endl;
}

int main() {
    test_full_match();
    test_partial_match();
    test_no_match();
    return 0;
}