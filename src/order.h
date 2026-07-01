#pragma once
#include <string>

// Basic Order structure
struct Order {
    int id;                 // Unique order ID
    std::string side;       // "BUY" or "SELL"
    std::string type;       // "LIMIT" or "MARKET"
    double price;           // Price (ignored for market orders)
    int quantity;           // Number of units to trade
    long timestamp;         // Simplified timestamp (can use orderId as proxy)
};