![Version](https://img.shields.io/badge/Godot-v4.4-informational) ![License](https://img.shields.io/badge/License-Apache%202.0-red.svg)

# Godot USD

OpenUSD Importer for Godot built using [tinyusdz](https://github.com/lighttransport/tinyusdz). Currently this just supports basic godot scene import from blender exported USD files.

## Testing

Tests are done inside gdscript using [gdUnit4](https://github.com/MikeSchulze/gdUnit4). See [project/test](project/test) directory.

## Supported OpenUSD Schemas

| Schema Name | Header File |
|----------|--------------|
| [UsdGeom](https://openusd.org/dev/api/usd_geom_page_front.html)  | [usd_geom.h](src/usd/usd_geom.h) |
| [UsdShade](https://openusd.org/dev/api/usd_shade_page_front.html) (UsdPreviewSurfaces through tydra) | [usd_shade.h](src/usd/usd_shade.h) |
| [UsdSkel](https://openusd.org/dev/api/usd_skel_page_front.html) (Conversion WIP)  | [usd_skel.h](src/usd/usd_skel.h) |

## Usage

This plugin implements a [EditorSceneFormatImporter](https://docs.godotengine.org/en/stable/classes/class_editorsceneformatimporter.html) for .usda files. So you once enabled you can import .usda files as godot scenes.

Since my main goal actually wasn't an importer, but rather getting richer scene data like topology from blender to godot this also exposes all APIs to gdscript.

A minimal example is:

```gdscript
var stage := UsdStage.new()
stage.load("res://usdstage.usda")

#you can use the usdview utility from openusd to copy prim paths
var prim = stage.get_prim_at_path(UsdPath.from_string("/some/path"))

#for getting root prims
var root_prims = stage.get_root_prims()

#getting the value of a xform prim. The value is just another wrapper around the prim to get typing. It converts values at call time
assert(prim.get_type() == UsdPrimType.USD_PRIM_TYPE_XFORM)
var prim_value: UsdGeomXform = prim.get_value()
```

## License

This project is licensed under the Apache 2.0 License [LICENSE](LICENSE).
Thirdparty notice is in [THIRDPARTY](THIRDPARTY.md) as well as [THIRDPARTY-tinyusdz](THIRDPARTY-tinyusdz.txt) for a list of thirdparty libraries used by tinyusdz.
