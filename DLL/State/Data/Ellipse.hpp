#pragma once

#include "Point.hpp"

template<typename T>
struct Ellipse
{
	Point<T> center;
	T a = T(0), b = T(0);

	Ellipse() = default;

	Ellipse(const T& x, const T& y, const T& a, const T& b)
		: center(x, y)
		, a(a)
		, b(b)
	{}

	Ellipse(const Point<T>& center, const T& a, const T& b)
		: center(center)
		, a(a)
		, b(b)
	{}

	Ellipse(const Point<T>& center, const Point<T>& dimensions)
		: center(center)
		, a(dimensions.x)
		, b(dimensions.y)
	{}

	Ellipse(const std::tuple<T, T, T, T>& t)
		: center(std::get<0>(t), std::get<1>(t))
		, a(std::get<2>(t))
		, b(std::get<3>(t))
	{}

	Ellipse(const std::tuple<Point<T>, T, T>& t)
		: center(std::get<0>(t))
		, a(std::get<1>(t))
		, b(std::get<2>(t))
	{}

	Ellipse(const std::pair<Point<T>, Point<T>>& t)
		: center(std::get<0>(t))
		, a(std::get<1>(t).x)
		, b(std::get<1>(t).y)
	{}

	operator std::tuple<T, T, T>() const
	{
		return { this->center.x, this->center.y, this->a, this->b };
	}

	operator std::tuple<Point<T>, T, T>() const
	{
		return { this->center, this->a, this->b };
	}

	operator std::pair<Point<T>, Point<T>>() const
	{
		return { this->center, { this->a, this->b } };
	}

	template<typename U>
	bool contains(const Point<U>& p) const
	{
		U term1 = p.x - U(center.x);
		U term2 = p.y - U(center.y);

		return
			(term1 * term1) / (a * a) + (term2 * term2) / (b * b) <= T(0);
	}
};
