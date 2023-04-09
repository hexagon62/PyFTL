#include "Bind.hpp"
#include "../State/Point.hpp"
#include "../State/Rect.hpp"
#include "../State/Ellipse.hpp"

namespace python_bindings
{

void bindGeometry(py::module_& module)
{
	py::class_<Point<int>>(module, "PointInt", "A point encoded in integers")
		.def(py::init<>())
		.def(py::init<const Point<int>&>())
		.def(py::init<const int&, const int&>())
		.def(py::init<const std::pair<int,int>&>())
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

	py::class_<Point<float>>(module, "PointFloat", "A point encoded in floating point numbers")
		.def(py::init<>())
		.def(py::init<const Point<int>&>())
		.def(py::init<const int&, const int&>())
		.def(py::init<const std::pair<int, int>&>())
		.def(py::init<const Point<float>&>())
		.def(py::init<const float&, const float&>())
		.def(py::init<const std::pair<float, float>&>())
		.def_readonly("x", &Point<float>::x)
		.def_readonly("y", &Point<float>::y)
		.def("__add__", &Point<float>::operator+)
		.def("__sub__", &Point<float>::operator-)
		.def("__mul__", &Point<float>::operator*)
		.def("__truediv__", &Point<float>::operator/)
		;

	py::implicitly_convertible<Point<int>, Point<float>>();
	py::implicitly_convertible<Point<float>, Point<int>>();
	//py::implicitly_convertible<Point<int>, std::pair<int, int>>();
	//py::implicitly_convertible<Point<int>, std::pair<float, float>>();
	//py::implicitly_convertible<Point<float>, std::pair<int, int>>();
	//py::implicitly_convertible<Point<float>, std::pair<float, float>>();
	py::implicitly_convertible<std::pair<int, int>, Point<int>>();
	py::implicitly_convertible<std::pair<int, int>, Point<float>>();
	py::implicitly_convertible<std::pair<float, float>, Point<int>>();
	py::implicitly_convertible<std::pair<float, float>, Point<float>>();

	py::class_<Rect<int>>(module, "RectInt", "A rectangle encoded in integers")
		.def(py::init<>())
		.def(py::init<const Rect<int>&>())
		.def(py::init<const int&, const int&, const int&, const int&>())
		.def(py::init<const Point<int>&, const Point<int>&>())
		.def(py::init<const Rect<float>&>())
		.def(py::init<const float&, const float&, const float&, const float&>())
		.def(py::init<const Point<float>&, const Point<float>&>())
		.def_readonly("x", &Rect<int>::x, "The x position of the upper left corner")
		.def_readonly("y", &Rect<int>::y, "The y position of the upper right corner")
		.def_readonly("w", &Rect<int>::w, "The width")
		.def_readonly("h", &Rect<int>::h, "The height")
		.def("contains", &Rect<int>::contains, "Check if the rectangle contains the point")
		.def("top_left", &Rect<int>::topLeft, "Get the top left corner of the rectangle")
		.def("top_center", &Rect<int>::topCenter, "Get the center of the top edge of the rectangle")
		.def("top_right", &Rect<int>::topRight, "Get the top right corner of the rectangle")
		.def("middle_left", &Rect<int>::middleLeft, "Get the center of the left edge of the rectangle")
		.def("center", &Rect<int>::center, "Get the center of the rectangle")
		.def("middle_right", &Rect<int>::middleRight, "Get the center of right edge of the rectangle")
		.def("bottom_left", &Rect<int>::bottomLeft, "Get the bottom left corner of the rectangle")
		.def("bottom_center", &Rect<int>::bottomCenter, "Get the center of the bottom edge of the rectangle")
		.def("bottom_right", &Rect<int>::bottomRight, "Get the bottom right corner of the rectangle")
		.def("__add__", &Rect<int>::operator+)
		.def("__sub__", &Rect<int>::operator-)
		;

	py::class_<Rect<float>>(module, "RectFloat", "A rectangle encoded in floating point numbers")
		.def(py::init<>())
		.def(py::init<const Rect<int>&>())
		.def(py::init<const int&, const int&, const int&, const int&>())
		.def(py::init<const Point<int>&, const Point<int>&>())
		.def(py::init<const Rect<float>&>())
		.def(py::init<const float&, const float&, const float&, const float&>())
		.def(py::init<const Point<float>&, const Point<float>&>())
		.def_readonly("x", &Rect<float>::x, "The x position of the upper left corner")
		.def_readonly("y", &Rect<float>::y, "The y position of the upper right corner")
		.def_readonly("w", &Rect<float>::w, "The width")
		.def_readonly("h", &Rect<float>::h, "The height")
		.def("contains", &Rect<float>::contains, "Check if the rectangle contains the point")
		.def("top_left", &Rect<float>::topLeft, "Get the top left corner of the rectangle")
		.def("top_center", &Rect<float>::topCenter, "Get the center of the top edge of the rectangle")
		.def("top_right", &Rect<float>::topRight, "Get the top right corner of the rectangle")
		.def("middle_left", &Rect<float>::middleLeft, "Get the center of the left edge of the rectangle")
		.def("center", &Rect<float>::center, "Get the center of the rectangle")
		.def("middle_right", &Rect<float>::middleRight, "Get the center of right edge of the rectangle")
		.def("bottom_left", &Rect<float>::bottomLeft, "Get the bottom left corner of the rectangle")
		.def("bottom_center", &Rect<float>::bottomCenter, "Get the center of the bottom edge of the rectangle")
		.def("bottom_right", &Rect<float>::bottomRight, "Get the bottom right corner of the rectangle")
		.def("__add__", &Rect<float>::operator+)
		.def("__sub__", &Rect<float>::operator-)
		;

	py::implicitly_convertible<Rect<int>, Rect<float>>();
	py::implicitly_convertible<Rect<float>, Rect<int>>();
	//py::implicitly_convertible<Rect<int>, std::tuple<int, int, int, int>>();
	//py::implicitly_convertible<Rect<int>, std::tuple<float, float, float, float>>();
	//py::implicitly_convertible<Rect<float>, std::tuple<int, int, int, int>>();
	//py::implicitly_convertible<Rect<float>, std::tuple<float, float, float, float>>();
	py::implicitly_convertible<std::tuple<int, int, int, int>, Rect<int>>();
	py::implicitly_convertible<std::tuple<int, int, int, int>, Rect<float>>();
	py::implicitly_convertible<std::tuple<float, float, float, float>, Rect<int>>();
	py::implicitly_convertible<std::tuple<float, float, float, float>, Rect<float>>();

	py::class_<Ellipse<float>>(module, "Ellipse", "An ellipse encoded in floating point numbers")
		.def(py::init<>())
		.def(py::init<const Ellipse<float>&>())
		.def(py::init<const Point<float>&, const float&, const float&>())
		.def(py::init<const float&, const float&, const float&, const float&>())
		.def_readonly("center", &Ellipse<float>::center, "The center")
		.def_readonly("a", &Ellipse<float>::a, "The x-axis")
		.def_readonly("b", &Ellipse<float>::b, "The y-axis")
		.def("contains", &Ellipse<float>::contains, "Check if the ellipse contains the point")
		.def("left", &Ellipse<float>::left, "Get the left-most point of the ellipse")
		.def("right", &Ellipse<float>::right, "Get the right-most point of the ellipse")
		.def("top", &Ellipse<float>::top, "Get the top-most point of the ellipse")
		.def("bottom", &Ellipse<float>::bottom, "Get the bottom-most point of the ellipse")
		.def("__add__", &Ellipse<float>::operator+)
		.def("__sub__", &Ellipse<float>::operator-)
		;

	//py::implicitly_convertible<Ellipse<float>, std::tuple<float, float, float, float>>();
	py::implicitly_convertible<std::tuple<float, float, float, float>, Ellipse<float>>();
}

}
