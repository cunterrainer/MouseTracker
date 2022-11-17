project "nativefiledialog"
    kind "StaticLib"
    language "C"

    files {
        "src/**.h",
        "src/nfd_common.c"
    }

    includedirs {
        "include/nfd"
    }

    filter "system:windows"
        files "src/nfd_win.cpp"

   filter "system:linux"

   filter "system:macosx"
   filter {}