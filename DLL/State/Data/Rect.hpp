#pragma once

#include "Point.hpp"

template<typename T>
struct Rect
{
	T x = T(0), y = T(0), w = T(0), h = T(0);

	Rect() = default;

	Rect(const T& x, const T& y, const T& w, const T& h)
		: x(x)
		, y(y)
		, w(w)
		, h(h)
	{}

	Rect(const Point<T>& pos, const Point<T>& dimensions)
		: x(pos.x)
		, y(pos.y)
		, w(dimensions.x)
		, h(dimensions.y)
	{}

	Rect(const std::tuple<T, T, T, T>& t)
		: x(std::get<0>(t))
		, y(std::get<1>(t))
		, w(std::get<2>(t))
		, h(std::get<3>(t))
	{}

	Rect(const std::pair<Point<T>, Point<T>>& t)
		: x(t.first.x)
		, y(t.first.y)
		, w(t.second.x)
		, h(t.second.y)
	{}

	operator std::tuple<T, T, T, T>() const
	{
		return { this->center, this->a, this->b };
	}

	operator std::pair<Point<T>, Point<T>>() const
	{
		return { { this->x, this->y }, { this->w, this->h } };
	}

	template<typename U>
	bool contains(const Point<U>& p) const
	{
		return
			p.x >= U(this->x) && p.x <= U(this->x + this->w) &&
			p.y >= U(this->y) && p.y <= U(this->y + this->h);
	}

	Point<T> center() const
	{
		return { this->x + this->w / T(2), this->y + this->h / T(2) };
	}
};
