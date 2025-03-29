#include "godot_cpp/classes/array_mesh.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/skeleton3d.hpp"
#include "usd/usd_geom.h"
#include "usd/usd_stage.h"

using namespace godot;
class UsdGodotSceneConverter : public Object {
	GDCLASS(UsdGodotSceneConverter, Object);

protected:
	static void _bind_methods();

public:
	UsdGodotSceneConverter();
	~UsdGodotSceneConverter();

	Ref<ArrayMesh> convert_mesh(const Ref<UsdPrimValueGeomMesh> &geom_mesh, const Ref<UsdLoadedMaterials> &materials, const Vector3::Axis up_axis);

	//need to apply Node3D transform from SkelRoot outside
	Skeleton3D *convert_skeleton(const Ref<UsdPrimValueSkeleton> &skeleton);
};
