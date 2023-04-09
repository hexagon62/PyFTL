#pragma once

#include "Point.hpp"

#include <tuple>

template<typename T>
struct Rect
{
	using component = T;

	T x = T(0), y = T(0), w = T(0), h = T(0);

	Rect() = default;

	template<typename U>
	Rect(const U& x, const U& y, const U& w, const U& h)
		: x(T(x))
		, y(T(y))
		, w(T(w))
		, h(T(h))
	{}

	template<typename U>
	Rect(const std::tuple<U, U, U, U>& t)
		: x(T(std::get<0>(t)))
		, y(T(std::get<1>(t)))
		, w(T(std::get<2>(t)))
		, h(T(std::get<3>(t)))
	{}

	template<typename U>
	Rect(const Point<U>& p1, const Point<U>& p2)
		: x(T(p1.x))
		, y(T(p1.y))
		, w(T(p2.x))
		, h(T(p2.y))
	{}

	template<typename U>
	Rect(const Rect<U>& other)
		: x(T(other.x))
		, y(T(other.y))
		, w(T(other.w))
		, h(T(other.h))
	{}

	operator std::tuple<T, T, T, T>() const
	{
		return { this->x, this->y, this->w, this->h };
	}

	bool contains(const Point<T>& p) const
	{
		return
			p.x >= this->x && p.x <= this->x + this->w &&
			p.y >= this->y && p.y <= this->y + this->h;
	}

	Point<T> topLeft() const
	{
		return { this->x, this->y };
	}

	Point<T> topCenter() const
	{
		return { this->x + this->w / T(2), this->y};
	}

	Point<T> topRight() const
	{
		return { this->x + this->w, this->y };
	}

	Point<T> middleLeft() const
	{
		return { this->x, this->y + this->h / T(2) };
	}

	Point<T> center() const
	{
		return { this->x + this->w / T(2), this->y + this->h / T(2) };
	}

	Point<T> middleRight() const
	{
		return { this->x + this->w, this->y + this->h / T(2) };
	}

	Point<T> bottomLeft() const
	{
		return { this->x, this->y + this->h };
	}

	Point<T> bottomCenter() const
	{
		return { this->x + this->w / T(2), this->y + this->h };
	}

	Point<T> bottomRight() const
	{
		return { this->x + this->w, this->y + this->h };
	}

	Rect& operator+=(const Point<T>& other)
	{
		this->x += other.x;
		this->y += other.y;
		return *this;
	}

	Rect& operator-=(const Point<T>& other)
	{
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}

	Rect operator+(const Point<T>& other) const
	{
		return { this->x + other.x, this->y + other.y, this->w, this->h };
	}

	Rect operator-(const Point<T>& other) const
	{
		return { this->x - other.x, this->y - other.y, this->w, this->h };
	}
};
