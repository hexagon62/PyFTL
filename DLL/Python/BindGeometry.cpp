#include "Bind.hpp"
#include "../State/Data/Point.hpp"
#include "../State/Data/Rect.hpp"
#include "../State/Data/Ellipse.hpp"

namespace python_bindings
{

void bindGeometry(py::module_& module)
{
	py::class_<Point<int>>(module, "PointInt", "A point encoded in integers")
		.def(py::init<>())
		.def(py::init<const Point<int>&>())
		.def(py::init<const int&, const int&>())
		.def(py::init<const std::pair<int, int>&>())
		.def(py::init<const Point<float>&>())
		.def(py::init<const float&, const float&>())
		.def(py::init<const std::pair<float, float>&>())
		.def_readonly("x", &Point<int>::x)
		.def_readonly("y", &Point<int>::y)
		.def("__add__", &Point<int>::operator+)
		.def("__sub__", &Point<int>::operator-)
		.def("__mul__", &Point<int>::operator*)
		.def("__truediv__", &Point<int>::operator/)
		;

	py::implicitly_convertible<std::pair<int, int>, Point<int>>();

	py::class_<Point<float>>(module, "PointFloat", "A point encoded in floating point numbers")
		.def(py::init<>())
		.def(py::init<const Point<float>&>())
		.def(py::init<const float&, const float&>())
		.def(py::init<const std::pair<float, float>&>())
		.def(py::init<const Point<int>&>())
		.def(py::init<const int&, const int&>())
		.def(py::init<const std::pair<int, int>&>())
		.def_readonly("x", &Point<float>::x)
		.def_readonly("y", &Point<float>::y)
		.def("__add__", &Point<float>::operator+)
		.def("__sub__", &Point<float>::operator-)
		.def("__mul__", &Point<float>::operator*)
		.def("__truediv__", &Point<float>::operator/)
		;

	py::implicitly_convertible<std::pair<float, float>, Point<float>>();

	py::class_<Rect<int>>(module, "RectInt", "A rectangle encoded in integers")
		.def(py::init<>())
		.def(py::init<const Rect<int>&>())
		.def(py::init<const int&, const int&, const int&, const int&>())
		.def(py::init<const Point<int>&, const Point<int>&>())
		.def(py::init<const std::tuple<int, int, int, int>&>())
		.def(py::init<const std::pair<Point<int>, Point<int>>&>())
		.def_readonly("x", &Rect<int>::x)
		.def_readonly("y", &Rect<int>::y)
		.def_readonly("w", &Rect<int>::w)
		.def_readonly("h", &Rect<int>::h)
		.def("contains", py::overload_cast<const Point<int>&>(&Rect<int>::contains<int>, py::const_), "Check if the rectangle contains the point")
		.def("contains", py::overload_cast<const Point<float>&>(&Rect<int>::contains<float>, py::const_), "Check if the rectangle contains the point")
		.def("center", &Rect<int>::center)
		;

	py::implicitly_convertible<std::tuple<int, int, int, int>, Rect<int>>();
	py::implicitly_convertible<std::pair<Point<int>, Point<int>>, Rect<int>>();

	py::class_<Rect<float>>(module, "RectFloat", "A rectangle encoded in floating point numbers")
		.def(py::init<>())
		.def(py::init<const Rect<float>&>())
		.def(py::init<const float&, const float&, const float&, const float&>())
		.def(py::init<const Point<float>&, const Point<float>&>())
		.def(py::init<const std::tuple<float, float, float, float>&>())
		.def(py::init<const std::pair<Point<float>, Point<float>>&>())
		.def_readonly("x", &Rect<float>::x, "The x position of the upper left corner")
		.def_readonly("y", &Rect<float>::y, "The y position of the upper right corner")
		.def_readonly("w", &Rect<float>::w, "The width")
		.def_readonly("h", &Rect<float>::h, "The height")
		.def("contains", py::overload_cast<const Point<int>&>(&Rect<float>::contains<int>, py::const_), "Check if the rectangle contains the point")
		.def("contains", py::overload_cast<const Point<float>&>(&Rect<float>::contains<float>, py::const_), "Check if the rectangle contains the point")
		.def("center", &Rect<float>::center, "Get the center of the rectangle")
		;

	py::implicitly_convertible<std::tuple<float, float, float, float>, Rect<float>>();
	py::implicitly_convertible<std::pair<Point<float>, Point<float>>, Rect<float>>();

	py::class_<Ellipse<int>>(module, "EllipseInt", "An ellipse encoded in integers")
		.def(py::init<>())
		.def(py::init<const Ellipse<int>&>())
		.def(py::init<const int&, const int&, const int&, const int&>())
		.def(py::init<const Point<int>&, const int&, const int&>())
		.def(py::init<const Point<int>&, const Point<int>&>())
		.def(py::init<const std::tuple<int, int, int, int>&>())
		.def(py::init<const std::tuple<Point<int>, int, int>&>())
		.def(py::init<const std::pair<Point<int>, Point<int>>&>())
		.def_readonly("center", &Ellipse<int>::center, "The center")
		.def_readonly("a", &Ellipse<int>::a, "The x-axis")
		.def_readonly("b", &Ellipse<int>::b, "The y-axis")
		.def("contains", py::overload_cast<const Point<int>&>(&Ellipse<int>::contains<int>, py::const_), "Check if the ellipse contains the point")
		.def("contains", py::overload_cast<const Point<float>&>(&Ellipse<int>::contains<float>, py::const_), "Check if the ellipse contains the point")
		;

	py::implicitly_convertible<std::tuple<int, int, int, int>, Ellipse<int>>();
	py::implicitly_convertible<std::tuple<Point<int>, int, int>, Ellipse<int>>();
	py::implicitly_convertible<std::pair<Point<int>, Point<int>>, Ellipse<int>>();

	py::class_<Ellipse<float>>(module, "EllipseFloat", "An ellipse encoded in floating point numbers")
		.def(py::init<>())
		.def(py::init<const Ellipse<float>&>())
		.def(py::init<const float&, const float&, const float&, const float&>())
		.def(py::init<const Point<float>&, const float&, const float&>())
		.def(py::init<const Point<float>&, const Point<float>&>())
		.def(py::init<const std::tuple<float, float, float, float>&>())
		.def(py::init<const std::tuple<Point<float>, float, float>&>())
		.def(py::init<const std::pair<Point<float>, Point<float>>&>())
		.def_readonly("center", &Ellipse<float>::center, "The center")
		.def_readonly("a", &Ellipse<float>::a, "The x-axis")
		.def_readonly("b", &Ellipse<float>::b, "The y-axis")
		.def("contains", py::overload_cast<const Point<int>&>(&Ellipse<float>::contains<int>, py::const_), "Check if the ellipse contains the point")
		.def("contains", py::overload_cast<const Point<float>&>(&Ellipse<float>::contains<float>, py::const_), "Check if the ellipse contains the point")
		;

	py::implicitly_convertible<std::tuple<float, float, float, float>, Ellipse<float>>();
	py::implicitly_convertible<std::tuple<Point<float>, float, float>, Ellipse<float>>();
	py::implicitly_convertible<std::pair<Point<float>, Point<float>>, Ellipse<float>>();
}

}
