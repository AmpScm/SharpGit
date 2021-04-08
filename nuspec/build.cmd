@echo off
setlocal enableextensions

REM Do this as separate if to avoid using stale variables later on
if "%1" == "-gh" (
  call "%0\..\..\scripts\gh.cache.bat" || exit /B 1
)

if "%1" == "-gh" (
  SET SHARPGIT_VER=%SHARPGIT_MAJOR%.%SHARPGIT_MINOR%.%SHARPGIT_PATCH%
) else if "%1" == "" (
  echo "%0 <sharpgit-ver>"
  exit /b 1
) else (
  SET SHARPGIT_VER=%2

  pushd ..\src
  msbuild /m /p:Configuration=Release /p:Platform=x86 /p:ForceAssemblyVersion=%SharpGit_VER%  /v:m /nologo || exit /B 1
  msbuild /m /p:Configuration=Release /p:Platform=x64 /p:ForceAssemblyVersion=%SharpGit_VER% /v:m /nologo || exit /B 1
  popd
)

echo Packaging using version %SHARPGIT_VER%
pushd %0\..
if NOT EXIST ".\obj\." mkdir obj
if NOT EXIST ".\bin\." mkdir bin

REM CALL :xmlpoke SharpGit.Core.nuspec //nu:metadata/nu:version %SharpGit_VER% || EXIT /B 1

nuget pack SharpGit.nuspec -version %SharpGit_VER% -OutputDirectory bin || exit /B 1
echo "--done--"
popd
goto :eof

:xmlpoke
msbuild /nologo /v:m xmlpoke.build "/p:File=%1" "/p:XPath=%2" "/p:Value=%3" || exit /B 1
exit /B 0
