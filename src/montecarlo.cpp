#include "option_pricing.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

namespace {
constexpr unsigned int varianceReductionSeed = 123456789U;

enum class ContractType {
    European,
    Asian,
    Barrier
};

struct PathResult {
    double terminalPrice;
    double averagePrice;
    bool barrierHit;
};

// Simulate one path using either the original or opposite random shocks.
PathResult simulatePath(double S0, double r, double sigma, double T, double barrier, bool isUpBarrier, int numTimeSteps, const std::vector<double>& randomNumbers, bool useNegativeNumbers) {
    double stockPrice = S0;
    double priceSum = 0.0;
    bool barrierHit = false;
    double timeStep = T / static_cast<double>(numTimeSteps);

    for (int step = 0; step < numTimeSteps; ++step) {
        double standardNormal = useNegativeNumbers ? -randomNumbers[step] : randomNumbers[step];
        stockPrice *= std::exp((r - 0.5 * sigma * sigma) * timeStep + sigma * std::sqrt(timeStep) * standardNormal);
        priceSum += stockPrice;

        if ((isUpBarrier && stockPrice >= barrier) || (!isUpBarrier && stockPrice <= barrier)) {
            barrierHit = true;
        }
    }

    return {stockPrice, priceSum / static_cast<double>(numTimeSteps), barrierHit};
}

// Calculate the discounted payoff for one simulated path.
double discountedPayoff(const PathResult& path, double K, double r, double T, ContractType contractType, bool isCallOption) {
    if (contractType == ContractType::Barrier && path.barrierHit) {
        return 0.0;
    }

    double payoffPrice = contractType == ContractType::Asian ? path.averagePrice : path.terminalPrice;
    double payoff = isCallOption ? callOptionPayoff(payoffPrice, K) : putOptionPayoff(payoffPrice, K);
    return std::exp(-r * T) * payoff;
}

// Calculate the sample variance of the simulated values.
double sampleVariance(const std::vector<double>& values) {
    double average = 0.0;
    for (double value : values) {
        average += value;
    }
    average /= static_cast<double>(values.size());

    double squaredDifferenceSum = 0.0;
    for (double value : values) {
        double difference = value - average;
        squaredDifferenceSum += difference * difference;
    }

    return squaredDifferenceSum / static_cast<double>(values.size() - 1);
}

// Calculate the average of the simulated values.
double average(const std::vector<double>& values) {
    double total = 0.0;
    for (double value : values) {
        total += value;
    }
    return total / static_cast<double>(values.size());
}

MonteCarloStatistics runVarianceReducedSimulation(double S0, double K, double r, double sigma, double T, int numSimulations, int numTimeSteps, double barrier, bool isUpBarrier, bool isCallOption, ContractType contractType, bool useAntitheticVariates, bool useControlVariate) {
    if (numSimulations <= 1 || numTimeSteps <= 0 || (useAntitheticVariates && numSimulations % 2 != 0)) {
        throw std::invalid_argument("numSimulations must be greater than one, numTimeSteps must be greater than zero, and antithetic simulations must be even");
    }

    int observationCount = useAntitheticVariates ? numSimulations / 2 : numSimulations;
    std::vector<double> targetValues;
    std::vector<double> controlValues;
    targetValues.reserve(observationCount);
    controlValues.reserve(observationCount);

    std::mt19937 generator(varianceReductionSeed);
    std::normal_distribution<double> distribution(0.0, 1.0);
    std::vector<double> randomNumbers(numTimeSteps);

    for (int simulation = 0; simulation < observationCount; ++simulation) {
        for (double& randomNumber : randomNumbers) {
            randomNumber = distribution(generator);
        }

        PathResult positivePath = simulatePath(S0, r, sigma, T, barrier, isUpBarrier, numTimeSteps, randomNumbers, false);
        double positiveTarget = discountedPayoff(positivePath, K, r, T, contractType, isCallOption);
        double positiveControl = std::exp(-r * T) * positivePath.terminalPrice;

        if (useAntitheticVariates) {
            PathResult negativePath = simulatePath(S0, r, sigma, T, barrier, isUpBarrier, numTimeSteps, randomNumbers, true);
            double negativeTarget = discountedPayoff(negativePath, K, r, T, contractType, isCallOption);
            double negativeControl = std::exp(-r * T) * negativePath.terminalPrice;
            targetValues.push_back((positiveTarget + negativeTarget) / 2.0);
            controlValues.push_back((positiveControl + negativeControl) / 2.0);
        } else {
            targetValues.push_back(positiveTarget);
            controlValues.push_back(positiveControl);
        }
    }

    double targetAverage = average(targetValues);
    double controlAverage = average(controlValues);

    double beta = 0.0;
    if (useControlVariate) {
        double covariance = 0.0;
        double controlVariance = 0.0;
        for (int observation = 0; observation < observationCount; ++observation) {
            double targetDifference = targetValues[observation] - targetAverage;
            double controlDifference = controlValues[observation] - controlAverage;
            covariance += targetDifference * controlDifference;
            controlVariance += controlDifference * controlDifference;
        }
        if (controlVariance > 0.0) {
            beta = covariance / controlVariance;
        }
    }

    std::vector<double> adjustedValues;
    adjustedValues.reserve(observationCount);
    for (int observation = 0; observation < observationCount; ++observation) {
        adjustedValues.push_back(targetValues[observation] - beta * (controlValues[observation] - S0));
    }

    double adjustedAverage = average(adjustedValues);

    double varianceBefore = sampleVariance(targetValues);
    double varianceAfter = sampleVariance(adjustedValues);
    return {
        adjustedAverage,
        varianceBefore / static_cast<double>(observationCount),
        varianceAfter / static_cast<double>(observationCount),
        std::sqrt(varianceAfter / static_cast<double>(observationCount)),
        beta
    };
}
}

// Function to generate normally distributed random numbers
double generateGaussianNoise(double mean, double stddev) {
    static std::mt19937 generator(std::random_device{}());
    std::normal_distribution<double> distribution(mean, stddev);
    return distribution(generator);
}

// Function to calculate the payoff of a European call option
double callOptionPayoff(double S, double K) {
    return std::max(S - K, 0.0);
}

// Function to calculate the payoff of a European put option
double putOptionPayoff(double S, double K) {
    return std::max(K - S, 0.0);
}

// Price a European option by simulating its terminal stock price.
double monteCarloOptionPricing(double S0, double K, double r, double sigma, double T, int numSimulations, bool isCallOption) {
    double payoffSum = 0.0;

    for (int i = 0; i < numSimulations; ++i) {
        // Generate a random price path
        double ST = S0 * std::exp((r - 0.5 * sigma * sigma) * T + sigma * std::sqrt(T) * generateGaussianNoise(0.0, 1.0));

        // Calculate the payoff for this path
        double payoff = isCallOption ? callOptionPayoff(ST, K) : putOptionPayoff(ST, K);

        // Accumulate the payoff
        payoffSum += payoff;
    }

    // Calculate the average payoff and discount it to present value
    double averagePayoff = payoffSum / static_cast<double>(numSimulations);
    return std::exp(-r * T) * averagePayoff;
}

// Price an arithmetic-average Asian option by simulating its price path.
double monteCarloAsianOptionPricing(double S0, double K, double r, double sigma, double T, int numSimulations, int numTimeSteps, bool isCallOption) {
    if (numSimulations <= 0 || numTimeSteps <= 0) {
        throw std::invalid_argument("numSimulations and numTimeSteps must be greater than zero");
    }

    double payoffSum = 0.0;
    double timeStep = T / static_cast<double>(numTimeSteps);

    for (int i = 0; i < numSimulations; ++i) {
        double stockPrice = S0;
        double priceSum = 0.0;

        for (int step = 0; step < numTimeSteps; ++step) {
            stockPrice *= std::exp((r - 0.5 * sigma * sigma) * timeStep + sigma * std::sqrt(timeStep) * generateGaussianNoise(0.0, 1.0));
            priceSum += stockPrice;
        }

        double averagePrice = priceSum / static_cast<double>(numTimeSteps);
        double payoff = isCallOption ? callOptionPayoff(averagePrice, K) : putOptionPayoff(averagePrice, K);
        payoffSum += payoff;
    }

    return std::exp(-r * T) * payoffSum / static_cast<double>(numSimulations);
}

// Price a discretely monitored knock-out barrier option.
double monteCarloBarrierOptionPricing(double S0, double K, double r, double sigma, double T, int numSimulations, int numTimeSteps, double barrier, bool isUpBarrier, bool isCallOption) {
    if (numSimulations <= 0 || numTimeSteps <= 0) {
        throw std::invalid_argument("numSimulations and numTimeSteps must be greater than zero");
    }

    double payoffSum = 0.0;
    double timeStep = T / static_cast<double>(numTimeSteps);

    for (int i = 0; i < numSimulations; ++i) {
        double stockPrice = S0;
        bool barrierHit = false;

        for (int step = 0; step < numTimeSteps; ++step) {
            stockPrice *= std::exp((r - 0.5 * sigma * sigma) * timeStep + sigma * std::sqrt(timeStep) * generateGaussianNoise(0.0, 1.0));

            if ((isUpBarrier && stockPrice >= barrier) || (!isUpBarrier && stockPrice <= barrier)) {
                barrierHit = true;
            }
        }

        if (!barrierHit) {
            double payoff = isCallOption ? callOptionPayoff(stockPrice, K) : putOptionPayoff(stockPrice, K);
            payoffSum += payoff;
        }
    }

    return std::exp(-r * T) * payoffSum / static_cast<double>(numSimulations);
}

MonteCarloStatistics monteCarloOptionPricingWithVarianceReduction(double S0, double K, double r, double sigma, double T, int numSimulations, bool isCallOption, bool useAntitheticVariates, bool useControlVariate) {
    return runVarianceReducedSimulation(S0, K, r, sigma, T, numSimulations, 1, 0.0, false, isCallOption, ContractType::European, useAntitheticVariates, useControlVariate);
}

MonteCarloStatistics monteCarloAsianOptionPricingWithVarianceReduction(double S0, double K, double r, double sigma, double T, int numSimulations, int numTimeSteps, bool isCallOption, bool useAntitheticVariates, bool useControlVariate) {
    return runVarianceReducedSimulation(S0, K, r, sigma, T, numSimulations, numTimeSteps, 0.0, false, isCallOption, ContractType::Asian, useAntitheticVariates, useControlVariate);
}

MonteCarloStatistics monteCarloBarrierOptionPricingWithVarianceReduction(double S0, double K, double r, double sigma, double T, int numSimulations, int numTimeSteps, double barrier, bool isUpBarrier, bool isCallOption, bool useAntitheticVariates, bool useControlVariate) {
    return runVarianceReducedSimulation(S0, K, r, sigma, T, numSimulations, numTimeSteps, barrier, isUpBarrier, isCallOption, ContractType::Barrier, useAntitheticVariates, useControlVariate);
}
