extends GdUnitTestSuite

func test_compare_gltf_skeleton():
	var gltf_scene: PackedScene = load("res://test/scenes/capsuleskel.blend")
	var expected_node: Node3D= gltf_scene.instantiate()
	var expected_skeleton: Skeleton3D = expected_node.get_node("Armature/Skeleton3D")
	assert_that(expected_skeleton is Skeleton3D)
	
	var usd_scene: PackedScene = load("res://test/scenes/capsuleskel.usda")
	var usd_node: Node3D = usd_scene.instantiate()
	var usd_skeleton: Skeleton3D = usd_node.get_child(0)
	assert_that(usd_skeleton is Skeleton3D)
	
	assert_bool(expected_skeleton.get_bone_count() ==  usd_skeleton.get_bone_count())
	assert_object(expected_skeleton.transform).is_equal(usd_skeleton.transform)
	for i in range(expected_skeleton.get_bone_count()):
		assert_str(expected_skeleton.get_bone_name(i).replace(".", "_")).is_equal(usd_skeleton.get_bone_name(i))
		assert_object(expected_skeleton.get_bone_rest(i).basis).is_equal(usd_skeleton.get_bone_rest(i).basis)
		assert_object(expected_skeleton.get_bone_rest(i)).is_equal(usd_skeleton.get_bone_rest(i))

	usd_node.queue_free()
	expected_node.queue_free()
