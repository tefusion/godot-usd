#include "utils/geom_utils.h"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/math.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/string.hpp"
#include <cmath>
#include <limits>

#include "external/mapbox/earcut/earcut.hpp"
#include "godot_cpp/variant/variant.hpp"

using namespace godot;

bool triangulate_polygon(
		const PackedVector3Array &points,
		const PackedInt32Array &face_vertex_counts,
		const PackedInt32Array &face_vertex_indices,
		PackedInt32Array &triangulated_face_vertex_counts,
		PackedInt32Array &triangulated_face_vertex_indices,
		PackedInt64Array &triangulated_to_orig_face_vertex_index_map,
		PackedInt32Array &triangulated_face_counts, String &error) {
	triangulated_face_vertex_counts.clear();
	triangulated_face_vertex_indices.clear();
	triangulated_to_orig_face_vertex_index_map.clear();
	triangulated_face_counts.clear();

	size_t face_index_offset = 0;

	// For each polygon(face)
	for (int i = 0; i < face_vertex_counts.size(); i++) {
		int npolys = face_vertex_counts[i];

		if (npolys < 3) {
			error = String("faceVertex count must be 3(triangle) or more(polygon), but got faceVertexCounts[{0}] = {1}").format(Array::make(i, npolys));
			return false;
		}

		if (face_index_offset + npolys > face_vertex_indices.size()) {
			error = String("Invalid faceVertexIndices or faceVertexCounts. faceVertex index exceeds faceVertexIndices.size() at [{0}]").format(Array::make(i));
			return false;
		}

		if (npolys == 3) {
			// No need for triangulation.
			triangulated_face_vertex_counts.push_back(3);
			triangulated_face_vertex_indices.push_back(face_vertex_indices[face_index_offset + 0]);
			triangulated_face_vertex_indices.push_back(face_vertex_indices[face_index_offset + 1]);
			triangulated_face_vertex_indices.push_back(face_vertex_indices[face_index_offset + 2]);
			triangulated_to_orig_face_vertex_index_map.push_back(face_index_offset + 0);
			triangulated_to_orig_face_vertex_index_map.push_back(face_index_offset + 1);
			triangulated_to_orig_face_vertex_index_map.push_back(face_index_offset + 2);
			triangulated_face_counts.push_back(1);
		} else if (npolys == 4) {
			// Use simple split
			// TODO: Split at shortest edge for better triangulation.
			triangulated_face_vertex_counts.push_back(3);
			triangulated_face_vertex_counts.push_back(3);

			triangulated_face_vertex_indices.push_back(face_vertex_indices[face_index_offset + 0]);
			triangulated_face_vertex_indices.push_back(face_vertex_indices[face_index_offset + 1]);
			triangulated_face_vertex_indices.push_back(face_vertex_indices[face_index_offset + 2]);

			triangulated_face_vertex_indices.push_back(face_vertex_indices[face_index_offset + 0]);
			triangulated_face_vertex_indices.push_back(face_vertex_indices[face_index_offset + 2]);
			triangulated_face_vertex_indices.push_back(face_vertex_indices[face_index_offset + 3]);

			triangulated_to_orig_face_vertex_index_map.push_back(face_index_offset + 0);
			triangulated_to_orig_face_vertex_index_map.push_back(face_index_offset + 1);
			triangulated_to_orig_face_vertex_index_map.push_back(face_index_offset + 2);
			triangulated_to_orig_face_vertex_index_map.push_back(face_index_offset + 0);
			triangulated_to_orig_face_vertex_index_map.push_back(face_index_offset + 2);
			triangulated_to_orig_face_vertex_index_map.push_back(face_index_offset + 3);
			triangulated_face_counts.push_back(2);
		} else {
			// Use double for accuracy. `float` precision may classify small-area polygon as degenerated.
			// Find the normal axis of the polygon using Newell's method
			Vector3 n(0, 0, 0);

			int vi0, vi0_2;

			for (int k = 0; k < npolys; ++k) {
				vi0 = face_vertex_indices[face_index_offset + k];

				int j = (k + 1) % npolys;
				vi0_2 = face_vertex_indices[face_index_offset + j];

				if (vi0 >= points.size()) {
					error = "Invalid vertex index.";
					return false;
				}

				if (vi0_2 >= points.size()) {
					error = "Invalid vertex index.";
					return false;
				}

				Vector3 v0 = points[vi0];
				Vector3 v1 = points[vi0_2];

				Vector3 point1(v0.x, v0.y, v0.z);
				Vector3 point2(v1.x, v1.y, v1.z);

				Vector3 a(point1.x - point2.x, point1.y - point2.y, point1.z - point2.z);
				Vector3 b(point1.x + point2.x, point1.y + point2.y, point1.z + point2.z);

				n.x += a.y * b.z;
				n.y += a.z * b.x;
				n.z += a.x * b.y;
			}

			real_t length_n = n.length();

			// Check if zero length normal
			if (Math::abs(length_n) < std::numeric_limits<double>::epsilon()) {
				error = "Degenerated polygon found.";
				return false;
			}

			// Normalize the normal vector
			n = n.normalized();

			Vector3 axis_w, axis_v, axis_u;
			axis_w.x = n.x;
			axis_w.y = n.y;
			axis_w.z = n.z;

			Vector3 a;
			if (Math::abs(axis_w.x) > 0.9999999) {
				a = Vector3(0, 1, 0);
			} else {
				a = Vector3(1, 0, 0);
			}

			axis_v = a.cross(axis_w).normalized();
			axis_u = axis_w.cross(axis_v);

			using Point2D = std::array<real_t, 2>;
			std::vector<Point2D> polyline;

			// Fill polygon data
			for (int k = 0; k < npolys; k++) {
				int vidx = face_vertex_indices[face_index_offset + k];

				Vector3 v = points[vidx];

				// world to local
				real_t x = v.dot(axis_u);
				real_t y = v.dot(axis_v);

				polyline.push_back({ x, y });
			}

			std::vector<std::vector<Point2D>> polygon_2d;
			polygon_2d.push_back(polyline);

			std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon_2d);
			//  => result = 3 * faces, clockwise

			if ((indices.size() % 3) != 0) {
				// This should not happen, though.
				error = "Failed to triangulate.";
				return false;
			}

			size_t ntris = indices.size() / 3;

			// Up to 2GB tris.
			if (ntris > size_t((std::numeric_limits<int32_t>::max)())) {
				error = "Too many triangles are generated.";
				return false;
			}

			for (size_t k = 0; k < ntris; k++) {
				triangulated_face_vertex_counts.push_back(3);
				triangulated_face_vertex_indices.push_back(face_vertex_indices[face_index_offset + indices[3 * k + 0]]);
				triangulated_face_vertex_indices.push_back(face_vertex_indices[face_index_offset + indices[3 * k + 1]]);
				triangulated_face_vertex_indices.push_back(face_vertex_indices[face_index_offset + indices[3 * k + 2]]);

				triangulated_to_orig_face_vertex_index_map.push_back(face_index_offset + indices[3 * k + 0]);
				triangulated_to_orig_face_vertex_index_map.push_back(face_index_offset + indices[3 * k + 1]);
				triangulated_to_orig_face_vertex_index_map.push_back(face_index_offset + indices[3 * k + 2]);
			}
			triangulated_face_counts.push_back(ntris);
		}

		face_index_offset += npolys;
	}

	return true;
}

PackedVector3Array apply_up_axis(const PackedVector3Array &array, const Vector3::Axis axis) {
	if (axis == Vector3::AXIS_Y) {
		return array;
	}

	PackedVector3Array result;
	result.resize(array.size());
	for (int i = 0; i < array.size(); i++) {
		result.set(i, apply_up_axis(array[i], axis));
	}
	return result;
}

Transform3D apply_up_axis(const Transform3D &transform, const Vector3::Axis axis) {
	if (axis == Vector3::AXIS_Y) {
		return transform;
	}

	Transform3D result;
	result.basis = apply_up_axis(transform.basis, axis);
	result.origin = apply_up_axis(transform.origin, axis);
	return result;
}

Basis apply_up_axis(const Basis &basis, const Vector3::Axis axis) {
	if (axis == Vector3::AXIS_Y) {
		return basis;
	}

	return basis.rotated(Vector3(1, 0, 0), Math_PI / 2.0);
}
