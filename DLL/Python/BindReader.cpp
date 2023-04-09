#include "Bind.hpp"
#include "../Reader.hpp"

namespace python_bindings
{

void bindReader(py::module_& module)
{
	module.def(
		"state",
		&Reader::getState,
		py::return_value_policy::reference,
		"Returns the current state of the game"
	);

	module.def(
		"set_poll_time",
		&Reader::setPollTime,
		py::arg("time") = Reader::DEFAULT_POLL_TIME,
		py::return_value_policy::reference,
		"Sets how long PyFTL waits before doing each poll & input in seconds"
	);

	module.def(
		"poll_time",
		&Reader::pollTime,
		"Returns how long PyFTL waits before doing each poll & input in seconds"
	);

	module.def(
		"now",
		&Reader::now,
		"Returns how long PyFTL has been running in seconds"
	);

	module.def(
		"reload",
		&Reader::reload,
		"Tells PyFTL to reimport your main module"
	);

	module.def(
		"quit",
		&Reader::quit,
		"Tells PyFTL to quit. Note this doesn't detach the injected DLL and you'll need to do that manually."
	);

	py::class_<ValueScopeGuard<bool>>(module, "ScopedBool",
		"A boolean that will automatically reset its value after leaving the scope it's defined in")
		.def(py::init<bool&, bool, bool>())
		.def("__enter__", &ValueScopeGuard<bool>::acquire)
		.def("__exit__", &ValueScopeGuard<bool>::release)
		.def("acquire", &ValueScopeGuard<bool>::acquire, "Sets the locked value")
		.def("release", &ValueScopeGuard<bool>::release, "Sets the unlocked value")
		.def("locked", &ValueScopeGuard<bool>::locked, "Checks if using the locked value")
		;
}

}