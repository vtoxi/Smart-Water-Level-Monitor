#include "web_server.h"
#include "pump_controller.h"
#include "config.h"

WebServer::WebServer(ConfigManager& configManager, uint16_t port)
    : configManager(configManager),
      server(port),
      sensor1(nullptr),
      sensor2(nullptr),
      pumpController(nullptr),
      running(false) {
}

bool WebServer::begin() {
    setupRoutes();
    server.begin();
    running = true;
    DEBUG_PRINTLN("Web server started");
    return true;
}

void WebServer::stop() {
    server.end();
    running = false;
    DEBUG_PRINTLN("Web server stopped");
}

void WebServer::setupRoutes() {
    // Main page
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleRoot(request);
    });
    
    // API endpoints
    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleStatus(request);
    });
    
    server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleConfig(request);
    });
    
    server.on("/api/config", HTTP_POST, [this](AsyncWebServerRequest* request) {
        // Body handled in onBody callback
    }, nullptr, [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        // Parse JSON body
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, data, len);
        
        if (error) {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }
        
        JsonObject config = doc.as<JsonObject>();
        if (validateConfig(config)) {
            handleConfigSave(request);
        } else {
            request->send(400, "application/json", "{\"error\":\"Invalid configuration\"}");
        }
    });
    
    server.on("/api/wifi/scan", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleWiFiScan(request);
    });
    
    server.on("/api/restart", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleRestart(request);
    });
    
    server.on("/api/reset", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleReset(request);
    });
    
    server.on("/api/pump", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handlePumpControl(request);
    });
    
    // OTA update endpoint
    server.on("/api/update", HTTP_POST, [this](AsyncWebServerRequest* request) {
        bool success = !Update.hasError();
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", 
            success ? "{\"success\":true}" : "{\"success\":false,\"error\":\"Update failed\"}");
        response->addHeader("Connection", "close");
        request->send(response);
        
        if (success) {
            delay(1000);
            ESP.restart();
        }
    }, [this](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
        handleOTAUpload(request, filename, index, data, len, final);
    });
    
    // 404 handler (for captive portal)
    server.onNotFound([this](AsyncWebServerRequest* request) {
        if (request->method() == HTTP_OPTIONS) {
            sendCORS(request);
        } else {
            // Redirect to main page in AP mode
            request->redirect("/");
        }
    });
}

void WebServer::handleRoot(AsyncWebServerRequest* request) {
    #ifdef ESP8266
        // ESP8266: Use chunked response to save RAM
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print(F("<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Water Monitor</title>"));
        response->print(F("<style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:Arial,sans-serif;padding:10px;background:#f0f0f0}.container{max-width:600px;margin:0 auto;background:#fff;padding:15px;border-radius:5px}"));
        response->print(F("h1{color:#333;font-size:20px;margin-bottom:10px}.card{background:#f9f9f9;padding:10px;margin:5px 0;border-radius:3px;border-left:3px solid #4CAF50}"));
        response->print(F(".level{font-size:32px;font-weight:bold;color:#4CAF50}.btn{padding:8px 15px;margin:3px;background:#2196F3;color:#fff;border:none;border-radius:3px;cursor:pointer}"));
        response->print(F(".btn:active{background:#1976D2}.btn-danger{background:#f44336}.form-group{margin:10px 0}label{display:block;margin-bottom:3px;font-weight:bold}"));
        response->print(F("input,select{width:100%;padding:6px;border:1px solid #ddd;border-radius:3px}.tabs{display:flex;border-bottom:1px solid #ddd;margin-bottom:10px}"));
        response->print(F(".tab{padding:8px 15px;cursor:pointer;border:none;background:none}.tab.active{border-bottom:2px solid #2196F3;color:#2196F3;font-weight:bold}"));
        response->print(F(".tab-content{display:none}.tab-content.active{display:block}</style></head><body><div class='container'><h1>💧 Water Monitor</h1>"));
        response->print(F("<div class='tabs'><button class='tab active' onclick=\"showTab('status')\">Status</button>"));
        response->print(F("<button class='tab' onclick=\"showTab('config')\">Config</button><button class='tab' onclick=\"showTab('system')\">System</button></div>"));
        response->print(F("<div id='status-tab' class='tab-content active'><div id='tank-display'></div><div class='card'><h3>Connections</h3>"));
        response->print(F("<p><strong>WiFi:</strong> <span id='wifi-status'>-</span></p><p><strong>MQTT:</strong> <span id='mqtt-status'>-</span></p></div>"));
        response->print(F("<div class='card'><h3>Pump</h3><p><strong>Status:</strong> <span id='pump-status'>-</span></p>"));
        response->print(F("<button class='btn' onclick=\"controlPump('on')\">ON</button><button class='btn btn-danger' onclick=\"controlPump('off')\">OFF</button></div></div>"));
        response->print(F("<div id='config-tab' class='tab-content'><form id='config-form'><h3>WiFi</h3><div class='form-group'><label>SSID:</label><input type='text' id='wifi-ssid'></div>"));
        response->print(F("<div class='form-group'><label>Password:</label><input type='password' id='wifi-password'></div><h3>MQTT</h3>"));
        response->print(F("<div class='form-group'><label>Broker:</label><input type='text' id='mqtt-broker'></div>"));
        response->print(F("<div class='form-group'><label>Port:</label><input type='number' id='mqtt-port' value='1883'></div>"));
        response->print(F("<button type='submit' class='btn'>Save</button></form></div>"));
        response->print(F("<div id='system-tab' class='tab-content'><h3>System</h3><button class='btn' onclick='restart()'>Restart</button>"));
        response->print(F("<button class='btn btn-danger' onclick='factoryReset()'>Reset</button></div></div>"));
        response->print(F("<script>let statusInterval;function showTab(tab){document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));"));
        response->print(F("document.querySelectorAll('.tab-content').forEach(c=>c.classList.remove('active'));event.target.classList.add('active');"));
        response->print(F("document.getElementById(tab+'-tab').classList.add('active');tab==='status'?startStatusUpdates():stopStatusUpdates()}"));
        response->print(F("function startStatusUpdates(){updateStatus();statusInterval=setInterval(updateStatus,3000)}"));
        response->print(F("function stopStatusUpdates(){clearInterval(statusInterval)}"));
        response->print(F("async function updateStatus(){try{const r=await fetch('/api/status');const d=await r.json();"));
        response->print(F("document.getElementById('tank-display').innerHTML=`<div class='card'><h2>${d.tank1?.name||'Tank'}</h2><div class='level'>${(d.tank1?.level||0).toFixed(0)}%</div></div>`;"));
        response->print(F("document.getElementById('wifi-status').textContent=d.wifi?'Connected':'Disconnected';"));
        response->print(F("document.getElementById('mqtt-status').textContent=d.mqtt?'Connected':'Disconnected';"));
        response->print(F("document.getElementById('pump-status').textContent=d.pump?'ON':'OFF'}catch(e){console.error(e)}}"));
        response->print(F("async function controlPump(action){try{await fetch('/api/pump',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action})});"));
        response->print(F("updateStatus()}catch(e){alert('Error: '+e)}}"));
        response->print(F("async function restart(){if(confirm('Restart?')){await fetch('/api/restart',{method:'POST'});alert('Restarting...')}}"));
        response->print(F("async function factoryReset(){if(confirm('Reset all settings?')){await fetch('/api/reset',{method:'POST'});alert('Resetting...')}}"));
        response->print(F("startStatusUpdates();</script></body></html>"));
        request->send(response);
    #else
        // ESP32: Use full-featured page with String (more RAM available)
        String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Water Level Monitor</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: Arial, sans-serif; padding: 20px; background: #f0f0f0; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #333; margin-bottom: 20px; }
        .status-card { background: #f9f9f9; padding: 15px; margin: 10px 0; border-radius: 5px; border-left: 4px solid #4CAF50; }
        .tank { display: inline-block; width: 48%; margin: 1%; text-align: center; }
        .level { font-size: 48px; font-weight: bold; color: #4CAF50; }
        .progress { width: 100%; height: 30px; background: #e0e0e0; border-radius: 15px; overflow: hidden; margin: 10px 0; }
        .progress-bar { height: 100%; background: linear-gradient(90deg, #4CAF50, #8BC34A); transition: width 0.3s; }
        .btn { padding: 10px 20px; margin: 5px; background: #2196F3; color: white; border: none; border-radius: 5px; cursor: pointer; }
        .btn:hover { background: #1976D2; }
        .btn-danger { background: #f44336; }
        .btn-danger:hover { background: #d32f2f; }
        .form-group { margin: 15px 0; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
        input, select { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; }
        .tabs { display: flex; border-bottom: 2px solid #ddd; margin-bottom: 20px; }
        .tab { padding: 10px 20px; cursor: pointer; border: none; background: none; }
        .tab.active { border-bottom: 2px solid #2196F3; color: #2196F3; font-weight: bold; }
        .tab-content { display: none; }
        .tab-content.active { display: block; }
    </style>
</head>
<body>
    <div class="container">
        <h1>💧 Water Level Monitor</h1>
        
        <div class="tabs">
            <button class="tab active" onclick="showTab('status')">Status</button>
            <button class="tab" onclick="showTab('config')">Configuration</button>
            <button class="tab" onclick="showTab('system')">System</button>
        </div>
        
        <div id="status-tab" class="tab-content active">
            <div id="tank-display"></div>
            <div class="status-card">
                <h3>Connection Status</h3>
                <p><strong>WiFi:</strong> <span id="wifi-status">-</span></p>
                <p><strong>MQTT:</strong> <span id="mqtt-status">-</span></p>
                <p><strong>BLE:</strong> <span id="ble-status">-</span></p>
            </div>
            <div class="status-card">
                <h3>Pump Control</h3>
                <p><strong>Status:</strong> <span id="pump-status">-</span></p>
                <button class="btn" onclick="controlPump('on')">Turn On</button>
                <button class="btn btn-danger" onclick="controlPump('off')">Turn Off</button>
            </div>
        </div>
        
        <div id="config-tab" class="tab-content">
            <form id="config-form">
                <h3>WiFi Configuration</h3>
                <div class="form-group">
                    <label>SSID:</label>
                    <input type="text" id="wifi-ssid" required>
                </div>
                <div class="form-group">
                    <label>Password:</label>
                    <input type="password" id="wifi-password" required>
                </div>
                
                <h3>MQTT Configuration</h3>
                <div class="form-group">
                    <label>Broker:</label>
                    <input type="text" id="mqtt-broker" required>
                </div>
                <div class="form-group">
                    <label>Port:</label>
                    <input type="number" id="mqtt-port" value="1883" required>
                </div>
                
                <h3>Tank Configuration</h3>
                <div class="form-group">
                    <label>Mode:</label>
                    <select id="tank-mode">
                        <option value="0">Single Tank</option>
                        <option value="1">Dual Tank</option>
                    </select>
                </div>
                <div class="form-group">
                    <label>Tank 1 Empty (cm):</label>
                    <input type="number" id="tank1-empty" step="0.1" required>
                </div>
                <div class="form-group">
                    <label>Tank 1 Full (cm):</label>
                    <input type="number" id="tank1-full" step="0.1" required>
                </div>
                
                <button type="submit" class="btn">Save Configuration</button>
            </form>
        </div>
        
        <div id="system-tab" class="tab-content">
            <h3>System Actions</h3>
            <button class="btn" onclick="restart()">Restart Device</button>
            <button class="btn btn-danger" onclick="factoryReset()">Factory Reset</button>
            
            <h3>Firmware Update</h3>
            <input type="file" id="firmware-file" accept=".bin">
            <button class="btn" onclick="uploadFirmware()">Upload Firmware</button>
        </div>
    </div>
    
    <script>
        let statusInterval;
        
        function showTab(tab) {
            document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
            event.target.classList.add('active');
            document.getElementById(tab + '-tab').classList.add('active');
            
            if (tab === 'status') {
                startStatusUpdates();
            } else {
                stopStatusUpdates();
            }
        }
        
        function startStatusUpdates() {
            updateStatus();
            statusInterval = setInterval(updateStatus, 2000);
        }
        
        function stopStatusUpdates() {
            clearInterval(statusInterval);
        }
        
        async function updateStatus() {
            try {
                const response = await fetch('/api/status');
                const data = await response.json();
                
                // Update tank display
                let tankHTML = '';
                if (data.tankMode === 0) {
                    tankHTML = `
                        <div class="tank">
                            <h2>${data.tank1.name}</h2>
                            <div class="level">${data.tank1.level.toFixed(0)}%</div>
                            <div class="progress">
                                <div class="progress-bar" style="width: ${data.tank1.level}%"></div>
                            </div>
                            <p>${data.tank1.distance.toFixed(1)} cm</p>
                        </div>
                    `;
                } else {
                    tankHTML = `
                        <div class="tank">
                            <h2>${data.tank1.name}</h2>
                            <div class="level">${data.tank1.level.toFixed(0)}%</div>
                            <div class="progress">
                                <div class="progress-bar" style="width: ${data.tank1.level}%"></div>
                            </div>
                        </div>
                        <div class="tank">
                            <h2>${data.tank2.name}</h2>
                            <div class="level">${data.tank2.level.toFixed(0)}%</div>
                            <div class="progress">
                                <div class="progress-bar" style="width: ${data.tank2.level}%"></div>
                            </div>
                        </div>
                    `;
                }
                document.getElementById('tank-display').innerHTML = tankHTML;
                
                // Update connection status
                document.getElementById('wifi-status').textContent = data.wifi ? 'Connected' : 'Disconnected';
                document.getElementById('mqtt-status').textContent = data.mqtt ? 'Connected' : 'Disconnected';
                document.getElementById('ble-status').textContent = data.ble ? 'Active' : 'Inactive';
                document.getElementById('pump-status').textContent = data.pump ? 'ON' : 'OFF';
            } catch (error) {
                console.error('Error updating status:', error);
            }
        }
        
        async function controlPump(action) {
            try {
                await fetch('/api/pump', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ action: action })
                });
                updateStatus();
            } catch (error) {
                alert('Error controlling pump: ' + error);
            }
        }
        
        async function restart() {
            if (confirm('Restart device?')) {
                await fetch('/api/restart', { method: 'POST' });
                alert('Device restarting...');
            }
        }
        
        async function factoryReset() {
            if (confirm('Factory reset? All settings will be lost!')) {
                await fetch('/api/reset', { method: 'POST' });
                alert('Device reset. Restarting...');
            }
        }
        
        startStatusUpdates();
    </script>
</body>
</html>
    )rawliteral";
        
        request->send(200, "text/html", html);
    #endif
}

void WebServer::handleStatus(AsyncWebServerRequest* request) {
    request->send(200, "application/json", getStatusJSON());
}

void WebServer::handleConfig(AsyncWebServerRequest* request) {
    request->send(200, "application/json", getConfigJSON());
}

void WebServer::handleConfigSave(AsyncWebServerRequest* request) {
    // Configuration saved during body processing
    configManager.saveConfig();
    request->send(200, "application/json", "{\"success\":true}");
}

void WebServer::handleWiFiScan(AsyncWebServerRequest* request) {
    int n = WiFi.scanNetworks();
    DynamicJsonDocument doc(2048);
    JsonArray networks = doc.createNestedArray("networks");
    
    for (int i = 0; i < n && i < 20; i++) {
        JsonObject network = networks.createNestedObject();
        network["ssid"] = WiFi.SSID(i);
        network["rssi"] = WiFi.RSSI(i);
        #ifdef ESP8266
            network["encryption"] = (WiFi.encryptionType(i) == AUTH_OPEN) ? "open" : "encrypted";
        #else
            network["encryption"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "open" : "encrypted";
        #endif
    }
    
    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
}

void WebServer::handleRestart(AsyncWebServerRequest* request) {
    request->send(200, "application/json", "{\"success\":true}");
    delay(1000);
    ESP.restart();
}

void WebServer::handleReset(AsyncWebServerRequest* request) {
    configManager.resetToDefaults();
    configManager.saveConfig();
    request->send(200, "application/json", "{\"success\":true}");
    delay(1000);
    ESP.restart();
}

void WebServer::handleOTAUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
    if (!index) {
        DEBUG_PRINTF("OTA Update Start: %s\n", filename.c_str());
        #ifdef ESP8266
            uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            if (!Update.begin(maxSketchSpace)) {
        #else
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        #endif
            Update.printError(Serial);
        }
    }
    
    if (Update.write(data, len) != len) {
        Update.printError(Serial);
    }
    
    if (final) {
        if (Update.end(true)) {
            DEBUG_PRINTF("OTA Update Success: %u bytes\n", index + len);
        } else {
            Update.printError(Serial);
        }
    }
}

void WebServer::handlePumpControl(AsyncWebServerRequest* request) {
    if (!pumpController) {
        request->send(500, "application/json", "{\"error\":\"Pump not available\"}");
        return;
    }
    
    if (request->hasParam("action", true)) {
        String action = request->getParam("action", true)->value();
        
        if (action == "on") {
            pumpController->turnOn();
        } else if (action == "off") {
            pumpController->turnOff();
        } else {
            request->send(400, "application/json", "{\"error\":\"Invalid action\"}");
            return;
        }
        
        request->send(200, "application/json", "{\"success\":true}");
    } else {
        request->send(400, "application/json", "{\"error\":\"Missing action parameter\"}");
    }
}

String WebServer::getStatusJSON() {
    DynamicJsonDocument doc(1024);
    const SystemConfig& config = configManager.getConfig();
    
    doc["tankMode"] = config.tankMode;
    doc["wifi"] = WiFi.status() == WL_CONNECTED;
    doc["mqtt"] = false; // Will be set by MQTT client
    doc["ble"] = true;
    doc["pump"] = pumpController ? pumpController->isRunning() : false;
    
    JsonObject tank1 = doc.createNestedObject("tank1");
    tank1["name"] = config.tank1Name;
    if (sensor1) {
        const SensorReading& reading = sensor1->getLastReading();
        tank1["level"] = reading.levelPercent;
        tank1["distance"] = reading.distanceCm;
        tank1["valid"] = reading.isValid;
    } else {
        tank1["level"] = 0;
        tank1["distance"] = 0;
        tank1["valid"] = false;
    }
    
    if (config.tankMode == DUAL_TANK && sensor2) {
        JsonObject tank2 = doc.createNestedObject("tank2");
        tank2["name"] = config.tank2Name;
        const SensorReading& reading = sensor2->getLastReading();
        tank2["level"] = reading.levelPercent;
        tank2["distance"] = reading.distanceCm;
        tank2["valid"] = reading.isValid;
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

String WebServer::getConfigJSON() {
    DynamicJsonDocument doc(2048);
    const SystemConfig& config = configManager.getConfig();
    
    doc["tankMode"] = config.tankMode;
    doc["wifiSSID"] = config.wifiSSID;
    doc["mqttBroker"] = config.mqttBroker;
    doc["mqttPort"] = config.mqttPort;
    doc["tank1Empty"] = config.tank1EmptyCm;
    doc["tank1Full"] = config.tank1FullCm;
    doc["tank2Empty"] = config.tank2EmptyCm;
    doc["tank2Full"] = config.tank2FullCm;
    doc["pumpMode"] = config.pumpMode;
    
    String output;
    serializeJson(doc, output);
    return output;
}

bool WebServer::validateConfig(JsonObject& config) {
    // Basic validation - add more as needed
    return true;
}

void WebServer::sendCORS(AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
}

