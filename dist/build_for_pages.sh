#!/bin/bash
set -e

echo "🔨 Building BGP Simulator for Cloudflare Pages..."

# Create output directory
mkdir -p dist

# Copy index.html with embedded CSS and JavaScript (no external files needed)
cp index.html dist/

# Copy WASM files if they exist (for potential future use)
if [ -f "bgp_simulator_wasm.wasm" ] && [ -f "bgp_simulator_wasm.js" ]; then
    echo "✅ WASM module found (optional for future features)"
    cp bgp_simulator_wasm.wasm dist/
    cp bgp_simulator_wasm.js dist/
fi

# Copy any other assets if they exist
if [ -d "assets" ]; then
    cp -r assets dist/
fi

echo "✅ Build complete! Files ready for Cloudflare Pages"
ls -lh dist/
exit 0
