#pragma once
#include "interpolator.h"

class yield_curve
{
public:
	yield_curve(const std::vector<std::pair<double, double>>& points) : _interpolator(points) {
	}

	double operator()(double x) const {
		return _interpolator.findValue(x);
	}
private:
	linear_interpolator _interpolator;
};





