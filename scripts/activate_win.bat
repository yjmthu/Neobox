@echo off
rem Windows activation script for the Neobox workspace.
rem
rem The "build" feature activates MSVC and the Windows SDK through
rem vs2022_win-64, so this script only has to expose the Qt6 that ships inside
rem this pixi environment. The conda layout keeps it under Library.
set "CMAKE_PREFIX_PATH=%CONDA_PREFIX%\Library"
