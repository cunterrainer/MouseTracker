project "ImGui"
    kind "StaticLib"
    language "C++"

    files {
        "src/**.cpp",
        "include/ImGui/**.h"
    }

    includedirs {
        "include/ImGui",
        "../glfw/include"
    }