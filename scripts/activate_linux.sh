#!/usr/bin/env bash
# Linux activation script for the Neobox workspace.
#
# The "build" feature activates gcc/g++ and the matching sysroot on its own,
# so this script only has to expose the Qt6 that ships inside this pixi
# environment.
export CMAKE_PREFIX_PATH="${CONDA_PREFIX}${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"
