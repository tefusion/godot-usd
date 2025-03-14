#pragma once

#include <cstddef>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "core/usd_prim_type.h"
#include "godot_cpp/variant/packed_vector2_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include "prim-types.hh"
#include "stage.hh"
#include "usdGeom.hh"

using namespace godot;

// Small wrapper classes with methods for pretty printing and getting values
class UsdPrimValue : public RefCounted {
	GDCLASS(UsdPrimValue, RefCounted);

protected:
	const tinyusdz::Prim *_prim = nullptr;
	//stage holds prim, done to prevent dangling pointers. Stage is also needed for some tydra methods
	//
	std::shared_ptr<tinyusdz::Stage> _stage = nullptr;
	static void _bind_methods();

public:
	static Ref<UsdPrimValue> create(const tinyusdz::Prim *p_prim, std::shared_ptr<tinyusdz::Stage> p_stage);
	virtual UsdPrimType::Type get_type() const;
};

class UsdPrimValueXform : public UsdPrimValue {
	GDCLASS(UsdPrimValueXform, UsdPrimValue);

protected:
	static void _bind_methods();

public:
	virtual UsdPrimType::Type get_type() const override;
	String _to_string() const;

	Transform3D get_transform() const;
	String get_name() const;
};

class UsdPrimValueGeomMesh : public UsdPrimValue {
	GDCLASS(UsdPrimValueGeomMesh, UsdPrimValue);

protected:
	static void _bind_methods();

public:
	virtual UsdPrimType::Type get_type() const override;

	String get_name() const;
	PackedVector3Array get_points() const;
	PackedVector3Array get_normals() const;
	PackedVector2Array get_uvs() const;
	size_t get_face_count() const;

	String _to_string() const;
};
