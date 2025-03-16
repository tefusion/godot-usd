#pragma once

#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_int64_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/string.hpp"

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
