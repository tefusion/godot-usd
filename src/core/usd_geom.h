#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "core/usd_prim_type.h"
#include "core/usd_prim_value.h"
#include "godot_cpp/core/binder_common.hpp"
#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include "prim-types.hh"

using namespace godot;

class UsdGeomPrimvar : public RefCounted {
	GDCLASS(UsdGeomPrimvar, RefCounted);

public:
	enum Interpolation {
		CONSTANT,
		UNIFORM,
		VARYING,
		VERTEX,
		FACEVARYING,
		INVALID
	};

	static Interpolation interpolation_from_internal(tinyusdz::Interpolation interpolation);
	static tinyusdz::Interpolation interpolation_to_internal(Interpolation interpolation);

	String get_name() const;
	void set_name(const String &name);

	Array get_values() const;
	void set_values(const Array &values);

	Interpolation get_interpolation() const;
	void set_interpolation(Interpolation interpolation);

	int get_element_size() const;
	void set_element_size(int element_size);

	PackedInt32Array get_indices() const;
	void set_indices(const PackedInt32Array &indices);
	/// Returns false if _indices is empty
	bool has_indices() const;

protected:
	static void _bind_methods();

private:
	String _name;
	Array _values;
	Interpolation _interpolation = INVALID;
	int _element_size = 1;
	PackedInt32Array _indices;
};

VARIANT_ENUM_CAST(UsdGeomPrimvar::Interpolation);

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
	// naming is oriented towards Godot ArrayMesh
	enum PrimVarType {
		PRIMVAR_TEX_UV,
		PRIMVAR_TEX_UV2,
		PRIMVAR_COLOR,
		PRIMVAR_BONES,
		PRIMVAR_WEIGHTS,
		PRIMVAR_INVALID,
	};

	static PrimVarType primvar_type_from_string(const String &name);
	static String primvar_type_to_string(PrimVarType type);

	virtual UsdPrimType::Type get_type() const override;

	String get_name() const;
	PackedVector3Array get_points() const;
	PackedVector3Array get_normals() const;
	PackedVector2Array get_uvs() const;
	size_t get_face_count() const;

	String _to_string() const;
};

VARIANT_ENUM_CAST(UsdPrimValueGeomMesh::PrimVarType);
