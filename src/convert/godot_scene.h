#include "core/usd_geom.h"
#include "core/usd_stage.h"
#include "godot_cpp/classes/array_mesh.hpp"
#include "godot_cpp/classes/object.hpp"

using namespace godot;
class UsdGodotSceneConverter : public Object {
	GDCLASS(UsdGodotSceneConverter, Object);

protected:
	static void _bind_methods();

public:
	UsdGodotSceneConverter();
	~UsdGodotSceneConverter();

	Ref<ArrayMesh> convert_mesh(const Ref<UsdPrimValueGeomMesh> &geom_mesh, const Ref<UsdLoadedMaterials> &materials);
};
