#pragma once
#include "order.h"
#include <queue>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <random>
#include <chrono>

struct BuyCompare {
    bool operator()(const Order& a, const Order& b);
};

struct SellCompare {
    bool operator()(const Order& a, const Order& b);
};

class OrderBook {
private:
    std::priority_queue<Order, std::vector<Order>, BuyCompare> buyOrders;
    std::priority_queue<Order, std::vector<Order>, SellCompare> sellOrders;
    std::unordered_map<int, Order> activeOrders;
    int orderId = 0;

public:
    int addOrder(const std::string& side, const std::string& type, double price, int quantity, bool log = true);
    void cancelOrder(int id);
    void printOrderBook(int depth = 5);
    void runStressTest(int N);

private:
    void matchBuy(Order& order, bool log);
    void matchSell(Order& order, bool log);
    void matchMarketBuy(Order& order, bool log);
    void matchMarketSell(Order& order, bool log);
};