#include "usd_shade.h"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"
#include "godot_cpp/classes/texture2d.hpp"
#include "godot_cpp/templates/vector.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "prim-types.hh"
#include "tydra/render-data.hh"
#include "tydra/scene-access.hh"

#include "utils/godot_utils.h"

using MaterialMap = std::map<std::string, const tinyusdz::Material *>;

Ref<UsdLoadedMaterials> extract_materials_impl(const tinyusdz::Stage &stage, const String &p_search_path = "") {
	Ref<UsdLoadedMaterials> godot_material_map = nullptr;
	tinyusdz::tydra::RenderSceneConverter converter;
	tinyusdz::tydra::RenderSceneConverterEnv env(stage);
	// We need to load materials since otherwise they won't be added to the image list
	// Main issue is that if tinyusdz doesn't support a image format, this won't work
	// TODO: search for images.emplace_back(texImage); in render-data.cc, we could try adjusting TextureImage to also support just holding a path
	env.scene_config.load_texture_assets = true;
	env.material_config.allow_texture_load_failure = false;
	std::string project_search_path = ProjectSettings::get_singleton()->globalize_path("res://").utf8().get_data();
	std::string custom_search_path = ProjectSettings::get_singleton()->globalize_path(p_search_path).utf8().get_data();
	env.set_search_paths({ custom_search_path, project_search_path });

	MaterialMap material_map;

	ERR_FAIL_COND_V(!tinyusdz::tydra::ListPrims(stage, material_map), godot_material_map);

	Vector<String> godot_material_paths;
	Vector<Ref<StandardMaterial3D>> godot_materials;
	Vector<Ref<godot::Texture2D>> godot_textures;
	Vector<Ref<godot::Image>> godot_images;
	Vector<String> godot_image_paths;

	// Convert materials
	std::vector<tinyusdz::tydra::RenderMaterial> render_materials;
	render_materials.resize(material_map.size());
	int material_index = 0;
	for (const auto &pair : material_map) {
		const std::string &material_path = pair.first;
		const tinyusdz::Path &abs_mat_path = tinyusdz::Path(material_path, "");
		const tinyusdz::Material *material = pair.second;
		bool sucess = converter.ConvertMaterial(env, abs_mat_path, *material, &render_materials[material_index]);
		ERR_CONTINUE_MSG(!sucess, String("Failed to convert material ") + material_path.c_str() + String("Load Error: ") + converter.GetError().c_str());
	}

	//Collect textures / images
	const std::vector<tinyusdz::tydra::UVTexture> &loaded_textures = converter.textures;
	const std::vector<tinyusdz::tydra::TextureImage> &loaded_images = converter.images;
	for (const auto &texture : loaded_textures) {
		const std::string &prim_path = texture.abs_path;
		const int64_t &image_id = texture.texture_image_id;
		ERR_CONTINUE_MSG(image_id < 0, "Failed loading texture image");
		const tinyusdz::tydra::TextureImage &image = loaded_images[image_id];
		const std::string &image_file_path = image.asset_identifier;
		String godot_image_file_path = String(image_file_path.c_str());
		godot_image_file_path = ProjectSettings::get_singleton()->localize_path(godot_image_file_path);
		Ref<Image> godot_image = godot::Image::load_from_file(godot_image_file_path);
		ERR_CONTINUE_MSG(godot_image.is_null(), String("Failed to load image ") + godot_image_file_path);
		godot_images.push_back(godot_image);
		godot_image_paths.push_back(godot_image_file_path);

		Ref<ImageTexture> godot_image_texture = ImageTexture::create_from_image(godot_image);
		godot_textures.push_back(godot_image_texture);
	}

	// Create Godot materials
	for (const auto &render_mat : render_materials) {
		Ref<StandardMaterial3D> mat;
		mat.instantiate();

		const tinyusdz::tydra::PreviewSurfaceShader &mat_shader = render_mat.surfaceShader;
		if (mat_shader.diffuseColor.is_texture()) {
			const int64_t &tex_id = mat_shader.diffuseColor.texture_id;
			ERR_CONTINUE_MSG(tex_id < 0, "Failed loading texture");
			const Ref<godot::Texture> &godot_texture = godot_textures[tex_id];
			mat->set_texture(BaseMaterial3D::TextureParam::TEXTURE_ALBEDO, godot_texture);

			//ref from gltf importer: https://github.com/godotengine/godot/blob/0028fd625e2c5b202e204bc12828cbca043213d3/modules/gltf/structures/gltf_texture_sampler.h#L87
			const tinyusdz::tydra::UVTexture &texture = loaded_textures[tex_id];

			//godot only supports full or no wrap
			bool texture_repeat = texture.wrapS == tinyusdz::tydra::UVTexture::WrapMode::REPEAT && texture.wrapT == tinyusdz::tydra::UVTexture::WrapMode::REPEAT;
			mat->set_flag(BaseMaterial3D::FLAG_USE_TEXTURE_REPEAT, texture_repeat);

			// TODO: TextureFilter? only saw that TextureImage has mipmap, but otherwise nothing

			//uv1 scale/offset
			const std::array<float, 2> &uv1_scale = texture.tx_scale;
			const std::array<float, 2> &uv1_offset = texture.tx_translation;

			//third component is just for triplanar textures
			const Vector3 godot_uv1_scale = Vector3(uv1_scale[0], uv1_scale[1], 1.0);
			const Vector3 godot_uv1_offset = Vector3(uv1_offset[0], uv1_offset[1], 0.0);
			mat->set_uv1_scale(godot_uv1_scale);
			mat->set_uv1_offset(godot_uv1_offset);
		} else {
			std::array<float, 3> diffuse_color = mat_shader.diffuseColor.value;
			Color godot_diffuse_color = Color(diffuse_color[0], diffuse_color[1], diffuse_color[2]);
			mat->set_albedo(godot_diffuse_color);
		}
		//TODO all other properties

		godot_material_paths.push_back(render_mat.abs_path.c_str());
		godot_materials.push_back(mat);
	}

	return UsdLoadedMaterials::create(godot_material_paths, godot_materials, godot_textures, godot_image_paths, godot_images);
}

Ref<UsdLoadedMaterials> UsdLoadedMaterials::create(const Vector<String> &material_paths,
		const Vector<Ref<StandardMaterial3D>> &materials,
		const Vector<Ref<godot::Texture2D>> &textures,
		const Vector<String> &image_paths,
		const Vector<Ref<godot::Image>> &images) {
	Ref<UsdLoadedMaterials> result;
	result.instantiate();
	result->set_material_paths(string_vector_to_typed_array(material_paths));
	result->set_materials(ref_vector_to_typed_array<StandardMaterial3D>(materials));
	result->set_textures(ref_vector_to_typed_array<Texture2D>(textures));
	result->set_image_paths(string_vector_to_typed_array(image_paths));
	result->set_images(ref_vector_to_typed_array<Image>(images));
	return result;
}

void UsdLoadedMaterials::set_material_paths(const TypedArray<String> &paths) {
	_material_paths = paths;
}

TypedArray<String> UsdLoadedMaterials::get_material_paths() const {
	return _material_paths;
}

void UsdLoadedMaterials::set_materials(const TypedArray<StandardMaterial3D> &materials) {
	_materials = materials;
}

TypedArray<StandardMaterial3D> UsdLoadedMaterials::get_materials() const {
	return _materials;
}

void UsdLoadedMaterials::set_textures(const TypedArray<godot::Texture2D> &textures) {
	_textures = textures;
}

TypedArray<godot::Texture2D> UsdLoadedMaterials::get_textures() const {
	return _textures;
}

void UsdLoadedMaterials::set_image_paths(const TypedArray<String> &paths) {
	_image_paths = paths;
}

TypedArray<String> UsdLoadedMaterials::get_image_paths() const {
	return _image_paths;
}

void UsdLoadedMaterials::set_images(const TypedArray<godot::Image> &images) {
	_images = images;
}

TypedArray<godot::Image> UsdLoadedMaterials::get_images() const {
	return _images;
}

String UsdLoadedMaterials::_to_string() const {
	String result = "UsdLoadedMaterials(";
	result += "materials: " + String::num_int64(_materials.size());
	result += ", textures: " + String::num_int64(_textures.size());
	result += ", images: " + String::num_int64(_images.size());
	result += ")";
	return result;
}

void UsdLoadedMaterials::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_material_paths", "paths"), &UsdLoadedMaterials::set_material_paths);
	ClassDB::bind_method(D_METHOD("get_material_paths"), &UsdLoadedMaterials::get_material_paths);

	ClassDB::bind_method(D_METHOD("set_materials", "materials"), &UsdLoadedMaterials::set_materials);
	ClassDB::bind_method(D_METHOD("get_materials"), &UsdLoadedMaterials::get_materials);

	ClassDB::bind_method(D_METHOD("set_textures", "textures"), &UsdLoadedMaterials::set_textures);
	ClassDB::bind_method(D_METHOD("get_textures"), &UsdLoadedMaterials::get_textures);

	ClassDB::bind_method(D_METHOD("set_image_paths", "paths"), &UsdLoadedMaterials::set_image_paths);
	ClassDB::bind_method(D_METHOD("get_image_paths"), &UsdLoadedMaterials::get_image_paths);

	ClassDB::bind_method(D_METHOD("set_images", "images"), &UsdLoadedMaterials::set_images);
	ClassDB::bind_method(D_METHOD("get_images"), &UsdLoadedMaterials::get_images);

	ClassDB::bind_method(D_METHOD("_to_string"), &UsdLoadedMaterials::_to_string);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "material_paths", PROPERTY_HINT_ARRAY_TYPE, "String"), "set_material_paths", "get_material_paths");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "materials", PROPERTY_HINT_ARRAY_TYPE, "StandardMaterial3D"), "set_materials", "get_materials");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "textures", PROPERTY_HINT_ARRAY_TYPE, "Texture2D"), "set_textures", "get_textures");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "image_paths", PROPERTY_HINT_ARRAY_TYPE, "String"), "set_image_paths", "get_image_paths");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "images", PROPERTY_HINT_ARRAY_TYPE, "Image"), "set_images", "get_images");
}
