// Global variables
let autoRefreshInterval = null;
let isAutoRefreshEnabled = true;

// DOM elements
const temperatureElement = document.getElementById('temperature');
const humidityElement = document.getElementById('humidity');
const pressureElement = document.getElementById('pressure');
const lastUpdateElement = document.getElementById('lastUpdate');
const relayStatusElement = document.getElementById('relayStatus');
const relayModeElement = document.getElementById('relayMode');
const connectionStatusElement = document.getElementById('connectionStatus');
const autoRefreshStatusElement = document.getElementById('autoRefreshStatus');
const toastElement = document.getElementById('toast');
const toastMessageElement = document.getElementById('toastMessage');

// Initialize the application
document.addEventListener('DOMContentLoaded', function() {
    console.log('ESP32 IoT System initialized');
    refreshData();
    startAutoRefresh();
});

// Show toast notification
function showToast(message, type = 'info') {
    toastMessageElement.textContent = message;
    toastElement.className = `toast ${type}`;
    
    // Show toast
    setTimeout(() => {
        toastElement.classList.remove('hidden');
    }, 100);
    
    // Hide toast after 3 seconds
    setTimeout(() => {
        toastElement.classList.add('hidden');
    }, 3000);
}

// Format timestamp
function formatTimestamp(timestamp) {
    const date = new Date(timestamp * 1000);
    return date.toLocaleTimeString();
}

// Fetch sensor data
async function fetchSensorData() {
    try {
        const response = await fetch('/api/sensors');
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data = await response.json();
        
        // Update UI
        temperatureElement.textContent = data.temperature.toFixed(1);
        humidityElement.textContent = data.humidity.toFixed(1);
        pressureElement.textContent = data.pressure.toFixed(1);
        lastUpdateElement.textContent = formatTimestamp(data.timestamp);
        
        // Update connection status
        connectionStatusElement.textContent = 'Connected';
        connectionStatusElement.className = 'status-value connected';
        
        return data;
    } catch (error) {
        console.error('Error fetching sensor data:', error);
        temperatureElement.textContent = '--';
        humidityElement.textContent = '--';
        pressureElement.textContent = '--';
        lastUpdateElement.textContent = 'Error';
        
        // Update connection status
        connectionStatusElement.textContent = 'Disconnected';
        connectionStatusElement.className = 'status-value disconnected';
        
        throw error;
    }
}

// Fetch relay status
async function fetchRelayStatus() {
    try {
        const response = await fetch('/api/relay');
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data = await response.json();
        
        // Update UI
        updateRelayStatusUI(data.state);
        updateRelayModeUI(data.mode);
        updateThresholdDisplay(data.threshold_high, data.threshold_low);
        
        return data;
    } catch (error) {
        console.error('Error fetching relay status:', error);
        relayStatusElement.textContent = 'ERROR';
        relayStatusElement.className = 'status-indicator error';
        throw error;
    }
}

// Update relay status UI
function updateRelayStatusUI(state) {
    if (state === 1) {
        relayStatusElement.textContent = 'ON';
        relayStatusElement.className = 'status-indicator on';
    } else {
        relayStatusElement.textContent = 'OFF';
        relayStatusElement.className = 'status-indicator off';
    }
}

// Update relay mode UI
function updateRelayModeUI(mode) {
    const manualControls = document.getElementById('manualControls');
    const manualBtn = document.getElementById('manualBtn');
    const autoBtn = document.getElementById('autoBtn');
    
    if (mode === 1) {
        relayModeElement.textContent = 'AUTO';
        relayModeElement.className = 'mode-indicator auto';
        manualControls.style.display = 'none'; // Ẩn nút manual khi ở chế độ auto
        manualBtn.classList.remove('btn-primary');
        autoBtn.classList.add('btn-primary');
    } else {
        relayModeElement.textContent = 'MANUAL';
        relayModeElement.className = 'mode-indicator manual';
        manualControls.style.display = 'flex';
        manualBtn.classList.add('btn-primary');
        autoBtn.classList.remove('btn-primary');
    }
}

// Update threshold display
function updateThresholdDisplay(tempHigh, tempLow) {
    document.getElementById('currentHigh').textContent = tempHigh.toFixed(1);
    document.getElementById('currentLow').textContent = tempLow.toFixed(1);
    document.getElementById('tempHigh').value = tempHigh;
    document.getElementById('tempLow').value = tempLow;
}

// Set relay state
async function setRelay(state) {
    try {
        // Show loading state
        relayStatusElement.classList.add('loading');
        
        const response = await fetch('/api/relay', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ state: state })
        });
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const data = await response.json();
        
        if (data.success) {
            updateRelayStatusUI(data.state);
            showToast(`Relay turned ${state ? 'ON' : 'OFF'}`, 'success');
        } else {
            throw new Error('Failed to set relay state');
        }
        
    } catch (error) {
        console.error('Error setting relay state:', error);
        showToast('Failed to control relay', 'error');
        
        // Refresh relay status to get current state
        fetchRelayStatus();
    } finally {
        relayStatusElement.classList.remove('loading');
    }
}

// Configure WiFi
async function configureWifi(event) {
    event.preventDefault();
    
    const ssid = document.getElementById('ssid').value;
    const password = document.getElementById('password').value;
    
    if (!ssid || !password) {
        showToast('Please enter both SSID and password', 'warning');
        return;
    }
    
    try {
        const response = await fetch('/api/wifi', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                ssid: ssid,
                password: password
            })
        });
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const data = await response.json();
        
        if (data.success) {
            showToast(data.message || 'WiFi credentials saved successfully', 'success');
            // Clear form
            document.getElementById('ssid').value = '';
            document.getElementById('password').value = '';
        } else {
            throw new Error('Failed to save WiFi credentials');
        }
        
    } catch (error) {
        console.error('Error configuring WiFi:', error);
        showToast('Failed to save WiFi credentials', 'error');
    }
}

// Refresh all data
async function refreshData() {
    try {
        // Add loading animation to sensor values
        temperatureElement.classList.add('loading');
        humidityElement.classList.add('loading');
        pressureElement.classList.add('loading');
        relayStatusElement.classList.add('loading');
        
        // Fetch all data concurrently
        const [sensorData, relayData] = await Promise.all([
            fetchSensorData(),
            fetchRelayStatus()
        ]);
        
        console.log('Data refreshed:', { sensorData, relayData });
        
    } catch (error) {
        console.error('Error refreshing data:', error);
        showToast('Failed to refresh data', 'error');
    } finally {
        // Remove loading animation
        temperatureElement.classList.remove('loading');
        humidityElement.classList.remove('loading');
        pressureElement.classList.remove('loading');
        relayStatusElement.classList.remove('loading');
    }
}

// Start auto-refresh
function startAutoRefresh() {
    if (autoRefreshInterval) {
        clearInterval(autoRefreshInterval);
    }
    
    autoRefreshInterval = setInterval(() => {
        if (isAutoRefreshEnabled) {
            refreshData();
        }
    }, 5000); // Refresh every 5 seconds
    
    updateAutoRefreshStatus();
}

// Toggle auto-refresh
function toggleAutoRefresh() {
    isAutoRefreshEnabled = !isAutoRefreshEnabled;
    updateAutoRefreshStatus();
    
    if (isAutoRefreshEnabled) {
        showToast('Auto-refresh enabled', 'success');
        startAutoRefresh();
    } else {
        showToast('Auto-refresh disabled', 'warning');
        if (autoRefreshInterval) {
            clearInterval(autoRefreshInterval);
            autoRefreshInterval = null;
        }
    }
}

// Update auto-refresh status UI
function updateAutoRefreshStatus() {
    if (isAutoRefreshEnabled) {
        autoRefreshStatusElement.textContent = 'ON';
        autoRefreshStatusElement.className = 'status-value connected';
    } else {
        autoRefreshStatusElement.textContent = 'OFF';
        autoRefreshStatusElement.className = 'status-value';
    }
}

// Handle visibility change to pause/resume auto-refresh when tab is not visible
document.addEventListener('visibilitychange', function() {
    if (document.hidden) {
        // Tab is not visible, pause auto-refresh
        if (autoRefreshInterval) {
            clearInterval(autoRefreshInterval);
            autoRefreshInterval = null;
        }
    } else {
        // Tab is visible again, resume auto-refresh if enabled
        if (isAutoRefreshEnabled && !autoRefreshInterval) {
            startAutoRefresh();
        }
    }
});

// Add keyboard shortcuts
document.addEventListener('keydown', function(event) {
    // Ctrl+R or F5 for refresh
    if ((event.ctrlKey && event.key === 'r') || event.key === 'F5') {
        event.preventDefault();
        refreshData();
        showToast('Data refreshed manually', 'info');
    }
    
    // Space bar to toggle relay
    if (event.code === 'Space' && event.target.tagName !== 'INPUT') {
        event.preventDefault();
        // Get current relay state and toggle
        const currentState = relayStatusElement.textContent === 'ON' ? 1 : 0;
        setRelay(currentState === 1 ? 0 : 1);
    }
});

// Add connection monitoring
function monitorConnection() {
    // Try to fetch data every 30 seconds to check connection
    setInterval(async () => {
        try {
            const response = await fetch('/api/sensors', {
                method: 'GET',
                cache: 'no-cache'
            });
            
            if (response.ok) {
                connectionStatusElement.textContent = 'Connected';
                connectionStatusElement.className = 'status-value connected';
            } else {
                throw new Error('Connection failed');
            }
        } catch (error) {
            connectionStatusElement.textContent = 'Disconnected';
            connectionStatusElement.className = 'status-value disconnected';
        }
    }, 30000);
}

// Start connection monitoring
monitorConnection();

// Set relay mode
async function setRelayMode(mode) {
    try {
        const response = await fetch('/api/relay', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ mode: mode })
        });
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const data = await response.json();
        
        if (data.success) {
            updateRelayStatusUI(data.state);
            updateRelayModeUI(data.mode);
            showToast(`Relay mode changed to ${mode ? 'AUTO' : 'MANUAL'}`, 'success');
        } else {
            throw new Error('Failed to set relay mode');
        }
        
    } catch (error) {
        console.error('Error setting relay mode:', error);
        showToast('Failed to change relay mode', 'error');
        
        // Refresh relay status to get current state
        fetchRelayStatus();
    }
}

// Set temperature thresholds
async function setThresholds(event) {
    event.preventDefault();
    
    const tempHigh = parseFloat(document.getElementById('tempHigh').value);
    const tempLow = parseFloat(document.getElementById('tempLow').value);
    
    if (tempHigh <= tempLow) {
        showToast('High temperature must be greater than low temperature', 'warning');
        return;
    }
    
    if (tempHigh < 0 || tempHigh > 100 || tempLow < 0 || tempLow > 100) {
        showToast('Temperature must be between 0-100°C', 'warning');
        return;
    }
    
    try {
        const response = await fetch('/api/thresholds', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                temp_high: tempHigh,
                temp_low: tempLow
            })
        });
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const data = await response.json();
        
        if (data.success) {
            updateThresholdDisplay(data.temp_high, data.temp_low);
            showToast(data.message || 'Temperature thresholds updated successfully', 'success');
        } else {
            throw new Error('Failed to save temperature thresholds');
        }
        
    } catch (error) {
        console.error('Error setting temperature thresholds:', error);
        showToast('Failed to save temperature thresholds', 'error');
    }
}

// Export functions to global scope for HTML onclick handlers
window.setRelay = setRelay;
window.setRelayMode = setRelayMode;
window.setThresholds = setThresholds;
window.configureWifi = configureWifi;
window.refreshData = refreshData;
window.toggleAutoRefresh = toggleAutoRefresh; 