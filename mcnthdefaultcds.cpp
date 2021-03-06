#include "stdafx.h"

#include <string>
#include <fstream>
#include <iomanip> 
#include <chrono>
#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <functional>
#include <sstream>

#include <iostream>
#include <ctime>
#include <chrono>
#include <limits>
#include <random>

#include "mkl.h"
#include "yield_curve.h"
#include "gaussian_copula.h"
#include "cds_pricer.h"
#include "hazard_rates.h"
#include "mc_engine.h"

// global variables
const int N = 5;
const int names_size = 5;

std::vector<double> discount_factors = { 1.0, 0.980121, 0.951322, 0.915126, 0.875607, 0.832838 };
std::vector<std::pair<double, double>> spot_rate = {
	{ 0.0, 1.0 },{ 1.0, 0.980121 },{ 2.0, 0.951322 },{ 3.0, 0.915126 },{ 4.0, 0.875607 },{ 5.0, 0.832838 }
};

/*
1.000     0.552     0.817     0.888     0.424
0.552     1.000     0.562     0.221     0.817
0.817     0.562     1.000     0.601     0.590
0.888     0.221     0.601     1.000     0.026
0.424     0.817     0.590     0.026     1.000
*/

std::vector<double> corr_matrix = {
	/*1.000, 0.8345, 0.1798, -0.6133, 0.4819,
	0.8345, 1.000, -0.1869, -0.5098, 0.4381,
	0.1798, -0.1869, 1.000, -0.0984, 0.0876,
	-0.6133, -0.5098, -0.0984, 1.000, 0.3943,
	0.4819, 0.4381, 0.0876, 0.3943, 1.000
	*/
	1.000, 0.552, 0.817, 0.888, 0.424,
    0.552, 1.000, 0.562, 0.221, 0.817,
    0.817, 0.562, 1.000, 0.601, 0.590,
    0.888, 0.221, 0.601, 1.000, 0.026,
    0.424, 0.817, 0.590, 0.026, 1.000
};

std::vector<std::vector<double>> default_times(names_size, std::vector<double>(N));
std::vector<std::vector<double>> hazard_rates(names_size, std::vector<double>(N));

std::vector<std::vector<double>> spreads = {	
 { 141.76, 165.36, 188.56, 207.32, 218.38 },
 { 751, 1164, 1874, 2000, 2150 },
 { 50.26, 77.50, 84.67, 125.25, 193.10 },
 { 21, 36, 42, 46, 67.28 },
 { 151.76, 225.36, 388.56, 407.32, 418.38 }
};

// TODO - implements psurv probability in code given the spreads. 
// currently calculated using CDS Bootstrapping V2.xls
//std::vector<std::vector<double>> psurv(names_size, std::vector<double>(N));
std::vector<std::vector<double>> psurv = {
	{ 0.9769, 0.9469, 0.9104, 0.8707, 0.8328 },
	{ 0.8800, 0.6916, 0.3763, 0.2485, 0.1368 },
	{ 0.9917, 0.9745, 0.9585, 0.9179, 0.8421 },
	{ 0.9965, 0.9880, 0.9791, 0.9695, 0.9433 },
	{  0.9753, 0.9281, 0.8204, 0.7595, 0.7033 }
};

std::vector<std::vector<double>> lambdas;

// print a vector to stdout 
void print_out(std::vector<double> &randoms) {
	std::copy(randoms.begin(), randoms.end(), std::ostream_iterator<double>(std::cout, " "));
	std::cout << std::endl;
}

/* Auxiliary routine: printing a matrix */
void print_matrix(const char* desc, MKL_INT m, MKL_INT n, double* a, MKL_INT lda) {
	MKL_INT i, j;
	printf("\n %s\n", desc);
	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++) printf(" %6.2f", a[i*lda + j]);
		printf("\n");
	}
}

void print_lower(double* A, int N) {
	int i, j;
	for (i = 0; i < N; ++i) {
		for (j = 0; j < N; ++j) {
			printf("%f ", (i >= j) ? A[i + j * N] : 0.0);
		}
		printf("\n");
	}
}

typedef std::numeric_limits< double > dbl;

int main()
{
	int samples = 1000;
	double price = 0;
	double std_err = 0;
	int names_size = 5;
	double expiry = 5;

	const int N = 5;
	int defaults = 1;
	const int basket_size = 5;	
	double dt = 0.1;
	double T = 5;;
	double recovery = 0.40;
	double rate = 0.008;

	// yield curve
	yield_curve zero_coupon(spot_rate); // rename to spot rate

	//hazard rate calculation
	for (int i = 0; i < basket_size; i++) {
		calculate_hazard_rates(N, dt, recovery, psurv[i], discount_factors, hazard_rates[i]);
	}
	
	// default times random generator
	gaussiancopula copula(basket_size, expiry, corr_matrix, hazard_rates);

	// basket_cds_pricer
	cdspricer basketcdspricer(basket_size, expiry, defaults, recovery, zero_coupon);

	// mc results vector
	std::vector<double> results;

	// run monte carlo simulation . 
	// todo apply convergence criteria
	mc(samples, dt, price, std_err, copula, basketcdspricer); 

	std::cout << price << std::endl;

	return 0;
}


