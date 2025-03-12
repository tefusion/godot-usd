#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "tinyusdz.hh"

using namespace godot;

/// Represents a USD path for a prim inside a stage or a property
class UsdPath : public RefCounted {
	GDCLASS(UsdPath, RefCounted);

private:
	tinyusdz::Path _path;

protected:
	static void _bind_methods();

public:
	/// Returns the full path as a string (includes both prim and property parts)
	String full_path() const;

	/// Returns only the prim part of the path
	String prim_path() const;

	/// Returns only the property part of the path
	String property_path() const;

	/// Sets the prim path component (e.g., "/root/prim1")
	void set_prim_path(const String &path);

	/// Sets the property path component (e.g., "visibility")
	void set_property_path(const String &property);

	/// Sets both prim and property components at once
	/// @param path The prim path (e.g., "/root/prim1")
	/// @param property The property name (e.g., "visibility")
	void set_prim_property_path(const String &path, const String &property);

	/// Returns true if this path represents a prim (has no property component)
	bool is_prim_path() const;

	/// Returns true if this path represents a property
	bool is_property_path() const;

	/// Returns true if this is a valid USD path
	bool is_valid() const;

	/// Returns the internal tinyusdz::Path object
	const tinyusdz::Path &get_path() const;

	/// Sets the internal tinyusdz::Path object
	void set_path(const tinyusdz::Path &path);

	UsdPath();
};
