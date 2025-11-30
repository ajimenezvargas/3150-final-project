#!/bin/bash
set -e

echo "🔨 Building BGP Simulator for Cloudflare Pages..."

# The dist/ folder is the output directory - nothing else to do!
# index.html is already in dist/ with embedded CSS and JavaScript

if [ -f "dist/index.html" ]; then
    echo "✅ index.html found"
else
    echo "❌ dist/index.html not found"
    exit 1
fi

echo "✅ Build complete! Files ready for Cloudflare Pages"
ls -lh dist/
exit 0
