#!/usr/bin/env python
import os
from glob import glob
from pathlib import Path
from SCons.Script import *

env = SConscript("extern/godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

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

Default(library)
