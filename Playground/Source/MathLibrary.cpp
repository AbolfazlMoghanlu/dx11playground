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

Matrix<float> Math::LookAt(const Vector3f Eye, const Vector3f& CameraForwardVector, const Vector3f& WorldUpVector)
{
// 	zaxis = normal(At - Eye)
// 	xaxis = normal(cross(Up, zaxis))
// 	yaxis = cross(zaxis, xaxis)
//
// 	xaxis.x           yaxis.x           zaxis.x          0
// 	xaxis.y           yaxis.y           zaxis.y          0
// 	xaxis.z           yaxis.z           zaxis.z          0
// 	- dot(xaxis, eye) - dot(yaxis, eye) - dot(zaxis, eye)  1

 	//Vector3f ZAxis = Target - Eye;
 	//ZAxis = ZAxis.Normalize();

	Vector3f ZAxis = CameraForwardVector;
	ZAxis = ZAxis.Normalize();

	Vector3f XAxis = Vector3f::CrossProduct(WorldUpVector, ZAxis);
	XAxis = XAxis.Normalize();

	Vector3f YAxis = Vector3f::CrossProduct(ZAxis, XAxis);

	float XDis = -Vector3f::DotProduct(XAxis, Eye);
	float YDis = -Vector3f::DotProduct(YAxis, Eye);
	float ZDis = -Vector3f::DotProduct(ZAxis, Eye);

	Matrix<float> Mat;

	Mat.M[0][0] = XAxis.X;	Mat.M[0][1] = YAxis.X;	Mat.M[0][2] = ZAxis.X;	Mat.M[0][3] = 0;
	Mat.M[1][0] = XAxis.Y;	Mat.M[1][1] = YAxis.Y;	Mat.M[1][2] = ZAxis.Y;	Mat.M[1][3] = 0;
	Mat.M[2][0] = XAxis.Z;	Mat.M[2][1] = YAxis.Z;	Mat.M[2][2] = ZAxis.Z;	Mat.M[2][3] = 0;
	Mat.M[2][0] = XDis;		Mat.M[3][1] = YDis;		Mat.M[3][2] = ZDis;		Mat.M[3][3] = 1;

	return Mat;
}
