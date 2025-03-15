#include "usd_geom.h"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

#include "tydra/scene-access.hh"
#include "type_utils.h"
#include "usdGeom.hh"
#include "value-types.hh"

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

void apply_euler_rotation(Transform3D &result_transform, const tinyusdz::XformOp &transform, EulerOrder order) {
	Vector3 angles;
	if (xform_get_value(transform, angles)) {
		Vector3 euler_rad(Math::deg_to_rad(angles.x),
				Math::deg_to_rad(angles.y),
				Math::deg_to_rad(angles.z));
		result_transform = result_transform * Transform3D(Basis::from_euler(euler_rad, order));
	}
}

UsdPrimType::Type UsdPrimValueXform::get_type() const {
	return UsdPrimType::USD_PRIM_TYPE_XFORM;
}

Transform3D UsdPrimValueXform::get_transform() const {
	const tinyusdz::Xform *xform = get_typed_prim<tinyusdz::Xform>(_prim);
	if (!xform) {
		return Transform3D();
	}

	Transform3D result_transform;
	const std::vector<tinyusdz::XformOp> &transforms = xform->xformOps;

	for (const auto &transform : transforms) {
		switch (transform.op_type) {
			case tinyusdz::XformOp::OpType::Transform: {
				Transform3D transform_matrix;
				if (xform_get_value(transform, transform_matrix)) {
					result_transform = result_transform * transform_matrix;
				}
				break;
			}
			case tinyusdz::XformOp::OpType::Translate: {
				Vector3 translation;
				if (xform_get_value(transform, translation)) {
					result_transform = result_transform.translated(translation);
				}
				break;
			}
			case tinyusdz::XformOp::OpType::Scale: {
				Vector3 scale;
				if (xform_get_value(transform, scale)) {
					result_transform.scale(scale);
				}
				break;
			}
			case tinyusdz::XformOp::OpType::RotateX: {
				double angle;
				if (xform_get_value(transform, angle)) {
					result_transform.rotate(Vector3(1, 0, 0), Math::deg_to_rad(angle));
				}
				break;
			}
			case tinyusdz::XformOp::OpType::RotateY: {
				double angle;
				if (xform_get_value(transform, angle)) {
					result_transform.rotate(Vector3(0, 1, 0), Math::deg_to_rad(angle));
				}
				break;
			}
			case tinyusdz::XformOp::OpType::RotateZ: {
				double angle;
				if (xform_get_value(transform, angle)) {
					result_transform.rotate(Vector3(0, 0, 1), Math::deg_to_rad(angle));
				}
				break;
			}
			case tinyusdz::XformOp::OpType::RotateXYZ:
				apply_euler_rotation(result_transform, transform, EULER_ORDER_XYZ);
				break;
			case tinyusdz::XformOp::OpType::RotateXZY:
				apply_euler_rotation(result_transform, transform, EULER_ORDER_XZY);
				break;
			case tinyusdz::XformOp::OpType::RotateYXZ:
				apply_euler_rotation(result_transform, transform, EULER_ORDER_YXZ);
				break;
			case tinyusdz::XformOp::OpType::RotateYZX:
				apply_euler_rotation(result_transform, transform, EULER_ORDER_YZX);
				break;
			case tinyusdz::XformOp::OpType::RotateZXY:
				apply_euler_rotation(result_transform, transform, EULER_ORDER_ZXY);
				break;
			case tinyusdz::XformOp::OpType::RotateZYX:
				apply_euler_rotation(result_transform, transform, EULER_ORDER_ZYX);
				break;

			case tinyusdz::XformOp::OpType::Orient: {
				Quaternion quaternion;
				if (xform_get_value(transform, quaternion)) {
					result_transform.basis = Basis(quaternion);
				}
				break;
			}
			case tinyusdz::XformOp::OpType::ResetXformStack:
				result_transform = Transform3D();
				break;
		}
	}

	return result_transform;
}

String UsdPrimValueXform::get_name() const {
	const tinyusdz::Xform *xform = get_typed_prim<tinyusdz::Xform>(_prim);
	if (!xform) {
		return String();
	}

	return String(xform->name.c_str());
}

String UsdPrimValueXform::_to_string() const {
	String result = "UsdPrimValueXform(";

	String name = get_name();
	if (!name.is_empty()) {
		result += "name: \"" + name + "\", ";
	}

	Transform3D transform = get_transform();
	const Vector3 position = transform.get_origin();
	const Basis basis = transform.get_basis();
	const Vector3 scale = basis.get_scale();
	const Quaternion rotation = basis.get_rotation_quaternion();

	result += "position: " + UtilityFunctions::var_to_str(position);
	result += ", rotation: " + UtilityFunctions::var_to_str(rotation);
	result += ", scale: " + UtilityFunctions::var_to_str(scale);

	result += ")";
	return result;
}

void UsdPrimValueXform::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_transform"), &UsdPrimValueXform::get_transform);
	ClassDB::bind_method(D_METHOD("get_name"), &UsdPrimValueXform::get_name);
	ClassDB::bind_method(D_METHOD("_to_string"), &UsdPrimValueXform::_to_string);

	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM3D, "transform"), "", "get_transform");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "", "get_name");
}

//////////////////////////////////////////////////////////////////////////
// UsdPrimValueGeomMesh
//////////////////////////////////////////////////////////////////////////

UsdPrimType::Type UsdPrimValueGeomMesh::get_type() const {
	return UsdPrimType::USD_PRIM_TYPE_MESH;
}

PackedVector3Array UsdPrimValueGeomMesh::get_points() const {
	PackedVector3Array godot_points;

	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	if (!mesh) {
		return godot_points;
	}

	auto points = mesh->get_points();
	godot_points.resize(points.size());
	for (size_t i = 0; i < points.size(); i++) {
		godot_points[i] = Vector3(points[i][0], points[i][1], points[i][2]);
	}

	return godot_points;
}

PackedVector3Array UsdPrimValueGeomMesh::get_normals() const {
	PackedVector3Array godot_normals;

	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	if (!mesh) {
		return godot_normals;
	}

	auto normals = mesh->get_normals();
	godot_normals.resize(normals.size());
	for (size_t i = 0; i < normals.size(); i++) {
		godot_normals[i] = Vector3(normals[i][0], normals[i][1], normals[i][2]);
	}

	return godot_normals;
}

PackedVector2Array UsdPrimValueGeomMesh::get_uvs() const {
	PackedVector2Array godot_uvs;

	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	if (!mesh) {
		return godot_uvs;
	}

	tinyusdz::GeomPrimvar primvar;
	std::string err;
	bool success = tinyusdz::tydra::GetGeomPrimvar(*_stage, mesh, "st", &primvar, &err);
	ERR_FAIL_COND_V_MSG(!success, godot_uvs, String("Failed to get primvar: ") + err.c_str());
	//TODO: I wasn't able to just use Value type so just used texcoord2f. Need to check if this can also be a double array
	std::vector<tinyusdz::value::texcoord2f> value;
	auto variability = primvar.get_attribute().variability();
	success = primvar.get_value(&value, &err);
	ERR_FAIL_COND_V_MSG(!success, godot_uvs, String("Failed to get value: ") + err.c_str());
	return to_variant(value);
}

PackedStringArray UsdPrimValueGeomMesh::get_primvar_names() const {
	return PackedStringArray();
	// PackedStringArray godot_primvars;

	// const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	// if (!mesh) {
	// 	return godot_primvars;
	// }

	// std::vector<tinyusdz::GeomPrimvar> primvars = tinyusdz::tydra::GetGeomPrimvars(*_stage, *mesh);
	// for (const auto &primvar : primvars) {
	// 	godot_primvars.push_back(primvar.name().c_str());
	// }

	// return godot_primvars;
}

String UsdPrimValueGeomMesh::get_name() const {
	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	if (!mesh) {
		return String();
	}

	return String(mesh->name.c_str());
}

size_t UsdPrimValueGeomMesh::get_face_count() const {
	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	if (!mesh) {
		return 0;
	}

	return mesh->get_faceVertexCounts().size();
}

String UsdPrimValueGeomMesh::_to_string() const {
	String name = get_name();
	PackedVector3Array points = get_points();
	PackedVector3Array normals = get_normals();

	String result = "UsdPrimValueGeomMesh(";
	if (!name.is_empty()) {
		result += "name: \"" + name + "\", ";
	}
	result += "points: " + String::num_int64(points.size());
	result += ", normals: " + String::num_int64(normals.size());
	result += ")";
	return result;
}

void UsdPrimValueGeomMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_name"), &UsdPrimValueGeomMesh::get_name);
	ClassDB::bind_method(D_METHOD("get_points"), &UsdPrimValueGeomMesh::get_points);
	ClassDB::bind_method(D_METHOD("get_normals"), &UsdPrimValueGeomMesh::get_normals);
	ClassDB::bind_method(D_METHOD("get_uvs"), &UsdPrimValueGeomMesh::get_uvs);
	ClassDB::bind_method(D_METHOD("get_primvar_names"), &UsdPrimValueGeomMesh::get_primvar_names);

	ClassDB::bind_method(D_METHOD("_to_string"), &UsdPrimValueGeomMesh::_to_string);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "points"), "", "get_points");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "normals"), "", "get_normals");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "", "get_name");
}
