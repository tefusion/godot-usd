#pragma once

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "stage.hh"

#include "usd/usd_common.h"

class UsdLoadedMaterials : public godot::RefCounted {
	GDCLASS(UsdLoadedMaterials, godot::RefCounted);

private:
	/// String -> StandardMaterial3D
	godot::TypedArray<godot::Texture2D> _textures;
	godot::PackedStringArray _image_paths;
	godot::TypedArray<godot::Image> _images;
	godot::HashMap<godot::String, godot::Ref<godot::StandardMaterial3D>> _material_map;

protected:
	static void _bind_methods();

public:
	static godot::Ref<UsdLoadedMaterials> create(const godot::Vector<godot::String> &material_paths,
			const godot::Vector<godot::Ref<godot::StandardMaterial3D>> &materials,
			const godot::Vector<godot::Ref<godot::Texture2D>> &textures,
			const godot::Vector<godot::String> &image_paths,
			const godot::Vector<godot::Ref<godot::Image>> &images);
	godot::PackedStringArray get_material_paths() const;
	godot::Ref<godot::StandardMaterial3D> get_material(const godot::String &abs_path) const;
	godot::Ref<godot::StandardMaterial3D> get_material_with_path(const godot::Ref<UsdPath> &path) const;
	bool has_material(const godot::String &abs_path) const;
	void set_materials(const godot::PackedStringArray &material_paths, const godot::TypedArray<godot::StandardMaterial3D> &materials);

	void set_textures(const godot::TypedArray<godot::Texture2D> &textures);
	godot::TypedArray<godot::Texture2D> get_textures() const;

	void set_image_paths(const godot::PackedStringArray &paths);
	godot::PackedStringArray get_image_paths() const;

	void set_images(const godot::TypedArray<godot::Image> &images);
	godot::TypedArray<godot::Image> get_images() const;

	godot::String _to_string() const;
};

godot::Ref<UsdLoadedMaterials> extract_materials_impl(const tinyusdz::Stage &stage, const godot::String &p_search_path);
