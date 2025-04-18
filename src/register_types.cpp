#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/classes/editor_plugin_registration.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "convert/editor_plugin.h"
#include "convert/godot_scene.h"
#include "convert/scene_format_importer.h"
#include "usd/usd_common.h"
#include "usd/usd_geom.h"
#include "usd/usd_prim.h"
#include "usd/usd_prim_type.h"
#include "usd/usd_prim_value.h"
#include "usd/usd_skel.h"
#include "usd/usd_stage.h"

namespace godot {
void gdextension_initialize(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		ClassDB::register_class<UsdPrim>();
		ClassDB::register_class<UsdStage>();
		ClassDB::register_class<UsdPath>();
		ClassDB::register_class<UsdPrimType>();
		ClassDB::register_class<UsdPrimValue>();
		ClassDB::register_class<UsdPrimValueXform>();
		ClassDB::register_class<UsdPrimValueGeomMesh>();
		ClassDB::register_class<UsdGeomPrimvar>();
		ClassDB::register_class<UsdPrimValueGeomMaterialSubset>();
		ClassDB::register_class<UsdLoadedMaterials>();
		ClassDB::register_class<UsdGeomMeshMaterialMap>();
		ClassDB::register_class<UsdGodotSceneConverter>();
		ClassDB::register_class<UsdPrimValueSkeleton>();
		ClassDB::register_class<UsdPrimValueSkeletonRoot>();
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		ClassDB::register_class<UsdSceneFormatImporter>();
		ClassDB::register_class<UsdEditorPlugin>();
		EditorPlugins::add_by_type<UsdEditorPlugin>();
	}
}

void gdextension_terminate(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
	}
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		EditorPlugins::remove_by_type<UsdEditorPlugin>();
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
} //namespace godot
