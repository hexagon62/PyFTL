#pragma once

#include "Point.hpp"

#include <tuple>
#include <concepts>

template<std::floating_point T, typename CenterT = T>
struct Ellipse
{
	using component = T;
	using center_component = CenterT;

	Point<CenterT> center;
	T a = T(0), b = T(0);

	Ellipse() = default;

	template<typename U, typename CenterU = U>
	Ellipse(const Point<CenterU>& center, const U& a, const U& b)
		: center(center)
		, a(T(a))
		, b(T(b))
	{}

	template<typename U, typename CenterU = U>
	Ellipse(const CenterU& x, const CenterU& y, const U& a, const U& b)
		: center(T(x), T(y))
		, a(T(a))
		, b(T(b))
	{}

	template<typename U, typename CenterU = U>
	Ellipse(const std::tuple<CenterU, CenterU, U, U>& t)
		: center(T(std::get<0>(t)), T(std::get<1>(t)))
		, a(T(std::get<2>(t)))
		, b(T(std::get<3>(t)))
	{}

	template<typename U, typename CenterU = U>
	Ellipse(const Ellipse<U, CenterU>& other)
		: center(other.center)
		, a(T(other.a))
		, b(T(other.b))
	{}

	operator std::tuple<CenterT, CenterT, T, T>() const
	{
		return { this->center.x, this->center.y, this->a, this->b };
	}

	Point<T> left() const
	{
		return { this->center.x - this->a / T(2), this->center.y };
	}

	Point<T> right() const
	{
		return { this->center.x + this->a / T(2), this->center.y };
	}

	Point<T> top() const
	{
		return { this->center.x, this->center.y - this->b / T(2) };
	}

	Point<T> bottom() const
	{
		return { this->center.x, this->center.y + this->b / T(2) };
	}

	bool contains(const Point<T>& p) const
	{
		Point<T> d = p - center;

		return
			(d.x * d.x) / (a * a) + (d.y * d.y) / (b * b) <= T(1);
	}

	Ellipse& operator+=(const Point<T>& other)
	{
		this->center += other.x;
		return *this;
	}

	Ellipse& operator-=(const Point<T>& other)
	{
		this->center -= other.x;
		return *this;
	}

	Ellipse operator+(const Point<T>& other) const
	{
		return { this->center + other, this->a, this->b };
	}

	Ellipse operator-(const Point<T>& other) const
	{
		return { this->center - other, this->a, this->b };
	}
};
