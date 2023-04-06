#include "Bind.hpp"
#include "../State/Reader.hpp"

namespace python_bindings
{

void bindReader(py::module_& module)
{
	module
		.def("state", &Reader::getState, py::return_value_policy::reference)
		.def("get_poll_delay", &Reader::getPollDelay, py::return_value_policy::reference)
		.def("set_poll_delay", &Reader::setPollDelay, py::return_value_policy::reference)
		.def("use_seperate_polling_thread", &Reader::setSeperateThread, py::arg("on") = true)
		.def("is_polling_thread_separated", &Reader::usingSeperateThread)
		.def("reload", &Reader::reload)
		.def("quit", &Reader::quit)
		;

	py::class_<ValueScopeGuard<bool>>(module, "ScopedBool", "A boolean that will automatically reset its value after leaving the scope it's defined in")
		.def(py::init<bool&, bool, bool>())
		.def("__enter__", &ValueScopeGuard<bool>::acquire)
		.def("__exit__", &ValueScopeGuard<bool>::release)
		.def("acquire", &ValueScopeGuard<bool>::acquire, "Sets the locked value")
		.def("release", &ValueScopeGuard<bool>::release, "Sets the unlocked value")
		.def("locked", &ValueScopeGuard<bool>::locked, "Checks if using the locked value")
		;
}

}