#include "usd_geom.h"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "usd/usd_common.h"

#include "token-type.hh"
#include "tydra/scene-access.hh"
#include "tydra/shader-network.hh"
#include "usd/usd_prim_value.h"
#include "usdGeom.hh"
#include "utils/godot_utils.h"
#include "utils/type_utils.h"
#include "value-types.hh"

using namespace godot;

UsdGeomPrimvar::Interpolation UsdGeomPrimvar::interpolation_from_internal(tinyusdz::Interpolation interpolation) {
	switch (interpolation) {
		case tinyusdz::Interpolation::Constant:
			return CONSTANT;
		case tinyusdz::Interpolation::Uniform:
			return UNIFORM;
		case tinyusdz::Interpolation::Varying:
			return VARYING;
		case tinyusdz::Interpolation::Vertex:
			return VERTEX;
		case tinyusdz::Interpolation::FaceVarying:
			return FACEVARYING;
		default:
			return INVALID;
	}
}

tinyusdz::Interpolation UsdGeomPrimvar::interpolation_to_internal(UsdGeomPrimvar::Interpolation interpolation) {
	switch (interpolation) {
		case CONSTANT:
			return tinyusdz::Interpolation::Constant;
		case UNIFORM:
			return tinyusdz::Interpolation::Uniform;
		case VARYING:
			return tinyusdz::Interpolation::Varying;
		case VERTEX:
			return tinyusdz::Interpolation::Vertex;
		case FACEVARYING:
			return tinyusdz::Interpolation::FaceVarying;
		default:
			return tinyusdz::Interpolation::Invalid;
	}
}

String UsdGeomPrimvar::get_name() const {
	return _name;
}

void UsdGeomPrimvar::set_name(const String &name) {
	_name = name;
}

Array UsdGeomPrimvar::get_values() const {
	return _values;
}

void UsdGeomPrimvar::set_values(const Array &values) {
	_values = values;
}

UsdGeomPrimvar::Interpolation UsdGeomPrimvar::get_interpolation() const {
	return _interpolation;
}

void UsdGeomPrimvar::set_interpolation(UsdGeomPrimvar::Interpolation interpolation) {
	_interpolation = interpolation;
}

int UsdGeomPrimvar::get_element_size() const {
	return _element_size;
}

void UsdGeomPrimvar::set_element_size(int element_size) {
	_element_size = element_size;
}

PackedInt32Array UsdGeomPrimvar::get_indices() const {
	return _indices;
}

void UsdGeomPrimvar::set_indices(const PackedInt32Array &indices) {
	_indices = indices;
}

bool UsdGeomPrimvar::has_indices() const {
	return !_indices.is_empty();
}

String UsdGeomPrimvar::_to_string() const {
	String info = "UsdGeomPrimvar: ";
	info += "name: " + _name + ", ";
	info += "interpolation: " + String::num_int64(_interpolation) + ", ";
	info += "element_size: " + String::num_int64(_element_size) + ", ";
	info += "values: " + String::num_int64(_values.size()) + ", ";
	info += "indices: " + String::num_int64(_indices.size());
	return info;
}

void UsdGeomPrimvar::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_name"), &UsdGeomPrimvar::get_name);
	ClassDB::bind_method(D_METHOD("set_name", "name"), &UsdGeomPrimvar::set_name);
	ClassDB::bind_method(D_METHOD("get_values"), &UsdGeomPrimvar::get_values);
	ClassDB::bind_method(D_METHOD("set_values", "values"), &UsdGeomPrimvar::set_values);
	ClassDB::bind_method(D_METHOD("get_interpolation"), &UsdGeomPrimvar::get_interpolation);
	ClassDB::bind_method(D_METHOD("set_interpolation", "interpolation"), &UsdGeomPrimvar::set_interpolation);
	ClassDB::bind_method(D_METHOD("get_element_size"), &UsdGeomPrimvar::get_element_size);
	ClassDB::bind_method(D_METHOD("set_element_size", "element_size"), &UsdGeomPrimvar::set_element_size);
	ClassDB::bind_method(D_METHOD("get_indices"), &UsdGeomPrimvar::get_indices);
	ClassDB::bind_method(D_METHOD("set_indices", "indices"), &UsdGeomPrimvar::set_indices);
	ClassDB::bind_method(D_METHOD("has_indices"), &UsdGeomPrimvar::has_indices);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_name", "get_name");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "values"), "set_values", "get_values");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "interpolation"), "set_interpolation", "get_interpolation");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "element_size"), "set_element_size", "get_element_size");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "indices"), "set_indices", "get_indices");

	BIND_ENUM_CONSTANT(CONSTANT);
	BIND_ENUM_CONSTANT(UNIFORM);
	BIND_ENUM_CONSTANT(VARYING);
	BIND_ENUM_CONSTANT(VERTEX);
	BIND_ENUM_CONSTANT(FACEVARYING);
	BIND_ENUM_CONSTANT(INVALID);
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
// UsdPrimValueGeomMaterialSubset
//////////////////////////////////////////////////////////////////////////

UsdPrimValueGeomMaterialSubset::ElementType UsdPrimValueGeomMaterialSubset::element_type_from_internal(tinyusdz::GeomSubset::ElementType type) {
	switch (type) {
		case tinyusdz::GeomSubset::ElementType::Face:
			return FACE;
		default:
			return POINT;
	}
}

tinyusdz::GeomSubset::ElementType UsdPrimValueGeomMaterialSubset::element_type_to_internal(UsdPrimValueGeomMaterialSubset::ElementType type) {
	switch (type) {
		case FACE:
			return tinyusdz::GeomSubset::ElementType::Face;
		default:
			return tinyusdz::GeomSubset::ElementType::Point;
	}
}

String UsdPrimValueGeomMaterialSubset::get_name() const {
	return _name;
}

void UsdPrimValueGeomMaterialSubset::set_name(const String &name) {
	_name = name;
}

Ref<UsdPath> UsdPrimValueGeomMaterialSubset::get_bound_material() const {
	return _bound_material_path;
}

void UsdPrimValueGeomMaterialSubset::set_bound_material(Ref<UsdPath> path) {
	_bound_material_path = path;
}

UsdPrimValueGeomMaterialSubset::ElementType UsdPrimValueGeomMaterialSubset::get_element_type() const {
	return _element_type;
}

void UsdPrimValueGeomMaterialSubset::set_element_type(UsdPrimValueGeomMaterialSubset::ElementType type) {
	_element_type = type;
}

PackedInt32Array UsdPrimValueGeomMaterialSubset::get_indices() const {
	return _indices;
}

void UsdPrimValueGeomMaterialSubset::set_indices(const PackedInt32Array &indices) {
	_indices = indices;
}

String UsdPrimValueGeomMaterialSubset::_to_string() const {
	String result = "UsdPrimValueGeomMaterialSubset(";

	result += "name: \"" + _name + "\", ";
	result += "bound_material: " + _bound_material_path->_to_string() + ", ";
	result += "element_type: " + UtilityFunctions::var_to_str(_element_type) + ", ";
	result += "indices: " + UtilityFunctions::var_to_str(_indices);

	result += ")";
	return result;
}

void UsdPrimValueGeomMaterialSubset::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_name"), &UsdPrimValueGeomMaterialSubset::get_name);
	ClassDB::bind_method(D_METHOD("set_name", "name"), &UsdPrimValueGeomMaterialSubset::set_name);

	ClassDB::bind_method(D_METHOD("get_bound_material"), &UsdPrimValueGeomMaterialSubset::get_bound_material);
	ClassDB::bind_method(D_METHOD("set_bound_material", "path"), &UsdPrimValueGeomMaterialSubset::set_bound_material);

	ClassDB::bind_method(D_METHOD("get_element_type"), &UsdPrimValueGeomMaterialSubset::get_element_type);
	ClassDB::bind_method(D_METHOD("set_element_type", "type"), &UsdPrimValueGeomMaterialSubset::set_element_type);

	ClassDB::bind_method(D_METHOD("get_indices"), &UsdPrimValueGeomMaterialSubset::get_indices);
	ClassDB::bind_method(D_METHOD("set_indices", "indices"), &UsdPrimValueGeomMaterialSubset::set_indices);

	ClassDB::bind_method(D_METHOD("_to_string"), &UsdPrimValueGeomMaterialSubset::_to_string);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_name", "get_name");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "bound_material", PROPERTY_HINT_RESOURCE_TYPE, "UsdPath"), "set_bound_material", "get_bound_material");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "element_type"), "set_element_type", "get_element_type");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "indices"), "set_indices", "get_indices");

	BIND_ENUM_CONSTANT(FACE);
	BIND_ENUM_CONSTANT(POINT);
}

//////////////////////////////////////////////////////////////////////////
// UsdGeomMeshMaterialMap
//////////////////////////////////////////////////////////////////////////

PackedInt32Array UsdGeomMeshMaterialMap::get_face_material_indices() const {
	return _face_material_indices;
}

void UsdGeomMeshMaterialMap::set_face_material_indices(const PackedInt32Array &indices) {
	_face_material_indices = indices;
}

TypedArray<UsdPath> UsdGeomMeshMaterialMap::get_materials() const {
	return _materials;
}

PackedStringArray UsdGeomMeshMaterialMap::get_surface_names() const {
	return _surface_names;
}

void UsdGeomMeshMaterialMap::set_surface_names(const PackedStringArray &names) {
	_surface_names = names;
}

void UsdGeomMeshMaterialMap::set_materials(const TypedArray<UsdPath> &materials) {
	_materials = materials;
}

bool UsdGeomMeshMaterialMap::is_mapped() const {
	return !_face_material_indices.is_empty();
}

String UsdGeomMeshMaterialMap::_to_string() const {
	String result = "UsdGeomMeshMaterialMap(";

	result += "face_material_indices: " + UtilityFunctions::var_to_str(_face_material_indices);
	result += ", materials: [";

	for (int i = 0; i < _materials.size(); i++) {
		if (i > 0) {
			result += ", ";
		}
		Ref<UsdPath> path = _materials[i];
		result += path->_to_string();
	}

	result += "])";
	return result;
}

void UsdGeomMeshMaterialMap::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_face_material_indices"), &UsdGeomMeshMaterialMap::get_face_material_indices);
	ClassDB::bind_method(D_METHOD("set_face_material_indices", "indices"), &UsdGeomMeshMaterialMap::set_face_material_indices);

	ClassDB::bind_method(D_METHOD("get_materials"), &UsdGeomMeshMaterialMap::get_materials);
	ClassDB::bind_method(D_METHOD("set_materials", "materials"), &UsdGeomMeshMaterialMap::set_materials);

	ClassDB::bind_method(D_METHOD("get_surface_names"), &UsdGeomMeshMaterialMap::get_surface_names);
	ClassDB::bind_method(D_METHOD("set_surface_names", "names"), &UsdGeomMeshMaterialMap::set_surface_names);

	ClassDB::bind_method(D_METHOD("is_mapped"), &UsdGeomMeshMaterialMap::is_mapped);

	ClassDB::bind_method(D_METHOD("_to_string"), &UsdGeomMeshMaterialMap::_to_string);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "face_material_indices"),
			"set_face_material_indices", "get_face_material_indices");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "materials", PROPERTY_HINT_ARRAY_TYPE, "UsdPath"),
			"set_materials", "get_materials");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "surface_names"), "set_surface_names", "get_surface_names");
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

String UsdPrimValueGeomMesh::get_primvar_name(const UsdPrimValueGeomMesh::PrimVarType type) const {
	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	if (!mesh) {
		return "";
	}

	const PackedStringArray primvar_names = UsdPrimValueGeomMesh::primvar_type_to_string(type);
	String primvar_name = "";
	for (int i = 0; i < primvar_names.size(); i++) {
		std::string prop_name = "primvars:" + std::string(primvar_names[i].utf8().get_data());
		const auto it = mesh->props.find(prop_name);
		if (it != mesh->props.end()) {
			primvar_name = primvar_names[i];
			break;
		}
	}
	return primvar_name;
}

bool UsdPrimValueGeomMesh::has_primvar(const UsdPrimValueGeomMesh::PrimVarType type) const {
	return !get_primvar_name(type).is_empty();
}

Ref<UsdGeomPrimvar> UsdPrimValueGeomMesh::get_primvar(const UsdPrimValueGeomMesh::PrimVarType type) const {
	Ref<UsdGeomPrimvar> result;
	result.instantiate();

	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	if (!mesh) {
		return result;
	}

	String primvar_name = get_primvar_name(type);
	if (primvar_name.is_empty()) {
		return result;
	}

	tinyusdz::GeomPrimvar primvar;
	std::string err;

	bool success = tinyusdz::tydra::GetGeomPrimvar(*_stage, mesh, primvar_name.utf8().get_data(), &primvar, &err);
	ERR_FAIL_COND_V_MSG(!success, result, String("Failed to get primvar: ") + err.c_str());

	result->set_name(primvar_name);

	result->set_interpolation(UsdGeomPrimvar::interpolation_from_internal(primvar.get_interpolation()));

	if (primvar.has_elementSize()) {
		result->set_element_size(primvar.get_elementSize());
	}

	Array values;
	switch (type) {
		case PRIMVAR_TEX_UV:
		case PRIMVAR_TEX_UV2: {
			std::vector<tinyusdz::value::texcoord2f> value;
			success = primvar.get_value(&value, &err);
			ERR_FAIL_COND_V_MSG(!success, result, String("Failed to get UV value: ") + err.c_str());
			values = to_variant(value);
			break;
		}
		case PRIMVAR_COLOR: {
			std::vector<tinyusdz::value::color3f> value;
			success = primvar.get_value(&value, &err);
			ERR_FAIL_COND_V_MSG(!success, result, String("Failed to get color value: ") + err.c_str());
			values = to_variant(value);
			break;
		}
		case PRIMVAR_BONES: {
			std::vector<int32_t> value;
			success = primvar.get_value(&value, &err);
			ERR_FAIL_COND_V_MSG(!success, result, String("Failed to get bone indices: ") + err.c_str());
			values = to_variant(value);
			break;
		}
		case PRIMVAR_WEIGHTS: {
			std::vector<float> value;
			success = primvar.get_value(&value, &err);
			ERR_FAIL_COND_V_MSG(!success, result, String("Failed to get bone weights: ") + err.c_str());
			values = to_variant(value);
			break;
		}
		default:
			ERR_FAIL_V_MSG(result, "Unsupported primvar type");
			break;
	}
	result->set_values(values);

	if (primvar.has_indices()) {
		std::vector<int32_t> indices_vec = primvar.get_default_indices();
		result->set_indices(to_variant(indices_vec));
	}

	return result;
}

Array UsdPrimValueGeomMesh::get_primvars() const {
	Array result;

	result.resize(PRIMVAR_INVALID);
	for (int i = 0; i < PRIMVAR_INVALID; i++) {
		Ref<UsdGeomPrimvar> primvar = get_primvar(static_cast<PrimVarType>(i));
		if (primvar->get_name().is_empty()) {
			continue;
		}
		result.set(i, primvar);
	}

	return result;
}

bool UsdPrimValueGeomMesh::has_geom_bind_transform() const {
	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	return mesh->has_primvar("skel:geomBindTransform");
}

Transform3D UsdPrimValueGeomMesh::get_geom_bind_transform() const {
	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);

	//just copied over from tydra render-data
	if (has_geom_bind_transform()) {
		tinyusdz::GeomPrimvar bindTransformPvar;

		if (!tinyusdz::tydra::GetGeomPrimvar(*_stage, mesh, "skel:geomBindTransform", &bindTransformPvar)) {
			ERR_FAIL_V_MSG(Transform3D(), "Failed to get `skel:geomBindTransform` attribute. "
										  "Ensure `skel:geomBindTransform` is type `matrix4d`");
		}

		tinyusdz::value::matrix4d bindTransform;
		if (!bindTransformPvar.get_value(&bindTransform)) {
			ERR_FAIL_V_MSG(Transform3D(), "Failed to get `skel:geomBindTransform` attribute. "
										  "Ensure `skel:geomBindTransform` is type `matrix4d`");
		}
		return to_variant(bindTransform);
	}

	return Transform3D();
}

String UsdPrimValueGeomMesh::get_name() const {
	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	if (!mesh) {
		return String();
	}

	return String(mesh->name.c_str());
}

size_t UsdPrimValueGeomMesh::get_face_count() const {
	return get_face_vertex_counts().size();
}

PackedInt32Array UsdPrimValueGeomMesh::get_face_vertex_counts() const {
	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	if (!mesh) {
		return PackedInt32Array();
	}

	const std::vector<int32_t> &face_vertex_counts = mesh->get_faceVertexCounts();
	PackedInt32Array godot_face_vertex_counts;
	godot_face_vertex_counts.resize(face_vertex_counts.size());
	for (size_t i = 0; i < face_vertex_counts.size(); i++) {
		godot_face_vertex_counts.set(i, face_vertex_counts[i]);
	}

	return godot_face_vertex_counts;
}

PackedInt32Array UsdPrimValueGeomMesh::get_face_vertex_indices() const {
	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	if (!mesh) {
		return PackedInt32Array();
	}

	const std::vector<int32_t> &face_vertex_indices = mesh->get_faceVertexIndices();
	PackedInt32Array godot_face_vertex_indices;
	godot_face_vertex_indices.resize(face_vertex_indices.size());
	for (size_t i = 0; i < face_vertex_indices.size(); i++) {
		godot_face_vertex_indices.set(i, face_vertex_indices[i]);
	}

	return godot_face_vertex_indices;
}

Ref<UsdPath> UsdPrimValueGeomMesh::get_directly_bound_material() const {
	Ref<UsdPath> godot_material_path;
	godot_material_path.instantiate();
	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	if (!mesh) {
		return godot_material_path;
	}
	tinyusdz::Path material_path;
	const tinyusdz::Material *material;
	std::string err;
	const std::string purpose = ""; //->all purposes

	bool success = tinyusdz::tydra::GetBoundMaterial(*_stage, _prim->absolute_path(), purpose, &material_path, &material, &err);

	ERR_FAIL_COND_V_MSG(!success, godot_material_path, String("Failed to get bound material: ") + err.c_str());

	godot_material_path->set_path(material_path);
	return godot_material_path;
}

Vector<Ref<UsdPrimValueGeomMaterialSubset>> UsdPrimValueGeomMesh::get_subset_materials() const {
	Vector<Ref<UsdPrimValueGeomMaterialSubset>> godot_materials;

	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	if (!mesh) {
		return godot_materials;
	}

	const tinyusdz::Token material_bind_family = tinyusdz::Token("materialBind");
	const std::vector<const tinyusdz::GeomSubset *> geom_subsets = tinyusdz::tydra::GetGeomSubsets(*_stage, _prim->absolute_path(), material_bind_family);

	if (geom_subsets.empty()) {
		return godot_materials;
	}

	Ref<UsdPath> default_material = get_directly_bound_material();
	for (const auto &subset : geom_subsets) {
		Ref<UsdPrimValueGeomMaterialSubset> godot_subset;
		godot_subset.instantiate();
		godot_subset->set_name(subset->name.c_str());
		godot_subset->set_element_type(UsdPrimValueGeomMaterialSubset::element_type_from_internal(subset->elementType.get_value()));

		const auto &indices_opt = subset->indices.get_value();
		if (indices_opt.has_value()) {
			const auto &indices = indices_opt.value();
			ERR_CONTINUE_MSG(!indices.is_scalar(), "Animated primvars are not supported");
			std::vector<int32_t> indices_val;
			bool success = indices.get_scalar(&indices_val);
			ERR_CONTINUE_MSG(!success, "Failed to get indices");
			godot_subset->set_indices(to_variant(indices_val));
		}

		auto material_binding = subset->materialBinding;
		if (material_binding.has_value()) {
			godot_subset->set_bound_material(UsdPath::create(material_binding->targetPath));
		} else {
			godot_subset->set_bound_material(default_material);
		}

		godot_materials.push_back(godot_subset);
	}

	return godot_materials;
}

TypedArray<UsdPrimValueGeomMaterialSubset> UsdPrimValueGeomMesh::get_subset_materials_godot() const {
	return ref_vector_to_typed_array(get_subset_materials());
}

Ref<UsdGeomMeshMaterialMap> UsdPrimValueGeomMesh::get_material_map() const {
	Ref<UsdGeomMeshMaterialMap> godot_material_map;
	godot_material_map.instantiate();

	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	if (!mesh) {
		return godot_material_map;
	}

	Vector<Ref<UsdPrimValueGeomMaterialSubset>> subset_materials = get_subset_materials();
	if (subset_materials.is_empty()) {
		godot_material_map->set_face_material_indices(PackedInt32Array());

		TypedArray<UsdPath> materials;
		materials.push_back(get_directly_bound_material());
		godot_material_map->set_materials(materials);
	} else {
		PackedInt32Array face_material_indices;
		size_t total_face_count = get_face_count();
		face_material_indices.resize(total_face_count);
		face_material_indices.fill(-1);
		TypedArray<UsdPath> materials;
		PackedStringArray surface_names;

		for (int material_idx = 0; material_idx < subset_materials.size(); material_idx++) {
			const Ref<UsdPrimValueGeomMaterialSubset> &subset = subset_materials[material_idx];
			const Ref<UsdPath> &material = subset->get_bound_material();
			const PackedInt32Array &indices = subset->get_indices();
			materials.push_back(material);
			surface_names.push_back(subset->get_name());

			ERR_CONTINUE_MSG(subset->get_element_type() != UsdPrimValueGeomMaterialSubset::ElementType::FACE, "Material subset element type is not FACE");

			for (const int &index : indices) {
				ERR_CONTINUE_MSG(index < 0 || index >= total_face_count, "Material subset index out of bounds");
				ERR_CONTINUE_MSG(face_material_indices[index] != -1, "Material subset index already assigned");
				face_material_indices.set(index, material_idx);
			}
		}

		godot_material_map->set_face_material_indices(face_material_indices);
		godot_material_map->set_materials(materials);
		godot_material_map->set_surface_names(surface_names);
	}

	return godot_material_map;
}

bool UsdPrimValueGeomMesh::has_skeleton() const {
	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	return mesh->skeleton.has_value();
}

Ref<UsdPrimValueSkeleton> UsdPrimValueGeomMesh::get_skeleton() const {
	const tinyusdz::GeomMesh *mesh = get_typed_prim<tinyusdz::GeomMesh>(_prim);
	const tinyusdz::Path skel_path = mesh->skeleton.value().targetPath;
	const auto &skel_prim_req = _stage->GetPrimAtPath(skel_path);
	ERR_FAIL_COND_V_MSG(!skel_prim_req.has_value(), Ref<UsdPrimValueSkeleton>(), "Skeleton prim not found");
	const tinyusdz::Prim *skel_prim = skel_prim_req.value();
	Ref<UsdPrimValue> val = UsdPrimValue::create(skel_prim, _stage);
	ERR_FAIL_COND_V_MSG(!val.is_valid(), Ref<UsdPrimValueSkeleton>(), "Failed to create skeleton prim value");
	ERR_FAIL_COND_V_MSG(val->get_type() != UsdPrimType::USD_PRIM_TYPE_SKELETON, Ref<UsdPrimValueSkeleton>(), "Skeleton prim value is not of type skeleton");
	return val;
}

UsdPrimValueGeomMesh::PrimVarType UsdPrimValueGeomMesh::primvar_type_from_string(const String &name) {
	if (name == "st" || name == "UVMap") {
		return PRIMVAR_TEX_UV;
	} else if (name == "st2") {
		return PRIMVAR_TEX_UV2;
	} else if (name == "skel:jointIndices") {
		return PRIMVAR_BONES;
	} else if (name == "skel:jointWeights") {
		return PRIMVAR_WEIGHTS;
	}
	//TODO: color array, didn't find any sample

	return PRIMVAR_INVALID;
}

PackedStringArray UsdPrimValueGeomMesh::primvar_type_to_string(const UsdPrimValueGeomMesh::PrimVarType type) {
	PackedStringArray result;
	switch (type) {
		case PRIMVAR_TEX_UV:
			result.push_back("st");
			result.push_back("UVMap");
			break;
		case PRIMVAR_TEX_UV2:
			result.push_back("st2");
			break;
		case PRIMVAR_BONES:
			result.push_back("skel:jointIndices");
			break;
		case PRIMVAR_WEIGHTS:
			result.push_back("skel:jointWeights");
			break;
		default:
			break;
	}
	return result;
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
	ClassDB::bind_method(D_METHOD("get_face_count"), &UsdPrimValueGeomMesh::get_face_count);
	ClassDB::bind_method(D_METHOD("get_face_vertex_counts"), &UsdPrimValueGeomMesh::get_face_vertex_counts);
	ClassDB::bind_method(D_METHOD("get_face_vertex_indices"), &UsdPrimValueGeomMesh::get_face_vertex_indices);
	ClassDB::bind_method(D_METHOD("get_directly_bound_material"), &UsdPrimValueGeomMesh::get_directly_bound_material);
	ClassDB::bind_method(D_METHOD("get_subset_materials"), &UsdPrimValueGeomMesh::get_subset_materials_godot);
	ClassDB::bind_method(D_METHOD("get_material_map"), &UsdPrimValueGeomMesh::get_material_map);
	ClassDB::bind_method(D_METHOD("get_primvar", "type"), &UsdPrimValueGeomMesh::get_primvar);
	ClassDB::bind_method(D_METHOD("get_primvar_name", "type"), &UsdPrimValueGeomMesh::get_primvar_name);
	ClassDB::bind_method(D_METHOD("has_primvar", "type"), &UsdPrimValueGeomMesh::has_primvar);
	ClassDB::bind_method(D_METHOD("get_primvars"), &UsdPrimValueGeomMesh::get_primvars);
	ClassDB::bind_method(D_METHOD("has_skeleton"), &UsdPrimValueGeomMesh::has_skeleton);
	ClassDB::bind_method(D_METHOD("get_skeleton"), &UsdPrimValueGeomMesh::get_skeleton);
	ClassDB::bind_method(D_METHOD("has_geom_bind_transform"), &UsdPrimValueGeomMesh::has_geom_bind_transform);
	ClassDB::bind_method(D_METHOD("get_geom_bind_transform"), &UsdPrimValueGeomMesh::get_geom_bind_transform);

	ClassDB::bind_static_method("UsdPrimValueGeomMesh", D_METHOD("primvar_type_from_string", "type"), &UsdPrimValueGeomMesh::primvar_type_from_string);
	ClassDB::bind_static_method("UsdPrimValueGeomMesh", D_METHOD("primvar_type_to_string", "type"), &UsdPrimValueGeomMesh::primvar_type_to_string);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "points"), "", "get_points");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "normals"), "", "get_normals");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "", "get_name");

	BIND_ENUM_CONSTANT(PRIMVAR_TEX_UV);
	BIND_ENUM_CONSTANT(PRIMVAR_TEX_UV2);
	BIND_ENUM_CONSTANT(PRIMVAR_COLOR);
	BIND_ENUM_CONSTANT(PRIMVAR_BONES);
	BIND_ENUM_CONSTANT(PRIMVAR_WEIGHTS);
}
