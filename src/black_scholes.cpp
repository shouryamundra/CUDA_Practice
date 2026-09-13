#include "option_pricing.hpp"

#include <cmath>

namespace {
double standardNormalCdf(double value) {
    return 0.5 * (1.0 + std::erf(value / std::sqrt(2.0)));
}
}

// Black-Scholes pricing for a European call or put option
double blackScholesOptionPricing(double S0, double K, double r, double sigma, double T, bool isCallOption) {
    double squareRootOfT = std::sqrt(T);
    double d1 = (std::log(S0 / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * squareRootOfT);
    double d2 = d1 - sigma * squareRootOfT;

    if (isCallOption) {
        return S0 * standardNormalCdf(d1) - K * std::exp(-r * T) * standardNormalCdf(d2);
    }

    return K * std::exp(-r * T) * standardNormalCdf(-d2) - S0 * standardNormalCdf(-d1);
}
