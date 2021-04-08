@echo off
SETLOCAL ENABLEEXTENSIONS

SET CACHE=%0\..\gh.cache.bat
SET RSPFILE=%0\..\msbuild-version.rsp

if NOT EXIST "%1" (
  echo "Usage: %0 <proj.h> <buildnr>"
  goto :eof
)

echo @echo off > %CACHE%

for /F "usebackq tokens=2,3" %%i in (`"type %1 |findstr /C:_VER_"`) do (
  SET %%i=%%j
  echo SET %%i=%%j>> %CACHE%
)

set    SHARPGIT_MAJOR=%LIBGIT2_VER_MAJOR%
set /a SHARPGIT_MINOR=%LIBGIT2_VER_MINOR% * 1000 + %LIBGIT2_VER_REVISION% * 10 + %LIBGIT2_VER_PATCH%
set    SHARPGIT_PATCH=%2

echo Found LIBGIT2 %LIBGIT2_VER_MAJOR%.%LIBGIT2_VER_MINOR%.%LIBGIT2_VER_REVISION%.%LIBGIT2_VER_PATCH% from header
echo Prepare building SharpGit %SHARPGIT_MAJOR%.%SHARPGIT_MINOR%.%SHARPGIT_PATCH%

(
  echo SET SHARPGIT_MAJOR=%SHARPGIT_MAJOR%
  echo SET SHARPGIT_MINOR=%SHARPGIT_MINOR%
  echo SET SHARPGIT_PATCH=%SHARPGIT_PATCH%
) >> %CACHE%

(
  echo /p:ForceAssemblyVersion=%SHARPGIT_MAJOR%.%SHARPGIT_MINOR%.%SHARPGIT_PATCH%
  echo /p:ForceAssemblyCompany="SharpGit Project, powered by AmpScm, CollabNet, QQn & GitHub"
  echo /p:ForceAssemblyCopyright="Apache 2.0 licensed. See https://github.com/ampscm/SharpGit"
  echo /p:BuildBotBuild=true
  echo /p:RestoreForce=true
) >> %RSPFILE%
