#pragma once
#undef min
#undef max
#include <iostream>

namespace MyMath
{
	extern const float pi;
	extern const float epsilon;
}

struct Vector2
{
	float x = 0;
	float y = 0;
	float LengthSq() const;
	float Length() const;
	void Normalize();

	Vector2& operator+=(const Vector2&);
	Vector2& operator-=(const Vector2&);
	Vector2& operator*=(float);
};

struct Vector3
{
	float x = 0;
	float y = 0;
	float z = 0;
	Vector3(float a, float b, float c);
};

Vector2 operator+(const Vector2& lhs, const Vector2& rhs);
Vector2 operator-(const Vector2& lhs, const Vector2& rhs);
Vector2 operator*(const Vector2& lhs, float s);
Vector2 operator*(float s, const Vector2& lhs);
Vector2 Reflect(const Vector2& normal, const Vector2& incoming);
Vector2 Normalize(const Vector2& vec);
Vector2 ProjectAOntoNormalizedB(const Vector2& A, const Vector2& B);
float DotProduct(const Vector2& A, const Vector2& B);
std::ostream& operator<<(std::ostream& os, const Vector2& vec);

bool IsNearEqaul(float A, float B, float epsilon);

float EaseInSine(float t);
float EaseInCubic(float t);
float EaseOutCubic(float t);
float EaseInBounce(float t);
float EaseOutBounce(float t);
float EaseInOutBounce(float t);

float Lerp(float t, float min, float max);

float Clamp(float value, float min, float max);

namespace MyMath
{
	template <typename T>
	T min(const T& a, const T& b)
	{
		if (a > b)
			return b;
		else return a;
	}

	template <typename T>
	T max(const T& a, const T& b)
	{
		if (a < b)
			return b;
		else return a;
	}
}