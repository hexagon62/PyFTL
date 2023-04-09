#include <cmath>
#include <concepts>
#include <limits>

namespace float_util
{

template<std::floating_point T>
T increment(T x)
{
	return std::nextafter(x, std::numeric_limits<T>::infinity());
}

template<std::floating_point T>
T decrement(T x)
{
	return std::nextafter(x, -std::numeric_limits<T>::infinity());
}

}
