#include "my_node.hpp"

#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "asset-resolution.hh"
#include "composition.hh"
#include "io-util.hh"
#include "pprinter.hh"
#include "str-util.hh"
#include "stream-reader.hh"
#include "usda-reader.hh"

using namespace godot;

void MyNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("hello_node"), &MyNode::hello_node);
}

MyNode::MyNode() {
}

MyNode::~MyNode() {
}

// Override built-in methods with your own logic. Make sure to declare them in the header as well!

void MyNode::_ready() {
}

void MyNode::_process(double delta) {
}

void MyNode::hello_node() {
	String godot_path = ProjectSettings::get_singleton()->globalize_path("res://HelloWorld.usda");
	UtilityFunctions::print(godot_path);
	std::string path = godot_path.ascii().get_data();
	bool exists = tinyusdz::io::USDFileExists(path);
	UtilityFunctions::print("Path exists: ", exists);
	if (!exists)
		return;

	if (!tinyusdz::IsUSDA(path)) {
		UtilityFunctions::printerr("File not USDA");
		return;
	}

	std::vector<uint8_t> data;
	std::string err;
	if (!tinyusdz::io::ReadWholeFile(&data, &err, path,
				/* filesize_max */ 0)) {
		UtilityFunctions::printerr("Failed to open file ", godot_path, err.c_str());
		return;
	}

	tinyusdz::StreamReader sr(data.data(), data.size(), /* swap endian */ false);
	tinyusdz::usda::USDAReader reader(&sr);

	uint32_t load_states = static_cast<uint32_t>(tinyusdz::LoadState::Toplevel);
	// load_states |= static_cast<uint32_t>(tinyusdz::LoadState::Sublayer);
	// load_states |= static_cast<uint32_t>(tinyusdz::LoadState::Payload);
	// load_states |= static_cast<uint32_t>(tinyusdz::LoadState::Reference);

	bool do_compose = false;
	bool as_primspec = do_compose ? true : false;
	bool ret = reader.read(load_states, as_primspec);

	if (!ret) {
		UtilityFunctions::printerr("Failed to process usda file.");
		return;
	}

	//composite
	// tinyusdz::Layer root_layer;
	// ret = reader.get_as_layer(&root_layer);
	// if (!ret) {
	// 	UtilityFunctions::printerr("Failed to get root layer");
	// 	return;
	// }
	// tinyusdz::Stage stage;
	// stage.metas() = root_layer.metas();
	// std::string debug = stage.dump_prim_tree();

	ret = reader.ReconstructStage();
	if (!ret) {
		std::cerr << "Failed to reconstruct Stage: \n";
		std::cerr << reader.get_error() << "\n";
		return;
	}

	tinyusdz::Stage stage = reader.get_stage();
	std::string exportString = stage.ExportToString();
	UtilityFunctions::print("stage: ", exportString.c_str());
}
