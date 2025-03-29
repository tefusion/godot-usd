#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "usd/usd_prim_type.h"
#include "usd/usd_prim_value.h"

using namespace godot;

class UsdPrimValueSkeletonRoot : public UsdPrimValue {
	GDCLASS(UsdPrimValueSkeletonRoot, UsdPrimValue);

protected:
	static void _bind_methods();

public:
	virtual UsdPrimType::Type get_type() const override;
};

class UsdPrimValueSkeleton : public UsdPrimValue {
	GDCLASS(UsdPrimValueSkeleton, UsdPrimValue);

protected:
	static void _bind_methods();

public:
	virtual UsdPrimType::Type get_type() const override;
	String _to_string() const;

	TypedArray<Transform3D> get_bind_transforms() const;
	PackedStringArray get_joints() const;
	Array get_bone_lengths() const;
	TypedArray<Transform3D> get_rest_transforms() const;
};
