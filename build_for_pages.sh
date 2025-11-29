#!/bin/bash
set -e

echo "🔨 Building BGP Simulator for Cloudflare Pages..."

# Create output directory
mkdir -p dist

# Check if WASM files exist in root
if [ -f "bgp_simulator_wasm.wasm" ] && [ -f "bgp_simulator_wasm.js" ]; then
    echo "✅ WASM module found"
    
    # Copy all necessary files to dist/
    cp bgp_simulator_wasm.wasm dist/
    cp bgp_simulator_wasm.js dist/
    cp index.html dist/
    
    # Copy any other assets (CSS, JS, etc.)
    if [ -f "style.css" ]; then
        cp style.css dist/
    fi
    
    if [ -d "assets" ]; then
        cp -r assets dist/
    fi
    
    echo "✅ Files copied to dist/"
    ls -lh dist/
    exit 0
else
    echo "❌ WASM files not found in root"
    echo "Please build locally first:"
    echo "  source /tmp/emsdk/emsdk_env.sh"
    echo "  cd build_wasm && emcmake cmake .. && emmake make"
    exit 1
fi
