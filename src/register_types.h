#pragma once

#include <godot_cpp/core/class_db.hpp>

namespace godot {
void gdextension_initialize(ModuleInitializationLevel p_level);
void gdextension_terminate(ModuleInitializationLevel p_level);
} //namespace godot
