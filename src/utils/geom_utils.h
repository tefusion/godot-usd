#pragma once

#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_int64_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/vector3.hpp"

/// ported from tinyusdz/src/tydra/render-data.cc TriangulatePolygon
bool triangulate_polygon(
		const godot::PackedVector3Array &points,
		const godot::PackedInt32Array &face_vertex_counts,
		const godot::PackedInt32Array &face_vertex_indices,
		godot::PackedInt32Array &triangulated_face_vertex_counts,
		godot::PackedInt32Array &triangulated_face_vertex_indices,
		godot::PackedInt64Array &triangulated_to_orig_face_vertex_index_map,
		godot::PackedInt32Array &triangulated_face_counts,
		godot::String &error);

//Up axis can be y or z. This converts the vector to have the correct up axis
inline godot::Vector3 apply_up_axis(const godot::Vector3 &value, const godot::Vector3::Axis axis) {
	if (axis == godot::Vector3::AXIS_Y) {
		return value;
	}
	return godot::Vector3(value.x, value.z, value.y);
}

godot::PackedVector3Array apply_up_axis(const godot::PackedVector3Array &array, const godot::Vector3::Axis axis);

godot::Transform3D apply_up_axis(const godot::Transform3D &transform, const godot::Vector3::Axis axis);

godot::Basis apply_up_axis(const godot::Basis &basis, const godot::Vector3::Axis axis);
