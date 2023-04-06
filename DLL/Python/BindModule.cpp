#include "Bind.hpp"

namespace python_bindings
{

void bindReader(py::module_& module);
void bindGeometry(py::module_& module);
void bindSystems(py::module_& module);
void bindHacking(py::module_& module);
void bindWeapons(py::module_& module);
void bindDrones(py::module_& module);
void bindCrew(py::module_& module);
void bindShipLayout(py::module_& module);
void bindShip(py::module_& module);
void bindSpace(py::module_& module);
void bindEvents(py::module_& module);
void bindStores(py::module_& module);
void bindStarMap(py::module_& module);
void bindMisc(py::module_& module);
void bindSettings(py::module_& module);
void bindBlueprints(py::module_& module);

}

PYBIND11_EMBEDDED_MODULE(ftl, module)
{
	using namespace python_bindings;

	bindReader(module);
	bindGeometry(module);
	bindSystems(module);
	bindWeapons(module);
	bindDrones(module);
	bindCrew(module);
	bindHacking(module);
	bindShipLayout(module);
	bindShip(module);
	bindSpace(module);
	bindEvents(module);
	bindStores(module);
	bindStarMap(module);
	bindMisc(module);
	bindSettings(module);
	bindBlueprints(module);
}
