#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "core/usd_common.h"
#include "core/usd_geom.h"
#include "core/usd_prim.h"
#include "core/usd_prim_type.h"
#include "core/usd_prim_value.h"
#include "core/usd_stage.h"

using namespace godot;

void gdextension_initialize(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		ClassDB::register_class<UsdPrim>();
		ClassDB::register_class<UsdStage>();
		ClassDB::register_class<UsdPath>();
		ClassDB::register_class<UsdPrimType>();
		ClassDB::register_class<UsdPrimValue>();
		ClassDB::register_class<UsdPrimValueXform>();
		ClassDB::register_class<UsdPrimValueGeomMesh>();
	}
}

void gdextension_terminate(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
	}
}

extern "C" {
GDExtensionBool GDE_EXPORT gdextension_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(gdextension_initialize);
	init_obj.register_terminator(gdextension_terminate);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
