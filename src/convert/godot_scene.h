#include "godot_cpp/classes/importer_mesh.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/skeleton3d.hpp"
#include "usd/usd_geom.h"
#include "usd/usd_stage.h"

class UsdGodotSceneConverter : public godot::Object {
	GDCLASS(UsdGodotSceneConverter, godot::Object);

protected:
	static void _bind_methods();

public:
	UsdGodotSceneConverter();
	~UsdGodotSceneConverter();

	godot::Ref<godot::ImporterMesh> convert_mesh(const godot::Ref<UsdPrimValueGeomMesh> &geom_mesh, const godot::Ref<UsdLoadedMaterials> &materials, const godot::Vector3::Axis up_axis);
	godot::Skeleton3D *convert_skeleton(const godot::Ref<UsdPrimValueSkeleton> &skeleton, const godot::Vector3::Axis up_axis);

	godot::Ref<godot::PackedScene> convert_scene(const godot::Ref<UsdStage> &p_stage);
};
