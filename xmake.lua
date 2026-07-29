set_xmakever("3.0.7")

local PROJECT_NAME = "CRDW"
local PROJECT_VERSION = "1.1.1b"

set_project(PROJECT_NAME)
set_version(PROJECT_VERSION)

option("version")
    set_default("AE")
    set_values("AE", "SE", "VR")
    set_showmenu(true)
    set_description("Controls build version")
option_end()

set_languages("c++23")
set_arch("x64")
set_encodings("utf-8")

includes("extern/CommonLibSSE")

add_rules("mode.release", "mode.releasedbg", "mode.debug")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

set_policy("build.optimization.lto", true)
set_runtimes(is_mode("debug") and "MTd" or "MT")

target(PROJECT_NAME)
    set_kind("shared")

    if is_config("version", "AE") then
        add_defines("SKYRIM_SUPPORT_AE")
        PROJECT_VERSION = "AE " .. PROJECT_VERSION
        PROJECT_NAME = PROJECT_NAME .. "_AE"
    elseif is_config("version", "SE") then
        PROJECT_VERSION = "SE " .. PROJECT_VERSION
        PROJECT_NAME = PROJECT_NAME .. "_SE"
    elseif is_config("version", "VR") then
        PROJECT_VERSION = "VR " .. PROJECT_VERSION
        PROJECT_NAME = PROJECT_NAME .. "_VR"
    end

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