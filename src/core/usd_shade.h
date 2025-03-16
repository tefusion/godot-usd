#include "core/usd_common.h"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"
#include "godot_cpp/classes/texture.hpp"
#include "godot_cpp/classes/texture2d.hpp"
#include "godot_cpp/templates/hash_map.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

#include "stage.hh"
#include "tinyusdz.hh"

using namespace godot;

class UsdLoadedMaterials : public RefCounted {
	GDCLASS(UsdLoadedMaterials, RefCounted);

private:
	/// String -> StandardMaterial3D
	TypedArray<godot::Texture2D> _textures;
	PackedStringArray _image_paths;
	TypedArray<godot::Image> _images;
	HashMap<String, Ref<StandardMaterial3D>> _material_map;

protected:
	static void _bind_methods();

public:
	static Ref<UsdLoadedMaterials> create(const Vector<String> &material_paths,
			const Vector<Ref<StandardMaterial3D>> &materials,
			const Vector<Ref<godot::Texture2D>> &textures,
			const Vector<String> &image_paths,
			const Vector<Ref<godot::Image>> &images);
	PackedStringArray get_material_paths() const;
	Ref<StandardMaterial3D> get_material(const String &abs_path) const;
	Ref<StandardMaterial3D> get_material_with_path(const Ref<UsdPath> &path) const;
	bool has_material(const String &abs_path) const;
	void set_materials(const PackedStringArray &material_paths, const TypedArray<StandardMaterial3D> &materials);

	void set_textures(const TypedArray<godot::Texture2D> &textures);
	TypedArray<godot::Texture2D> get_textures() const;

	void set_image_paths(const PackedStringArray &paths);
	PackedStringArray get_image_paths() const;

	void set_images(const TypedArray<godot::Image> &images);
	TypedArray<godot::Image> get_images() const;

	String _to_string() const;
};

Ref<UsdLoadedMaterials> extract_materials_impl(const tinyusdz::Stage &stage, const String &p_search_path);
