#pragma once

// #include <iostream>
// #include <math.h>
#include <assert.h>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef _MSC_VER
#define FN_NAME __FUNCTION__
#else
#define FN_NAME __func__
#endif

#define Clamp(x, a, b) (x < b ? (x > a ? x : a) : b)
#define Interp(x1, x2, t) (x1 + (x2 - x1) * t)

#define PRINT(print_type, ...) {printf("[%c] line %d in function %s: ", print_type, __LINE__, FN_NAME);printf(__VA_ARGS__);printf("\n");}

#define INFO(...) PRINT('I', __VA_ARGS__)
#define WARN(...) PRINT('W', __VA_ARGS__)

#define CHECK(status) if(status!=0){ERROR("check failed with err .\n", status);}

//typedef unsigned int Color;
typedef union Color
{
	uint32_t iValue;
	uint8_t aValue[4];
	struct argb_t
	{
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	}argb;

	Color() { argb.a = argb.r = argb.g = argb.b = 0; }
	Color(int c)
	{
		iValue = c;
	}

	Color(uint8_t r, uint8_t g, uint8_t b)
	{
		argb.a = 0;
		argb.r = r;
		argb.g = g;
		argb.b = b;
	}

	float operator [](int i)
	{
		assert(i >= 0 && i < 4);
		return aValue[i];
	}

	Color operator +(const Color& right)
	{
		return Color(aValue[2] + right.aValue[2], aValue[1] + right.aValue[1], aValue[0] + right.aValue[0]);
	}

	Color operator -(const Color& right)
	{
		return Color(aValue[2] - right.aValue[2], aValue[1] - right.aValue[1], aValue[0] - right.aValue[0]);
	}
	Color operator *(float k)
	{
		return Color(aValue[2] * k, aValue[1] * k, aValue[0] * k);
	}

	operator int() { return iValue; }
};

typedef class vec2 : public glm::vec2
{
public:
	vec2()
	{
		this->x = 0.0f;
		this->y = 0.0f;
	}

	vec2& operator=(const glm::vec2& input)
	{
		this->x = input.x;
		this->y = input.y;
		return *this;
	}

	vec2& operator=(const glm::vec3& input)
	{
		this->x = input.x;
		this->y = input.y;
		return *this;
	}

	vec2& operator=(const glm::vec4& input)
	{
		this->x = input.x;
		this->y = input.y;
		return *this;
	}

	void interp(vec2& a, vec2& b, float alpha)
	{
		this->x = Interp(a.x, b.x, alpha);
		this->y = Interp(a.y, b.y, alpha);
	}
};

typedef class vec3 : public glm::vec3
{
public:
	vec3()
	{
		this->x = 0.0f;
		this->y = 0.0f;
		this->z = 0.0f;
	}

	vec3(float v)
	{
		this->x = v;
		this->y = v;
		this->z = v;
	}

	vec3(glm::vec4 input)
	{
		this->x = input.x;
		this->y = input.y;
		this->z = input.z;
	}

	vec3& operator=(const glm::vec3& input)
	{
		this->x = input.x;
		this->y = input.y;
		this->z = input.z;
		return *this;
	}

	void interp(vec3& a, vec3& b, float alpha)
	{
		this->x = Interp(a.x, b.x, alpha);
		this->y = Interp(a.y, b.y, alpha);
		this->z = Interp(a.z, b.z, alpha);
	}
};

typedef class vec4 : public glm::vec4
{
public:
	vec4()
	{
		this->x = 0.0f;
		this->y = 0.0f;
		this->z = 0.0f;
		this->w = 1.0f;
	}

	vec4(glm::vec3 input)
	{
		this->x = input.x;
		this->y = input.y;
		this->z = input.z;
		this->w = 1.0f;
	}

	vec4& operator=(const glm::vec4& input)
	{
		this->x = input.x;
		this->y = input.y;
		this->z = input.z;
		this->w = input.w;
		return *this;
	}

	glm::vec3 xyz()
	{
		return glm::vec3(x, y, z);
	}

	void interp(vec4& a, vec4& b, float alpha)
	{
		this->x = Interp(a.x, b.x, alpha);
		this->y = Interp(a.y, b.y, alpha);
		this->z = Interp(a.z, b.z, alpha);
		//this->w = Interp(a.w, b.w, alpha);
	}
};