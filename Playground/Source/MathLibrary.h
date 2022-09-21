#pragma once

#define SMALL_NUMBER		(1.e-8f)
#define PI_ON_180_DEGREES 0.0174532778 // PI / 180

#include "Matrix.h"
#include "Vector.h"

class Math
{
public:
	template<typename t>
	static t Clamp(t Value, t Min, t Max)
	{
		return Value < Min ? Min : (Value > Max ? Max : Value);
	}

	template<typename T>
	static T DegreesToRadians(T const& Degrees)
	{
		return Degrees * (T)PI_ON_180_DEGREES;
	}

	static float Sin(float Value)
	{
		return sinf(Value);
	}


	static float Cos(float Value)
	{
		return cosf(Value);
	}

	static void SinCos(float* SinRes, float* CosRes, float Value)
	{
		*SinRes = Sin(Value);
		*CosRes = Cos(Value);
	}

	static float Abs(float A);
	static double TruncToDouble(double A);
	static float Mod(float A, float B);

	static Matrix<float> LookAt(const Vector3f Eye, const Vector3f& CameraForwardVector, const Vector3f& WorldUpVector);
};
