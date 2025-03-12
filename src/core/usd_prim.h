#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "tinyusdz.hh"
#include "usd_common.h"

using namespace godot;

class UsdPrim : public RefCounted {
	GDCLASS(UsdPrim, RefCounted);

private:
	std::shared_ptr<tinyusdz::Stage> _stage;
	Ref<UsdPath> _path;

public:
	enum UsdPrimType {
		USD_PRIM_TYPE_XFORM,
		USD_PRIM_TYPE_MESH,
		USD_PRIM_TYPE_CAMERA,
		USD_PRIM_TYPE_LIGHT,
		USD_PRIM_TYPE_MATERIAL,
		USD_PRIM_TYPE_SHADER,
		USD_PRIM_TYPE_TEXTURE,
		USD_PRIM_TYPE_UNKNOWN,
	};

	static Ref<UsdPrim> create(std::shared_ptr<tinyusdz::Stage> stage, const tinyusdz::Path &path);
	static void _bind_methods();

	String get_type_name() const;
	UsdPrimType get_type() const;
	int get_type_gd() const;

	bool is_valid() const;

	void set_path(Ref<UsdPath> path);
	Ref<UsdPath> get_path() const;

	UsdPrim();
};

VARIANT_ENUM_CAST(UsdPrim::UsdPrimType);
