extends Node

func process_prim(prim: UsdPrim):
	if prim.get_type() == UsdPrimType.USD_PRIM_TYPE_UNKNOWN:
		print("Unknown type ", prim.get_type_name())
	elif prim.get_type() == UsdPrimType.USD_PRIM_TYPE_XFORM:
		var value: UsdPrimValueXform = prim.get_value()
		print(value)
	else:
		print("Known type ", prim.get_type(), " ", prim.get_type_name())
	
	for p in prim.get_children():
		process_prim(p)

func _ready() -> void:
	var stage := UsdStage.new()
	stage.load("res://2meshes.usda")
	var root_prims:=stage.get_root_prims()
	
	for prim in root_prims:
		process_prim(prim)
		
