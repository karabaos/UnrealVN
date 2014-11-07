@echo off
setlocal
pushd %~dp0

rem Sync the dependencies...
.\Engine\Binaries\DotNET\GitDependencies.exe %*
if ERRORLEVEL 1 goto error

rem Setup the git hooks
if not exist .git\hooks goto no_git_hooks_directory
echo Registering git hooks...
echo #!/bin/sh >.git\hooks\post-checkout
echo Engine/Binaries/DotNET/GitDependencies.exe >>.git\hooks\post-checkout
echo #!/bin/sh >.git\hooks\post-merge
echo Engine/Binaries/DotNET/GitDependencies.exe >>.git\hooks\post-merge
:no_git_hooks_directory

rem Register the engine installation...
if not exist .\Engine\Binaries\Win64\UnrealVersionSelector-Win64-Shipping.exe goto :no_unreal_version_selector
.\Engine\Binaries\Win64\UnrealVersionSelector-Win64-Shipping.exe /quiet
:no_unreal_version_selector

rem Done!
goto :EOF

rem Error happened. Wait for a keypress before quitting.
:error
pause
