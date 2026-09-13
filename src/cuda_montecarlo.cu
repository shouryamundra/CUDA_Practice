#include "cuda_montecarlo.hpp"

#include <cuda_runtime.h>
#include <curand_kernel.h>

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr int threadsPerBlock = 256;
constexpr unsigned long long randomSeed = 123456789ULL;

// Turn CUDA errors into readable C++ exceptions.
void checkCudaError(cudaError_t error, const char* operation) {
    if (error != cudaSuccess) {
        throw std::runtime_error(std::string(operation) + ": " + cudaGetErrorString(error));
    }
}

// Generate one option payoff per GPU thread.
__global__ void monteCarloKernel(double S0, double K, double r, double sigma, double T, int numSimulations, bool isCallOption, double* payoffs) {
    int simulationIndex = blockIdx.x * blockDim.x + threadIdx.x;
    if (simulationIndex >= numSimulations) {
        return;
    }

    curandState randomState;
    curand_init(randomSeed, simulationIndex, 0, &randomState);
    double standardNormal = curand_normal_double(&randomState);
    double terminalPrice = S0 * exp((r - 0.5 * sigma * sigma) * T + sigma * sqrt(T) * standardNormal);
    double payoff = isCallOption ? fmax(terminalPrice - K, 0.0) : fmax(K - terminalPrice, 0.0);

    payoffs[simulationIndex] = payoff;
}
}

double cudaMonteCarloOptionPricing(double S0, double K, double r, double sigma, double T, int numSimulations, bool isCallOption) {
    // Reject an invalid simulation count before allocating GPU memory.
    if (numSimulations <= 0) {
        throw std::invalid_argument("numSimulations must be greater than zero");
    }

    double* devicePayoffs = nullptr;
    std::vector<double> payoffs(numSimulations);
    const int blocks = (numSimulations + threadsPerBlock - 1) / threadsPerBlock;
    const std::size_t bytes = numSimulations * sizeof(double);

    // Allocate space for one payoff per simulation on the GPU.
    checkCudaError(cudaMalloc(&devicePayoffs, bytes), "cudaMalloc");

    // Launch enough GPU threads to cover every simulation.
    monteCarloKernel<<<blocks, threadsPerBlock>>>(S0, K, r, sigma, T, numSimulations, isCallOption, devicePayoffs);
    checkCudaError(cudaGetLastError(), "monteCarloKernel launch");
    checkCudaError(cudaDeviceSynchronize(), "monteCarloKernel execution");
    checkCudaError(cudaMemcpy(payoffs.data(), devicePayoffs, bytes, cudaMemcpyDeviceToHost), "cudaMemcpy");
    checkCudaError(cudaFree(devicePayoffs), "cudaFree");

    // Sum the GPU-generated payoffs and discount the average to today.
    double payoffSum = 0.0;
    for (double payoff : payoffs) {
        payoffSum += payoff;
    }

    return std::exp(-r * T) * payoffSum / static_cast<double>(numSimulations);
}
