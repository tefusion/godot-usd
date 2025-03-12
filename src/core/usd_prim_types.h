#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "core/usd_prim_type.h"
#include "godot_cpp/variant/transform3d.hpp"
#include "usd_prim_type.h"

using namespace godot;

namespace tinyusdz {
class Xform;
}

class UsdPrimValue : public RefCounted {
	GDCLASS(UsdPrimValue, RefCounted);

protected:
	static void _bind_methods();

public:
	virtual UsdPrimType::Type get_type() const;
};

class UsdPrimValueXform : public UsdPrimValue {
	GDCLASS(UsdPrimValueXform, UsdPrimValue);

private:
	Transform3D _transform;
	String _name;

protected:
	static void _bind_methods();

public:
	static Ref<UsdPrimValueXform> create(const tinyusdz::Xform *xform);
	UsdPrimType::Type get_type() const override;
	String _to_string() const;

	void set_transform(const Transform3D &p_transform);
	Transform3D get_transform() const;

	void set_name(const String &p_name);
	String get_name() const;
};
