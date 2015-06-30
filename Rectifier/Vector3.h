#pragma once
#include <vector>
#include <algorithm>
#include <string>
#include <assert.h>
#include <math.h>

using namespace std;

typedef struct
{
	float X;
	float Y;
	float Z;
} Vector3;

typedef struct
{
	float X;
	float Y;
} Vector2;

typedef struct
{
	float M11;
	float M12;
	float M13;
	float M14;
	float M21;
	float M22;
	float M23;
	float M24;
	float M31;
	float M32;
	float M33;
	float M34;
	float M41;
	float M42;
	float M43;
	float M44;
} Matrix;

typedef struct
{
	float X;
	float Y;
	float Z;
	float W;
} Quaternion;

class CMathHelper
{
public:
	//Returns the Cartesian coordinate for one axis of a point that is defined by a given triangle and two normalized barycentric (areal) coordinates.
	float Barycentric(float value1, float value2, float value3, float amount1, float amount2);
	// Performs a Catmull-Rom interpolation using the specified positions.
	float CatmullRom(float value1, float value2, float value3, float value4, float amount);
	// Restricts a value to be within a specified range.
	float Clamp(float value, float min, float max);
	// Performs a Hermite spline interpolation.
	float Hermite(float value1, float tangent1, float value2, float tangent2, float amount);
	// Linearly interpolates between two values.
	float Lerp(float value1, float value2, float amount);
	// Returns the greater of two values.
	float Max(float value1, float value2);
	// Returns the lesser of two values.
	float Min(float value1, float value2);
	// Interpolates between two values using a cubic equation.
	float SmoothStep(float value1, float value2, float amount);
};

bool operator ==(Vector3 value1, Vector3 value2);
bool operator !=(Vector3 value1, Vector3 value2);
Vector3 operator +(Vector3 value1, Vector3 value2);
Vector3 operator -(Vector3 value);
Vector3 operator -(Vector3 value1, Vector3 value2);
Vector3 operator *(Vector3 value1, Vector3 value2);
Vector3 operator *(Vector3 value, float scaleFactor);
Vector3 operator *(float scaleFactor, Vector3 value);
Vector3 operator /(Vector3 value1, Vector3 value2);
Vector3 operator /(Vector3 value, float divider);

class CVector3
{
public:
	// Constructor
	CVector3(float x, float y, float z);
	CVector3(float value);
	CVector3(Vector2 value, float z); 
	
	Vector3 Zero    (void);
	Vector3 One     (void);
	Vector3 UnitX   (void);
	Vector3 UnitY   (void);
	Vector3 UnitZ   (void);
	Vector3 Up      (void);
	Vector3 Down    (void);
	Vector3 Right   (void);
	Vector3 Left    (void);
	Vector3 Forward (void);
	Vector3 Backward(void);

	Vector3 Add(Vector3 value1, Vector3 value2);
	void    Add(Vector3 &value1, Vector3 &value2, Vector3 &result);
	Vector3 Barycentric(Vector3 value1, Vector3 value2, Vector3 value3, float amount1, float amount2);
	void    Barycentric(Vector3 &value1, Vector3 &value2, Vector3 &value3, float amount1, float amount2, Vector3 &result);
	Vector3 CatmullRom(Vector3 value1, Vector3 value2, Vector3 value3, Vector3 value4, float amount);
	void    CatmullRom(Vector3 &value1, Vector3 &value2, Vector3 &value3, Vector3 &value4, float amount, Vector3 &result);
	Vector3 Clamp(Vector3 value1, Vector3 min, Vector3 max);
	void    Clamp(Vector3 &value1, Vector3 &min, Vector3 &max, Vector3 &result);
	Vector3 Cross(Vector3 vector1, Vector3 vector2);
	void    Cross(Vector3 &vector1, Vector3 &vector2, Vector3 &result);
	float   Distance(Vector3 vector1, Vector3 vector2);
	void    Distance(Vector3 &value1, Vector3 &value2, float &result);
	float   DistanceSquared(Vector3 value1, Vector3 value2);
	void    DistanceSquared(Vector3 &value1, Vector3 &value2, float &result);
	Vector3 Divide(Vector3 value1, Vector3 value2);
	Vector3 Divide(Vector3 value1, float value2);
	void    Divide(Vector3 &value1, float divisor, Vector3 &result);
	void    Divide(Vector3 &value1, Vector3 &value2, Vector3 &result);
	float   Dot(Vector3 vector1, Vector3 vector2);
	void    Dot(Vector3 &vector1, Vector3 &vector2, float &result);
//	bool    Equals(object obj);  // WD: not sure the best way to implement this in C++, use template?  
	bool    Equals(Vector3 other);
	int		GetHashCode(void);
	Vector3 Hermite(Vector3 value1, Vector3 tangent1, Vector3 value2, Vector3 tangent2, float amount);
	void    Hermite(Vector3 &value1, Vector3 &tangent1, Vector3 &value2, Vector3 &tangent2, float amount, Vector3 &result);
	float   Length(void);
	float	LengthSquared(void);
	Vector3 Lerp(Vector3 value1, Vector3 value2, float amount);
	void    Lerp(Vector3 &value1, Vector3 &value2, float amount, Vector3 &result);
	Vector3 Max(Vector3 value1, Vector3 value2);
	void    Max(Vector3 &value1, Vector3 &value2, Vector3 &result);
	Vector3 Min(Vector3 value1, Vector3 value2);
	void    Min(Vector3 &value1, Vector3 &value2, Vector3 &result);
	Vector3 Multiply(Vector3 value1, Vector3 value2);
	Vector3 Multiply(Vector3 value1, float scaleFactor);
	void    Multiply(Vector3 &value1, float scaleFactor, Vector3 &result);
	void    Multiply(Vector3 &value1, Vector3 &value2, Vector3 &result);
	Vector3 Negate(Vector3 value);
	void    Negate(Vector3 &value, Vector3 &result);
	void    Normalize(void);
	Vector3 Normalize(Vector3 vector);
	void    Normalize(Vector3 &value, Vector3 &result);
	Vector3 Reflect(Vector3 vector, Vector3 normal);
	void    Reflect(Vector3 &vector, Vector3 &normal, Vector3 &result);
	Vector3 SmoothStep(Vector3 value1, Vector3 value2, float amount);
	void    SmoothStep(Vector3 &value1, Vector3 &value2, float amount, Vector3 &result);
	Vector3 Subtract(Vector3 value1, Vector3 value2);
	void    Subtract(Vector3 &value1, Vector3 &value2, Vector3 &result);
	string  DebugDisplayString(void);
	string  ToString(void);
	Vector3 Transform(Vector3 position, Matrix matrix);
	void    Transform(Vector3 &position, Matrix &matrix, Vector3 &result);
	void	Transform(vector<Vector3> sourceArray, Matrix &matrix, vector<Vector3> destinationArray);
	void    Transform(vector<Vector3> sourceArray, int sourceIndex, Matrix &matrix, vector<Vector3> destinationArray, int destinationIndex, int length);
	Vector3 Transform(Vector3 vec, Quaternion quat);
	void    Transform(Vector3 &value, Quaternion &rotation, Vector3 &result);
	void    Transform(vector<Vector3> sourceArray, Quaternion &rotation, vector<Vector3> destinationArray);
	void    Transform(vector<Vector3> sourceArray, int sourceIndex, Quaternion &rotation, vector<Vector3> destinationArray, int destinationIndex, int length);
	Vector3	TransformNormal(Vector3 normal, Matrix matrix);
	void    TransformNormal(Vector3 &normal, Matrix &matrix, Vector3 &result);
	void    TransformNormal(vector<Vector3> sourceArray, Matrix &matrix, vector<Vector3> destinationArray);

private:	
	float X;
	float Y;
	float Z;
	Vector3 zero;
	Vector3 one;
	Vector3 unitX;
	Vector3 unitY;
	Vector3 unitZ;
	Vector3 up;
	Vector3 down;
	Vector3 right;
	Vector3 left;
	Vector3 forward;
	Vector3 backward;
	CMathHelper mh;

	void Init();

};
