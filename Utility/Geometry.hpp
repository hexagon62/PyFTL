#pragma once

template<typename T>
struct Point
{
	T x = T{ 0 }, y = T{ 0 };

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

using Pointf = Point<float>;
using Pointi = Point<int>;

template<typename T>
struct Rect
{
	T left = T{ 0 };
	T top = T{ 0 };
	T width = T{ 0 };
	T height = T{ 0 };
};

using Rectf = Rect<float>;
using Recti = Rect<int>;
