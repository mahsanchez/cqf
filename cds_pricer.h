#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <functional>

#include "yield_curve.h"

class cdspricer {
public:
	cdspricer(int basket_size, double T, int defaults,  double recovery, yield_curve& zero_coupon)
		: _basket_size(basket_size), expiry(T), _defaults(defaults), _recovery(recovery), _zero_coupon(zero_coupon) 
	{
	}

	//Computes the cds price for a basket of cds options
	void price(double &premium_leg, double &default_leg, std::vector<double> &nth_default_times, std::vector<double> &year_fraction)
	{
		/*
		std::vector<int> defaulted_names;
		for (int idx = 0; idx < _basket_size; idx++) {
			int default_counter = 0;
			std::for_each(default_times[idx].begin(), default_times[idx].end(), [&default_counter, this](double x) mutable {
				if ((x < expiry) && (std::max(x, 0.25) != 0.25) ){
					default_counter = default_counter + 1;
				}
			});
			if (default_counter > 0) {
				defaulted_names.push_back(idx);
			}
		}

		std::vector<double> nth_default_times;

		for (std::vector<int>::iterator it = defaulted_names.begin(); it != defaulted_names.end(); ++it) {
			std::vector<double> v = default_times[*it];
			std::vector<double>::iterator result = std::min_element(std::begin(v), std::end(v));
			int index = std::distance(std::begin(v), result);
			double tau = v[index];
			nth_default_times.push_back(tau);
		}
		*/

		// We do not need the code above couse you already receive the nth_default_times

		// no defaults
		bool defaulted_basket = false;
		if ((nth_default_times.size() == 0) || (nth_default_times.size() <= _defaults)) {
			nth_default_times.clear();
			for (int i = 0; i < (int)expiry; i++) {
				nth_default_times.push_back(i);
			}
		}
		//  case of multiple defaults
		else if (nth_default_times.size() > 1) {
			std::sort(nth_default_times.begin(), nth_default_times.end());
			defaulted_basket = true;
		}

		// We need  to calculate the exact default times for each tau in nth_default_times
		// this could help to calculate the delta to be used in the accrual calculation in case of default

		// if latest default time do not coincide with the tenors get the defaults(th) default time

		// if no dt default calculate premium at the nd of each period code here .. end of the story no default
		// if there is no default , keep accruing the premium until the 5Y and (expiry). This will only be simulated
		// value of PL

		//calculate the spread based on which tenor the defualt happen

		// always calculate the premium if defaulted or not
		// nnondefaultedentities instead of remain_basket / reference
		//nnondefaultentities instead of remain_basket / reference
		std::vector<double> t = { 0, 1, 2, 3, 4 };
		int nnondefaultedentities = _basket_size;

		premium_leg += _zero_coupon(t[1]) + _zero_coupon(nth_default_times[0]) * year_fraction[0];
		premium_leg *= (nnondefaultedentities / _basket_size);

		if (_defaults > 1) {
			for (int i = 2; i <= nth_default_times.size(); i++) {
				premium_leg += _zero_coupon(t[i])*(t[i] - nth_default_times[i-1]) + _zero_coupon(nth_default_times[i]) * year_fraction[i];
				nnondefaultedentities = nnondefaultedentities - 1;
				premium_leg *= (nnondefaultedentities / _basket_size);
			}
	    }
		/*
		premium_leg = _zero_coupon(nth_default_times[0]);
		premium_leg = std::accumulate(nth_default_times.begin(), nth_default_times.begin() + _defaults, premium_leg,
			[&nth_default_times, &nnondefaultedentities, &i, this](double t0, double t) {
			double result = t0;
			result += _zero_coupon(t) * (t - nth_default_times[i - 1]) * nnondefaultedentities / _basket_size; 
			i++;  nnondefaultedentities--;
			return result;
		  }
		);
		*/

		// price default_leg if defaulted occurred in the basket
		// TODO - calculate accrual for each exact default time do not fit with tenor+ 0.25 (floor/ceil)
		// pass a lambda function to calculate exact default time
		// calculate the accrual (day count fraction), interest rate to calculate the acrrual year
		// premium_leg = premium_leg - accrual
		if (defaulted_basket) {
			double tau = nth_default_times[_defaults-1];
			default_leg = (1 - _recovery) * _zero_coupon(tau) * 0.2;
		}
		
	}

	double getExpiry() {
		return expiry;
	}

	int getBasketSize() {
		return _basket_size;
	}

private:

	int _basket_size;
	double expiry;
	int _defaults;
	double _recovery; 
	yield_curve& _zero_coupon;
};


