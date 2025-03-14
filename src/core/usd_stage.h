#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "godot_cpp/variant/typed_array.hpp"
#include "tinyusdz.hh"

#include "usd_common.h"
#include "usd_prim.h"

using namespace godot;

/// Represents a USD stage
/// Once loaded can't change values so this is a read-only object
class UsdStage : public RefCounted {
	GDCLASS(UsdStage, RefCounted);

private:
	std::shared_ptr<tinyusdz::Stage> _stage;

protected:
	static void _bind_methods();

public:
	static tinyusdz::Stage *load_stage(const String &path);
	static Ref<UsdStage> create(std::shared_ptr<tinyusdz::Stage> stage);

	bool load(const String &path);
	bool is_valid() const;

	Ref<UsdPrim> get_prim_at_path(Ref<UsdPath> path) const;

	TypedArray<UsdPrim> get_root_prims() const;

	UsdStage();
};
