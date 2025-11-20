#!/bin/bash
set -e

echo "🔨 Building BGP Simulator for Cloudflare Pages..."

# Check if WASM files exist in root
if [ -f "bgp_simulator_wasm.wasm" ] && [ -f "bgp_simulator_wasm.js" ]; then
    echo "✅ WASM module found"
    echo "✅ App files ready for deployment"
    ls -lh bgp_simulator_wasm.wasm
    ls -lh bgp_simulator_wasm.js
    exit 0
else
    echo "❌ WASM files not found in root"
    echo "Please build locally first:"
    echo "  source /tmp/emsdk/emsdk_env.sh"
    echo "  cd build_wasm && emcmake cmake .. && emmake make"
    exit 1
fi
