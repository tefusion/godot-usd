#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "prim-types.hh"
#include "usd/usd_common.h"
#include "usd/usd_prim_type.h"
#include "usd/usd_prim_value.h"
#include "usd/usd_skel.h"

class UsdGeomPrimvar : public godot::RefCounted {
	GDCLASS(UsdGeomPrimvar, godot::RefCounted);

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

	godot::String get_name() const;
	void set_name(const godot::String &name);

	godot::Array get_values() const;
	void set_values(const godot::Array &values);

	Interpolation get_interpolation() const;
	void set_interpolation(Interpolation interpolation);

	int get_element_size() const;
	void set_element_size(int element_size);

	godot::PackedInt32Array get_indices() const;
	void set_indices(const godot::PackedInt32Array &indices);
	/// Returns false if _indices is empty
	bool has_indices() const;

	godot::String _to_string() const;

protected:
	static void _bind_methods();

private:
	godot::String _name;
	godot::Array _values;
	Interpolation _interpolation = INVALID;
	int _element_size = 1;
	godot::PackedInt32Array _indices;
};

VARIANT_ENUM_CAST(UsdGeomPrimvar::Interpolation);

class UsdPrimValueXform : public UsdPrimValue {
	GDCLASS(UsdPrimValueXform, UsdPrimValue);

protected:
	static void _bind_methods();

public:
	virtual UsdPrimType::Type get_type() const override;
	godot::String _to_string() const;

	godot::Transform3D get_transform() const;
	godot::String get_name() const;
};

/// For now just handling the materialBind subset family to avoid having to create a generic class
class UsdPrimValueGeomMaterialSubset : public UsdPrimValue {
	GDCLASS(UsdPrimValueGeomMaterialSubset, UsdPrimValue);

public:
	enum ElementType {
		FACE,
		POINT,
	};

	static ElementType element_type_from_internal(tinyusdz::GeomSubset::ElementType type);
	static tinyusdz::GeomSubset::ElementType element_type_to_internal(ElementType type);

	godot::String get_name() const;
	void set_name(const godot::String &name);

	godot::Ref<UsdPath> get_bound_material() const;
	void set_bound_material(godot::Ref<UsdPath> path);

	/// For Material subsets this should always be FACE
	ElementType get_element_type() const;
	void set_element_type(ElementType type);

	godot::PackedInt32Array get_indices() const;
	void set_indices(const godot::PackedInt32Array &indices);

	godot::String _to_string() const;

protected:
	static void _bind_methods();

private:
	godot::String _name;
	godot::Ref<UsdPath> _bound_material_path;
	ElementType _element_type = FACE;
	godot::PackedInt32Array _indices;
};

VARIANT_ENUM_CAST(UsdPrimValueGeomMaterialSubset::ElementType);

class UsdGeomMeshMaterialMap : public godot::RefCounted {
	GDCLASS(UsdGeomMeshMaterialMap, godot::RefCounted);

private:
	// ElementTypes are always FACE
	godot::PackedInt32Array _face_material_indices;
	godot::TypedArray<UsdPath> _materials;
	godot::PackedStringArray _surface_names;

protected:
	static void _bind_methods();

public:
	godot::PackedInt32Array get_face_material_indices() const;
	void set_face_material_indices(const godot::PackedInt32Array &indices);

	/// Returns false if _face_material_indices is empty. This is done if 1 or no materials are bound
	bool is_mapped() const;

	godot::TypedArray<UsdPath> get_materials() const;
	void set_materials(const godot::TypedArray<UsdPath> &materials);

	godot::PackedStringArray get_surface_names() const;
	void set_surface_names(const godot::PackedStringArray &names);

	godot::String _to_string() const;
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

	static PrimVarType primvar_type_from_string(const godot::String &name);
	static godot::PackedStringArray primvar_type_to_string(const PrimVarType type);

	/// Returns used primvar name. If empty, the primvar is not present
	godot::String get_primvar_name(const PrimVarType type) const;
	bool has_primvar(const PrimVarType type) const;
	godot::Ref<UsdGeomPrimvar> get_primvar(const PrimVarType type) const;
	godot::Array get_primvars() const;

	bool has_geom_bind_transform() const;
	godot::Transform3D get_geom_bind_transform() const;

	virtual UsdPrimType::Type get_type() const override;

	godot::String get_name() const;
	godot::PackedVector3Array get_points() const;
	godot::PackedVector3Array get_normals() const;
	size_t get_face_count() const;
	godot::PackedInt32Array get_face_vertex_counts() const;
	godot::PackedInt32Array get_face_vertex_indices() const;

	bool has_directly_bound_material() const;
	godot::Ref<UsdPath> get_directly_bound_material() const;
	godot::Vector<godot::Ref<UsdPrimValueGeomMaterialSubset>> get_subset_materials() const;
	godot::TypedArray<UsdPrimValueGeomMaterialSubset> get_subset_materials_godot() const;
	godot::Ref<UsdGeomMeshMaterialMap> get_material_map() const;

	bool has_skeleton() const;
	godot::Ref<UsdPrimValueSkeleton> get_skeleton() const;

	godot::String _to_string() const;
};

VARIANT_ENUM_CAST(UsdPrimValueGeomMesh::PrimVarType);
