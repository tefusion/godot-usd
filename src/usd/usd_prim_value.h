#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "usd/usd_prim_type.h"

#include "prim-types.hh"
#include "stage.hh"

using namespace godot;

// Small wrapper classes with methods for pretty printing and getting values
class UsdPrimValue : public RefCounted {
	GDCLASS(UsdPrimValue, RefCounted);

protected:
	const tinyusdz::Prim *_prim = nullptr;
	//stage holds prim, done to prevent dangling pointers. Stage is also needed for some tydra methods
	std::shared_ptr<tinyusdz::Stage> _stage = nullptr;
	static void _bind_methods();

public:
	static Ref<UsdPrimValue> create(const tinyusdz::Prim *p_prim, std::shared_ptr<tinyusdz::Stage> p_stage);
	virtual UsdPrimType::Type get_type() const;
};
