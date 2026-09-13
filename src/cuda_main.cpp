#include <iostream>

#include "cuda_montecarlo.hpp"

int main() {
    // Set the option inputs and simulation count.
    double S0 = 100.0;
    double K = 100.0;
    double r = 0.05;
    double sigma = 0.2;
    double T = 1.0;
    int numSimulations = 100000;

    // Price the call and put options on the GPU.
    double callPrice = cudaMonteCarloOptionPricing(S0, K, r, sigma, T, numSimulations, true);
    double putPrice = cudaMonteCarloOptionPricing(S0, K, r, sigma, T, numSimulations, false);

    // Print the GPU pricing results.
    std::cout << "CUDA Monte Carlo European Call Option Price: " << callPrice << std::endl;
    std::cout << "CUDA Monte Carlo European Put Option Price: " << putPrice << std::endl;

    return 0;
}
