#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"
#include "godot_cpp/classes/texture.hpp"
#include "godot_cpp/classes/texture2d.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

#include "stage.hh"
#include "tinyusdz.hh"

using namespace godot;

class UsdLoadedMaterials : public RefCounted {
	GDCLASS(UsdLoadedMaterials, RefCounted);

private:
	/// String -> StandardMaterial3D
	TypedArray<String> _material_paths;
	TypedArray<StandardMaterial3D> _materials;
	TypedArray<godot::Texture2D> _textures;
	TypedArray<String> _image_paths;
	TypedArray<godot::Image> _images;

protected:
	static void _bind_methods();

public:
	static Ref<UsdLoadedMaterials> create(const Vector<String> &material_paths,
			const Vector<Ref<StandardMaterial3D>> &materials,
			const Vector<Ref<godot::Texture2D>> &textures,
			const Vector<String> &image_paths,
			const Vector<Ref<godot::Image>> &images);

	void set_material_paths(const TypedArray<String> &paths);
	TypedArray<String> get_material_paths() const;

	void set_materials(const TypedArray<StandardMaterial3D> &materials);
	TypedArray<StandardMaterial3D> get_materials() const;

	void set_textures(const TypedArray<godot::Texture2D> &textures);
	TypedArray<godot::Texture2D> get_textures() const;

	void set_image_paths(const TypedArray<String> &paths);
	TypedArray<String> get_image_paths() const;

	void set_images(const TypedArray<godot::Image> &images);
	TypedArray<godot::Image> get_images() const;

	String _to_string() const;
};

Ref<UsdLoadedMaterials> extract_materials_impl(const tinyusdz::Stage &stage, const String &p_search_path);
