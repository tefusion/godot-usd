#pragma once

#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/core/class_db.hpp>

class UsdEditorPlugin : public godot::EditorPlugin {
	GDCLASS(UsdEditorPlugin, godot::EditorPlugin);

protected:
	static void _bind_methods();

public:
	virtual void _enter_tree() override;
	virtual void _exit_tree() override;
};
