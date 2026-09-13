double generateGaussianNoise(double mean, double stddev);

double callOptionPayoff(double S, double K);

double putOptionPayoff(double S, double K);

double monteCarloOptionPricing(double S0, double K, double r, double sigma, double T, int numSimulations, bool isCallOption);

double monteCarloAsianOptionPricing(double S0, double K, double r, double sigma, double T, int numSimulations, int numTimeSteps, bool isCallOption);

double monteCarloBarrierOptionPricing(double S0, double K, double r, double sigma, double T, int numSimulations, int numTimeSteps, double barrier, bool isUpBarrier, bool isCallOption);

struct MonteCarloStatistics {
	double price;
	double estimatorVarianceBefore;
	double estimatorVarianceAfter;
	double standardError;
	double beta;
};

MonteCarloStatistics monteCarloOptionPricingWithVarianceReduction(double S0, double K, double r, double sigma, double T, int numSimulations, bool isCallOption, bool useAntitheticVariates, bool useControlVariate);

MonteCarloStatistics monteCarloAsianOptionPricingWithVarianceReduction(double S0, double K, double r, double sigma, double T, int numSimulations, int numTimeSteps, bool isCallOption, bool useAntitheticVariates, bool useControlVariate);

MonteCarloStatistics monteCarloBarrierOptionPricingWithVarianceReduction(double S0, double K, double r, double sigma, double T, int numSimulations, int numTimeSteps, double barrier, bool isUpBarrier, bool isCallOption, bool useAntitheticVariates, bool useControlVariate);

double blackScholesOptionPricing(double S0, double K, double r, double sigma, double T, bool isCallOption);
