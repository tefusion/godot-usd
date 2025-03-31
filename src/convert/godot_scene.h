#pragma once

#include <godot_cpp/classes/importer_mesh.hpp>
#include <godot_cpp/classes/importer_mesh_instance3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/skeleton3d.hpp>

#include "usd/usd_geom.h"
#include "usd/usd_prim.h"
#include "usd/usd_stage.h"

class UsdGodotSceneConverter : public godot::RefCounted {
	GDCLASS(UsdGodotSceneConverter, godot::RefCounted);

private:
	godot::Ref<UsdStage> _stage;
	godot::Ref<UsdLoadedMaterials> _materials;

protected:
	static void _bind_methods();

public:
	UsdGodotSceneConverter();
	~UsdGodotSceneConverter();
	bool load(const godot::Ref<UsdStage> &stage);

	godot::Ref<godot::ImporterMesh> convert_mesh(const godot::Ref<UsdPrimValueGeomMesh> &geom_mesh, const godot::Vector3::Axis up_axis);

	godot::Skeleton3D *convert_skeleton(const godot::Ref<UsdPrimValueSkeleton> &skeleton, const godot::Vector3::Axis up_axis);
	godot::Node3D *convert_xform(const godot::Ref<UsdPrim> &xform, godot::Node3D *parent, const godot::Vector3::Axis up_axis);
	godot::Skeleton3D *convert_skeleton_root(const godot::Ref<UsdPrim> &skeleton_root_prim, godot::Node3D *parent, const godot::Vector3::Axis up_axis);
	godot::ImporterMeshInstance3D *convert_mesh_instance(const godot::Ref<UsdPrim> &mesh_instance_prim, godot::Node3D *parent, const godot::Vector3::Axis up_axis);

	void convert_prim_children(const godot::Ref<UsdPrim> &prim, godot::Node3D *parent, const godot::Vector3::Axis up_axis);

	godot::Node *convert_prim(const godot::Ref<UsdPrim> &prim, godot::Node3D *parent, const godot::Vector3::Axis up_axis);
};
