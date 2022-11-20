#pragma once
/*
    For some reason clang preferably links against main() if available even though the build
    settings would link against WinMain().
    However in order to link against WinMain() I created this macro.
    GCC & MSVC properly link against WinMain().
    Premake will define all these macros for you if you select clang as compiler.
*/
#if defined CLANG && defined RELEASE && defined WINDOWS
#ifdef _WINBASE_
#define main() WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
#define main() APIENTRY WinMain(void*, void*, char*, int)
#endif
#endif