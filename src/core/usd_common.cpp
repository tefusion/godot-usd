#include "usd_common.h"

void UsdPath::_bind_methods() {
	ClassDB::bind_static_method("UsdPath", D_METHOD("from_string", "path"), &UsdPath::from_string);
	ClassDB::bind_method(D_METHOD("full_path"), &UsdPath::full_path);
	ClassDB::bind_method(D_METHOD("prim_path"), &UsdPath::prim_path);
	ClassDB::bind_method(D_METHOD("property_path"), &UsdPath::property_path);
	ClassDB::bind_method(D_METHOD("set_prim_path", "path"), &UsdPath::set_prim_path);
	ClassDB::bind_method(D_METHOD("set_property_path", "property"), &UsdPath::set_property_path);
	ClassDB::bind_method(D_METHOD("set_prim_property_path", "path", "property"), &UsdPath::set_prim_property_path);
	ClassDB::bind_method(D_METHOD("is_prim_path"), &UsdPath::is_prim_path);
	ClassDB::bind_method(D_METHOD("is_property_path"), &UsdPath::is_property_path);
	ClassDB::bind_method(D_METHOD("is_valid"), &UsdPath::is_valid);
	ClassDB::bind_method(D_METHOD("_to_string"), &UsdPath::_to_string);
}

Ref<UsdPath> UsdPath::create(const tinyusdz::Path &path) {
	Ref<UsdPath> usd_path;
	usd_path.instantiate();
	usd_path->set_path(path);
	return usd_path;
}

Ref<UsdPath> UsdPath::from_string(const String &path) {
	Ref<UsdPath> usd_path;
	usd_path.instantiate();
	usd_path->set_path(tinyusdz::Path(path.utf8().get_data(), ""));
	return usd_path;
}

String UsdPath::full_path() const {
	return String(_path.full_path_name().c_str());
}

String UsdPath::prim_path() const {
	return String(_path.prim_part().c_str());
}

String UsdPath::property_path() const {
	return String(_path.prop_part().c_str());
}

void UsdPath::set_prim_path(const String &path) {
	_path = tinyusdz::Path(path.utf8().get_data(), "");
}

void UsdPath::set_property_path(const String &property) {
	_path = tinyusdz::Path("", property.utf8().get_data());
}

void UsdPath::set_prim_property_path(const String &path, const String &property) {
	_path = tinyusdz::Path(path.utf8().get_data(), property.utf8().get_data());
}

bool UsdPath::is_prim_path() const {
	return _path.is_prim_path();
}

bool UsdPath::is_property_path() const {
	return _path.is_property_path();
}

bool UsdPath::is_valid() const {
	return _path.is_valid();
}

const tinyusdz::Path &UsdPath::get_path() const {
	return _path;
}

void UsdPath::set_path(const tinyusdz::Path &path) {
	_path = path;
}

String UsdPath::_to_string() const {
	return String(_path.full_path_name().c_str());
}

UsdPath::UsdPath() {
	_path = tinyusdz::Path();
}
