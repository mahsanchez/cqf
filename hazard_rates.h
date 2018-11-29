#pragma once

#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <functional>
#include <limits>

// bootstrap Survival probabilities
double bootstrap_sp(int index, double dt, double L, std::vector<double>& p, std::vector<double>& df, std::vector<double>& sp) {
	if (index == 0) {
		return 1.0;
	}
	if (index == 1) {
		return L / (L + dt * sp[index]);
	}
	double sum = 0.0;
	for (int i = 1; i < index; i++) {
		sum += df[i] * (L * p[i - 1] - (L + dt * sp[index]) * p[i]);
	}
	sum = sum / (df[index] * (L + dt * sp[index]));
	sum = sum + (p[index - 1] * L) / (L + dt * sp[index]);

	double result = sum;
	return result;
}

// calculate hazard rate term structure
void calculate_hazard_rates(int N, double dt, double recovery, std::vector<double>& psurv, std::vector<double>& discount_factors, std::vector<double> &lambda) {
	double L = 1.0 - recovery;
	double alpha = 1 / dt;
	lambda[0] = 0.0;
	lambda[1] = -alpha * log(psurv[1]);
	for (int i = 2; i < N; i++) {
		lambda[i] = -alpha * log(psurv[i] / psurv[i - 1]);
	}
}