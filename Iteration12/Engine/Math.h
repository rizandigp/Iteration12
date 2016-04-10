#pragma once

#include <cmath>
#include <algorithm>
#include <xnamath.h>

#define POS_INFINITY = std::numeric_limits<Real>::infinity();
#define NEG_INFINITY = -std::numeric_limits<Real>::infinity();
#define PI               3.141592654f
#define TWO_PI              6.283185307f
#define ONE_DIV_PI           0.318309886f
#define ONE_DIV_2PI          0.159154943f
#define PI_DIV2           1.570796327f
#define PI_DIV4           0.785398163f

template <typename T>
static T clamp(T val, T minval, T maxval)
{
	//assert (minval <= maxval && "Invalid clamp range");
	return (std::max)((std::min)(val, maxval), minval);
}

inline bool floatEqual(float x, float y, float tolerance = 1e-03)
{
	return (abs(x-y) <= tolerance);
}

inline float UnitRandom()
{
	return float(rand() / RAND_MAX);
}

inline float random(float min, float max )
{
	return float(rand()) / RAND_MAX * (max-min) + min;
}

__forceinline float sqr(float x)
{
	return x*x;
}

// Quake's fast inverse square root
__forceinline float fastInvSqrt(float x)
{
    float xhalf = 0.5f * x;
    int i = *(int*)&x;            // store floating-point bits in integer
    i = 0x5f3759df - (i >> 1);    // initial guess for Newton's method
    x = *(float*)&i;              // convert new bits into float
    x = x*(1.5f - xhalf*x*x);     // One round of Newton's method
    return x;
   }

__forceinline float invSqrt(float x)
{
	return 1/sqrt(x);
}

inline float lerp(float lhs, float rhs, float alpha)
{
    return (rhs - lhs) * alpha + lhs;
}

inline int lerp(int lhs, int rhs, float alpha)
{
    return int((rhs - lhs) * alpha + .5f) + lhs;
}