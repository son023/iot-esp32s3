let autoRefreshInterval = null;
let isAutoRefreshEnabled = true;

const aht22TemperatureElement = document.getElementById('aht22-temperature');
const aht22HumidityElement = document.getElementById('aht22-humidity');
const aht22StatusElement = document.getElementById('aht22-status');

const bmp180TemperatureElement = document.getElementById('bmp180-temperature');
const bmp180PressureElement = document.getElementById('bmp180-pressure');
const bmp180StatusElement = document.getElementById('bmp180-status');

const lastUpdateElement = document.getElementById('lastUpdate');
const relayStatusElement = document.getElementById('relayStatus');
const relayModeElement = document.getElementById('relayMode');
const connectionStatusElement = document.getElementById('connectionStatus');
const autoRefreshStatusElement = document.getElementById('autoRefreshStatus');
const toastElement = document.getElementById('toast');
const toastMessageElement = document.getElementById('toastMessage');


document.addEventListener('DOMContentLoaded', async function() {
    console.log('ESP32 IoT System initialized');
    await loadInitialThresholds();
    
    refreshData();
    startAutoRefresh();
});

async function loadInitialThresholds() {
    try {
        const response = await fetch('/api/relay');
        if (response.ok) {
            const data = await response.json();
            document.getElementById('tempHigh').value = data.threshold_high;
            document.getElementById('tempLow').value = data.threshold_low;
        }
    } catch (error) {
        console.error('Error loading initial thresholds:', error);
        document.getElementById('tempHigh').value = 30;
        document.getElementById('tempLow').value = 25;
    }
}

function showToast(message, type = 'info') {
    toastMessageElement.textContent = message;
    toastElement.className = `toast ${type}`;
    setTimeout(() => {
        toastElement.classList.remove('hidden');
    }, 100);
    
    setTimeout(() => {
        toastElement.classList.add('hidden');
    }, 3000);
}

function formatTimestamp() {
    const now = new Date();
    return now.toLocaleTimeString(); 
}

async function fetchSensorData() {
    try {
        const response = await fetch('/api/sensors');
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data = await response.json();
        
        if (data.aht22.available) {
            aht22TemperatureElement.textContent = data.aht22.temperature.toFixed(1);
            aht22HumidityElement.textContent = data.aht22.humidity.toFixed(1);
            aht22StatusElement.textContent = 'Online';
            aht22StatusElement.className = 'sensor-value online';
        } else {
            aht22TemperatureElement.textContent = '--';
            aht22HumidityElement.textContent = '--';
            aht22StatusElement.textContent = 'Offline';
            aht22StatusElement.className = 'sensor-value offline';
        }
        
        if (data.bmp180.available) {
            bmp180TemperatureElement.textContent = data.bmp180.temperature.toFixed(1);
            bmp180PressureElement.textContent = data.bmp180.pressure.toFixed(1);
            bmp180StatusElement.textContent = 'Online';
            bmp180StatusElement.className = 'sensor-value online';
        } else {
            bmp180TemperatureElement.textContent = '--';
            bmp180PressureElement.textContent = '--';
            bmp180StatusElement.textContent = 'Offline';
            bmp180StatusElement.className = 'sensor-value offline';
        }
        
        lastUpdateElement.textContent = formatTimestamp();
        
        connectionStatusElement.textContent = 'Connected';
        connectionStatusElement.className = 'status-value connected';
        
        return data;
    } catch (error) {
        console.error('Error fetching sensor data:', error);
        aht22TemperatureElement.textContent = '--';
        aht22HumidityElement.textContent = '--';
        aht22StatusElement.textContent = 'Error';
        bmp180TemperatureElement.textContent = '--';
        bmp180PressureElement.textContent = '--';
        bmp180StatusElement.textContent = 'Error';
        lastUpdateElement.textContent = 'Error';
        
        connectionStatusElement.textContent = 'Disconnected';
        connectionStatusElement.className = 'status-value disconnected';
        
        throw error;
    }
}

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

function updateRelayStatusUI(state) {
    if (state === 1) {
        relayStatusElement.textContent = 'ON';
        relayStatusElement.className = 'status-indicator on';
    } else {
        relayStatusElement.textContent = 'OFF';
        relayStatusElement.className = 'status-indicator off';
    }
}

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

function updateThresholdDisplay(tempHigh, tempLow) {
    document.getElementById('currentHigh').textContent = tempHigh.toFixed(1);
    document.getElementById('currentLow').textContent = tempLow.toFixed(1);
}


async function setRelay(state) {
    try {
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
        fetchRelayStatus();
    } finally {
        relayStatusElement.classList.remove('loading');
    }
}

async function refreshData() {
    try {
        aht22TemperatureElement.classList.add('loading');
        aht22HumidityElement.classList.add('loading');
        bmp180TemperatureElement.classList.add('loading');
        bmp180PressureElement.classList.add('loading');
        relayStatusElement.classList.add('loading');
        
        const [sensorData, relayData] = await Promise.all([
            fetchSensorData(),
            fetchRelayStatus()
        ]);
        
        console.log('Data refreshed:', { sensorData, relayData });
        
    } catch (error) {
        console.error('Error refreshing data:', error);
        showToast('Failed to refresh data', 'error');
    } finally {
        aht22TemperatureElement.classList.remove('loading');
        aht22HumidityElement.classList.remove('loading');
        bmp180TemperatureElement.classList.remove('loading');
        bmp180PressureElement.classList.remove('loading');
        relayStatusElement.classList.remove('loading');
    }
}

function startAutoRefresh() {
    if (autoRefreshInterval) {
        clearInterval(autoRefreshInterval);
    }
    
    autoRefreshInterval = setInterval(() => {
        if (isAutoRefreshEnabled) {
            refreshData();
        }
    }, 1000);
    
    updateAutoRefreshStatus();
}

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

function updateAutoRefreshStatus() {
    if (isAutoRefreshEnabled) {
        autoRefreshStatusElement.textContent = 'ON';
        autoRefreshStatusElement.className = 'status-value connected';
    } else {
        autoRefreshStatusElement.textContent = 'OFF';
        autoRefreshStatusElement.className = 'status-value';
    }
}

document.addEventListener('visibilitychange', function() {
    if (document.hidden) {
        if (autoRefreshInterval) {
            clearInterval(autoRefreshInterval);
            autoRefreshInterval = null;
        }
    } else {
        if (isAutoRefreshEnabled && !autoRefreshInterval) {
            startAutoRefresh();
        }
    }
});

document.addEventListener('keydown', function(event) {
    if ((event.ctrlKey && event.key === 'r') || event.key === 'F5') {
        event.preventDefault();
        refreshData();
        showToast('Data refreshed manually', 'info');
    }
    
    if (event.code === 'Space' && event.target.tagName !== 'INPUT') {
        event.preventDefault();
        const currentState = relayStatusElement.textContent === 'ON' ? 1 : 0;
        setRelay(currentState === 1 ? 0 : 1);
    }
});

function monitorConnection() {
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

monitorConnection();

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
        fetchRelayStatus();
    }
}

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

window.setRelay = setRelay;
window.setRelayMode = setRelayMode;
window.setThresholds = setThresholds;
window.refreshData = refreshData;
window.toggleAutoRefresh = toggleAutoRefresh; 