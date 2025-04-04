#include "usd_prim_value.h"
#include "usd_geom.h"
#include "usd_prim.h"

using namespace godot;

//godot doesn't work well with constructors with arguments so we just make an empty ref and set values in the create method
template <typename T>
static Ref<T> create_typed() {
	Ref<T> ref;
	ref.instantiate();
	return ref;
}

Ref<UsdPrimValue> UsdPrimValue::create(const tinyusdz::Prim *p_prim, std::shared_ptr<tinyusdz::Stage> p_stage) {
	if (!p_prim || !p_stage) {
		return Ref<UsdPrimValue>();
	}

	UsdPrimType::Type type = UsdPrim::get_prim_type(p_prim);
	Ref<UsdPrimValue> prim_value = nullptr;

	switch (type) {
		case UsdPrimType::USD_PRIM_TYPE_XFORM:
			prim_value = create_typed<UsdPrimValueXform>();
			break;
		case UsdPrimType::USD_PRIM_TYPE_MESH:
			prim_value = create_typed<UsdPrimValueGeomMesh>();
			break;
		case UsdPrimType::USD_PRIM_TYPE_SKELETON:
			prim_value = create_typed<UsdPrimValueSkeleton>();
			break;
		case UsdPrimType::USD_PRIM_TYPE_SKELETON_ROOT:
			prim_value = create_typed<UsdPrimValueSkeletonRoot>();
			break;
		default:
			prim_value = create_typed<UsdPrimValue>();
	}
	prim_value->_prim = p_prim;
	prim_value->_stage = p_stage;
	return prim_value;
}

UsdPrimType::Type UsdPrimValue::get_type() const {
	return UsdPrimType::USD_PRIM_TYPE_UNKNOWN;
}

void UsdPrimValue::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_type"), &UsdPrimValue::get_type);
}
