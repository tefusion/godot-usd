#include "usd_prim_types.h"
#include "core/usd_prim.h"
#include "core/usd_prim_type.h"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "prim-types.hh"
#include "type_utils.h"
#include "usdGeom.hh"

Ref<UsdPrimValue> UsdPrimValue::create(const UsdPrim *p_prim) {
	if (!p_prim) {
		return Ref<UsdPrimValue>();
	}

	UsdPrimType::Type type = p_prim->get_type();

	switch (type) {
		case UsdPrimType::USD_PRIM_TYPE_XFORM:
			return UsdPrimValueXform::create(p_prim);
		case UsdPrimType::USD_PRIM_TYPE_MESH:
			return UsdPrimValueGeomMesh::create(p_prim);
		default:
			Ref<UsdPrimValue> ref;
			ref.instantiate();
			ref->_prim = p_prim;
			return ref;
	}
}

UsdPrimType::Type UsdPrimValue::get_type() const {
	return UsdPrimType::USD_PRIM_TYPE_UNKNOWN;
}

void UsdPrimValue::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_type"), &UsdPrimValue::get_type);
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

Ref<UsdPrimValueXform> UsdPrimValueXform::create(const UsdPrim *p_prim) {
	Ref<UsdPrimValueXform> ref;
	ref.instantiate();
	ref->_prim = p_prim;
	return ref;
}

UsdPrimType::Type UsdPrimValueXform::get_type() const {
	return UsdPrimType::USD_PRIM_TYPE_XFORM;
}

Transform3D UsdPrimValueXform::get_transform() const {
	if (!_prim) {
		return Transform3D();
	}

	const tinyusdz::Prim *prim = _prim->internal_prim();
	if (!prim || !prim->is<tinyusdz::Xform>()) {
		return Transform3D();
	}

	const tinyusdz::Xform *xform = prim->as<tinyusdz::Xform>();
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
	if (!_prim) {
		return String();
	}

	const tinyusdz::Prim *prim = _prim->internal_prim();
	if (!prim || !prim->is<tinyusdz::Xform>()) {
		return String();
	}

	const tinyusdz::Xform *xform = prim->as<tinyusdz::Xform>();
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

Ref<UsdPrimValueGeomMesh> UsdPrimValueGeomMesh::create(const UsdPrim *p_prim) {
	Ref<UsdPrimValueGeomMesh> ref;
	ref.instantiate();
	ref->_prim = p_prim;
	return ref;
}

UsdPrimType::Type UsdPrimValueGeomMesh::get_type() const {
	return UsdPrimType::USD_PRIM_TYPE_MESH;
}

PackedVector3Array UsdPrimValueGeomMesh::get_points() const {
	PackedVector3Array godot_points;

	if (!_prim) {
		return godot_points;
	}

	const tinyusdz::Prim *prim = _prim->internal_prim();
	if (!prim || !prim->is<tinyusdz::GeomMesh>()) {
		return godot_points;
	}

	const tinyusdz::GeomMesh *mesh = prim->as<tinyusdz::GeomMesh>();
	auto points = mesh->get_points();
	godot_points.resize(points.size());
	for (size_t i = 0; i < points.size(); i++) {
		godot_points[i] = Vector3(points[i][0], points[i][1], points[i][2]);
	}

	return godot_points;
}

PackedVector3Array UsdPrimValueGeomMesh::get_normals() const {
	PackedVector3Array godot_normals;

	if (!_prim) {
		return godot_normals;
	}

	const tinyusdz::Prim *prim = _prim->internal_prim();
	if (!prim || !prim->is<tinyusdz::GeomMesh>()) {
		return godot_normals;
	}

	const tinyusdz::GeomMesh *mesh = prim->as<tinyusdz::GeomMesh>();
	auto normals = mesh->get_normals();
	godot_normals.resize(normals.size());
	for (size_t i = 0; i < normals.size(); i++) {
		godot_normals[i] = Vector3(normals[i][0], normals[i][1], normals[i][2]);
	}

	return godot_normals;
}

String UsdPrimValueGeomMesh::get_name() const {
	if (!_prim) {
		return String();
	}

	const tinyusdz::Prim *prim = _prim->internal_prim();
	if (!prim || !prim->is<tinyusdz::GeomMesh>()) {
		return String();
	}

	const tinyusdz::GeomMesh *mesh = prim->as<tinyusdz::GeomMesh>();

	return String(mesh->name.c_str());
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
	ClassDB::bind_method(D_METHOD("get_points"), &UsdPrimValueGeomMesh::get_points);
	ClassDB::bind_method(D_METHOD("get_normals"), &UsdPrimValueGeomMesh::get_normals);
	ClassDB::bind_method(D_METHOD("get_name"), &UsdPrimValueGeomMesh::get_name);

	ClassDB::bind_method(D_METHOD("_to_string"), &UsdPrimValueGeomMesh::_to_string);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "points"), "", "get_points");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "normals"), "", "get_normals");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "", "get_name");
}
