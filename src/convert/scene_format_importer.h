#pragma once

#include <godot_cpp/classes/editor_scene_format_importer.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

class UsdSceneFormatImporter : public godot::EditorSceneFormatImporter {
	GDCLASS(UsdSceneFormatImporter, godot::EditorSceneFormatImporter);

protected:
	static void _bind_methods();

public:
	virtual godot::PackedStringArray _get_extensions() const override;
	virtual godot::Object *_import_scene(const godot::String &p_path, uint32_t p_flags, const godot::Dictionary &p_options) override;
	virtual void _get_import_options(const godot::String &p_path) override;
	virtual godot::Variant _get_option_visibility(const godot::String &p_path, bool p_for_animation, const godot::String &p_option) const override;
};
