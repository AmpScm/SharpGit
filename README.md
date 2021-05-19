# SharpGit - Wrapping libgit2 within a clean .Net wrapper for .Net 4+ and .Net Core

[![latest version](https://img.shields.io/nuget/v/SharpGit)](https://www.nuget.org/packages/SharpGit)

Use the library libgit2, implemented the .Net way, reusable and available as simple to use package on NuGet

https://www.nuget.org/packages/SharpGit/

Setup build environment using:

    mkdir dev
    cd dev
    git clone https://github.com/Microsoft/vcpkg.git
    git clone https://github.com/AmpScm/SharpGit.git
    cd vcpkg
    bootstrap-vcpkg.bat
    vcpkg install libgit2:x86-windows-static-md libgit2:x64-windows-static-md apr-util:x86-windows-static-md apr-util:x64-windows-static-md
    cd ..
