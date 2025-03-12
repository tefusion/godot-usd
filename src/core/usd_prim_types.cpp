#include "usd_prim_types.h"
#include "core/usd_prim_type.h"
#include "godot_cpp/variant/utility_functions.hpp"
#include "prim-types.hh"
#include "type_utils.h"
#include "usdGeom.hh"

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

Ref<UsdPrimValueXform> UsdPrimValueXform::create(const tinyusdz::Xform *xform) {
	Ref<UsdPrimValueXform> ref;
	ref.instantiate();

	// Process transformation
	Transform3D result_transform;

	if (xform) {
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
					result_transform = Transform3D(); // Reset to identity
					break;
			}
		}

		ref->_transform = result_transform;
		ref->_name = String(xform->name.c_str());
	}

	return ref;
}

UsdPrimType::Type UsdPrimValueXform::get_type() const {
	return UsdPrimType::USD_PRIM_TYPE_XFORM;
}

void UsdPrimValueXform::set_transform(const Transform3D &p_transform) {
	_transform = p_transform;
}

Transform3D UsdPrimValueXform::get_transform() const {
	return _transform;
}

void UsdPrimValueXform::set_name(const String &p_name) {
	_name = p_name;
}

String UsdPrimValueXform::get_name() const {
	return _name;
}

String UsdPrimValueXform::_to_string() const {
	String result = "UsdPrimValueXform(";

	if (!_name.is_empty()) {
		result += "name: \"" + _name + "\", ";
	}

	const Vector3 position = _transform.get_origin();
	const Basis basis = _transform.get_basis();
	const Vector3 scale = basis.get_scale();
	const Quaternion rotation = basis.get_rotation_quaternion();

	result += "position: " + UtilityFunctions::var_to_str(position);
	result += ", rotation: " + UtilityFunctions::var_to_str(rotation);
	result += ", scale: " + UtilityFunctions::var_to_str(scale);

	result += ")";
	return result;
}

void UsdPrimValueXform::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_transform", "transform"), &UsdPrimValueXform::set_transform);
	ClassDB::bind_method(D_METHOD("get_transform"), &UsdPrimValueXform::get_transform);
	ClassDB::bind_method(D_METHOD("set_name", "name"), &UsdPrimValueXform::set_name);
	ClassDB::bind_method(D_METHOD("get_name"), &UsdPrimValueXform::get_name);
	ClassDB::bind_method(D_METHOD("_to_string"), &UsdPrimValueXform::_to_string);

	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM3D, "transform"), "set_transform", "get_transform");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_name", "get_name");
}
