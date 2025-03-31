#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/variant/typed_array.hpp>

#include "usd_common.h"
#include "usd_prim.h"
#include "usd_shade.h"

/// Represents a USD stage
/// Once loaded can't change values so this is a read-only object
class UsdStage : public godot::RefCounted {
	GDCLASS(UsdStage, RefCounted);

private:
	std::shared_ptr<tinyusdz::Stage> _stage;
	godot::String _loaded_path = "";

protected:
	static void _bind_methods();

public:
	static tinyusdz::Stage *load_stage(const godot::String &path);
	static godot::Ref<UsdStage> create(std::shared_ptr<tinyusdz::Stage> stage);

	bool load(const godot::String &path);
	bool is_valid() const;

	godot::Ref<UsdPrim> get_prim_at_path(godot::Ref<UsdPath> path) const;

	godot::TypedArray<UsdPrim> get_root_prims() const;

	void set_loaded_path(const godot::String &path) { _loaded_path = path; }
	godot::String get_loaded_path() const { return _loaded_path; }

	godot::Ref<UsdLoadedMaterials> extract_materials() const;

	godot::Vector3::Axis get_up_axis() const;

	UsdStage();
};
