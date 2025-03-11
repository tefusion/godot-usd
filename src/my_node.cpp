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
#include "type_utils.h"
#include "usda-reader.hh"

using namespace godot;

void MyNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("hello_node"), &MyNode::hello_node);
}

MyNode::MyNode() {
}

MyNode::~MyNode() {
}

void MyNode::_ready() {
}

void MyNode::_process(double delta) {
}

void apply_euler_rotation(Transform3D &result_transform, const tinyusdz::XformOp &transform, EulerOrder order) {
	Vector3 angles;
	if (xform_get_value(transform, angles)) {
		Vector3 euler_rad(Math::deg_to_rad(angles.x),
				Math::deg_to_rad(angles.y),
				Math::deg_to_rad(angles.z));
		result_transform = result_transform * Transform3D(Basis::from_euler(euler_rad, order));
	}
}

void MyNode::hello_node() {
	String godot_path = ProjectSettings::get_singleton()->globalize_path("res://2meshes.usda");
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
	auto root_prims = stage.root_prims();
	for (const auto &prim : root_prims) {
		std::string prim_name = prim.element_name();
		const std::vector<tinyusdz::Prim> &children = prim.children();
		UtilityFunctions::print("prim_name: ", prim_name.c_str());
		for (const auto &child : children) {
			std::string child_name = child.element_name();
			std::string type = child.type_name();
			UtilityFunctions::print("child_name: ", child_name.c_str(), " type: ", type.c_str());

			if (child.is<tinyusdz::Xform>()) {
				const tinyusdz::Xform *xform = child.as<tinyusdz::Xform>();
				const std::vector<tinyusdz::XformOp> &transforms = xform->xformOps;

				Transform3D result_transform;
				for (const auto &transform : transforms) {
					switch (transform.op_type) {
						case tinyusdz::XformOp::OpType::Transform: {
							Transform3D transform_matrix;
							if (xform_get_value(transform, transform_matrix)) {
								result_transform = result_transform * transform_matrix;
							}
							break;
						}
						case tinyusdz::XformOp::OpType::Translate: {
							Vector3 translation;
							if (xform_get_value(transform, translation)) {
								result_transform = result_transform.translated(translation);
							}
							break;
						}
						case tinyusdz::XformOp::OpType::Scale: {
							Vector3 scale;
							if (xform_get_value(transform, scale)) {
								result_transform.scale(scale);
							}
							break;
						}
						case tinyusdz::XformOp::OpType::RotateX: {
							double angle;
							if (xform_get_value(transform, angle)) {
								result_transform.rotate(Vector3(1, 0, 0), Math::deg_to_rad(angle));
							}
							break;
						}
						case tinyusdz::XformOp::OpType::RotateY: {
							double angle;
							if (xform_get_value(transform, angle)) {
								result_transform.rotate(Vector3(0, 1, 0), Math::deg_to_rad(angle));
							}
							break;
						}
						case tinyusdz::XformOp::OpType::RotateZ: {
							double angle;
							if (xform_get_value(transform, angle)) {
								result_transform.rotate(Vector3(0, 0, 1), Math::deg_to_rad(angle));
							}
							break;
						}
						case tinyusdz::XformOp::OpType::RotateXYZ:
							apply_euler_rotation(result_transform, transform, EULER_ORDER_XYZ);
							break;
						case tinyusdz::XformOp::OpType::RotateXZY:
							apply_euler_rotation(result_transform, transform, EULER_ORDER_XZY);
							break;
						case tinyusdz::XformOp::OpType::RotateYXZ:
							apply_euler_rotation(result_transform, transform, EULER_ORDER_YXZ);
							break;
						case tinyusdz::XformOp::OpType::RotateYZX:
							apply_euler_rotation(result_transform, transform, EULER_ORDER_YZX);
							break;
						case tinyusdz::XformOp::OpType::RotateZXY:
							apply_euler_rotation(result_transform, transform, EULER_ORDER_ZXY);
							break;
						case tinyusdz::XformOp::OpType::RotateZYX:
							apply_euler_rotation(result_transform, transform, EULER_ORDER_ZYX);
							break;

						case tinyusdz::XformOp::OpType::Orient: {
							Quaternion quaternion;
							if (xform_get_value(transform, quaternion)) {
								result_transform.basis = Basis(quaternion);
							}
							break;
						}
						case tinyusdz::XformOp::OpType::ResetXformStack:
							result_transform = Transform3D(); // Reset to identity
							break;
					}
				}

				UtilityFunctions::print("result_transform: ", result_transform.basis.get_scale_local());
			}

			else if (child.is<tinyusdz::GeomMesh>()) {
				const tinyusdz::GeomMesh *geom_mesh = child.as<tinyusdz::GeomMesh>();
			}
		}
	}
	std::string exportString = stage.ExportToString();
	UtilityFunctions::print("stage: ", exportString.c_str());
}
