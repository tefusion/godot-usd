#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "usd/usd_prim_type.h"
#include "usd/usd_prim_value.h"
#include "usd_common.h"

using namespace godot;

class UsdPrim : public RefCounted {
	GDCLASS(UsdPrim, RefCounted);

private:
	std::shared_ptr<tinyusdz::Stage> _stage;
	Ref<UsdPath> _path;

protected:
	static void _bind_methods();

public:
	static Ref<UsdPrim> create(std::shared_ptr<tinyusdz::Stage> stage, const tinyusdz::Path &path);
	static UsdPrimType::Type get_prim_type(const tinyusdz::Prim *prim);

	String get_type_name() const;
	UsdPrimType::Type get_type() const;

	bool is_valid() const;

	void set_path(Ref<UsdPath> path);
	Ref<UsdPath> get_path() const;

	TypedArray<UsdPrim> get_children() const;

	const tinyusdz::Prim *internal_prim() const;

	Ref<UsdPrimValue> get_value() const;

	UsdPrim();
};
