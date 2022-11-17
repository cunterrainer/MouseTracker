workspace "MouseTracker"
    platforms { "x64", "x86" }
    configurations {
        "Debug",
        "Release"
    }
    startproject "MouseTracker"

outputdir = "/BIN/%{cfg.buildcfg}/%{cfg.architecture}/"
-- get current working directory
cwd = os.getcwd()

targetdir(cwd .. outputdir .. "%{prj.name}/bin")
objdir(cwd .. outputdir .. "%{prj.name}/bin-int")

filter { "platforms:x64" }
    architecture "x64"
    defines "X64"
filter { "platforms:x86" }
    architecture "x86"
    defines "X86"

filter "system:windows"
    defines "WINDOWS"

filter { "configurations:Debug" }
    runtime "Debug"
    symbols "on"
    optimize "off"
    defines "DEBUG"
filter { "configurations:Release" }
    runtime "Release"
    symbols "off"
    optimize "Speed"
    defines "RELEASE"
filter {}

-- only for visual studio
flags {
    "MultiProcessorCompile"
}
staticruntime "on"
removeunreferencedcodedata "on"
defines "USING_IMGUI"

include "MouseTracker"
include "Dependencies/glfw"
include "Dependencies/imgui"
include "Dependencies/nativefiledialog"