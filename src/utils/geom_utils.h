#pragma once

#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_int64_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/vector3.hpp"

using namespace godot;

/// ported from tinyusdz/src/tydra/render-data.cc TriangulatePolygon
bool triangulate_polygon(
		const PackedVector3Array &points,
		const PackedInt32Array &face_vertex_counts,
		const PackedInt32Array &face_vertex_indices,
		PackedInt32Array &triangulated_face_vertex_counts,
		PackedInt32Array &triangulated_face_vertex_indices,
		PackedInt64Array &triangulated_to_orig_face_vertex_index_map,
		PackedInt32Array &triangulated_face_counts,
		String &error);

//Up axis can be y or z. This converts the vector to have the correct up axis
inline Vector3 apply_up_axis(const Vector3 &value, const Vector3::Axis axis) {
	if (axis == Vector3::AXIS_Y) {
		return value;
	}
	return Vector3(value.x, value.z, value.y);
}

PackedVector3Array apply_up_axis(const PackedVector3Array &array, const Vector3::Axis axis);

Transform3D apply_up_axis(const Transform3D &transform, const Vector3::Axis axis);

Basis apply_up_axis(const Basis &basis, const Vector3::Axis axis);
