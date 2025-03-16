#include "usd_shade.h"
#include "core/usd_common.h"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"
#include "godot_cpp/classes/texture2d.hpp"
#include "godot_cpp/templates/vector.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "prim-types.hh"
#include "tydra/render-data.hh"
#include "tydra/scene-access.hh"

#include "utils/godot_utils.h"

using MaterialMap = std::map<std::string, const tinyusdz::Material *>;

// Macro for applying texture to a material
#define APPLY_TEXTURE(lname, rname)                                    \
	const int64_t &tex_id = mat_shader.lname.texture_id;               \
	ERR_FAIL_COND_V_MSG(tex_id < 0, mat, "Failed loading texture");    \
	const Ref<godot::Texture> &godot_texture = godot_textures[tex_id]; \
	mat->set_texture(BaseMaterial3D::TextureParam::rname, godot_texture);

// Macro for handling material properties with texture fallback
#define HANDLE_MATERIAL_PROPERTY(lname, rname, setter) \
	if (mat_shader.lname.is_texture()) {               \
		APPLY_TEXTURE(lname, rname)                    \
	} else {                                           \
		mat->setter(mat_shader.lname.value);           \
	}

Ref<StandardMaterial3D> create_godot_material(
		const tinyusdz::tydra::RenderMaterial &render_mat,
		const std::vector<tinyusdz::tydra::UVTexture> &loaded_textures,
		const Vector<Ref<godot::Texture2D>> &godot_textures) {
	Ref<StandardMaterial3D> mat;
	mat.instantiate();

	const tinyusdz::tydra::PreviewSurfaceShader &mat_shader = render_mat.surfaceShader;

	// Handle diffuse color / albedo (special case with color conversion)
	if (mat_shader.diffuseColor.is_texture()) {
		APPLY_TEXTURE(diffuseColor, TEXTURE_ALBEDO)

		//ref from gltf importer: https://github.com/godotengine/godot/blob/0028fd625e2c5b202e204bc12828cbca043213d3/modules/gltf/structures/gltf_texture_sampler.h#L87
		const tinyusdz::tydra::UVTexture &texture = loaded_textures[tex_id];

		//godot only supports full or no wrap
		bool texture_repeat = texture.wrapS == tinyusdz::tydra::UVTexture::WrapMode::REPEAT &&
				texture.wrapT == tinyusdz::tydra::UVTexture::WrapMode::REPEAT;
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

	HANDLE_MATERIAL_PROPERTY(metallic, TEXTURE_METALLIC, set_metallic)
	HANDLE_MATERIAL_PROPERTY(roughness, TEXTURE_ROUGHNESS, set_roughness)

	if (mat_shader.emissiveColor.is_texture()) {
		APPLY_TEXTURE(emissiveColor, TEXTURE_EMISSION)
	} else {
		std::array<float, 3> emissive_color = mat_shader.emissiveColor.value;
		Color godot_emissive = Color(emissive_color[0], emissive_color[1], emissive_color[2]);
		mat->set_emission(godot_emissive);
	}

	if (mat_shader.normal.is_texture()) {
		APPLY_TEXTURE(normal, TEXTURE_NORMAL)
	}

	if (mat_shader.clearcoat.value > 0.0f || mat_shader.clearcoat.is_texture()) {
		HANDLE_MATERIAL_PROPERTY(clearcoat, TEXTURE_CLEARCOAT, set_clearcoat)
		HANDLE_MATERIAL_PROPERTY(clearcoatRoughness, TEXTURE_CLEARCOAT, set_clearcoat_roughness)
	}

	if (mat_shader.occlusion.is_texture()) {
		APPLY_TEXTURE(occlusion, TEXTURE_AMBIENT_OCCLUSION)
	}

	//TODO: probably not correct
	if (mat_shader.opacity.value < 1.0f) {
		mat->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);
		mat->set_alpha_scissor_threshold(mat_shader.opacityThreshold.value);
	}

	if (mat_shader.ior.value != 1.5f) {
		mat->set_refraction(mat_shader.ior.value - 1.0f);
	}

	return mat;
}

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
		godot_material_paths.push_back(render_mat.abs_path.c_str());
		godot_materials.push_back(create_godot_material(render_mat, loaded_textures, godot_textures));
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
	result->set_materials(string_vector_to_typed_array(material_paths), ref_vector_to_typed_array(materials));
	result->set_textures(ref_vector_to_typed_array<Texture2D>(textures));
	result->set_image_paths(string_vector_to_typed_array(image_paths));
	result->set_images(ref_vector_to_typed_array<Image>(images));
	return result;
}

void UsdLoadedMaterials::set_textures(const TypedArray<godot::Texture2D> &textures) {
	_textures = textures;
}

TypedArray<godot::Texture2D> UsdLoadedMaterials::get_textures() const {
	return _textures;
}

void UsdLoadedMaterials::set_image_paths(const PackedStringArray &paths) {
	_image_paths = paths;
}

PackedStringArray UsdLoadedMaterials::get_image_paths() const {
	return _image_paths;
}

void UsdLoadedMaterials::set_images(const TypedArray<godot::Image> &images) {
	_images = images;
}

TypedArray<godot::Image> UsdLoadedMaterials::get_images() const {
	return _images;
}

PackedStringArray UsdLoadedMaterials::get_material_paths() const {
	PackedStringArray paths;
	for (HashMap<String, Ref<StandardMaterial3D>>::ConstIterator it = _material_map.begin();
			it != _material_map.end(); ++it) {
		paths.push_back(it->key);
	}
	return paths;
}

Ref<StandardMaterial3D> UsdLoadedMaterials::get_material(const String &abs_path) const {
	ERR_FAIL_COND_V_MSG(!_material_map.has(abs_path), Ref<StandardMaterial3D>(), "Material not found: " + abs_path);
	return _material_map[abs_path];
}

Ref<StandardMaterial3D> UsdLoadedMaterials::get_material_with_path(const Ref<UsdPath> &path) const {
	return get_material(path->full_path());
}

bool UsdLoadedMaterials::has_material(const String &abs_path) const {
	return _material_map.has(abs_path);
}

void UsdLoadedMaterials::set_materials(const PackedStringArray &material_paths,
		const TypedArray<StandardMaterial3D> &materials) {
	_material_map.clear();
	for (int i = 0; i < material_paths.size(); i++) {
		if (i < materials.size()) {
			Ref<StandardMaterial3D> material = materials[i];
			if (material.is_valid()) {
				_material_map[material_paths[i]] = material;
			}
		}
	}
}

String UsdLoadedMaterials::_to_string() const {
	String result = "UsdLoadedMaterials(";
	result += "materials: " + String::num_int64(_material_map.size());
	result += ", textures: " + String::num_int64(_textures.size());
	result += ", images: " + String::num_int64(_images.size());
	result += ")";
	return result;
}

void UsdLoadedMaterials::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_material_paths"), &UsdLoadedMaterials::get_material_paths);
	ClassDB::bind_method(D_METHOD("get_material", "abs_path"), &UsdLoadedMaterials::get_material);
	ClassDB::bind_method(D_METHOD("get_material_with_path", "path"), &UsdLoadedMaterials::get_material_with_path);
	ClassDB::bind_method(D_METHOD("has_material", "abs_path"), &UsdLoadedMaterials::has_material);
	ClassDB::bind_method(D_METHOD("set_materials", "material_paths", "materials"), &UsdLoadedMaterials::set_materials);

	ClassDB::bind_method(D_METHOD("set_textures", "textures"), &UsdLoadedMaterials::set_textures);
	ClassDB::bind_method(D_METHOD("get_textures"), &UsdLoadedMaterials::get_textures);

	ClassDB::bind_method(D_METHOD("set_image_paths", "paths"), &UsdLoadedMaterials::set_image_paths);
	ClassDB::bind_method(D_METHOD("get_image_paths"), &UsdLoadedMaterials::get_image_paths);

	ClassDB::bind_method(D_METHOD("set_images", "images"), &UsdLoadedMaterials::set_images);
	ClassDB::bind_method(D_METHOD("get_images"), &UsdLoadedMaterials::get_images);

	ClassDB::bind_method(D_METHOD("_to_string"), &UsdLoadedMaterials::_to_string);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "textures", PROPERTY_HINT_ARRAY_TYPE, "Texture2D"), "set_textures", "get_textures");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "image_paths"), "set_image_paths", "get_image_paths");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "images", PROPERTY_HINT_ARRAY_TYPE, "Image"), "set_images", "get_images");
}
