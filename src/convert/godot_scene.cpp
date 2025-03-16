#include "godot_scene.h"
#include "godot_cpp/classes/array_mesh.hpp"
#include "godot_cpp/variant/array.hpp"
#include "utils/geom_utils.h"

void UsdGodotSceneConverter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("convert_mesh", "geom_mesh", "materials"), &UsdGodotSceneConverter::convert_mesh);
}

UsdGodotSceneConverter::UsdGodotSceneConverter() {
}

UsdGodotSceneConverter::~UsdGodotSceneConverter() {
}

Ref<ArrayMesh> UsdGodotSceneConverter::convert_mesh(const Ref<UsdPrimValueGeomMesh> &geom_mesh, const Ref<UsdLoadedMaterials> &materials) {
	ERR_FAIL_COND_V_MSG(geom_mesh.is_null(), nullptr, "GeomMesh is null");

	Ref<ArrayMesh> mesh;
	mesh.instantiate();

	PackedVector3Array points = geom_mesh->get_points();
	PackedVector3Array normals = geom_mesh->get_normals();
	PackedInt32Array face_vertex_counts = geom_mesh->get_face_vertex_counts();
	PackedInt32Array face_vertex_indices = geom_mesh->get_face_vertex_indices();

	Ref<UsdGeomMeshMaterialMap> material_map = geom_mesh->get_material_map();

	Ref<UsdGeomPrimvar> uv_primvar;
	bool has_uvs = geom_mesh->has_primvar(UsdPrimValueGeomMesh::PRIMVAR_TEX_UV);
	if (has_uvs) {
		uv_primvar = geom_mesh->get_primvar(UsdPrimValueGeomMesh::PRIMVAR_TEX_UV);
	}

	PackedInt32Array triangulated_face_vertex_counts;
	PackedInt32Array triangulated_face_vertex_indices;
	PackedInt64Array triangulated_to_orig_face_vertex_index_map;
	PackedInt32Array triangulated_face_counts;
	String error;

	bool success = triangulate_polygon(
			points,
			face_vertex_counts,
			face_vertex_indices,
			triangulated_face_vertex_counts,
			triangulated_face_vertex_indices,
			triangulated_to_orig_face_vertex_index_map,
			triangulated_face_counts,
			error);

	ERR_FAIL_COND_V_MSG(!success, nullptr, "Failed to triangulate mesh: " + error);

	PackedInt32Array face_material_indices;
	TypedArray<UsdPath> material_paths;
	PackedStringArray surface_names;
	bool has_mapped_materials = material_map->is_mapped();

	if (has_mapped_materials) {
		face_material_indices = material_map->get_face_material_indices();
		material_paths = material_map->get_materials();
		surface_names = material_map->get_surface_names();
	} else {
		material_paths = material_map->get_materials();
	}

	HashMap<int, Vector<int>> surface_indices;

	int face_offset = 0;
	int tri_face_index = 0;

	if (has_mapped_materials) {
		for (int i = 0; i < triangulated_face_counts.size(); i++) {
			int num_tris = triangulated_face_counts[i];
			int material_idx = face_material_indices[i];

			if (!surface_indices.has(material_idx)) {
				surface_indices[material_idx] = Vector<int>();
			}

			for (int j = 0; j < num_tris; j++) {
				surface_indices[material_idx].push_back(tri_face_index);
				tri_face_index++;
			}

			face_offset += face_vertex_counts[i];
		}
	} else {
		int material_idx = 0;

		int total_tris = 0;
		for (int i = 0; i < triangulated_face_counts.size(); i++) {
			total_tris += triangulated_face_counts[i];
		}

		Vector<int> all_triangles;
		all_triangles.resize(total_tris);
		for (int i = 0; i < total_tris; i++) {
			all_triangles.write[i] = i;
		}
		surface_indices[material_idx] = all_triangles;
	}

	for (const KeyValue<int, Vector<int>> &kv : surface_indices) {
		int material_idx = kv.key;
		const Vector<int> &triangle_indices = kv.value;

		Array surface_arrays;
		surface_arrays.resize(ArrayMesh::ARRAY_MAX);

		PackedVector3Array surface_vertices;
		PackedVector3Array surface_normals;
		PackedVector2Array surface_uvs;
		PackedInt32Array surface_indices;

		for (int tri_idx : triangle_indices) {
			int base_idx = tri_idx * 3;

			for (int j = 0; j < 3; j++) {
				int vertex_index = triangulated_face_vertex_indices[base_idx + j];
				surface_vertices.push_back(points[vertex_index]);

				if (!normals.is_empty() && vertex_index < normals.size()) {
					surface_normals.push_back(normals[vertex_index]);
				}

				surface_indices.push_back(surface_vertices.size() - 1);
			}
		}

		if (has_uvs) {
			Array uv_values = uv_primvar->get_values();
			UsdGeomPrimvar::Interpolation interp = uv_primvar->get_interpolation();

			if (interp == UsdGeomPrimvar::VERTEX) {
				surface_uvs.resize(surface_vertices.size());

				for (int i = 0; i < triangle_indices.size(); i++) {
					int tri_idx = triangle_indices[i];
					int base_idx = tri_idx * 3;

					for (int j = 0; j < 3; j++) {
						int vertex_index = triangulated_face_vertex_indices[base_idx + j];
						int surface_idx = i * 3 + j;

						if (vertex_index < uv_values.size()) {
							Vector2 uv = uv_values[vertex_index];
							uv.y = 1.0 - uv.y;
							surface_uvs[surface_idx] = uv;
						}
					}
				}
			} else if (interp == UsdGeomPrimvar::FACEVARYING) {
				surface_uvs.resize(surface_vertices.size());

				for (int i = 0; i < surface_indices.size(); i++) {
					int orig_face_idx = triangulated_to_orig_face_vertex_index_map[i];

					if (orig_face_idx < uv_values.size()) {
						Vector2 uv = uv_values[orig_face_idx];
						uv.y = 1.0 - uv.y;
						surface_uvs[i] = uv;
					}
				}
			}
		}

		surface_arrays[ArrayMesh::ARRAY_VERTEX] = surface_vertices;

		if (!surface_normals.is_empty()) {
			surface_arrays[ArrayMesh::ARRAY_NORMAL] = surface_normals;
		}

		if (!surface_uvs.is_empty()) {
			surface_arrays[ArrayMesh::ARRAY_TEX_UV] = surface_uvs;
		}

		surface_arrays[ArrayMesh::ARRAY_INDEX] = surface_indices;

		Ref<StandardMaterial3D> material = nullptr;
		if (materials.is_valid() && material_idx >= 0 && material_idx < material_paths.size()) {
			Ref<UsdPath> material_path = material_paths[material_idx];
			if (material_path.is_valid()) {
				material = materials->get_material_with_path(material_path);
			}
		}

		int surface_idx = mesh->get_surface_count();
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, surface_arrays);

		if (material.is_valid()) {
			mesh->surface_set_material(surface_idx, material);
			if (material_idx < surface_names.size()) {
				mesh->surface_set_name(surface_idx, surface_names[material_idx]);
			}
		}
	}

	return mesh;
}
