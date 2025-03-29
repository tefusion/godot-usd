#include "usd_skel.h"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "usdSkel.hh"
#include "utils/type_utils.h"

UsdPrimType::Type UsdPrimValueSkeleton::get_type() const {
	return UsdPrimType::USD_PRIM_TYPE_SKELETON;
}

TypedArray<Transform3D> UsdPrimValueSkeleton::get_bind_transforms() const {
	TypedArray<Transform3D> godot_transforms;

	const tinyusdz::Skeleton *skeleton = get_typed_prim<tinyusdz::Skeleton>(_prim);
	if (!skeleton) {
		return godot_transforms;
	}

	if (!skeleton->bindTransforms.has_value()) {
		return godot_transforms;
	}

	const auto &bind_transforms_attr = skeleton->bindTransforms;
	std::vector<tinyusdz::value::matrix4d> bind_transforms;
	bool success = bind_transforms_attr.get_value(&bind_transforms);
	ERR_FAIL_COND_V_MSG(!success, godot_transforms, "Failed to get bind transforms");

	for (const auto &matrix : bind_transforms) {
		Variant transform = to_variant(tinyusdz::value::Value(matrix));
		godot_transforms.push_back(transform);
	}

	return godot_transforms;
}

PackedStringArray UsdPrimValueSkeleton::get_joints() const {
	PackedStringArray godot_joints;

	const tinyusdz::Skeleton *skeleton = get_typed_prim<tinyusdz::Skeleton>(_prim);
	if (!skeleton) {
		return godot_joints;
	}

	const auto &joints_attr = skeleton->joints.has_value() ? skeleton->joints : skeleton->jointNames;
	if (!joints_attr.has_value()) {
		return godot_joints;
	}

	std::vector<tinyusdz::value::token> joints;
	bool success = joints_attr.get_value(&joints);
	ERR_FAIL_COND_V_MSG(!success, godot_joints, "Failed to get joints");

	godot_joints.resize(joints.size());
	for (size_t i = 0; i < joints.size(); i++) {
		godot_joints[i] = String(joints[i].str().c_str());
	}

	return godot_joints;
}

TypedArray<Transform3D> UsdPrimValueSkeleton::get_rest_transforms() const {
	TypedArray<Transform3D> godot_transforms;

	const tinyusdz::Skeleton *skeleton = get_typed_prim<tinyusdz::Skeleton>(_prim);
	if (!skeleton) {
		return godot_transforms;
	}

	if (!skeleton->restTransforms.has_value()) {
		return godot_transforms;
	}

	const auto &rest_transforms_attr = skeleton->restTransforms;
	std::vector<tinyusdz::value::matrix4d> rest_transforms;
	bool success = rest_transforms_attr.get_value(&rest_transforms);
	ERR_FAIL_COND_V_MSG(!success, godot_transforms, "Failed to get rest transforms");

	for (const auto &matrix : rest_transforms) {
		Variant transform = to_variant(tinyusdz::value::Value(matrix));
		godot_transforms.push_back(transform);
	}

	return godot_transforms;
}

Array UsdPrimValueSkeleton::get_bone_lengths() const {
	Array bone_lengths;

	TypedArray<Transform3D> rest_transforms = get_rest_transforms();
	if (rest_transforms.size() == 0) {
		return bone_lengths;
	}

	bone_lengths.resize(rest_transforms.size());
	for (int i = 0; i < rest_transforms.size(); i++) {
		Transform3D transform = rest_transforms[i];
		bone_lengths[i] = transform.origin.length();
	}

	return bone_lengths;
}

String UsdPrimValueSkeleton::_to_string() const {
	const tinyusdz::Skeleton *skeleton = get_typed_prim<tinyusdz::Skeleton>(_prim);
	if (!skeleton) {
		return "UsdPrimValueGeomSkeleton(invalid)";
	}

	String result = "UsdPrimValueGeomSkeleton(";
	result += "name: \"" + String(skeleton->name.c_str()) + "\", ";

	PackedStringArray joints = get_joints();
	result += "joints: " + String::num_int64(joints.size()) + ", ";

	TypedArray<Transform3D> bind_transforms = get_bind_transforms();
	result += "bind_transforms: " + String::num_int64(bind_transforms.size()) + ", ";

	TypedArray<Transform3D> rest_transforms = get_rest_transforms();
	result += "rest_transforms: " + String::num_int64(rest_transforms.size());

	result += ")";
	return result;
}

void UsdPrimValueSkeleton::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_bind_transforms"), &UsdPrimValueSkeleton::get_bind_transforms);
	ClassDB::bind_method(D_METHOD("get_joints"), &UsdPrimValueSkeleton::get_joints);
	ClassDB::bind_method(D_METHOD("get_bone_lengths"), &UsdPrimValueSkeleton::get_bone_lengths);
	ClassDB::bind_method(D_METHOD("get_rest_transforms"), &UsdPrimValueSkeleton::get_rest_transforms);
	ClassDB::bind_method(D_METHOD("_to_string"), &UsdPrimValueSkeleton::_to_string);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "bind_transforms", PROPERTY_HINT_ARRAY_TYPE, "Transform3D"), "", "get_bind_transforms");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "joints"), "", "get_joints");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "bone_lengths"), "", "get_bone_lengths");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "rest_transforms", PROPERTY_HINT_ARRAY_TYPE, "Transform3D"), "", "get_rest_transforms");
}

void UsdPrimValueSkeletonRoot::_bind_methods() {
}

UsdPrimType::Type UsdPrimValueSkeletonRoot::get_type() const {
	return UsdPrimType::USD_PRIM_TYPE_SKELETON_ROOT;
}
