#pragma once

#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <string>
#include "mkl.h"

class gaussiancopula {
public:
	gaussiancopula(int basket_size, int expiry, std::vector<double>& corr_matrix, std::vector<std::vector<double>>& lambdas, std::vector<std::vector<double>> psurv)
		:  _basket_size(basket_size), _expiry(expiry), _corr_matrix(corr_matrix), _cumulative_lambdas(lambdas), _lambdas(lambdas), _psurv(psurv)
	{
		//initialize intel mkl uniform random generator engine
		auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		vslNewStream(&_stream, VSL_BRNG_MT19937, (int)seed); // indepentely identical distributed normal random variables

		// apply cholesky to obtain lower matrix - TODO throw an exception if not positive definite
		int n = basket_size;
		int result = LAPACKE_dpotrf(LAPACK_ROW_MAJOR, 'L', n, &_corr_matrix[0], n);
		if (result == 0) {
			lower_matrix(_corr_matrix, _corr_matrix);
		}

		//calculate cumulative lambdas
		for (int i = 0; i < _basket_size; i++) {
			std::partial_sum(lambdas[i].begin(), lambdas[i].end(), _cumulative_lambdas[i].begin());
		}
	}

	~gaussiancopula() {
		vslDeleteStream(&_stream);
	}

	//Computes the corresponding Y value for X using linear interpolation
	/* Default Times generator using a gaussian copula */
	void generate(std::vector<double>& default_times, std::vector<double>& year_fractions) {
		std::vector<double> U(_basket_size);
		std::vector<double> u(_basket_size);
		std::vector<double> y(_basket_size, 0.0);

		// initialize i.i.d random values in the vector randoms
		vdRngGaussian(VSL_RNG_METHOD_GAUSSIAN_ICDF, _stream, _basket_size, &U[0], 0.0, 1.0);

		// correlate randoms : multiply correlation_matrix with random vectors [1..basket_size]
		const int n = _basket_size;
		const int m = _basket_size;
		double alpha = 1.0;
		double beta = 1.0;
		cblas_dgemv(CblasRowMajor, CblasNoTrans, m, n, alpha, &_corr_matrix[0], n, &U[0], 1, beta, &y[0], 1);

		// calculate Normal Cumulative Distribution function 
		vdCdfNorm(n, &y[0], &u[0]);

		// calculate default_times && year_fraction
		// first arrival default_time, do not continue after the end till expiry
		// return a vector of all five names default_times [0.30, 0.10, 0.55, 4.0]
		for (int i = 0; i < _basket_size; i++) {
			double term = log(1 - u[i]);
			double tau = -1;
			double yfr = 1.0;
			for (int j = 0; (j < _expiry) && (tau == -1); j++) {
				if ( std::abs(term) <= _cumulative_lambdas[i][j]) {
					tau = -term / _cumulative_lambdas[i][j];
					yfr = cyear_fraction(u[i], i, j);
					default_times.push_back(tau);
					year_fractions.push_back(yfr);
				}
			}
		}

		//apply variance reduction
	}

private:

	inline double cyear_fraction(double u, int i, int j) {
		double result = -(1/lambdas[i][j]) * log( (1 - u)/ _psurv[i][j-1] );
		return result;
	}

	void lower_matrix(std::vector<double>& A, std::vector<double>& B) {
		int i, j;
		int N = _basket_size;
		for (i = 0; i < N; ++i) {
			for (j = 0; j < N; ++j) {
				int index = i * N + j;
				B[index] = (i >= j) ? A[index] : 0.0;
			}
		}
	}

	int _basket_size;
	int _expiry;
	std::vector<double>& _corr_matrix;
	std::vector<std::vector<double>> _cumulative_lambdas;
	std::vector<std::vector<double>> _lambdas;
	std::vector<std::vector<double>> _psurv;
	VSLStreamStatePtr _stream; //intel mkl gaussian uniform random generator 
};

