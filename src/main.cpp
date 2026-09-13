#include <iostream>
#include <iomanip>
#include "option_pricing.hpp"

void printStatistics(const char* label, const MonteCarloStatistics& statistics, double plainEstimatorVariance) {
    double varianceReduction = 100.0 * (plainEstimatorVariance - statistics.estimatorVarianceAfter) / plainEstimatorVariance;

    std::cout << label << std::endl;
    std::cout << "  Price: " << statistics.price << std::endl;
    std::cout << "  Estimator variance before variance reduction: " << statistics.estimatorVarianceBefore << std::endl;
    std::cout << "  Estimator variance after variance reduction: " << statistics.estimatorVarianceAfter << std::endl;
    std::cout << "  Standard error: " << statistics.standardError << std::endl;
    std::cout << "  Variance reduction versus plain Monte Carlo: " << varianceReduction << "%" << std::endl;
    std::cout << "  Control beta: " << statistics.beta << std::endl;
}

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
    double asianCallPrice = monteCarloAsianOptionPricing(S0, K, r, sigma, T, numSimulations, 252, true);
    double asianPutPrice = monteCarloAsianOptionPricing(S0, K, r, sigma, T, numSimulations, 252, false);
    double barrierCallPrice = monteCarloBarrierOptionPricing(S0, K, r, sigma, T, numSimulations, 252, 120.0, true, true);
    double barrierPutPrice = monteCarloBarrierOptionPricing(S0, K, r, sigma, T, numSimulations, 252, 80.0, false, false);
    double blackScholesCallPrice = blackScholesOptionPricing(S0, K, r, sigma, T, true);
    double blackScholesPutPrice = blackScholesOptionPricing(S0, K, r, sigma, T, false);

    // Compare variance-reduction methods using the same reproducible paths.
    MonteCarloStatistics plainStatistics = monteCarloOptionPricingWithVarianceReduction(S0, K, r, sigma, T, numSimulations, true, false, false);
    MonteCarloStatistics antitheticStatistics = monteCarloOptionPricingWithVarianceReduction(S0, K, r, sigma, T, numSimulations, true, true, false);
    MonteCarloStatistics controlStatistics = monteCarloOptionPricingWithVarianceReduction(S0, K, r, sigma, T, numSimulations, true, false, true);
    MonteCarloStatistics combinedStatistics = monteCarloOptionPricingWithVarianceReduction(S0, K, r, sigma, T, numSimulations, true, true, true);
    MonteCarloStatistics asianStatistics = monteCarloAsianOptionPricingWithVarianceReduction(S0, K, r, sigma, T, numSimulations, 252, true, true, true);
    MonteCarloStatistics barrierStatistics = monteCarloBarrierOptionPricingWithVarianceReduction(S0, K, r, sigma, T, numSimulations, 252, 120.0, true, true, true, true);

    // Output the results
    std::cout << "Monte Carlo European Call Option Price: " << monteCarloCallPrice << std::endl;
    std::cout << "Monte Carlo European Put Option Price: " << monteCarloPutPrice << std::endl;
    std::cout << "Monte Carlo Asian Call Option Price: " << asianCallPrice << std::endl;
    std::cout << "Monte Carlo Asian Put Option Price: " << asianPutPrice << std::endl;
    std::cout << "Monte Carlo Up-and-Out Call Option Price: " << barrierCallPrice << std::endl;
    std::cout << "Monte Carlo Down-and-Out Put Option Price: " << barrierPutPrice << std::endl;
    std::cout << "Black-Scholes European Call Option Price: " << blackScholesCallPrice << std::endl;
    std::cout << "Black-Scholes European Put Option Price: " << blackScholesPutPrice << std::endl;

    std::cout << std::fixed << std::setprecision(6) << std::endl;
    printStatistics("Plain European call Monte Carlo:", plainStatistics, plainStatistics.estimatorVarianceAfter);
    printStatistics("Antithetic European call Monte Carlo:", antitheticStatistics, plainStatistics.estimatorVarianceAfter);
    printStatistics("Control-variate European call Monte Carlo:", controlStatistics, plainStatistics.estimatorVarianceAfter);
    printStatistics("Combined European call Monte Carlo:", combinedStatistics, plainStatistics.estimatorVarianceAfter);
    printStatistics("Combined Asian call Monte Carlo:", asianStatistics, plainStatistics.estimatorVarianceAfter);
    printStatistics("Combined up-and-out call Monte Carlo:", barrierStatistics, plainStatistics.estimatorVarianceAfter);

    return 0;
}