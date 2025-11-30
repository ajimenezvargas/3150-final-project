let simulator = null;
let caida_data = '';
let announcements_data = '';
let rov_data = '';
let lastResults = null;


// Initialize when WASM module loads
Module.onRuntimeInitialized = async function() {
    console.log('WASM module initialized');
    try {
        simulator = new Module.BGPSimulator();
        showStatus('WASM simulator loaded and ready!', 'success');
    } catch (error) {
        showStatus('Error initializing simulator: ' + error.message, 'error');
    }
};

// File input handlers
document.getElementById('caida-file').addEventListener('change', handleCAIDAFile);
document.getElementById('announcements-file').addEventListener('change', handleAnnouncementsFile);
document.getElementById('rov-file').addEventListener('change', handleROVFile);

function handleCAIDAFile(event) {
    const file = event.target.files[0];
    if (!file) return;

    const size = (file.size / 1024 / 1024).toFixed(2);
    const maxSize = 50; // 50MB limit for safety

    // Validate file size
    if (file.size > maxSize * 1024 * 1024) {
        document.getElementById('caida-status').textContent = `Error: File too large (${size}MB, max ${maxSize}MB). Try a smaller dataset.`;
        document.getElementById('caida-status').classList.add('error');
        return;
    }

    const reader = new FileReader();
    reader.onload = function(e) {
        caida_data = e.target.result;
        document.getElementById('caida-status').textContent = `✓ Loaded (${size} MB)`;
        document.getElementById('caida-status').classList.remove('error');
    };
    reader.onerror = function() {
        document.getElementById('caida-status').textContent = 'Error reading file';
        document.getElementById('caida-status').classList.add('error');
    };
    reader.readAsText(file);
}

function handleAnnouncementsFile(event) {
    const file = event.target.files[0];
    if (!file) return;

    const size = (file.size / 1024).toFixed(2);
    const maxSize = 100 * 1024 * 1024; // 100MB limit for safety

    // Validate file size
    if (file.size > maxSize) {
        document.getElementById('announcements-status').textContent = `Error: File too large (${size}KB, max ${(maxSize / 1024).toFixed(0)}KB)`;
        document.getElementById('announcements-status').classList.add('error');
        return;
    }

    const reader = new FileReader();
    reader.onload = function(e) {
        announcements_data = e.target.result;
        document.getElementById('announcements-status').textContent = `✓ Loaded (${size} KB)`;
        document.getElementById('announcements-status').classList.remove('error');
    };
    reader.onerror = function() {
        document.getElementById('announcements-status').textContent = 'Error reading file';
        document.getElementById('announcements-status').classList.add('error');
    };
    reader.readAsText(file);
}

function handleROVFile(event) {
    const file = event.target.files[0];
    if (!file) return;

    const size = (file.size / 1024).toFixed(2);
    const maxSize = 10 * 1024 * 1024; // 10MB limit for safety

    // Validate file size
    if (file.size > maxSize) {
        document.getElementById('rov-status').textContent = `Error: File too large (${size}KB, max ${(maxSize / 1024).toFixed(0)}KB)`;
        document.getElementById('rov-status').classList.add('error');
        return;
    }

    const reader = new FileReader();
    reader.onload = function(e) {
        rov_data = e.target.result;
        document.getElementById('rov-status').textContent = `✓ Loaded (${size} KB)`;
        document.getElementById('rov-status').classList.remove('error');
    };
    reader.onerror = function() {
        document.getElementById('rov-status').textContent = 'Error reading file';
        document.getElementById('rov-status').classList.add('error');
    };
    reader.readAsText(file);
}

async function runSimulation() {
    if (!simulator) {
        showStatus('WASM simulator not initialized', 'error');
        return;
    }

    // Validate inputs
    if (!caida_data) {
        showStatus('Please upload CAIDA AS relationships file', 'error');
        return;
    }

    if (!announcements_data) {
        showStatus('Please upload announcements CSV file', 'error');
        return;
    }

    const targetASN = document.getElementById('target-asn').value;
    if (!targetASN) {
        showStatus('Please enter a target ASN', 'error');
        return;
    }

    // Disable button during simulation
    const runButton = document.getElementById('run-button');
    const originalText = runButton.textContent;
    runButton.disabled = true;
    runButton.innerHTML = '<span class="spinner"></span>Running simulation...';

    try {
        showStatus('Loading CAIDA data...', 'loading');

        // Load data into simulator
        console.log('Loading CAIDA data...');
        const caidaLoaded = simulator.loadCAIDAData(caida_data);
        if (!caidaLoaded) {
            throw new Error('Failed to load CAIDA data');
        }

        showStatus('Loading announcements...', 'loading');
        console.log('Loading announcements...');
        const announcementsLoaded = simulator.loadAnnouncements(announcements_data);
        if (!announcementsLoaded) {
            throw new Error('Failed to load announcements');
        }

        // Load ROV data if provided
        if (rov_data) {
            showStatus('Loading ROV ASNs...', 'loading');
            console.log('Loading ROV ASNs...');
            simulator.loadROVASNs(rov_data);
        }

        // Run simulation
        showStatus('Running simulation...', 'loading');
        console.log('Running simulation...');
        const result = simulator.runSimulation();
        const resultObj = JSON.parse(result);

        if (resultObj.status !== 'success') {
            throw new Error('Simulation failed: ' + resultObj.error);
        }

        // Get routing info for target ASN
        showStatus('Retrieving routing information...', 'loading');
        console.log('Getting routing info for ASN', targetASN);
        const routingInfo = simulator.getRoutingInfo(parseInt(targetASN));
        const routingObj = JSON.parse(routingInfo);

        // Store results for export
        lastResults = simulator.exportRoutingTables();

        // Display results
        displayResults(resultObj, routingObj, parseInt(targetASN));
        showStatus('Simulation completed successfully!', 'success');

    } catch (error) {
        console.error('Simulation error:', error);
        showStatus('Error: ' + error.message, 'error');
    } finally {
        runButton.disabled = false;
        runButton.textContent = originalText;
    }
}

function displayResults(stats, routingInfo, targetASN) {
    // Update stats
    document.getElementById('stat-ases').textContent = stats.total_ases.toLocaleString();
    document.getElementById('stat-routes').textContent = stats.total_routes.toLocaleString();
    document.getElementById('stat-announced').textContent = stats.announcements_seeded;
    document.getElementById('query-asn').textContent = targetASN;

    // Display routing table
    const routesDiv = document.getElementById('routes-table');
    routesDiv.innerHTML = '';

    if (routingInfo.error) {
        routesDiv.innerHTML = `<p style="color: #e74c3c; font-weight: 600;">ASN not found in topology</p>`;
    } else if (routingInfo.routes && routingInfo.routes.length > 0) {
        routingInfo.routes.forEach(route => {
            const row = document.createElement('div');
            row.className = 'route-row';
            row.innerHTML = `<span class="prefix">${route.prefix}</span>`;
            routesDiv.appendChild(row);
        });

        // Add a summary
        const summary = document.createElement('div');
        summary.style.marginTop = '15px';
        summary.style.padding = '15px';
        summary.style.backgroundColor = 'rgba(26, 200, 237, 0.1)';
        summary.style.borderRadius = '6px';
        summary.innerHTML = `<strong>Total Routes:</strong> ${routingInfo.route_count}`;
        routesDiv.appendChild(summary);
    } else {
        routesDiv.innerHTML = `<p style="color: #7f8c8d;">No routes found for this ASN</p>`;
    }

    // Show results section
    document.getElementById('results-section').style.display = 'block';
    document.querySelector('html').scrollIntoView({ behavior: 'smooth' });
}

function downloadResults() {
    if (!lastResults) {
        showStatus('No results to export. Run simulation first.', 'error');
        return;
    }

    const element = document.createElement('a');
    element.setAttribute('href', 'data:text/csv;charset=utf-8,' + encodeURIComponent(lastResults));
    element.setAttribute('download', 'bgp_routing_tables.csv');
    element.style.display = 'none';
    document.body.appendChild(element);
    element.click();
    document.body.removeChild(element);

    showStatus('Results downloaded successfully!', 'success');
}

function resetSimulation() {
    if (simulator) {
        simulator.reset();
    }

    // Clear data
    caida_data = '';
    announcements_data = '';
    rov_data = '';
    lastResults = null;

    // Clear file inputs
    document.getElementById('caida-file').value = '';
    document.getElementById('announcements-file').value = '';
    document.getElementById('rov-file').value = '';

    // Clear status messages
    document.getElementById('caida-status').textContent = '';
    document.getElementById('announcements-status').textContent = '';
    document.getElementById('rov-status').textContent = '';

    // Clear form input
    document.getElementById('target-asn').value = '';

    // Hide results section
    document.getElementById('results-section').style.display = 'none';

    showStatus('Simulation reset', 'info');
}

function showStatus(message, type) {
    const statusDiv = document.getElementById('status-message');
    statusDiv.textContent = message;
    statusDiv.className = 'status-message show ' + type;

    // Auto-hide after 5 seconds for success messages
    if (type === 'success') {
        setTimeout(() => {
            statusDiv.classList.remove('show');
        }, 5000);
    }
}

// Smooth scroll for results
document.addEventListener('DOMContentLoaded', function() {
    console.log('Page loaded, waiting for WASM module...');
});
