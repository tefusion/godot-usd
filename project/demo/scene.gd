extends Node

func _ready() -> void:
	var stage := UsdStage.new()
	stage.load("res://2meshes.usda")
	var root_prims:=stage.get_root_prims()
	
	for prim in root_prims:
		print(prim.get_type_name())
