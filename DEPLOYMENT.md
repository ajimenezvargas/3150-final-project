# Deployment Guide: BGP Simulator on Cloudflare Pages

This guide walks you through deploying the BGP Simulator to Cloudflare Pages with automatic deployments from GitHub.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Step 1: Set Up GitHub Repository](#step-1-set-up-github-repository)
3. [Step 2: Create Cloudflare Account](#step-2-create-cloudflare-account)
4. [Step 3: Configure Cloudflare Pages](#step-3-configure-cloudflare-pages)
5. [Step 4: Custom Domain Setup](#step-4-custom-domain-setup)
6. [Step 5: Test Your Deployment](#step-5-test-your-deployment)
7. [Troubleshooting](#troubleshooting)

## Prerequisites

You'll need:
- GitHub account
- Cloudflare account (free tier works)
- Domain name (~$10/year from any registrar)
- ~30 minutes for setup

## Step 1: Set Up GitHub Repository

### 1.1 Create a GitHub Repository

1. Go to [GitHub](https://github.com/new)
2. Create a new repository named `bgp-simulator`
3. Choose "Public" (for free HTTPS with Cloudflare Pages)
4. Don't initialize with README (we have one)
5. Click "Create repository"

### 1.2 Push Your Code

```bash
cd BGPSimulator

# Initialize git if not already done
git init
git add .
git commit -m "Initial commit: BGP Simulator with WASM support"

# Add remote
git remote add origin https://github.com/YOUR_USERNAME/bgp-simulator.git
git branch -M main
git push -u origin main
```

### 1.3 Verify Repository Contents

Your repository should contain:

```
✓ .gitignore
✓ README.md
✓ DEPLOYMENT.md (this file)
✓ CMakeLists.txt
✓ Makefile
✓ build_wasm.sh
✓ wrangler.toml
✓ include/ (header files)
✓ src/ (source files)
✓ dist/ (web files)
│   ├── index.html
│   ├── styles.css
│   ├── app.js
│   └── (bgp_simulator_wasm.wasm & .js will be generated)
└── test_data/
```

## Step 2: Create Cloudflare Account

### 2.1 Sign Up

1. Go to [Cloudflare Dashboard](https://dash.cloudflare.com/sign-up)
2. Sign up with email and password
3. Verify your email

### 2.2 Purchase a Domain (Optional)

If you don't have a domain:

1. In Cloudflare Dashboard, click **Domain Registration**
2. Search for your desired domain name
3. Add to cart (~$10/year)
4. Complete checkout

**OR** register at another provider (GoDaddy, Namecheap, etc.) and point to Cloudflare nameservers later.

## Step 3: Configure Cloudflare Pages

### 3.1 Connect GitHub to Cloudflare

1. In Cloudflare Dashboard, click **Workers & Pages** → **Pages**
2. Click **Connect to Git**
3. Click **GitHub**
4. A popup opens - authorize Cloudflare on GitHub
   - Click "Authorize Cloudflare"
   - Follow the GitHub authentication flow
   - Give Cloudflare access to your repositories

### 3.2 Select Repository

1. After authorization, select your GitHub account
2. Search for `bgp-simulator` repository
3. Click to select it
4. Click **Begin setup**

### 3.3 Configure Build Settings

You'll see a build configuration form. Fill it out:

**Project name**: `bgp-simulator`

**Production branch**: `main`

**Framework preset**: `None` (since it's a static site with WASM)

**Build command**:
```bash
bash build_wasm.sh && mkdir -p dist && ls -la dist/
```

**Build output directory**: `dist`

**Root directory** (advanced): `/`

**Environment variables** (advanced):
- Add: `EMSDK_QUIET=1` (to reduce build output)

### 3.4 Deploy

Click **Save and Deploy**

Cloudflare will:
1. Clone your repository
2. Run the build command
3. Compile the C++ to WASM
4. Deploy the `dist/` folder to Pages
5. Give you a `.pages.dev` URL

**⏱️ First build takes 3-5 minutes** (WASM compilation is slow)

### 3.5 Monitor Deployment

You can monitor the build in the **Deployments** tab:
- Pending → Building → Deployed ✓
- Click on a deployment to see logs
- Scroll to see detailed build output

## Step 4: Custom Domain Setup

### Option A: If you registered domain with Cloudflare

1. In Pages project, click **Custom domains**
2. Click **Set up a domain**
3. Follow the wizard to complete setup
4. Takes 5-10 minutes to propagate

### Option B: If domain is registered elsewhere

#### Step 1: Get Cloudflare Nameservers

1. In Pages project, click **Custom domains**
2. Click **Set up a domain**
3. Cloudflare shows you 2 nameservers (e.g., `irene.ns.cloudflare.com`)
4. Copy these

#### Step 2: Update Registrar

Go to your domain registrar (GoDaddy, Namecheap, etc.):

1. Find "Nameservers" or "DNS" settings
2. Remove old nameservers
3. Add Cloudflare nameservers:
   - `irene.ns.cloudflare.com`
   - `nathan.ns.cloudflare.com`
   (The actual names vary - use Cloudflare's)

#### Step 3: Verify in Cloudflare

Back in Cloudflare:
1. Click **Check nameservers**
2. Wait 5-10 minutes for propagation
3. Status changes to "Active" ✓

### 3.4 HTTPS Certificate

Cloudflare automatically:
- Issues free SSL/TLS certificate
- Redirects HTTP → HTTPS
- Enables automatic HTTPS

No additional setup needed!

## Step 5: Test Your Deployment

### 5.1 Visit Your Site

Open your browser and go to:
- `https://bgp-simulator.pages.dev` (automatic URL)
- OR `https://yourdomain.com` (after custom domain setup)

### 5.2 Verify WASM Module

1. Open **Developer Console** (F12)
2. Look for: `"WASM module initialized"`
3. Check **Network** tab - confirm `.wasm` file loaded
4. Test upload a file and run simulation

### 5.3 Check Performance

1. Go to **Performance** tab in DevTools
2. First Load: ~2-3 seconds
3. Memory: Efficient due to WASM
4. Network: .wasm file should be cached

## Continuous Deployment

Once set up, deployment is **automatic**:

```bash
# Make a change locally
echo "# Updated" >> README.md

# Push to GitHub
git add README.md
git commit -m "Update readme"
git push origin main
```

Cloudflare automatically:
1. Detects the push
2. Builds the project (3-5 min for first build, ~1 min after)
3. Deploys to production
4. You can monitor in Pages → Deployments

## Troubleshooting

### Build Fails: "emcmake not found"

**Problem**: Build fails with `emcmake: command not found`

**Solution**: Ensure `build_wasm.sh` properly sources emsdk:

```bash
# In build_wasm.sh, add:
source /tmp/emsdk/emsdk_env.sh
```

### WASM File Not Loading

**Problem**: Console shows `Failed to instantiate module`

**Solution**:
1. Check if `.wasm` file exists in `dist/`
2. Verify MIME type is `application/wasm`
3. Cloudflare Pages sets this automatically
4. If issue persists, check build logs in Deployments tab

### Slow First Load

**Problem**: Website takes 10+ seconds to load

**Solution**:
- WASM module is large (~177 KB)
- First load includes module parsing
- Second visit uses browser cache
- For large CAIDA files (>100MB), parsing takes time on first run

### Custom Domain Not Working

**Problem**: Domain shows Cloudflare error page

**Solution**:
1. Verify nameservers are correct: `dig yourdomain.com`
2. Check DNS records in Cloudflare dashboard
3. Wait 24-48 hours for full propagation
4. Clear browser cache: Ctrl+Shift+Del

### Preview Deployment URL Doesn't Work

**Problem**: `branch.bgp-simulator.pages.dev` shows error

**Solution**:
1. Make sure branch name is valid (no spaces/special chars)
2. Check build logs in Deployments
3. WASM module must be built for preview to work

## Advanced: Environment-Specific Builds

To use different CAIDA datasets per environment:

1. Add environment variable in Pages:
   - `ENVIRONMENT=production`
2. Update CMakeLists.txt to use the variable
3. Rebuild

## Monitoring & Analytics

Cloudflare provides free analytics:

1. In Pages project, click **Analytics**
2. See:
   - Requests/day
   - Unique visitors
   - Top pages
   - Errors

## Database Integration (Optional)

To log simulations to a database:

1. Set up Cloudflare Workers KV or D1
2. Create a Cloudflare Worker API endpoint
3. Modify `app.js` to POST results

(Out of scope for this guide)

## Disaster Recovery

Your site is backed up by:
- GitHub repository (source code)
- Cloudflare (deployments)

To redeploy if needed:

```bash
# Push to GitHub again
git push origin main

# Cloudflare auto-redeploys
```

## Cost Breakdown

- **Domain**: $10-15/year
- **Cloudflare**: FREE (including HTTPS, CDN, analytics)
- **GitHub**: FREE
- **Total**: ~$10-15/year for a professional website

## Next Steps

1. ✅ Deploy to Cloudflare Pages
2. Share the URL: `https://yourdomain.com`
3. Test with sample CAIDA data
4. Collect feedback from users
5. Monitor performance and errors

## Support

For issues:
- Check [Cloudflare Pages Docs](https://developers.cloudflare.com/pages/)
- Review build logs in Deployments tab
- Check GitHub Issues on your repo
- Emscripten Documentation

---

**Congratulations!** Your BGP Simulator is now live on the internet! 🎉

Share the link with colleagues, professors, and the networking community.

For questions or improvements, open a GitHub Issue or Pull Request.
