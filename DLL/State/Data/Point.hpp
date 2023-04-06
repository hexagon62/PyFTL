#pragma once

#include <utility>
#include <tuple>

template<typename T>
struct Point
{
	T x = T(0), y = T(0);

	Point() = default;
	Point(const Point& other) = default;
	Point(const T& x, const T& y) : x(x), y(y) {}
	Point(const std::pair<T, T>& p) : x(p.first), y(p.second) {}

	template<typename U>
	Point(const Point<U>& other)
		: x(T(other.x))
		, y(T(other.y))
	{}

	template<typename U>
	Point(const U& x, const U& y)
		: x(T(x))
		, y(T(y))
	{}

	template<typename U>
	Point(const std::pair<U, U>& p)
		: x(T(p.first))
		, y(T(p.second))
	{}

	operator std::pair<T, T>() const
	{
		return { this->x, this->y };
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
