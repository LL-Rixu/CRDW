set_xmakever("3.0.7")

-- Project details
set_project("CRDW")
set_version("1.0.0")

add_defines("SKYRIM_SUPPORT_AE")

-- CommonLibNG requires C++23 and a 64-bit target
set_languages("c++23")
set_arch("x64")
set_encodings("utf-8")

add_rules("mode.release", "mode.releasedbg")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

set_policy("build.optimization.lto", true)
set_runtimes(is_mode("debug") and "MTd" or "MT")

-- 1. Include CommonLibSSE-NG's own xmake script from your extern folder
includes("extern/CommonLibSSE")
set_optimize("fastest")

target("CRDW")
    set_kind("shared")

    -- 2. Link against the commonlibsse-ng target provided by the submodule
    add_deps("commonlibsse")

    -- 3. The plugin metadata rule is registered by the included submodule script
    add_rules("commonlibsse.plugin", {
        name = "CRDW",
        author = "Rixu",
        version = "1.0.0",
        description = "Caches the Recrusive Directory Walk"
    })

    add_cxflags("/Zl")
    add_ldflags("/PDBALTPATH:%_PDB%")
    set_strip("all")

    -- Source files
    add_files("source/**.cpp")
    add_includedirs("include")

    set_filename("CRDW.dll")