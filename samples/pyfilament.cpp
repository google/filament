#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/pybind11.h>

#include <imgui.h>

#include <utils/Path.h>

#include <filament/Engine.h>
#include <filament/DebugRegistry.h>
#include <filament/IndirectLight.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec4.h>
#include <math/norm.h>

#include "app/Config.h"
#include "app/IBL.h"
#include "app/FilamentApp.h"
#include "app/MeshAssimp.h"


namespace py = pybind11;

using namespace math;
using namespace filament;
using namespace filamat;
using namespace utils;

int add(int i, int j) {
    return i + j;
}

extern int main_(Config config, std::vector<std::string> files);

PYBIND11_MODULE(pyfilament, m) {
    // m.def("add", &add, "A function which adds two numbers");
    py::class_<FilamentApp>(m, "Filament_App")
        .def("run", &FilamentApp::run)
        .def("get", &FilamentApp::get);

    // py::class_<filament::Engine> engine(m, "engine");
    py::enum_<Engine::Backend>(m, "Backend")
        .value("Default", Engine::Backend::DEFAULT)
        .value("Opengl", Engine::Backend::OPENGL)
        .value("Vulkan", Engine::Backend::VULKAN)
        .export_values();


    py::class_<Config>(m, "Config")
        .def(py::init<>())
        .def_readwrite("title", &Config::title)
        .def_readwrite("ibl_dir", &Config::iblDirectory)
        .def_readwrite("scale", &Config::scale)
        .def_readwrite("split_view", &Config::splitView)
        .def_property("backend",
            [] (Config &c) { return c.backend;},
            [] (Config &c, std::string val) {
            c.backend = (val == "OPENGL" ?
                Engine::Backend::OPENGL :
                Engine::Backend::VULKAN);
            });
        // .def(py::init<std::string, std::string, float, bool, Engine::Backend>());

    m.def("main", &main_);
}
