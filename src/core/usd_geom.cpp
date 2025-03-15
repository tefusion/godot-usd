#include "usd_geom.h"
#include "core/usd_common.h"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

#include "token-type.hh"
#include "tydra/scene-access.hh"
#include "tydra/shader-network.hh"
#include "usdGeom.hh"
#include "utils/type_utils.h"
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

// Add this to bind the methods of UsdPrimvar
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
		case tinyusdz::GeomSubset::ElementType::Point:
			return POINT;
		default:
			return FACE; // Default to FACE if unknown
	}
}

tinyusdz::GeomSubset::ElementType UsdPrimValueGeomMaterialSubset::element_type_to_internal(UsdPrimValueGeomMaterialSubset::ElementType type) {
	switch (type) {
		case FACE:
			return tinyusdz::GeomSubset::ElementType::Face;
		case POINT:
			return tinyusdz::GeomSubset::ElementType::Point;
		default:
			return tinyusdz::GeomSubset::ElementType::Face; // Default to Face if unknown
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

TypedArray<UsdPrimValueGeomMaterialSubset> UsdPrimValueGeomMesh::get_materials() const {
	TypedArray<UsdPrimValueGeomMaterialSubset> godot_materials;

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

UsdPrimValueGeomMesh::PrimVarType UsdPrimValueGeomMesh::primvar_type_from_string(const String &name) {
	if (name == "st") {
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

String UsdPrimValueGeomMesh::primvar_type_to_string(UsdPrimValueGeomMesh::PrimVarType type) {
	switch (type) {
		case PRIMVAR_TEX_UV:
			return "st";
		case PRIMVAR_TEX_UV2:
			return "st2";
		case PRIMVAR_BONES:
			return "skel:jointIndices";
		case PRIMVAR_WEIGHTS:
			return "skel:jointWeights";
		default:
			return String();
	}
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
	ClassDB::bind_method(D_METHOD("get_face_count"), &UsdPrimValueGeomMesh::get_face_count);
	ClassDB::bind_method(D_METHOD("get_face_vertex_counts"), &UsdPrimValueGeomMesh::get_face_vertex_counts);
	ClassDB::bind_method(D_METHOD("get_face_vertex_indices"), &UsdPrimValueGeomMesh::get_face_vertex_indices);
	ClassDB::bind_method(D_METHOD("get_directly_bound_material"), &UsdPrimValueGeomMesh::get_directly_bound_material);
	ClassDB::bind_method(D_METHOD("get_materials"), &UsdPrimValueGeomMesh::get_materials);

	ClassDB::bind_static_method("UsdPrimValueGeomMesh", D_METHOD("primvar_type_from_string", "type"), &UsdPrimValueGeomMesh::primvar_type_from_string);
	ClassDB::bind_static_method("UsdPrimValueGeomMesh", D_METHOD("primvar_type_to_string", "type"), &UsdPrimValueGeomMesh::primvar_type_to_string);

	ClassDB::bind_method(D_METHOD("_to_string"), &UsdPrimValueGeomMesh::_to_string);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "points"), "", "get_points");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "normals"), "", "get_normals");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "", "get_name");

	BIND_ENUM_CONSTANT(PRIMVAR_TEX_UV);
	BIND_ENUM_CONSTANT(PRIMVAR_TEX_UV2);
	BIND_ENUM_CONSTANT(PRIMVAR_COLOR);
	BIND_ENUM_CONSTANT(PRIMVAR_BONES);
	BIND_ENUM_CONSTANT(PRIMVAR_WEIGHTS);
}
