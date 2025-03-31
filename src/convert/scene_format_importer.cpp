#include "scene_format_importer.h"
#include "godot_scene.h"
#include "usd/usd_stage.h"
#include "utils/geom_utils.h"
#include "utils/godot_utils.h"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void UsdSceneFormatImporter::_bind_methods() {
}

PackedStringArray UsdSceneFormatImporter::_get_extensions() const {
	PackedStringArray extensions;
	extensions.push_back("usda");
	return extensions;
}

Object *UsdSceneFormatImporter::_import_scene(const String &p_path, uint32_t p_flags, const Dictionary &p_options) {
	Ref<UsdStage> stage;
	stage.instantiate();

	if (!stage->load(p_path)) {
		UtilityFunctions::push_error("Failed to load USD stage from path: ", p_path);
		return nullptr;
	}

	Ref<UsdGodotSceneConverter> converter;
	converter.instantiate();

	if (!converter->load(stage)) {
		UtilityFunctions::push_error("Failed to initialize scene converter with stage");
		return nullptr;
	}

	const Vector3::Axis up_axis = stage->get_up_axis();
	Vector<Ref<UsdPrim>> root_prims = typed_array_to_ref_vector(stage->get_root_prims());

	if (root_prims.is_empty()) {
		UtilityFunctions::push_error("No root prims found in USD stage");
		return nullptr;
	}

	// If single root prim is found use that as root, if not just use one named root with default transform
	Node3D *root_node = memnew(Node3D);
	root_node->set_name("root");

	if (root_prims.size() == 1 && root_prims[0]->get_type() == UsdPrimType::USD_PRIM_TYPE_XFORM) {
		Ref<UsdPrimValueXform> value = root_prims[0]->get_value();
		Transform3D xform = apply_up_axis(value->get_transform(), up_axis);
		root_node->set_transform(xform);
		root_node->set_name(value->get_name());

		root_prims = typed_array_to_ref_vector(root_prims[0]->get_children());
	}

	for (int i = 0; i < root_prims.size(); i++) {
		converter->convert_prim(root_prims[i], root_node, up_axis);
	}

	return root_node;
}

void UsdSceneFormatImporter::_get_import_options(const String &p_path) {
}

Variant UsdSceneFormatImporter::_get_option_visibility(const String &p_path, bool p_for_animation, const String &p_option) const {
	return true;
}
