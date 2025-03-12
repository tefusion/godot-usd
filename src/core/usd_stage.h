#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "godot_cpp/variant/typed_array.hpp"
#include "tinyusdz.hh"

#include "usd_common.h"
#include "usd_prim.h"

using namespace godot;

class UsdStage : public RefCounted {
	GDCLASS(UsdStage, RefCounted);

private:
	std::shared_ptr<tinyusdz::Stage> _stage;

public:
	static tinyusdz::Stage *load_stage(const String &path);
	static Ref<UsdStage> create(std::shared_ptr<tinyusdz::Stage> stage);

	bool load(const String &path);
	static void _bind_methods();
	bool is_valid() const;

	Ref<UsdPrim> get_prim_at_path(Ref<UsdPath> path) const;

	TypedArray<UsdPrim> get_root_prims() const;

	UsdStage();
};
