set_xmakever("3.0.7")

local PROJECT_NAME = "CRDW"
local PROJECT_VERSION = "1.1.4"
local PROJECT_STAGE = ""

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

    local build = "AE "
    if is_config("version", "AE") then
        add_defines("SKYRIM_SUPPORT_AE=1")
    elseif is_config("version", "SE") then
        build = "SE "
    elseif is_config("version", "VR") then
        add_defines("SKYRIM_SUPPORT_VR=1")
        build = "VR "
    end

    add_defines("PROJECT_NAME=\"" .. PROJECT_NAME .. PROJECT_STAGE .. "\"")
    add_defines("PROJECT_VERSION=\"" .. build .. PROJECT_VERSION .. "\"")

    add_deps("commonlibsse")

    add_rules("commonlibsse.plugin", {
        name = PROJECT_NAME .. PROJECT_STAGE,
        author = "Rixu",
        version = PROJECT_VERSION,
        description = "Caches the Recrusive Directory Walk"
    })

    add_cxflags("cl::/GF", "cl::/Gy")
    add_ldflags("cl::/OPT:ICF")

    add_files("source/**.cpp")
    add_includedirs("include", "source")

    set_filename(PROJECT_NAME .. ".dll")

after_build(function (target)
        local build_dir = target:targetdir()
        local dll_path = target:targetfile()
        local pdb_path = target:symbolfile()
        local ini_path = path.join(os.projectdir(), "CRDW.ini")
        
        local staging_dir = path.join(build_dir, "staging")
        local plugin_dir = path.join(staging_dir, "Data/SKSE/Plugins")

        local build = " AE"
        if is_config("version", "SE") then
            build = " SE"
        elseif is_config("version", "VR") then
            build = " VR"
        end

        os.tryrm(staging_dir)
        os.mkdir(plugin_dir)

        os.cp(dll_path, plugin_dir)

        if os.isfile(pdb_path) then
            os.cp(pdb_path, plugin_dir)
        end

        if os.isfile(ini_path) then
            os.cp(ini_path, plugin_dir)
        end

        local archive_file = path.absolute(path.join(build_dir, target:name() .. build .. ".7z"))
        
        os.tryrm(archive_file)
    
        local exit_code = os.execv("7z", {"a", archive_file, "Data"}, {curdir = staging_dir})

        os.tryrm(staging_dir)
    end)