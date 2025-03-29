#pragma once

#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "value-types.hh"
#include <prim-types.hh>

// General conversion function from TinyUSDZ value to Godot Variant
// This handles all the supported types
godot::Variant to_variant(const tinyusdz::value::Value &usd_value);

// TODO check if something like this really doesn't already exist
inline godot::Variant::Type get_godot_type(const godot::Vector3 &) { return godot::Variant::VECTOR3; }
inline godot::Variant::Type get_godot_type(const godot::Transform3D &) { return godot::Variant::TRANSFORM3D; }
inline godot::Variant::Type get_godot_type(const godot::Quaternion &) { return godot::Variant::QUATERNION; }
inline godot::Variant::Type get_godot_type(const double &) { return godot::Variant::FLOAT; }
inline godot::Variant::Type get_godot_type(const float &) { return godot::Variant::FLOAT; }
inline godot::Variant::Type get_godot_type(const int &) { return godot::Variant::INT; }
inline godot::Variant::Type get_godot_type(const bool &) { return godot::Variant::BOOL; }
inline godot::Variant::Type get_godot_type(const godot::String &) { return godot::Variant::STRING; }
inline godot::Variant::Type get_godot_type(const godot::Color &) { return godot::Variant::COLOR; }
inline godot::Variant::Type get_godot_type(const godot::Basis &) { return godot::Variant::BASIS; }
///////////////////////////////////////////

template <typename T>
bool xform_get_value(const tinyusdz::XformOp &xform, T &result) {
	auto scalar = xform.get_scalar();
	if (scalar) {
		godot::Variant variant_result = to_variant(scalar.value());
		if (variant_result.get_type() == get_godot_type(result)) {
			result = variant_result;
			return true;
		}
	}
	return false;
}

template <typename T>
const T *get_typed_prim(const tinyusdz::Prim *_prim) {
	if (!_prim) {
		return nullptr;
	}

	if (!_prim->is<T>()) {
		return nullptr;
	}

	return _prim->as<T>();
}
