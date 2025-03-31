extends GdUnitTestSuite

func test_load_basic_mesh():
	var usd_scene: PackedScene = load("res://test/scenes/2meshes.usda")
	assert_that(usd_scene).is_not_null()

	var usd_node: Node3D = usd_scene.instantiate()
	assert_that(usd_node).is_not_null()

	assert_int(usd_node.get_child_count()).is_greater(0)

	var mesh_instance = _find_first_mesh_instance(usd_node)
	assert_that(mesh_instance).is_not_null()

	var mesh = mesh_instance.mesh
	assert_that(mesh).is_not_null()
	assert_int(mesh.get_surface_count()).is_greater(0)

	usd_node.queue_free()

func test_mesh_has_valid_arrays():
	var usd_scene: PackedScene = load("res://test/scenes/texturedmesh.usda")
	var usd_node: Node3D = usd_scene.instantiate()

	var mesh_instance = _find_first_mesh_instance(usd_node)
	assert_that(mesh_instance).is_not_null()

	var mesh: ArrayMesh = mesh_instance.mesh
	assert_that(mesh).is_not_null()

	for surface_idx in range(mesh.get_surface_count()):
		var arrays = mesh.surface_get_arrays(surface_idx)

		assert_that(arrays[Mesh.ARRAY_VERTEX]).is_not_null()
		assert_int(arrays[Mesh.ARRAY_VERTEX].size()).is_greater(0)

		assert_that(arrays[Mesh.ARRAY_NORMAL]).is_not_null()
		assert_int(arrays[Mesh.ARRAY_NORMAL].size()).is_greater(0)

		if arrays[Mesh.ARRAY_TEX_UV].size() > 0:
			assert_int(arrays[Mesh.ARRAY_TEX_UV].size()).is_equal(arrays[Mesh.ARRAY_VERTEX].size())

	usd_node.queue_free()

func test_textured_mesh_has_material():
	var usd_scene: PackedScene = load("res://test/scenes/texturedmesh.usda")
	var usd_node: Node3D = usd_scene.instantiate()

	var mesh_instance = _find_first_mesh_instance(usd_node)
	assert_that(mesh_instance).is_not_null()

	var mesh = mesh_instance.mesh
	assert_that(mesh).is_not_null()

	assert_int(mesh_instance.mesh.get_surface_count()).is_greater_equal(0)

	if mesh_instance.get_surface_override_material_count() > 0:
		var material = mesh_instance.mesh.surface_get_material(0)
		assert_that(material).is_not_null()

	if mesh.get_surface_count() > 0:
		var material = mesh.surface_get_material(0)
		if material != null:
			assert(material is BaseMaterial3D)

	usd_node.queue_free()


func test_mesh_has_valid_indices():
	var usd_scene: PackedScene = load("res://test/scenes/2meshes.usda")
	var usd_node: Node3D = usd_scene.instantiate()

	var mesh_instance = _find_first_mesh_instance(usd_node)
	assert_that(mesh_instance).is_not_null()

	var mesh: ArrayMesh = mesh_instance.mesh
	assert_that(mesh).is_not_null()

	for surface_idx in range(mesh.get_surface_count()):
		var arrays = mesh.surface_get_arrays(surface_idx)

		var indices = arrays[Mesh.ARRAY_INDEX]
		if indices != null and indices.size() > 0:
			var vertices = arrays[Mesh.ARRAY_VERTEX]
			var max_index = 0
			for i in range(indices.size()):
				if indices[i] > max_index:
					max_index = indices[i]

			assert_int(max_index).is_less(vertices.size())

	usd_node.queue_free()

func _find_first_mesh_instance(node: Node) -> MeshInstance3D:
	if node is MeshInstance3D:
		return node

	for child in node.get_children():
		var result = _find_first_mesh_instance(child)
		if result != null:
			return result

	return null

func _find_child_mesh_instances(node: Node) -> Array:
	var results = []

	for child in node.get_children():
		if child is MeshInstance3D:
			results.append(child)
		results.append_array(_find_child_mesh_instances(child))

	return results
