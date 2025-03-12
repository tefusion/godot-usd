#!/usr/bin/env python
import os
from glob import glob
from pathlib import Path
from SCons.Script import *

env = SConscript("extern/godot-cpp/SConstruct")
env.Append(CXXFLAGS=['-fexceptions'])
env.Append(CFLAGS=['-fexceptions'])

env.Tool('compilation_db')

cpp_defines = []
if "CPPDEFINES" in env:
    cpp_defines = env["CPPDEFINES"]
cpp_defines.append("_USE_MATH_DEFINES")
env.Append(CPPDEFINES=cpp_defines)

env.Append(CPPPATH=["src/", "extern/tinyusdz/src", "extern/tinyusdz/src/external", "extern/tinyusdz/src/lz4", "extern/OpenSubdiv"])

patterns = ["extern/tinyusdz/src/*.c", "extern/tinyusdz/src/*.cc", "extern/tinyusdz/src/*.cpp"]
exclude_patterns = ["python", "nanobind"]

# Collect all files matching the patterns
tinyusdz_sources = []
for pattern in patterns:
    tinyusdz_sources.extend(glob(pattern))

# Filter out files that match any of the exclude patterns
tinyusdz_sources = [f for f in tinyusdz_sources if not any(exclude in os.path.basename(f) for exclude in exclude_patterns)]

# add lz4
tinyusdz_sources += Glob("extern/tinyusdz/src/lz4/*.c")

tinyusdz_sources += Glob("extern/tinyusdz/src/tydra/*.cc")

extern_direct = [
    "miniz.c"
]
for lib in extern_direct:
    lib_path = f"extern/tinyusdz/src/external/{lib}"
    full_path = os.path.abspath(lib_path)
    tinyusdz_sources.append(full_path)

# Add tinyusdz source files
#tinyusdz_sources = Glob("extern/tinyusdz/src/*.c") + Glob("extern/tinyusdz/src/*.cc") + Glob("extern/tinyusdz/src/*.cpp")

# List of external libraries to include with their include paths
extern_libs = [
    ("alac", "codec"),
    ("fast_float", "include"),
    ("filesystem", "include"),
    ("floaxie", "floaxie"),
    ("glob", "include"),
    ("jsonhpp", "nlohmann"),
    ("jsteemann", ""),
    ("mapbox", "earcut"),
    ("OpenFBX", "src"),
    ("simple_match", "include"),
    ("string_id", ""),
]

# Add external library source files and include paths
for lib, include_subpath in extern_libs:
    lib_path = f"extern/tinyusdz/src/external/{lib}"
    include_path = f"{lib_path}/{include_subpath}" if include_subpath else lib_path
    tinyusdz_sources += Glob(f"{lib_path}/**/*.c")
    tinyusdz_sources += Glob(f"{lib_path}/**/*.cc")
    tinyusdz_sources += Glob(f"{lib_path}/**/*.cpp")
    env.Append(CPPPATH=[include_path])

# Add OpenSubdiv source files
opensubdiv_dir = "extern/OpenSubdiv/opensubdiv"
opensubdiv_sources = [
    "far/error.cpp",
    "far/topologyDescriptor.cpp",
    "far/topologyRefiner.cpp",
    "far/topologyRefinerFactory.cpp",
    "sdc/crease.cpp",
    "sdc/typeTraits.cpp",
    "vtr/fvarLevel.cpp",
    "vtr/fvarRefinement.cpp",
    "vtr/level.cpp",
    "vtr/quadRefinement.cpp",
    "vtr/refinement.cpp",
    "vtr/sparseSelector.cpp",
    "vtr/triRefinement.cpp",
]
opensubdiv_sources = [f"{opensubdiv_dir}/{file}" for file in opensubdiv_sources]

# Combine the sources
sources = Glob("src/**/*.cpp") + Glob("src/*.cpp") + tinyusdz_sources + opensubdiv_sources

addon_path = "project/addons/godot_usd"
extension_name = "godot_usd"

# Handle the build suffix to remove .dev. if present
build_suffix = env["suffix"].replace(".dev.", ".")

# Create the library target (e.g. libgodot_usd.linux.template_debug.x86_64.so)
if env["platform"] == "macos":
    library = env.SharedLibrary(
        "{0}/bin/lib{1}.{2}.{3}.framework/{1}.{2}.{3}".format(
            addon_path,
            extension_name,
            env["platform"],
            build_suffix,
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "{}/bin/lib{}{}{}".format(
            addon_path,
            extension_name,
            build_suffix,
            env["SHLIBSUFFIX"],
        ),
        source=sources,
    )

# Enable cache if provided (see build.yml)
scons_cache_path = os.environ.get("SCONS_CACHE")
if scons_cache_path is not None:
    CacheDir(scons_cache_path)
    print("SCons cache is enabled. Cache path: '" + scons_cache_path + "'")

env["COMPILATIONDB_USE_ABSPATH"] = True
compile_commands = env.CompilationDatabase()
Alias('compdb', compile_commands)

Default(library)
