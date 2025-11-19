# Quick Start Guide

Get your BGP Simulator running in 5 minutes!

## Option 1: Run Locally (Development)

### 1. Build WASM

```bash
cd BGPSimulator
source /tmp/emsdk/emsdk_env.sh
cd build_wasm
emcmake cmake ..
emmake make
cd ..
```

### 2. Start Web Server

```bash
cd dist
python3 -m http.server 8000
```

### 3. Open Browser

Visit: `http://localhost:8000`

## Option 2: Deploy to Cloudflare Pages (5 minutes)

### 1. Push to GitHub

```bash
git add .
git commit -m "Deploy to Cloudflare Pages"
git push origin main
```

### 2. Connect to Cloudflare Pages

1. Go to [Cloudflare Dashboard](https://dash.cloudflare.com)
2. **Workers & Pages** → **Pages** → **Connect to Git**
3. Select your `bgp-simulator` repository
4. Build settings:
   - Build command: `bash build_wasm.sh && mkdir -p dist`
   - Output directory: `dist`
5. Click **Save and Deploy**

### 3. Get Your URL

- Auto: `https://bgp-simulator.pages.dev`
- Custom: Add your domain in Pages → Custom Domains

## Using the Simulator

### Upload Files

1. **CAIDA Data** (required): AS relationship file
2. **Announcements** (required): CSV with ASN, prefix, ROV flag
3. **ROV ASNs** (optional): ASNs with Route Origin Validation

### Run Simulation

1. Enter target ASN (e.g., `15169` for Google)
2. Click "Run Simulation"
3. View routes for that ASN

### Export Results

Click "Download CSV" to export all routing tables

## Sample Files

Try with test data:

```bash
# Already in test_data/ directory:
cat test_data/announcements.csv
cat test_data/rov_asns.csv

# And use:
data/as-rel.txt
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| WASM not loading | Check developer console (F12), verify file in dist/ |
| Build fails locally | Run `source /tmp/emsdk/emsdk_env.sh` first |
| Cloudflare build fails | Check build logs in Deployments tab |
| Slow to load large files | Large CAIDA files take time to parse (normal) |

## Next Steps

- Read [DEPLOYMENT.md](DEPLOYMENT.md) for custom domain setup
- Check [README.md](README.md) for full documentation
- Review [file format](README.md#usage) requirements

---

**Live Site**: Soon! Just push to GitHub and wait 5 minutes 🚀
