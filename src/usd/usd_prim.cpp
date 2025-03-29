#include "usd_prim.h"
#include "usd/usd_prim_type.h"
#include "value-types.hh"

UsdPrimType::Type UsdPrim::get_prim_type(const tinyusdz::Prim *prim) {
	switch (prim->type_id()) {
		case tinyusdz::value::TYPE_ID_GEOM_XFORM:
			return UsdPrimType::USD_PRIM_TYPE_XFORM;
		case tinyusdz::value::TYPE_ID_GEOM_MESH:
			return UsdPrimType::USD_PRIM_TYPE_MESH;
		case tinyusdz::value::TYPE_ID_GEOM_CAMERA:
			return UsdPrimType::USD_PRIM_TYPE_CAMERA;
		case tinyusdz::value::TYPE_ID_MATERIAL:
			return UsdPrimType::USD_PRIM_TYPE_MATERIAL;
		case tinyusdz::value::TYPE_ID_SHADER:
			return UsdPrimType::USD_PRIM_TYPE_SHADER;
		case tinyusdz::value::TYPE_ID_SCOPE:
			return UsdPrimType::USD_PRIM_TYPE_SCOPE;
		case tinyusdz::value::TYPE_ID_GEOM_GEOMSUBSET:
			return UsdPrimType::USD_PRIM_TYPE_GEOM_SUBSET;
		case tinyusdz::value::TYPE_ID_SKELETON:
			return UsdPrimType::USD_PRIM_TYPE_SKELETON;
		case tinyusdz::value::TYPE_ID_SKEL_ROOT:
			return UsdPrimType::USD_PRIM_TYPE_SKELETON_ROOT;
		default:
			return UsdPrimType::USD_PRIM_TYPE_UNKNOWN;
	}
}

Ref<UsdPrim> UsdPrim::create(std::shared_ptr<tinyusdz::Stage> stage, const tinyusdz::Path &path) {
	if (!stage) {
		return Ref<UsdPrim>();
	}

	Ref<UsdPrim> prim;
	prim.instantiate();
	prim->_stage = stage;
	prim->_path.instantiate();
	prim->_path->set_path(path);
	return prim;
}

void UsdPrim::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_type_name"), &UsdPrim::get_type_name);
	ClassDB::bind_method(D_METHOD("get_type"), &UsdPrim::get_type);
	ClassDB::bind_method(D_METHOD("is_valid"), &UsdPrim::is_valid);
	ClassDB::bind_method(D_METHOD("get_path"), &UsdPrim::get_path);
	ClassDB::bind_method(D_METHOD("set_path", "path"), &UsdPrim::set_path);
	ClassDB::bind_method(D_METHOD("get_children"), &UsdPrim::get_children);
	ClassDB::bind_method(D_METHOD("get_value"), &UsdPrim::get_value);
}

String UsdPrim::get_type_name() const {
	if (!is_valid())
		return String();

	const tinyusdz::Prim *prim = internal_prim();
	if (!prim)
		return String();

	return String(prim->type_name().c_str());
}

UsdPrimType::Type UsdPrim::get_type() const {
	const tinyusdz::Prim *prim = internal_prim();
	if (!prim)
		return UsdPrimType::USD_PRIM_TYPE_UNKNOWN;
	return UsdPrim::get_prim_type(prim);
}

void UsdPrim::set_path(Ref<UsdPath> path) {
	_path = path;
}

Ref<UsdPath> UsdPrim::get_path() const {
	return _path;
}

TypedArray<UsdPrim> UsdPrim::get_children() const {
	TypedArray<UsdPrim> children;
	if (!is_valid())
		return children;

	const std::vector<tinyusdz::Prim> &prims = internal_prim()->children();
	for (const auto &prim : prims) {
		children.push_back(create(_stage, prim.absolute_path()));
	}

	return children;
}

Ref<UsdPrimValue> UsdPrim::get_value() const {
	if (!is_valid())
		return Ref<UsdPrimValue>();

	return UsdPrimValue::create(this->internal_prim(), _stage);
}

bool UsdPrim::is_valid() const {
	return _stage != nullptr && _stage->GetPrimAtPath(_path->get_path());
}

const tinyusdz::Prim *UsdPrim::internal_prim() const {
	if (!is_valid())
		return nullptr;
	return _stage->GetPrimAtPath(_path->get_path()).value_or(nullptr);
}

UsdPrim::UsdPrim() :
		_stage(nullptr) {
}
