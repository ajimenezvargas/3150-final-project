#!/bin/bash
set -e

echo "🔨 Building BGP Simulator for Cloudflare Pages..."

# Since WASM is pre-built and committed to git, we just need to verify it exists
if [ -f "dist/bgp_simulator_wasm.wasm" ] && [ -f "dist/bgp_simulator_wasm.js" ]; then
    echo "✅ WASM module found"
    echo "✅ dist/ folder ready for deployment"
    ls -lh dist/bgp_simulator_wasm.wasm
    ls -lh dist/bgp_simulator_wasm.js
    exit 0
else
    echo "❌ WASM files not found in dist/"
    echo "Please build locally first:"
    echo "  source /tmp/emsdk/emsdk_env.sh"
    echo "  cd build_wasm && emcmake cmake .. && emmake make"
    exit 1
fi
