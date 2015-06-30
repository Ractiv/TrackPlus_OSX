#include "Vector3.h"

// Returns the Cartesian coordinate for one axis of a point that is defined by a given triangle and two normalized barycentric (areal) coordinates.
// value1: The coordinate on one axis of vertex 1 of the defining triangle.
// value2: The coordinate on the same axis of vertex 2 of the defining triangle.
// value3: The coordinate on the same axis of vertex 3 of the defining triangle.
// amount1: The normalized barycentric (areal) coordinate b2, equal to the weighting factor for vertex 2, the coordinate of which is specified in value2.
// amount2: The normalized barycentric (areal) coordinate b3, equal to the weighting factor for vertex 3, the coordinate of which is specified in value3.
// return: Cartesian coordinate of the specified point with respect to the axis being used.
float CMathHelper::Barycentric(float value1, float value2, float value3, float amount1, float amount2)
{
	return (value1 + (value2 - value1) * amount1 + (value3 - value1) * amount2);
}

// Performs a Catmull - Rom interpolation using the specified positions.
// value1: The first position in the interpolation.
// value2: The second position in the interpolation.
// value3: The third position in the interpolation.
// value4: The fourth position in the interpolation.
// amount: Weighting factor.
// return: A position that is the result of the Catmull-Rom interpolation.
float CMathHelper::CatmullRom(float value1, float value2, float value3, float value4, float amount)
{
	// Using formula from http://www.mvps.org/directx/articles/catmull/
	// Internally using doubles not to lose precission
	double amountSquared = amount * amount;
	double amountCubed = amountSquared * amount;
	return ((float)(0.5 * (2.0 * value2 + (value3 - value1) * amount + (2.0 * value1 - 5.0 * value2 + 4.0 * value3 - value4) * amountSquared + (3.0 * value2 - value1 - 3.0 * value3 + value4) * amountCubed)));
}

// Restricts a value to be within a specified range.
// value: The value to clamp.
// min: The minimum value. If value is less than min, min will be returned.
// max: The maximum value. If value is greater than max, max will be returned.
// return: The clamped value.
float CMathHelper::Clamp(float value, float min, float max)
{
	// First we check to see if we're greater than the max
	value = (value > max) ? max : value;
	// Then we check to see if we're less than the min.
	value = (value < min) ? min : value;
	// There's no check to see if min > max.
	return value;
}

// Performs a Hermite spline interpolation.
// value1: Source position.
// tangent1: Source tangent.
// value2: Source position.
// tangent2: Source tangent.
// amount: Weighting factor.
// return: The result of the Hermite spline interpolation.
float CMathHelper::Hermite(float value1, float tangent1, float value2, float tangent2, float amount)
{
	// All transformed to double not to lose precission
	// Otherwise, for high numbers of param:amount the result is NaN instead of Infinity
	double v1 = value1, v2 = value2, t1 = tangent1, t2 = tangent2, s = amount, result;
	double sCubed = s * s * s;
	double sSquared = s * s;

	if (amount == 0.0)
		result = value1;
	else if (amount == 1.0)
		result = value2;
	else
		result = (2 * v1 - 2 * v2 + t2 + t1) * sCubed +	(3 * v2 - 3 * v1 - 2 * t1 - t2) * sSquared + t1 * s + v1;
	return ((float)result);
}

// Linearly interpolates between two values.
// value1: Source value.
// value2: Source value.
// amount: Value between 0 and 1 indicating the weight of value2.
// return: Interpolated value.
// Remark: This method performs the linear interpolation based on the following formula: value1 + (value2 - value1) * amount
//         Passing amount a value of 0 will cause value1 to be returned, a value of 1 will cause value2 to be returned.
float CMathHelper::Lerp(float value1, float value2, float amount)
{
	return (value1 + (value2 - value1) * amount);
}

// Returns the greater of two values.
// value1: Source value.
// value2: Source value.
// Return: The greater value.
float CMathHelper::Max(float value1, float value2)
{
	return (max(value1, value2));
}

// Returns the lesser of two values.
// value1: Source value.
// value2: Source value.
// Return: The lesser value.
float CMathHelper::Min(float value1, float value2)
{
	return (min(value1, value2));
}

// Interpolates between two values using a cubic equation.
// value1: Source value.
// value2: Source value.
// amount: Weighting value.
// Return: Interpolated value.
float CMathHelper::SmoothStep(float value1, float value2, float amount)
{
	// It is expected that 0 < amount < 1
	// If amount < 0, return value1
	// If amount > 1, return value2
	float result = Clamp(amount, 0.0f, 1.0f);
	result = Hermite(value1, 0.0f, value2, 0.0f, result);

	return result;
}


//CMatrix::CMatrix( float m11, float m12, float m13, float m14,\
//				  float m21, float m22, float m23, float m24,\
//				  float m31, float m32, float m33, float m34,\
//				  float m41, float m42, float m43, float m44 )
//	: M11(m11)
//	, M12(m12)
//	, M13(m13)
//	, M14(m14)
//	, M21(m21)
//	, M22(m22)
//	, M23(m23)
//	, M24(m24)
//	, M31(m31)
//	, M32(m32)
//	, M33(m33)
//	, M34(m34)
//	, M41(m41)
//	, M42(m42)
//	, M43(m43)
//	, M44(m44)
//{
//}

bool operator ==(Vector3 value1, Vector3 value2)
{
	return (value1.X == value2.X\
		&& value1.Y == value2.Y\
		&& value1.Z == value2.Z);
}

bool operator !=(Vector3 value1, Vector3 value2)
{
	return !(value1 == value2);
}

Vector3 operator +(Vector3 value1, Vector3 value2)
{
	value1.X += value2.X;
	value1.Y += value2.Y;
	value1.Z += value2.Z;
	return value1;
}

Vector3 operator -(Vector3 value)
{
	value.X = -value.X;
	value.Y = -value.Y;
	value.Z = -value.Z;
	return value;
}

Vector3 operator -(Vector3 value1, Vector3 value2)
{
	value1.X -= value2.X;
	value1.Y -= value2.Y;
	value1.Z -= value2.Z;
	return value1;
}

Vector3 operator *(Vector3 value1, Vector3 value2)
{
	value1.X *= value2.X;
	value1.Y *= value2.Y;
	value1.Z *= value2.Z;
	return value1;
}

Vector3 operator *(Vector3 value, float scaleFactor)
{
	value.X *= scaleFactor;
	value.Y *= scaleFactor;
	value.Z *= scaleFactor;
	return value;
}

Vector3 operator *(float scaleFactor, Vector3 value)
{
	value.X *= scaleFactor;
	value.Y *= scaleFactor;
	value.Z *= scaleFactor;
	return value;
}

Vector3 operator /(Vector3 value1, Vector3 value2)
{
	value1.X /= value2.X;
	value1.Y /= value2.Y;
	value1.Z /= value2.Z;
	return value1;
}

Vector3 operator /(Vector3 value, float divider)
{
	float factor = 1 / divider;
	value.X *= factor;
	value.Y *= factor;
	value.Z *= factor;
	return value;
}

CVector3::CVector3(float x, float y, float z)
	: X(x)
	, Y(y)
	, Z(z)
{
	Init();
}

CVector3::CVector3(float value)
	: X(value)
	, Y(value)
	, Z(value)
{
	Init();
}

CVector3::CVector3(Vector2 value, float z)
	: X(value.X)
	, Y(value.Y)
	, Z(z)
{
	Init();
}

void CVector3::Init()
{
	zero.X     = 0.0; zero.Y     = 0.0; zero.Z     = 0.0;
	one.X      = 1.0; one.Y      = 1.0; one.Z      = 1.0;
	unitX.X    = 1.0; unitX.Y    = 0.0; unitX.Z    = 0.0;
	unitY.X    = 0.0; unitY.Y    = 1.0; unitY.Z    = 0.0;
	unitZ.X    = 0.0; unitZ.Y    = 0.0; unitZ.Z    = 1.0;
	up.X       = 0.0; up.Y       = 1.0; up.Z       = 0.0;
	down.X     = 0.0; down.Y     =-1.0; down.Z     = 0.0;
	right.X	   = 1.0; right.Y    = 0.0; right.Z    = 0.0;
	left.X     =-1.0; left.Y     = 0.0; left.Z     = 0.0;
	forward.X  = 0.0; forward.Y  = 0.0; forward.Z  =-1.0;
	backward.X = 0.0; backward.Y = 0.0; backward.Z = 1.0;
}

Vector3 CVector3::Zero()
{
	return zero;
}

Vector3 CVector3::One()
{
	return one;
}

Vector3 CVector3::UnitX()
{
	return unitX;
}

Vector3 CVector3::UnitY()
{
	return unitY;
}

Vector3 CVector3::UnitZ()
{
	return unitZ;
}

Vector3 CVector3::Up()
{
	return up;
}

Vector3 CVector3::Down()
{
	return down;
}

Vector3 CVector3::Right()
{
	return right;
}

Vector3 CVector3::Left()
{
	return left;
}

Vector3 CVector3::Forward()
{
	return forward;
}

Vector3 CVector3::Backward()
{
	return backward;
}

Vector3 CVector3::Add(Vector3 value1, Vector3 value2)
{
	value1.X += value2.X;
	value1.Y += value2.Y;
	value1.Z += value2.Z;
	return value1;
}

void CVector3::Add(Vector3 &value1, Vector3 &value2, Vector3 &result)
{
	result.X = value1.X + value2.X;
	result.Y = value1.Y + value2.Y;
	result.Z = value1.Z + value2.Z;
}

Vector3 CVector3::Barycentric(Vector3 value1, Vector3 value2, Vector3 value3, float amount1, float amount2)
{ 
	Vector3 barycentric3 = { mh.Barycentric(value1.X, value2.X, value3.X, amount1, amount2),\
							 mh.Barycentric(value1.Y, value2.Y, value3.Y, amount1, amount2),\
							 mh.Barycentric(value1.Z, value2.Z, value3.Z, amount1, amount2) };
	return barycentric3;
}

void CVector3::Barycentric(Vector3 &value1, Vector3 &value2, Vector3 &value3, float amount1, float amount2, Vector3 &result)
{
	result.X = mh.Barycentric(value1.X, value2.X, value3.X, amount1, amount2);
	result.Y = mh.Barycentric(value1.Y, value2.Y, value3.Y, amount1, amount2);
	result.Z = mh.Barycentric(value1.Z, value2.Z, value3.Z, amount1, amount2);
}

Vector3 CVector3::CatmullRom(Vector3 value1, Vector3 value2, Vector3 value3, Vector3 value4, float amount)
{
	Vector3 catmullRom3 = { mh.CatmullRom(value1.X, value2.X, value3.X, value4.X, amount),\
							mh.CatmullRom(value1.Y, value2.Y, value3.Y, value4.Y, amount),\
							mh.CatmullRom(value1.Z, value2.Z, value3.Z, value4.Z, amount) };
	return catmullRom3;
}

void CVector3::CatmullRom(Vector3 &value1, Vector3 &value2, Vector3 &value3, Vector3 &value4, float amount, Vector3 &result)
{
	result.X = mh.CatmullRom(value1.X, value2.X, value3.X, value4.X, amount);
	result.Y = mh.CatmullRom(value1.Y, value2.Y, value3.Y, value4.Y, amount);
	result.Z = mh.CatmullRom(value1.Z, value2.Z, value3.Z, value4.Z, amount);
}

Vector3 CVector3::Clamp(Vector3 value1, Vector3 min, Vector3 max)
{
	Vector3 clamp3 = { mh.Clamp(value1.X, min.X, max.X),\
					   mh.Clamp(value1.Y, min.Y, max.Y),\
					   mh.Clamp(value1.Z, min.Z, max.Z) };
	return clamp3;
}

void CVector3::Clamp(Vector3 &value1, Vector3 &min, Vector3 &max, Vector3 &result)
{
	result.X = mh.Clamp(value1.X, min.X, max.X);
	result.Y = mh.Clamp(value1.Y, min.Y, max.Y);
	result.Z = mh.Clamp(value1.Z, min.Z, max.Z);
}

Vector3 CVector3::Cross(Vector3 vector1, Vector3 vector2)
{
	Cross(vector1, vector2, vector1);
	X = vector1.X;
	Y = vector1.X;
	Z = vector1.Z;
	return vector1;
}

void CVector3::Cross(Vector3 &vector1, Vector3 &vector2, Vector3 &result)
{
	float x = vector1.Y * vector2.Z - vector2.Y * vector1.Z;
	float y = -(vector1.X * vector2.Z - vector2.X * vector1.Z);
	float z = vector1.X * vector2.Y - vector2.X * vector1.Y;
	result.X = x;
	result.Y = y;
	result.Z = z;
}

float CVector3::Distance(Vector3 vector1, Vector3 vector2)
{
	float result;
	DistanceSquared(vector1, vector2, result);
	return (sqrt(result));
}

void CVector3::Distance(Vector3 &value1, Vector3 &value2, float &result)
{
	DistanceSquared(value1, value2, result);
	result = (sqrt(result));
}

float CVector3::DistanceSquared(Vector3 value1, Vector3 value2)
{
	float result;
	DistanceSquared(value1, value2, result);
	return result;
}

void CVector3::DistanceSquared(Vector3 &value1, Vector3 &value2, float &result)
{
	result = (  (value1.X - value2.X) * (value1.X - value2.X) +\
				(value1.Y - value2.Y) * (value1.Y - value2.Y) +\
				(value1.Z - value2.Z) * (value1.Z - value2.Z)  );
}

Vector3 CVector3::Divide(Vector3 value1, Vector3 value2)
{
	value1.X /= value2.X;
	value1.Y /= value2.Y;
	value1.Z /= value2.Z;
	return value1;
}

Vector3 CVector3::Divide(Vector3 value1, float value2)
{
	float factor = 1 / value2;
	value1.X *= factor;
	value1.Y *= factor;
	value1.Z *= factor;
	return value1;
}

void CVector3::Divide(Vector3 &value1, float divisor, Vector3 &result)
{
	float factor = 1 / divisor;
	result.X = value1.X * factor;
	result.Y = value1.Y * factor;
	result.Z = value1.Z * factor;
}

void CVector3::Divide(Vector3 &value1, Vector3 &value2, Vector3 &result)
{
	result.X = value1.X / value2.X;
	result.Y = value1.Y / value2.Y;
	result.Z = value1.Z / value2.Z;
}

float CVector3::Dot(Vector3 vector1, Vector3 vector2)
{
	return (vector1.X * vector2.X + vector1.Y * vector2.Y + vector1.Z * vector2.Z);
}

void CVector3::Dot(Vector3 &vector1, Vector3 &vector2, float &result)
{
	result = (vector1.X * vector2.X + vector1.Y * vector2.Y + vector1.Z * vector2.Z);
}

// WD: not sure the best way to implement this in C++ for now, use template?
//public override bool Equals(object obj)
//{
//	if (!(obj is Vector3))
//		return false;
//
//	var other = (Vector3)obj;
//	return  X == other.X &&
//		Y == other.Y &&
//		Z == other.Z;
//}

bool CVector3::Equals(Vector3 other)
{
	return (X == other.X &&	Y == other.Y &&	Z == other.Z);
}

int CVector3::GetHashCode()
{
	return ((int)(X + Y + Z));
}

Vector3 CVector3::Hermite(Vector3 value1, Vector3 tangent1, Vector3 value2, Vector3 tangent2, float amount)
{
	Vector3 result;
	Hermite(value1, tangent1, value2, tangent2, amount, result);
	return result;
}

void CVector3::Hermite(Vector3 &value1, Vector3 &tangent1, Vector3 &value2, Vector3 &tangent2, float amount, Vector3 &result)
{
	result.X = mh.Hermite(value1.X, tangent1.X, value2.X, tangent2.X, amount);
	result.Y = mh.Hermite(value1.Y, tangent1.Y, value2.Y, tangent2.Y, amount);
	result.Z = mh.Hermite(value1.Z, tangent1.Z, value2.Z, tangent2.Z, amount);
}

float CVector3::Length()
{
	float result;
	Vector3 thisV3 = { X, Y, Z };
	DistanceSquared(thisV3, zero, result);
	return (sqrt(result));
}

float CVector3::LengthSquared()
{
	float result;
	Vector3 thisV3 = { X, Y, Z };
	DistanceSquared(thisV3, zero, result);
	return result;
}

Vector3 CVector3::Lerp(Vector3 value1, Vector3 value2, float amount)
{
	Vector3 lerp3 = { mh.Lerp(value1.X, value2.X, amount),\
					  mh.Lerp(value1.Y, value2.Y, amount),\
					  mh.Lerp(value1.Z, value2.Z, amount) };
	return lerp3;
}

void CVector3::Lerp(Vector3 &value1, Vector3 &value2, float amount, Vector3 &result)
{
	result.X = mh.Lerp(value1.X, value2.X, amount);
	result.Y = mh.Lerp(value1.Y, value2.Y, amount);
	result.Z = mh.Lerp(value1.Z, value2.Z, amount);
}

Vector3 CVector3::Max(Vector3 value1, Vector3 value2)
{ 
	Vector3 max3 = { mh.Max(value1.X, value2.X), mh.Max(value1.Y, value2.Y), mh.Max(value1.Z, value2.Z) };
	return max3;
}

void CVector3::Max(Vector3 &value1, Vector3 &value2, Vector3 &result)
{
	result.X = mh.Max(value1.X, value2.X);
	result.Y = mh.Max(value1.Y, value2.Y);
	result.Z = mh.Max(value1.Z, value2.Z);
}

Vector3 CVector3::Min(Vector3 value1, Vector3 value2)
{
	Vector3 min3 = { mh.Min(value1.X, value2.X), mh.Min(value1.Y, value2.Y), mh.Min(value1.Z, value2.Z) };
	return min3;
}

void CVector3::Min(Vector3 &value1, Vector3 &value2, Vector3 &result)
{
	result.X = mh.Min(value1.X, value2.X);
	result.Y = mh.Min(value1.Y, value2.Y);
	result.Z = mh.Min(value1.Z, value2.Z);
}

Vector3 CVector3::Multiply(Vector3 value1, Vector3 value2)
{
	value1.X *= value2.X;
	value1.Y *= value2.Y;
	value1.Z *= value2.Z;
	return value1;
}

Vector3 CVector3::Multiply(Vector3 value1, float scaleFactor)
{
	value1.X *= scaleFactor;
	value1.Y *= scaleFactor;
	value1.Z *= scaleFactor;
	return value1;
}

void CVector3::Multiply(Vector3 &value1, float scaleFactor, Vector3 &result)
{
	result.X = value1.X * scaleFactor;
	result.Y = value1.Y * scaleFactor;
	result.Z = value1.Z * scaleFactor;
}

void CVector3::Multiply(Vector3 &value1, Vector3 &value2, Vector3 &result)
{
	result.X = value1.X * value2.X;
	result.Y = value1.Y * value2.Y;
	result.Z = value1.Z * value2.Z;
}

// Returns a Vector3 pointing in the opposite direction of value
// value: The vector to negate.
// Return: The vector negation of value
Vector3 CVector3::Negate(Vector3 value)
{
	value.X = -value.X;
	value.Y = -value.Y;
	value.Z = -value.Z;
	return value;
}

// Stores a Vector3 pointing in the opposite direction of value
// value: The vector to negate.
// Result: The vector that the negation of value will be stored in.
void CVector3::Negate(Vector3 &value, Vector3 &result)
{
	result.X = -value.X;
	result.Y = -value.Y;
	result.Z = -value.Z;
}

void CVector3::Normalize()
{
	Vector3 thisV3 = { X, Y, Z };
	Normalize(thisV3, thisV3);
}

Vector3 CVector3::Normalize(Vector3 vector)
{
	Normalize(vector, vector);
	return vector;
}

void CVector3::Normalize(Vector3 &value, Vector3 &result)
{
	float factor;
	Distance(value, zero, factor);
	factor = 1.0 / factor;
	result.X = value.X * factor;
	result.Y = value.Y * factor;
	result.Z = value.Z * factor;
}

Vector3 CVector3::Reflect(Vector3 vector, Vector3 normal)
{
	// I is the original array
	// N is the normal of the incident plane
	// R = I - (2 * N * ( DotProduct[ I,N] ))
	Vector3 reflectedVector;
	// inline the dotProduct here instead of calling method
	float dotProduct = ((vector.X * normal.X) + (vector.Y * normal.Y)) + (vector.Z * normal.Z);
	reflectedVector.X = vector.X - (2.0f * normal.X) * dotProduct;
	reflectedVector.Y = vector.Y - (2.0f * normal.Y) * dotProduct;
	reflectedVector.Z = vector.Z - (2.0f * normal.Z) * dotProduct;

	return reflectedVector;
}

void CVector3::Reflect(Vector3 &vector, Vector3 &normal, Vector3 &result)
{
	// I is the original array
	// N is the normal of the incident plane
	// R = I - (2 * N * ( DotProduct[ I,N] ))

	// inline the dotProduct here instead of calling method
	float dotProduct = ((vector.X * normal.X) + (vector.Y * normal.Y)) + (vector.Z * normal.Z);
	result.X = vector.X - (2.0f * normal.X) * dotProduct;
	result.Y = vector.Y - (2.0f * normal.Y) * dotProduct;
	result.Z = vector.Z - (2.0f * normal.Z) * dotProduct;
}

Vector3 CVector3::SmoothStep(Vector3 value1, Vector3 value2, float amount)
{
	Vector3 smoothStep3 = { mh.SmoothStep(value1.X, value2.X, amount),\
							mh.SmoothStep(value1.Y, value2.Y, amount),\
							mh.SmoothStep(value1.Z, value2.Z, amount) };
	return smoothStep3;
}

void CVector3::SmoothStep(Vector3 &value1, Vector3 &value2, float amount, Vector3 &result)
{
	result.X = mh.SmoothStep(value1.X, value2.X, amount);
	result.Y = mh.SmoothStep(value1.Y, value2.Y, amount);
	result.Z = mh.SmoothStep(value1.Z, value2.Z, amount);
}

// Performs vector subtraction on value1 and value2
// value1: The vector to be subtracted from.
// value2: The vector to be subtracted from value1.
// Return: The result of the vector subtraction.
Vector3 CVector3::Subtract(Vector3 value1, Vector3 value2)
{
	value1.X -= value2.X;
	value1.Y -= value2.Y;
	value1.Z -= value2.Z;
	return value1;
}

// Performs vector subtraction on value1 and value2
// value1: The vector to be subtracted from.
// value2: The vector to be subtracted from value1.
// result: The result of the vector subtraction.
void CVector3::Subtract(Vector3 &value1, Vector3 &value2, Vector3 &result)
{
	result.X = value1.X - value2.X;
	result.Y = value1.Y - value2.Y;
	result.Z = value1.Z - value2.Z;
}

string CVector3::DebugDisplayString()
{
	return (to_string(X) + "  " + to_string(Y) + "  ", to_string(Z));
}

string CVector3::ToString()
{
	return ("{ X:" + to_string(X) + " Y : " + to_string(Y) + " Z : " + to_string(Z) + " }");
}

Vector3 CVector3::Transform(Vector3 position, Matrix matrix)
{
	Transform(position, matrix, position);
	return position;
}

void CVector3::Transform(Vector3 &position, Matrix &matrix, Vector3 &result)
{
	result.X = (position.X * matrix.M11) + (position.Y * matrix.M21) + (position.Z * matrix.M31) + matrix.M41;
	result.Y = (position.X * matrix.M12) + (position.Y * matrix.M22) + (position.Z * matrix.M32) + matrix.M42;
	result.Z = (position.X * matrix.M13) + (position.Y * matrix.M23) + (position.Z * matrix.M33) + matrix.M43;
}

void CVector3::Transform(vector<Vector3> sourceArray, Matrix &matrix, vector<Vector3> destinationArray)
{
	assert(destinationArray.size() >= sourceArray.size()); // otherwise, the destination array is smaller than the source array.

	for (int i = 0; i < sourceArray.size(); i++)
	{
		Vector3 position = sourceArray[i];
		destinationArray[i] = { ((position.X*matrix.M11) + (position.Y*matrix.M21) + (position.Z*matrix.M31) + matrix.M41),\
								((position.X*matrix.M12) + (position.Y*matrix.M22) + (position.Z*matrix.M32) + matrix.M42),\
								((position.X*matrix.M13) + (position.Y*matrix.M23) + (position.Z*matrix.M33) + matrix.M43) };
	}
}

void CVector3::Transform(vector<Vector3> sourceArray, int sourceIndex, Matrix &matrix, vector<Vector3> destinationArray, int destinationIndex, int length)
{
	assert((sourceArray.size() - sourceIndex) >= length); //otherwise, the source array is too small for the given sourceIndex and length.
	assert((destinationArray.size() - destinationIndex) >= length); //otherwise, the destination array is too small for the given destinationIndex and length.

	for (int i = 0; i < length; i++)
	{
		Vector3 position = sourceArray[sourceIndex + i];
		destinationArray[destinationIndex + i] =\
			{((position.X * matrix.M11) + (position.Y * matrix.M21) + (position.Z * matrix.M31) + matrix.M41),\
			 ((position.X * matrix.M12) + (position.Y * matrix.M22) + (position.Z * matrix.M32) + matrix.M42),\
			 ((position.X * matrix.M13) + (position.Y * matrix.M23) + (position.Z * matrix.M33) + matrix.M43)};
	}
}

// Transforms a vector by a quaternion rotation.
// vec: The vector to transform.
// quat: The quaternion to rotate the vector by.
// return: The result of the operation.
Vector3 CVector3::Transform(Vector3 vec, Quaternion quat)
{
	Vector3 result;
	Transform(vec, quat, result);
	return result;
}

// Transforms a vector by a quaternion rotation.
// value: The vector to transform.
// rotation: The quaternion to rotate the vector by.
// result: The result of the operation.
void CVector3::Transform(Vector3 &value, Quaternion &rotation, Vector3 &result)
{
	float x = 2 * (rotation.Y * value.Z - rotation.Z * value.Y);
	float y = 2 * (rotation.Z * value.X - rotation.X * value.Z);
	float z = 2 * (rotation.X * value.Y - rotation.Y * value.X);

	result.X = value.X + x * rotation.W + (rotation.Y * z - rotation.Z * y);
	result.Y = value.Y + y * rotation.W + (rotation.Z * x - rotation.X * z);
	result.Z = value.Z + z * rotation.W + (rotation.X * y - rotation.Y * x);
}

// Transforms an array of vectors by a quaternion rotation.
// sourceArray: The vectors to transform
// rotation: The quaternion to rotate the vector by.
// destinationArray: The result of the operation.
void CVector3::Transform(vector<Vector3> sourceArray, Quaternion &rotation, vector<Vector3> destinationArray)
{
	assert(destinationArray.size() >= sourceArray.size()); // otherwise, "The destination array is smaller than the source array."

	for (unsigned int i = 0; i < sourceArray.size(); i++)
	{
		Vector3 position = sourceArray[i];

		float x = 2 * (rotation.Y * position.Z - rotation.Z * position.Y);
		float y = 2 * (rotation.Z * position.X - rotation.X * position.Z);
		float z = 2 * (rotation.X * position.Y - rotation.Y * position.X);

		destinationArray[i] = {position.X + x * rotation.W + (rotation.Y * z - rotation.Z * y), \
							   position.Y + y * rotation.W + (rotation.Z * x - rotation.X * z), \
							   position.Z + z * rotation.W + (rotation.X * y - rotation.Y * x)};
	}
}

// Transforms an array of vectors within a given range by a quaternion rotation.
// sourceArray: The vectors to transform.
// sourceIndex: The starting index in the source array
// rotation: The quaternion to rotate the vector by.
// destinationArray: The array to store the result of the operation.
// destinationIndex: The starting index in the destination array.
// length: The number of vectors to transform.
void CVector3::Transform(vector<Vector3> sourceArray, int sourceIndex, Quaternion &rotation, vector<Vector3> destinationArray, int destinationIndex, int length)
{
	assert((sourceArray.size() - sourceIndex) >= length);  //otherwise,	"The source array is too small for the given sourceIndex and length."
	assert((destinationArray.size() - destinationIndex) >= length); //otherwise, "The destination array is too small for the given destinationIndex and length."

	for (int i = 0; i < length; i++)
	{
		Vector3 position = sourceArray[sourceIndex + i];

		float x = 2 * (rotation.Y * position.Z - rotation.Z * position.Y);
		float y = 2 * (rotation.Z * position.X - rotation.X * position.Z);
		float z = 2 * (rotation.X * position.Y - rotation.Y * position.X);

		destinationArray[destinationIndex + i] = {position.X + x * rotation.W + (rotation.Y * z - rotation.Z * y), \
												  position.Y + y * rotation.W + (rotation.Z * x - rotation.X * z), \
												  position.Z + z * rotation.W + (rotation.X * y - rotation.Y * x)};
	}
}

Vector3 CVector3::TransformNormal(Vector3 normal, Matrix matrix)
{
	TransformNormal(normal, matrix, normal);
	return normal;
}

void CVector3::TransformNormal(Vector3 &normal, Matrix &matrix, Vector3 &result)
{
	result.X = (normal.X * matrix.M11) + (normal.Y * matrix.M21) + (normal.Z * matrix.M31);
	result.Y = (normal.X * matrix.M12) + (normal.Y * matrix.M22) + (normal.Z * matrix.M32);
	result.Z = (normal.X * matrix.M13) + (normal.Y * matrix.M23) + (normal.Z * matrix.M33);
}

void CVector3::TransformNormal(vector<Vector3> sourceArray, Matrix &matrix, vector<Vector3> destinationArray)
{
	assert(destinationArray.size() >= sourceArray.size());  // otherwise, "The destination array is smaller than the source array."

	for (unsigned i = 0; i < sourceArray.size(); i++)
	{
		Vector3 normal = sourceArray[i];
		destinationArray[i] =\
			{(normal.X*matrix.M11) + (normal.Y*matrix.M21) + (normal.Z*matrix.M31), \
			 (normal.X*matrix.M12) + (normal.Y*matrix.M22) + (normal.Z*matrix.M32), \
			 (normal.X*matrix.M13) + (normal.Y*matrix.M23) + (normal.Z*matrix.M33)};
	}
}
