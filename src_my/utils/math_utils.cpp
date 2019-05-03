#include "math_utils.hpp"
#include <math.h>

namespace nora {
	float inv_sqrt(float x)
	{
		float x_half = 0.5f * x;
		int i = *(int*)&x;
		i = 0x5f375a86 - (i>>1);
		x = *(float*)&i;
		x = x * (1.5f - x_half * x * x);
		x = x * (1.5f - x_half * x * x);
		x = x * (1.5f - x_half * x * x);
		return 1 / x;
	}

	bool point_in_circle(float px, float py, float cx, float cy, float raduis) {
		return pow(px - cx, 2) + pow(py - cy, 2) <= pow(raduis, 2);
	}

}
