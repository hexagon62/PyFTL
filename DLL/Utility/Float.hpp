#include <cmath>
#include <concepts>
#include <limits>

namespace float_util
{

template<std::floating_point T>
void increment(T& x)
{
	x = std::nextafter(x, std::numeric_limits<T>::infinity());
}

template<std::floating_point T>
void decrement(T& x)
{
	x = std::nextafter(x, -std::numeric_limits<T>::infinity());
}

}
