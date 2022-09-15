#include "pch.h"
#include "MathLibrary.h"

float Math::Abs(float A)
{
	return fabsf(A);
}

double Math::TruncToDouble(double A)
{
	return trunc(A);
}

float Math::Mod(float A, float B)
{
	const float AbsB = Abs(B);

	if (B < SMALL_NUMBER)
	{
		return 0.0;
	}

	const double DA = double(A);
	const double DB = double(B);

	const double Div = DA / DB;
	const double IntPortion = TruncToDouble(Div) * DB;
	const double Result = DA - IntPortion;

	return float(Result);
}