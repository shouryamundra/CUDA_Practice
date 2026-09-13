#include <iostream>
#include "option_pricing.hpp"

int main() {
    // Option parameters
    double S0 = 100.0;   // Initial stock price
    double K = 100.0;    // Strike price
    double r = 0.05;     // Risk-free rate
    double sigma = 0.2;  // Volatility
    double T = 1;      // Time to maturity (1 year)
    int numSimulations = 100000; // Number of simulations

    // Calculate option prices
    double monteCarloCallPrice = monteCarloOptionPricing(S0, K, r, sigma, T, numSimulations, true);
    double monteCarloPutPrice = monteCarloOptionPricing(S0, K, r, sigma, T, numSimulations, false);
    double blackScholesCallPrice = blackScholesOptionPricing(S0, K, r, sigma, T, true);
    double blackScholesPutPrice = blackScholesOptionPricing(S0, K, r, sigma, T, false);

    // Output the results
    std::cout << "Monte Carlo European Call Option Price: " << monteCarloCallPrice << std::endl;
    std::cout << "Monte Carlo European Put Option Price: " << monteCarloPutPrice << std::endl;
    std::cout << "Black-Scholes European Call Option Price: " << blackScholesCallPrice << std::endl;
    std::cout << "Black-Scholes European Put Option Price: " << blackScholesPutPrice << std::endl;

    return 0;
}