#pragma once

#include <utility>

template<typename T>
struct Point
{
	using component = T;

	T x = T(0), y = T(0);

	constexpr Point() = default;

	template<typename U>
	constexpr Point(const Point<U>& other)
		: x(T(other.x))
		, y(T(other.y))
	{}

	template<typename U>
	constexpr Point(const std::pair<U, U>& other)
		: x(T(other.first))
		, y(T(other.second))
	{}

	template<typename U>
	constexpr Point(const U& x, const U& y)
		: x(T(x))
		, y(T(y))
	{}

	template<typename U>
	operator std::pair<U, U>() const
	{
		return { U(this->x), U(this->y) };
	}

	Point& operator+=(const Point& other)
	{
		this->x += other.x;
		this->y += other.y;
		return *this;
	}

	Point& operator-=(const Point& other)
	{
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}

	Point& operator*=(const T& factor)
	{
		this->x *= factor;
		this->y *= factor;
		return *this;
	}

	Point& operator/=(const T& factor)
	{
		this->x /= factor;
		this->y /= factor;
		return *this;
	}

	Point operator+(const Point& other) const
	{
		return { this->x + other.x, this->y + other.y };
	}

	Point operator-(const Point& other) const
	{
		return { this->x - other.x, this->y - other.y };
	}

	Point operator*(const T& factor) const
	{
		return { this->x * factor, this->y * factor };
	}

	Point operator/(const T& factor) const
	{
		return { this->x / factor, this->y / factor };
	}
};
