#include "Bind.hpp"
#include "../State/Reader.hpp"

namespace python_bindings
{

void bindReader(py::module_& module)
{
	module.def("state", &Reader::getState, py::return_value_policy::reference, "Returns the current state of the game")
		.def("get_poll_delay", &Reader::getPollDelay, py::return_value_policy::reference, "Returns how long PyFTL waits before doing each poll & input")
		.def("set_poll_delay", &Reader::setPollDelay, py::arg("delay") = Reader::DEFAULT_POLL_DELAY, py::return_value_policy::reference, "Sets how long PyFTL waits before doing each poll & input")
		.def("use_seperate_polling_thread", &Reader::setSeperateThread, py::arg("on") = true, "Tells PyFTL to call on_update and do polls in separate threads")
		.def("is_polling_thread_separated", &Reader::usingSeperateThread, "Checks if on_update and polling are in separate threads")
		.def("reload", &Reader::reload, "Tells PyFTL to reload your code")
		.def("quit", &Reader::quit, "Tells PyFTL to quit. Note this doesn't detach the injected DLL and you'll need to do that manually.")
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