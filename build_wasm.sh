#!/bin/bash

# Source Emscripten environment
source /tmp/emsdk/emsdk_env.sh

# Create build directory
mkdir -p build_wasm
cd build_wasm

# Build with CMake and Emscripten
emcmake cmake ..
emmake make

cd ..

echo "WASM build complete! Output files in build_wasm/"
