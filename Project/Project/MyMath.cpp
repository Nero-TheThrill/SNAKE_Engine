#include "MyMath.h"
#include <cassert>
#include <cmath>
#include <iostream>

namespace MyMath
{
	const float pi = 3.14159265359f;
	const float epsilon = 1.588822487827890658830492025e-7;
}

Vector2 operator+(const Vector2& lhs, const Vector2& rhs)
{
	return { lhs.x + rhs.x, lhs.y + rhs.y };
}

float Vector2::LengthSq() const
{
	return x * x + y * y;
}

float Vector2::Length() const
{
	return sqrtf(LengthSq());
}

void Vector2::Normalize()
{
	float l = Length();
	assert(l > 0);
	l = 1.0f / l;
	x *= l;
	y *= l;
}

Vector2& Vector2::operator+=(const Vector2& rhs)
{
	x += rhs.x;
	y += rhs.y;
	return *this;
}

Vector2& Vector2::operator-=(const Vector2& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

Vector2& Vector2::operator*=(float rhs)
{
	x *= rhs;
	y *= rhs;
	return *this;
}

Vector2 operator-(const Vector2& lhs, const Vector2& rhs)
{
	return { lhs.x - rhs.x, lhs.y - rhs.y };
}

Vector2 operator*(const Vector2& lhs, float s)
{
	return { lhs.x * s, lhs.y * s };
}

Vector2 operator*(float s, const Vector2& rhs)
{
	return rhs * s;
}

bool IsNearEqaul(float A, float B, float epsilon = MyMath::epsilon)
{
	return (A - B <= epsilon);
}

Vector2 Reflect(const Vector2& normal, const Vector2& incoming)
{
	assert(IsNearEqaul(normal.LengthSq(), 1.0f));
	Vector2 projected = ProjectAOntoNormalizedB(incoming, normal) * 2;
	return  incoming - projected;
}

Vector2 Normalize(const Vector2& vec)
{
	float length = vec.Length();
	assert(length > 0);
	return { vec.x / length, vec.y / length };


}

Vector2 ProjectAOntoNormalizedB(const Vector2& A, const Vector2& B)
{
	float dot = DotProduct(A, B);
	return B * dot;
}


std::ostream& operator<<(std::ostream& os, const Vector2& vec)
{
	os << "(" << vec.x << ", " << vec.y << ")";
	return os;
}

float DotProduct(const Vector2& A, const Vector2& B)
{
	return A.x * B.x + A.y * B.y;
}

float EaseInSine(float t)
{
	return 1 - std::cosf((t * MyMath::pi) / 2.f);
}

float EaseInCubic(float t)
{
	return t * t * t;
}

float EaseOutCubic(float t)
{
	return 1 - EaseInCubic(1 - t);
}

float EaseInBounce(float t)
{
	return 1 - EaseOutBounce(1 - t);
}

float EaseOutBounce(float t)
{
	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (t < 1 / d1)
	{
		return n1 * t * t;
	}
	else if (t < 2 / d1)
	{
		t -= 1.5f / d1;
		return n1 * t * t + 0.75;
	}
	else if (t < 2.5f / d1)
	{
		t -= 2.25f / d1;
		return n1 * t * t + 0.9375;
	}
	else
	{
		t -= 2.625f / d1;
		return n1 * t * t + 0.984375;
	}
}

float EaseInOutBounce(float t)
{
	if (t < 0.5)
		return (1 - EaseOutBounce(1 - 2 * t)) / 2.f;
	else
		return (1 + EaseOutBounce(2 * t - 1)) / 2.f;
}

float Lerp(float t, float min, float max)
{
	return min * (1 - t) + max * t;
}

float Clamp(float value, float min, float max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

Vector3::Vector3(float a, float b, float c)
	:x(a), y(b), z(c)
{
}