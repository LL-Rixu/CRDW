set_xmakever("3.0.7")

local PROJECT_NAME = "CRDW"
local PROJECT_VERSION = "1.1.0"

set_project(PROJECT_NAME)
set_version(PROJECT_VERSION)

set_languages("c++23")
set_arch("x64")
set_encodings("utf-8")

add_defines("SKYRIM_SUPPORT_AE")
includes("extern/CommonLibSSE")

add_rules("mode.release", "mode.releasedbg", "mode.debug")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

set_policy("build.optimization.lto", true)
set_runtimes(is_mode("debug") and "MTd" or "MT")

target(PROJECT_NAME)
    set_kind("shared")

    add_deps("commonlibsse")

    add_rules("commonlibsse.plugin", {
        name = PROJECT_NAME,
        author = "Rixu",
        version = PROJECT_VERSION,
        description = "Caches the Recrusive Directory Walk"
    })

    add_cxflags("cl::/GF", "cl::/Gy")
    add_ldflags("cl::/OPT:ICF")

    add_files("source/**.cpp")
    add_includedirs("include", "source")

    set_filename(PROJECT_NAME .. ".dll")