#include "editor_plugin.h"
#include "scene_format_importer.h"
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void UsdEditorPlugin::_bind_methods() {
}

void UsdEditorPlugin::_enter_tree() {
	Ref<UsdSceneFormatImporter> importer;
	importer.instantiate();
	add_scene_format_importer_plugin(importer);
}

void UsdEditorPlugin::_exit_tree() {
	Ref<UsdSceneFormatImporter> importer;
	importer.instantiate();
	remove_scene_format_importer_plugin(importer);
}
