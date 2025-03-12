#include "usd_prim.h"
#include "value-types.hh"

namespace {
UsdPrim::UsdPrimType get_prim_type(const tinyusdz::Prim &prim) {
	switch (prim.type_id()) {
		case tinyusdz::value::TYPE_ID_GEOM_XFORM:
			return UsdPrim::USD_PRIM_TYPE_XFORM;
		case tinyusdz::value::TYPE_ID_GEOM_MESH:
			return UsdPrim::USD_PRIM_TYPE_MESH;
		case tinyusdz::value::TYPE_ID_GEOM_CAMERA:
			return UsdPrim::USD_PRIM_TYPE_CAMERA;
		case tinyusdz::value::TYPE_ID_MATERIAL:
			return UsdPrim::USD_PRIM_TYPE_MATERIAL;
		case tinyusdz::value::TYPE_ID_SHADER:
			return UsdPrim::USD_PRIM_TYPE_SHADER;
		case tinyusdz::value::TYPE_ID_SCOPE:
			return UsdPrim::USD_PRIM_TYPE_SCOPE;
		case tinyusdz::value::TYPE_ID_GEOM_GEOMSUBSET:
			return UsdPrim::USD_PRIM_TYPE_GEOM_SUBSET;
		//case tinyusdz::value::TYPE_ID_LUX_DOME:
		//TODO: check what lights we can support
		default:
			return UsdPrim::USD_PRIM_TYPE_UNKNOWN;
	}
}
} //namespace

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

	BIND_ENUM_CONSTANT(USD_PRIM_TYPE_XFORM);
	BIND_ENUM_CONSTANT(USD_PRIM_TYPE_MESH);
	BIND_ENUM_CONSTANT(USD_PRIM_TYPE_CAMERA);
	BIND_ENUM_CONSTANT(USD_PRIM_TYPE_MATERIAL);
	BIND_ENUM_CONSTANT(USD_PRIM_TYPE_SHADER);
	BIND_ENUM_CONSTANT(USD_PRIM_TYPE_SCOPE);
	BIND_ENUM_CONSTANT(USD_PRIM_TYPE_GEOM_SUBSET);
	BIND_ENUM_CONSTANT(USD_PRIM_TYPE_UNKNOWN);
}

String UsdPrim::get_type_name() const {
	if (!is_valid())
		return String();

	const tinyusdz::Prim *prim = internal_prim();
	if (!prim)
		return String();

	return String(prim->type_name().c_str());
}

UsdPrim::UsdPrimType UsdPrim::get_type() const {
	const tinyusdz::Prim *prim = internal_prim();
	if (!prim)
		return USD_PRIM_TYPE_UNKNOWN;
	return get_prim_type(*prim);
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
