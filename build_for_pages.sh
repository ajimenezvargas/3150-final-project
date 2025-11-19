#!/bin/bash
set -e

echo "🔨 Building BGP Simulator for Cloudflare Pages..."

# Check if we already have a built dist/
if [ -f "dist/bgp_simulator_wasm.wasm" ] && [ -f "dist/bgp_simulator_wasm.js" ]; then
    echo "✅ WASM already built, skipping compilation"
    echo "📦 Files ready in dist/"
    exit 0
fi

# Try to use emsdk if available
if ! command -v emcc &> /dev/null; then
    echo "⚠️  Emscripten not found in PATH, installing..."

    # Install emsdk
    git clone https://github.com/emscripten-core/emsdk.git /tmp/emsdk_build
    cd /tmp/emsdk_build
    ./emsdk install latest
    ./emsdk activate latest
    source ./emsdk_env.sh
    cd - > /dev/null
fi

# Source Emscripten environment
if [ -f "/tmp/emsdk_build/emsdk_env.sh" ]; then
    source /tmp/emsdk_build/emsdk_env.sh
elif [ -f "/tmp/emsdk/emsdk_env.sh" ]; then
    source /tmp/emsdk/emsdk_env.sh
elif command -v emcc &> /dev/null; then
    echo "✅ Emscripten found in PATH"
else
    echo "❌ Could not find Emscripten"
    exit 1
fi

# Build WASM
echo "🔨 Compiling C++ to WebAssembly..."
mkdir -p build_wasm
cd build_wasm

emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
emmake make -j4

cd ..

# Verify output
if [ -f "dist/bgp_simulator_wasm.wasm" ]; then
    echo "✅ Build successful!"
    echo "📦 WASM: $(ls -lh dist/bgp_simulator_wasm.wasm | awk '{print $5}')"
    echo "📄 JS: $(ls -lh dist/bgp_simulator_wasm.js | awk '{print $5}')"
else
    echo "❌ Build failed - WASM file not found"
    exit 1
fi
