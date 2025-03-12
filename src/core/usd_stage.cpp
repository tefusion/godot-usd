#include "usd_stage.h"
#include <godot_cpp/classes/project_settings.hpp>

#include "asset-resolution.hh"
#include "composition.hh"
#include "io-util.hh"
#include "pprinter.hh"
#include "str-util.hh"
#include "stream-reader.hh"
#include "usdShade.hh"
#include "usda-reader.hh"

tinyusdz::Stage *UsdStage::load_stage(const String &path) {
	String global_path = ProjectSettings::get_singleton()->globalize_path(path);
	std::string file_path = global_path.utf8().get_data();

	if (!tinyusdz::io::USDFileExists(file_path)) {
		return nullptr;
	}

	if (!tinyusdz::IsUSDA(file_path)) {
		return nullptr;
	}

	std::vector<uint8_t> data;
	std::string err;
	if (!tinyusdz::io::ReadWholeFile(&data, &err, file_path, /* filesize_max */ 0)) {
		return nullptr;
	}

	tinyusdz::StreamReader sr(data.data(), data.size(), /* swap endian */ false);
	tinyusdz::usda::USDAReader reader(&sr);

	uint32_t load_states = static_cast<uint32_t>(tinyusdz::LoadState::Toplevel);
	bool do_compose = false;
	bool as_primspec = do_compose ? true : false;

	if (!reader.read(load_states, as_primspec)) {
		return nullptr;
	}

	if (!reader.ReconstructStage()) {
		return nullptr;
	}

	// Create new stage and move the data
	auto stage = new tinyusdz::Stage(reader.get_stage());
	return stage;
}

Ref<UsdStage> UsdStage::create(std::shared_ptr<tinyusdz::Stage> stage) {
	Ref<UsdStage> usd_stage;
	usd_stage.instantiate();
	usd_stage->_stage = stage;
	return usd_stage;
}

void UsdStage::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load", "path"), &UsdStage::load);
	ClassDB::bind_method(D_METHOD("is_valid"), &UsdStage::is_valid);
	ClassDB::bind_method(D_METHOD("get_prim_at_path", "path"), &UsdStage::get_prim_at_path);
	ClassDB::bind_method(D_METHOD("get_root_prims"), &UsdStage::get_root_prims);
}

bool UsdStage::load(const String &path) {
	tinyusdz::Stage *stage = load_stage(path);
	if (stage) {
		_stage = std::shared_ptr<tinyusdz::Stage>(stage);
		return true;
	}
	return false;
}

Ref<UsdPrim> UsdStage::get_prim_at_path(Ref<UsdPath> path) const {
	if (!is_valid() || path.is_null() || !path->is_valid()) {
		return Ref<UsdPrim>();
	}

	return UsdPrim::create(_stage, path->get_path());
}

TypedArray<UsdPrim> UsdStage::get_root_prims() const {
	TypedArray<UsdPrim> root_prims;
	ERR_FAIL_COND_V(!is_valid(), root_prims);

	const auto &prims = _stage->root_prims();
	for (const auto &prim : prims) {
		root_prims.push_back(UsdPrim::create(_stage, prim.absolute_path()));
	}

	return root_prims;
}

bool UsdStage::is_valid() const {
	return _stage != nullptr;
}

UsdStage::UsdStage() :
		_stage(nullptr) {
}
