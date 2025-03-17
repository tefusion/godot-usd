#pragma once

#include "godot_cpp/classes/object.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class UsdPrimType : public Object {
	GDCLASS(UsdPrimType, Object);

public:
	enum Type {
		USD_PRIM_TYPE_XFORM,
		USD_PRIM_TYPE_MESH,
		USD_PRIM_TYPE_CAMERA,
		USD_PRIM_TYPE_MATERIAL,
		USD_PRIM_TYPE_SHADER,
		USD_PRIM_TYPE_SCOPE,
		USD_PRIM_TYPE_GEOM_SUBSET,
		USD_PRIM_TYPE_UNKNOWN,
	};

protected:
	static void _bind_methods();
};

VARIANT_ENUM_CAST(UsdPrimType::Type);
