#pragma once

#include "mkl.h"
#include "yield_curve.h"
#include "gaussian_copula.h"
#include "cds_pricer.h"

/* Monte Carlo algorithm implementation */
// TODO - calculate monte carlo std_err
// A cnverge criteria for the Monte Carlo estimates
// TODO - implement a Importance Sampling reduction method
// TODO - Store sampling in blocks and avg using reduction to improve floating point accuracy with std::reduce https://en.cppreference.com/w/cpp/algorithm/reduce
// TODO - Do the summation on blocks of 1024 elements and accumulate to reduce potential floating point errors
// TODO - Paralelize the internal loop, generate random block for all sampling simulations, store results in shared mememory and apply parallel reduction in memory : potential CUDA Kernel

void mc(const int samples, double dt, double &payoff, double &std_err, gaussiancopula &copula, cdspricer &pricer/*VarianceReduction vr, */ ) {
	double sum = 0.0;
	double _error = 0.0;
	double premium_leg = 0.0, default_leg = 0.0;
	double premium = 0.0, defaultg = 0.0;
	const int basket_size = pricer.getBasketSize();
	const int expiry = (int) pricer.getExpiry();
	std::vector<double> default_times;
	std::vector<double> year_fraction;
	std::vector<double> discount_factors(expiry);

	// loop over dt then number of scenarios like HJM MC implementation
	for (int i = 0; i < samples/dt; i++) {
		copula.generate(default_times, year_fraction);
		pricer.price(premium_leg, default_leg, default_times);
		premium = premium + premium_leg;
		defaultg = defaultg + default_leg;
	}

	premium = premium/samples;
	defaultg = defaultg/samples;
	payoff = defaultg/premium;
}
