#include "godot_scene.h"

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/importer_mesh.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/skeleton3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "usd/usd_geom.h"
#include "usd/usd_prim.h"
#include "usd/usd_prim_type.h"
#include "usd/usd_stage.h"
#include "utils/geom_utils.h"
#include "utils/godot_utils.h"

using namespace godot;

Vector3::Axis DEFAULT_UP_AXIS = Vector3::Axis::AXIS_Y;

Node *get_owner(Node *node) {
	Node *owner = node->get_owner();
	if (owner == nullptr) {
		owner = node;
	}
	return owner;
}

bool is_mesh_instance(const Ref<UsdPrim> &prim) {
	//check if children are meshes
	const TypedArray<UsdPrim> &children = prim->get_children();
	if (children.size() != 1) {
		return false;
	}
	const Ref<UsdPrim> &child = children[0];
	return child->get_type() == UsdPrimType::USD_PRIM_TYPE_MESH;
}

Ref<ImporterMesh> UsdGodotSceneConverter::convert_mesh(const Ref<UsdPrimValueGeomMesh> &geom_mesh, const Vector3::Axis up_axis) {
	ERR_FAIL_COND_V_MSG(geom_mesh.is_null(), nullptr, "GeomMesh is null");
	ERR_FAIL_COND_V_MSG(_materials.is_null(), nullptr, "Materials is null");

	Ref<ImporterMesh> mesh;
	mesh.instantiate();
	mesh->set_name(geom_mesh->get_name());

	PackedVector3Array points = apply_up_axis(geom_mesh->get_points(), up_axis);
	PackedVector3Array normals = apply_up_axis(geom_mesh->get_normals(), up_axis);
	PackedInt32Array face_vertex_counts = geom_mesh->get_face_vertex_counts();
	PackedInt32Array face_vertex_indices = geom_mesh->get_face_vertex_indices();

	Ref<UsdGeomMeshMaterialMap> material_map = geom_mesh->get_material_map();

	Ref<UsdGeomPrimvar> uv_primvar;
	bool has_uvs = geom_mesh->has_primvar(UsdPrimValueGeomMesh::PRIMVAR_TEX_UV);
	if (has_uvs) {
		uv_primvar = geom_mesh->get_primvar(UsdPrimValueGeomMesh::PRIMVAR_TEX_UV);
	}

	Ref<UsdGeomPrimvar> bone_primvar;
	bool has_bones = geom_mesh->has_primvar(UsdPrimValueGeomMesh::PRIMVAR_BONES);
	if (has_bones) {
		bone_primvar = geom_mesh->get_primvar(UsdPrimValueGeomMesh::PRIMVAR_BONES);
	}

	Ref<UsdGeomPrimvar> weight_primvar;
	bool has_weights = geom_mesh->has_primvar(UsdPrimValueGeomMesh::PRIMVAR_WEIGHTS);
	if (has_weights) {
		weight_primvar = geom_mesh->get_primvar(UsdPrimValueGeomMesh::PRIMVAR_WEIGHTS);
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

	for (int material_idx = 0; material_idx < surface_indices.size(); material_idx++) {
		const Vector<int> &triangle_indices = surface_indices[material_idx];

		Array surface_arrays;
		surface_arrays.resize(Mesh::ARRAY_MAX);

		PackedVector3Array surface_vertices;
		PackedVector3Array surface_normals;
		PackedVector2Array surface_uvs;
		PackedInt32Array surface_indices;

		PackedFloat32Array surface_weights;
		PackedInt32Array surface_bones;
		bool has_skin = has_bones && has_weights;

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

		if (has_skin) {
			Array bone_values = bone_primvar->get_values();
			Array weight_values = weight_primvar->get_values();
			UsdGeomPrimvar::Interpolation bone_interp = bone_primvar->get_interpolation();
			UsdGeomPrimvar::Interpolation weight_interp = weight_primvar->get_interpolation();

			ERR_CONTINUE_MSG(bone_interp != UsdGeomPrimvar::VERTEX, "Bone primvar interpolation must be VERTEX");
			ERR_CONTINUE_MSG(weight_interp != UsdGeomPrimvar::VERTEX, "Weight primvar interpolation must be VERTEX");

			int skin_element_size = bone_primvar->get_element_size();
			int godot_skin_element_size = skin_element_size >= 8 ? 8 : 4;

			surface_bones.resize(surface_vertices.size() * godot_skin_element_size);
			surface_weights.resize(surface_vertices.size() * godot_skin_element_size);

			for (int i = 0; i < triangle_indices.size(); i++) {
				int tri_idx = triangle_indices[i];
				int base_idx = tri_idx * 3;

				for (int j = 0; j < 3; j++) {
					int vertex_index = triangulated_face_vertex_indices[base_idx + j];
					int surface_idx = i * 3 + j;

					if (vertex_index < bone_values.size()) {
						Array bone_indices = bone_values[vertex_index];
						Array bone_weights = weight_values[vertex_index];

						for (int k = 0; k < godot_skin_element_size; k++) {
							int bone_idx = -1;
							real_t bone_weight = 0.0;

							if (k < skin_element_size && k < bone_indices.size()) {
								bone_idx = bone_indices[k];
								bone_weight = bone_weights[k];
							}

							surface_bones[surface_idx * godot_skin_element_size + k] = bone_idx;
							surface_weights[surface_idx * godot_skin_element_size + k] = bone_weight;
						}
					}
				}
			}
		}

		int64_t surface_flags = Mesh::ARRAY_FORMAT_VERTEX | Mesh::ARRAY_FORMAT_INDEX;
		surface_arrays[Mesh::ARRAY_VERTEX] = surface_vertices;
		surface_arrays[Mesh::ARRAY_INDEX] = surface_indices;

		if (!surface_normals.is_empty()) {
			surface_flags |= Mesh::ARRAY_FORMAT_NORMAL;
			surface_arrays[Mesh::ARRAY_NORMAL] = surface_normals;
		}

		if (!surface_uvs.is_empty()) {
			surface_flags |= Mesh::ARRAY_FORMAT_TEX_UV;
			surface_arrays[Mesh::ARRAY_TEX_UV] = surface_uvs;
		}

		if (has_skin) {
			surface_arrays[Mesh::ARRAY_BONES] = surface_bones;
			surface_arrays[Mesh::ARRAY_WEIGHTS] = surface_weights;

			surface_flags |= Mesh::ARRAY_FORMAT_WEIGHTS;
			surface_flags |= Mesh::ARRAY_FORMAT_BONES;
			if (weight_primvar->get_element_size() > 4) {
				surface_flags |= Mesh::ARRAY_FLAG_USE_8_BONE_WEIGHTS;
			}
		}

		Ref<StandardMaterial3D> material = nullptr;
		if (_materials.is_valid() && material_idx >= 0 && material_idx < material_paths.size()) {
			Ref<UsdPath> material_path = material_paths[material_idx];
			if (material_path.is_valid()) {
				material = _materials->get_material_with_path(material_path);
			}
		}

		int surface_idx = mesh->get_surface_count();
		String surface_name = surface_names.size() > material_idx ? surface_names[material_idx] : String();
		Ref<StandardMaterial3D> godot_material = material.is_valid() ? material : nullptr;
		mesh->add_surface(Mesh::PRIMITIVE_TRIANGLES, surface_arrays, {}, {}, godot_material, surface_name, surface_flags);
	}

	return mesh;
}

Skeleton3D *UsdGodotSceneConverter::convert_skeleton(const Ref<UsdPrimValueSkeleton> &skeleton, const Vector3::Axis up_axis) {
	ERR_FAIL_COND_V_MSG(skeleton.is_null(), nullptr, "Skeleton is null");

	Skeleton3D *godot_skeleton = memnew(Skeleton3D);

	// Can infer hierarchy from naming e.g. "Bone/Bone1" means Bone1 is child of Bone
	PackedStringArray joints = skeleton->get_joints();
	Vector<Transform3D> rest_transforms = skeleton->get_rest_transforms();

	HashMap<String, int> bone_name_to_idx;

	for (int bone_idx = 0; bone_idx < joints.size(); bone_idx++) {
		String joint_name = joints[bone_idx];
		Transform3D rest_transform = rest_transforms[bone_idx];
		rest_transform.basis = apply_up_axis(rest_transform.basis, up_axis);

		PackedStringArray path = joint_name.split("/");
		String bone_name = path[path.size() - 1];

		int parent_idx = -1;
		if (path.size() > 1) {
			String parent_path = String();
			for (int j = 0; j < path.size() - 1; j++) {
				if (j > 0) {
					parent_path += "/";
				}
				parent_path += path[j];
			}
			parent_idx = bone_name_to_idx.has(parent_path) ? bone_name_to_idx[parent_path] : -1;
		}

		godot_skeleton->add_bone(bone_name);
		godot_skeleton->set_bone_parent(bone_idx, parent_idx);
		godot_skeleton->set_bone_rest(bone_idx, rest_transform);
		godot_skeleton->set_bone_pose(bone_idx, rest_transform);

		bone_name_to_idx[joint_name] = bone_idx;
	}

	return godot_skeleton;
}

Node3D *UsdGodotSceneConverter::convert_xform(const Ref<UsdPrim> &xform_prim, Node3D *parent, const Vector3::Axis up_axis) {
	ERR_FAIL_COND_V(xform_prim.is_null(), nullptr);
	Ref<UsdPrimValueXform> xform = xform_prim->get_value();
	ERR_FAIL_COND_V(xform.is_null(), nullptr);

	if (is_mesh_instance(xform_prim)) {
		return convert_mesh_instance(xform_prim, parent, up_axis);
	}

	Node3D *node = memnew(Node3D);

	node->set_name(xform->get_name());
	node->set_transform(apply_up_axis(xform->get_transform(), up_axis));

	if (parent) {
		parent->add_child(node);
		node->set_owner(get_owner(parent));
	}

	convert_prim_children(xform_prim, node, up_axis);

	return node;
}

Skeleton3D *UsdGodotSceneConverter::convert_skeleton_root(const Ref<UsdPrim> &skeleton_root_prim, Node3D *parent, const Vector3::Axis up_axis) {
	ERR_FAIL_COND_V(skeleton_root_prim.is_null(), nullptr);
	const Ref<UsdPrimValueSkeletonRoot> skeleton_root = skeleton_root_prim->get_value();
	ERR_FAIL_COND_V(skeleton_root.is_null(), nullptr);

	Vector<Ref<UsdPrim>> children = typed_array_to_ref_vector(skeleton_root_prim->get_children());

	//Skeleton is probably always at index 0, but didn't find that anywhere so just checking all children
	Skeleton3D *skeleton = nullptr;
	for (int i = 0; i < children.size(); i++) {
		if (children[i]->get_type() == UsdPrimType::USD_PRIM_TYPE_SKELETON) {
			skeleton = convert_skeleton(children[i]->get_value(), up_axis);
			break;
		}
	}

	ERR_FAIL_COND_V_MSG(!skeleton, skeleton, "Skeleton not found in SkeletonRoot");

	if (parent) {
		parent->add_child(skeleton);
		skeleton->set_owner(parent);
	}

	for (int i = 0; i < children.size(); i++) {
		if (children[i]->get_type() != UsdPrimType::USD_PRIM_TYPE_SKELETON) {
			convert_prim(children[i], skeleton, up_axis);
		}
	}

	return skeleton;
}

ImporterMeshInstance3D *UsdGodotSceneConverter::convert_mesh_instance(const Ref<UsdPrim> &mesh_instance_prim, Node3D *parent, const Vector3::Axis up_axis) {
	ERR_FAIL_COND_V(mesh_instance_prim.is_null(), nullptr);
	ImporterMeshInstance3D *mesh_instance = memnew(ImporterMeshInstance3D);

	Ref<UsdPrimValueGeomMesh> geom_mesh;
	if (mesh_instance_prim->get_type() == UsdPrimType::USD_PRIM_TYPE_XFORM) {
		Ref<UsdPrimValueXform> xform = mesh_instance_prim->get_value();
		mesh_instance->set_transform(apply_up_axis(xform->get_transform(), up_axis));

		const TypedArray<UsdPrim> &children = mesh_instance_prim->get_children();
		ERR_FAIL_COND_V_MSG(children.size() != 1, nullptr, "Expected one child for mesh instance");
		Ref<UsdPrim> child = children[0];
		geom_mesh = child->get_value();
	} else {
		ERR_FAIL_COND_V_MSG(mesh_instance_prim->get_type() != UsdPrimType::USD_PRIM_TYPE_MESH, nullptr, "Expected mesh instance prim to be of type mesh");
		geom_mesh = mesh_instance_prim->get_value();
	}

	ERR_FAIL_COND_V(geom_mesh.is_null(), nullptr);

	if (parent) {
		parent->add_child(mesh_instance);
		mesh_instance->set_owner(get_owner(parent));

		Skeleton3D *skeleton = Object::cast_to<Skeleton3D>(parent);
		if (skeleton) {
			mesh_instance->set_skeleton_path(mesh_instance->get_path_to(parent));
		}
	}

	mesh_instance->set_name(geom_mesh->get_name());
	mesh_instance->set_mesh(convert_mesh(geom_mesh, up_axis));

	return mesh_instance;
}

void UsdGodotSceneConverter::convert_prim_children(const Ref<UsdPrim> &prim, Node3D *parent, const Vector3::Axis up_axis) {
	ERR_FAIL_COND(prim.is_null());

	TypedArray<UsdPrim> children = prim->get_children();
	for (int i = 0; i < children.size(); i++) {
		convert_prim(children[i], parent, up_axis);
	}
}

Node *UsdGodotSceneConverter::convert_prim(const Ref<UsdPrim> &prim, Node3D *parent, const Vector3::Axis up_axis) {
	ERR_FAIL_COND_V(prim.is_null(), nullptr);

	switch (prim->get_type()) {
		case UsdPrimType::USD_PRIM_TYPE_XFORM:
			return convert_xform(prim, parent, up_axis);
		case UsdPrimType::USD_PRIM_TYPE_SKELETON_ROOT:
			return convert_skeleton_root(prim, parent, up_axis);
		case UsdPrimType::USD_PRIM_TYPE_MESH:
			return convert_mesh_instance(prim->get_value(), parent, up_axis);

		default:
			ERR_FAIL_V_MSG(nullptr, "Failed to convert prim of type: " + prim->get_type_name());
	}
}

bool UsdGodotSceneConverter::load(const Ref<UsdStage> &stage) {
	ERR_FAIL_COND_V(stage.is_null(), false);
	_stage = stage;
	_materials = _stage->extract_materials();
	return true;
}

UsdGodotSceneConverter::UsdGodotSceneConverter() {
}

UsdGodotSceneConverter::~UsdGodotSceneConverter() {
}

void UsdGodotSceneConverter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load", "stage"), &UsdGodotSceneConverter::load);

	ClassDB::bind_method(D_METHOD("convert_mesh", "geom_mesh", "up_axis"), &UsdGodotSceneConverter::convert_mesh, DEFVAL(DEFAULT_UP_AXIS));
	ClassDB::bind_method(D_METHOD("convert_skeleton", "skeleton", "up_axis"), &UsdGodotSceneConverter::convert_skeleton, DEFVAL(DEFAULT_UP_AXIS));
	ClassDB::bind_method(D_METHOD("convert_xform", "xform", "parent", "up_axis"), &UsdGodotSceneConverter::convert_xform, DEFVAL(nullptr), DEFVAL(DEFAULT_UP_AXIS));
	ClassDB::bind_method(D_METHOD("convert_skeleton_root", "skeleton_root_prim", "parent", "up_axis"), &UsdGodotSceneConverter::convert_skeleton_root, DEFVAL(nullptr), DEFVAL(DEFAULT_UP_AXIS));
	ClassDB::bind_method(D_METHOD("convert_mesh_instance", "geom_mesh", "parent", "up_axis"), &UsdGodotSceneConverter::convert_mesh_instance, DEFVAL(nullptr), DEFVAL(DEFAULT_UP_AXIS));
	ClassDB::bind_method(D_METHOD("convert_prim_children", "prim", "parent", "up_axis"), &UsdGodotSceneConverter::convert_prim_children, DEFVAL(nullptr), DEFVAL(DEFAULT_UP_AXIS));
	ClassDB::bind_method(D_METHOD("convert_prim", "prim", "parent", "up_axis"), &UsdGodotSceneConverter::convert_prim, DEFVAL(nullptr), DEFVAL(DEFAULT_UP_AXIS));
}
