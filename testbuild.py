from ast import arg
import os, shutil
import subprocess as sp
from termcolor import colored
from sys import argv, platform


def cls():
    os.system('cls' if os.name=='nt' else 'clear')


def RemoveFolder(path):
    if os.path.exists(path) and os.path.isdir(path):
        shutil.rmtree(path)


def ExecProcess(args):
    print("Process: ", end="")
    for arg in args:
        print(arg, end=" ")
    print()
    process = sp.Popen(args, stdout=sp.PIPE, stderr=sp.PIPE, universal_newlines=True)
    stdout, stderr = process.communicate()
    if process.returncode == 0:
        print(colored(f"Stdout:\n{stdout}\nStderr:\n{stderr}\nReturn code: {process.returncode}", "green"), end="\n\n")
    else:
        print(colored(f"Stdout:\n{stdout}\nStderr:\n{stderr}\nReturn code: {process.returncode}", "red"), end="\n\n")
    return process.returncode == 0


def ExecProcessList(argsList):
    for arg in argsList:
        if not ExecProcess(arg):
            return False
    return True


def main():
    useGcc = False
    useClang = False
    if len(argv) == 1:
        useGcc = True
        useClang = True
    else:
        for arg in argv:
            if arg.lower() == "gcc":
                useGcc = True
            elif arg.lower() == "clang":
                useClang = True

    if platform == "win32":
        exePath = "vendor/premake5.exe"
    elif platform == "linux":
        exePath = "vendor/premake5"
    else:
        print("Unsupported operating system: " + platform)
        return

    cls()
    binDir = "BIN/"
    gccProc = [[exePath, "gmake", "--cc=gcc"], ["make", "-j", "config=debug_x64"], ["make", "-j", "config=release_x64"]]
    clangProc = [[exePath, "gmake", "--cc=clang"],
                 ["make", "-j", "config=debug_x64"], ["make", "-j", "config=release_x64"],
                 ["make", "-j", "config=debug_x86"], ["make", "-j", "config=release_x86"]]

    if useGcc:
        RemoveFolder(binDir)
        if not ExecProcessList(gccProc):
            return
    if useClang:
        RemoveFolder(binDir)
        ExecProcessList(clangProc)


if __name__=="__main__":
    main()