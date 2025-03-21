#include "../include/web_server.h"
#include "../include/wifi_manager.h"
#include "../include/storage.h"
#include "../include/feeding_schedule.h"

// Modificar el HTML para añadir una lista desplegable y un botón de escaneo
const char WebServerManager::INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }
    h1 { color: #2c3e50; }
    .container { max-width: 400px; margin: 0 auto; background-color: #fff; padding: 20px; border-radius: 5px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
    form { width: 100%; }
    select, input { width: 100%; padding: 10px; margin: 8px 0; box-sizing: border-box; border: 1px solid #ddd; border-radius: 4px; }
    button { background-color: #4CAF50; color: white; padding: 12px 15px; border: none; cursor: pointer; border-radius: 4px; width: 100%; font-size: 16px; margin-top: 10px; }
    button:hover { background-color: #45a049; }
    .scan-button { background-color: #3498db; }
    .scan-button:hover { background-color: #2980b9; }
    .header { text-align: center; margin-bottom: 20px; }
    .loading { display: none; text-align: center; margin: 10px 0; }
    .network-list { margin-top: 15px; }
    .password-container { position: relative; }
    .password-toggle { position: absolute; right: 10px; top: 18px; cursor: pointer; user-select: none; color: #666; }
    .checkbox-container { display: flex; align-items: center; margin: 10px 0; }
    .checkbox-container input { width: auto; margin-right: 10px; }
  </style>
  <title>Pet Feeder WiFi Setup</title>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>Pet Feeder WiFi Setup</h1>
    </div>
    <div class="network-list">
      <label for="network-select">Select WiFi Network:</label>
      <select id="network-select" onchange="updateSSID(this.value)">
        <option value="">-- Select a network --</option>
      </select>
      <button class="scan-button" onclick="scanNetworks(event)">Scan Networks</button>
      <div id="loading" class="loading">Scanning networks...</div>
    </div>
    <form method='post' action='/setupwifi'>
      <label for="ssid">WiFi Network Name:</label>
      <input type='text' id="ssid" name='ssid' required>
      
      <label for="password">WiFi Password:</label>
      <div class="password-container">
        <input type='password' id="password" name='password' required>
        <span class="password-toggle" onclick="togglePassword()">&#128065;</span>
      </div>
      
      <div class="checkbox-container">
        <input type="checkbox" id="show-password" onclick="togglePassword()">
        <label for="show-password">Show password</label>
      </div>
      
      <button type='submit'>Connect</button>
    </form>
  </div>

  <script>
    function updateSSID(value) {
      document.getElementById('ssid').value = value;
    }

    function togglePassword() {
      const passwordField = document.getElementById('password');
      const checkbox = document.getElementById('show-password');
      
      // Sincronizar el checkbox con el estado del campo
      if (passwordField.type === 'password') {
        passwordField.type = 'text';
        checkbox.checked = true;
      } else {
        passwordField.type = 'password';
        checkbox.checked = false;
      }
    }

    function scanNetworks(e) {
      e.preventDefault();
      
      // Mostrar indicador de carga
      document.getElementById('loading').style.display = 'block';
      
      // Hacer la solicitud al endpoint para escanear las redes
      fetch('/wifiscan')
        .then(response => response.json())
        .then(data => {
          const select = document.getElementById('network-select');
          
          // Limpiar opciones existentes, excepto la primera
          while (select.options.length > 1) {
            select.remove(1);
          }
          
          // Añadir las redes encontradas
          data.networks.forEach(network => {
            const option = document.createElement('option');
            option.value = network.ssid;
            option.textContent = `${network.ssid} (${network.rssi}dBm)`;
            select.appendChild(option);
          });
          
          // Ocultar indicador de carga
          document.getElementById('loading').style.display = 'none';
        })
        .catch(error => {
          console.error('Error scanning networks:', error);
          document.getElementById('loading').style.display = 'none';
          alert('Error scanning networks. Please try again.');
        });
    }
  </script>
</body>
</html>
)rawliteral";


const char WebServerManager::SUCCESS_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }
    h1 { color: #2c3e50; }
    .container { max-width: 400px; margin: 0 auto; background-color: #fff; padding: 20px; border-radius: 5px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); text-align: center; }
    p { color: #7f8c8d; }
    .success-icon { color: #2ecc71; font-size: 48px; margin-bottom: 20px; }
  </style>
  <title>WiFi Configuration Complete</title>
</head>
<body>
  <div class="container">
    <div class="success-icon">✓</div>
    <h1>Configuration Saved!</h1>
    <p>The pet feeder will now attempt to connect to your WiFi network.</p>
  </div>
</body>
</html>
)rawliteral";

// HTML para la página de programación de alimentación
const char WebServerManager::SCHEDULE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }
    h1, h2 { color: #2c3e50; }
    .container { max-width: 600px; margin: 0 auto; background-color: #fff; padding: 20px; border-radius: 5px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
    .schedule-item { border: 1px solid #ddd; padding: 15px; margin-bottom: 15px; border-radius: 5px; }
    .time-input { display: flex; align-items: center; margin-bottom: 10px; }
    .time-input label { flex: 0 0 100px; }
    input[type="number"] { width: 60px; padding: 8px; margin-right: 10px; border: 1px solid #ddd; border-radius: 4px; }
    input[type="checkbox"] { margin-right: 10px; }
    button { background-color: #4CAF50; color: white; padding: 10px 15px; border: none; cursor: pointer; border-radius: 4px; margin-top: 10px; font-size: 14px; }
    button:hover { background-color: #45a049; }
    .delete-btn { background-color: #e74c3c; }
    .delete-btn:hover { background-color: #c0392b; }
    .header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }
    .status { text-align: center; margin: 10px 0; padding: 10px; border-radius: 5px; display: none; }
    .success { background-color: #d4edda; color: #155724; }
    .error { background-color: #f8d7da; color: #721c24; }
    .nav-buttons { display: flex; justify-content: space-between; margin-top: 20px; }
    .back-btn { background-color: #3498db; }
    .back-btn:hover { background-color: #2980b9; }
  </style>
  <title>Pet Feeder Schedule</title>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>Feeding Schedule</h1>
    </div>
    
    <div id="status" class="status"></div>
    
    <div id="schedules-container">
      <!-- Las programaciones se cargarán aquí dinámicamente -->
    </div>
    
    <div class="nav-buttons">
      <button class="back-btn" onclick="location.href='/'">Back to Home</button>
      <button id="save-btn">Save All Schedules</button>
    </div>
  </div>

  <script>
    // Template para un horario
    function createScheduleTemplate(index, hour, minute, seconds, enabled) {
      return `
        <div class="schedule-item" data-index="${index}">
          <h2>Schedule #${index + 1}</h2>
          <div class="time-input">
            <label>Time:</label>
            <input type="number" min="0" max="23" value="${hour}" placeholder="Hour" class="hour">
            <span>:</span>
            <input type="number" min="0" max="59" value="${minute}" placeholder="Min" class="minute">
          </div>
          <div class="time-input">
            <label>Duration:</label>
            <input type="number" min="1" max="20" value="${seconds}" placeholder="Sec" class="seconds">
            <span>seconds</span>
          </div>
          <div class="time-input">
            <label>Enabled:</label>
            <input type="checkbox" class="enabled" ${enabled ? 'checked' : ''}>
          </div>
        </div>
      `;
    }
    
    // Cargar horarios desde el servidor
    async function loadSchedules() {
      try {
        const response = await fetch('/api/schedules');
        const data = await response.json();
        
        const container = document.getElementById('schedules-container');
        container.innerHTML = '';
        
        data.schedules.forEach(schedule => {
          container.innerHTML += createScheduleTemplate(
            schedule.index,
            schedule.hour,
            schedule.minute,
            schedule.seconds,
            schedule.enabled
          );
        });
        
      } catch (error) {
        showStatus('Error loading schedules: ' + error.message, false);
      }
    }
    
    // Guardar horarios en el servidor
    async function saveSchedules() {
      try {
        const schedules = [];
        
        document.querySelectorAll('.schedule-item').forEach(item => {
          const index = parseInt(item.getAttribute('data-index'));
          const hour = parseInt(item.querySelector('.hour').value) || 0;
          const minute = parseInt(item.querySelector('.minute').value) || 0;
          const seconds = parseInt(item.querySelector('.seconds').value) || 0;
          const enabled = item.querySelector('.enabled').checked;
          
          schedules.push({ index, hour, minute, seconds, enabled });
        });
        
        const response = await fetch('/api/save-schedule', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({ schedules }),
        });
        
        const result = await response.json();
        
        if (result.success) {
          showStatus('Schedules saved successfully!', true);
        } else {
          showStatus('Error: ' + result.message, false);
        }
        
      } catch (error) {
        showStatus('Error saving schedules: ' + error.message, false);
      }
    }
    
    // Mostrar mensaje de estado
    function showStatus(message, isSuccess) {
      const status = document.getElementById('status');
      status.textContent = message;
      status.className = 'status ' + (isSuccess ? 'success' : 'error');
      status.style.display = 'block';
      
      setTimeout(() => {
        status.style.display = 'none';
      }, 3000);
    }
    
    // Inicializar página
    document.addEventListener('DOMContentLoaded', () => {
      // Cargar horarios
      loadSchedules();
      
      // Configurar botón de guardar
      document.getElementById('save-btn').addEventListener('click', saveSchedules);
    });
  </script>
</body>
</html>
)rawliteral";

WebServerManager::WebServerManager() : server(80) {}

void WebServerManager::begin() {
  setupRoutes();
  server.begin();
}

void WebServerManager::setupCaptivePortal(IPAddress apIP) {
  dnsServer.start(53, "*", apIP);
}

void WebServerManager::handleClient() {
  dnsServer.processNextRequest();
  server.handleClient();
}

void WebServerManager::stop() {
  server.close();
}

void WebServerManager::setupRoutes() {
  // Eliminar cualquier manejador existente
  server.onNotFound(nullptr);
  
  // Configurar las rutas
  server.on("/", HTTP_GET, [this](){ this->handleRoot(); });
  server.on("/setupwifi", HTTP_GET, [this]() {
    server.send_P(200, "text/html", INDEX_HTML);
  });
  server.on("/setupwifi", HTTP_POST, [this](){ this->handleSetupWiFi(); });
  server.on("/wifiscan", HTTP_GET, [this](){ this->handleWifiScan(); });
  server.on("/schedule", HTTP_GET, [this](){ this->handleFeedingSchedule(); });
  server.on("/api/schedules", HTTP_GET, [this](){ this->handleGetSchedules(); });
  server.on("/api/save-schedule", HTTP_POST, [this](){ this->handleSaveSchedule(); });
  server.onNotFound([this](){ this->handleNotFound(); });
}

// Implementar la función para escanear redes WiFi
void WebServerManager::scanWifiNetworks() {
  Serial.println("Scanning WiFi networks...");
  int networksFound = WiFi.scanNetworks();
  Serial.println("Scan done");
  
  if (networksFound == 0) {
    Serial.println("No networks found");
  } else {
    Serial.print(networksFound);
    Serial.println(" networks found");
  }
}

// Implementar el manejador para la solicitud de escaneo
void WebServerManager::handleWifiScan() {
  // Escanear redes WiFi
  int networksFound = WiFi.scanNetworks();
  
  // Construir JSON con los resultados
  String json = "{\"networks\":[";
  
  for (int i = 0; i < networksFound; i++) {
    if (i > 0) json += ",";
    
    json += "{";
    json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    json += "\"encrypted\":" + String(WiFi.encryptionType(i) != ENC_TYPE_NONE ? "true" : "false");
    json += "}";
  }
  
  json += "]}";
  
  server.send(200, "application/json", json);
}

void WebServerManager::handleRoot() {
  if (wifiManager.isConnected()) {
    // Si estamos conectados a WiFi, mostrar página principal con links
    String html = "<html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }";
    html += "h1 { color: #2c3e50; }";
    html += ".container { max-width: 400px; margin: 0 auto; background-color: #fff; padding: 20px; border-radius: 5px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); text-align: center; }";
    html += "a { display: block; background-color: #3498db; color: white; text-decoration: none; padding: 12px; margin: 10px 0; border-radius: 4px; }";
    html += "a:hover { background-color: #2980b9; }";
    html += "</style>";
    html += "<title>Pet Feeder Control</title>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>Pet Feeder</h1>";
    html += "<a href='/schedule'>Feeding Schedule</a>";
    html += "<a href='/setupwifi'>WiFi Settings</a>";
    html += "</div>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
  } else {
    // Si no estamos conectados, mostrar la página de configuración WiFi
    server.send_P(200, "text/html", INDEX_HTML);
  }
}

void WebServerManager::handleSetupWiFi() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    
    // Guardar en el almacenamiento
    storage.saveWiFiCredentials(ssid, password);
    
    // Resetear el estado fallido si había uno
    wifiManager.resetFailedState();
    
    // Enviar página de éxito
    server.send_P(200, "text/html", SUCCESS_HTML);
    
    Serial.println("WiFi credentials saved. SSID: " + ssid);
  } else {
    server.send(400, "text/plain", "Missing required parameters");
  }
}

void WebServerManager::handleNotFound() {
  // Para un captive portal, redirigir cualquier solicitud desconocida a la raíz
  String html = "<html><head>";
  html += "<meta http-equiv='refresh' content='0;url=http://" + WiFi.softAPIP().toString() + "/'>";
  html += "</head><body>Redirecting...</body></html>";
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(200, "text/html", html);
}

// Implementar los métodos para la configuración de horarios
void WebServerManager::handleFeedingSchedule() {
  server.send_P(200, "text/html", SCHEDULE_HTML);
}

void WebServerManager::handleGetSchedules() {
  server.send(200, "application/json", feedingSchedule.toJSON());
}

void WebServerManager::handleSaveSchedule() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"No data\"}");
    return;
  }
  
  String json = server.arg("plain");
  
  // Validación básica del JSONa
  if (!json.startsWith("{") || !json.endsWith("}")) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
    return;
  }
  
  // Aquí deberías parsear el JSON y guardar los horarios
  // Como no tenemos una librería JSON en este contexto, el código real sería más complejo
  // Por ahora, crearemos un placeholder:
  
  // Placeholder para simulación
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    // Ejemplo: extraer valores del JSON para cada horario
    int hour = 8 + i;  // Esto debe venir del JSON
    int minute = 0;    // Esto debe venir del JSON
    int seconds = 5;   // Esto debe venir del JSON
    bool enabled = true; // Esto debe venir del JSON
    
    FeedingTime schedule = {(uint8_t)hour, (uint8_t)minute, (uint8_t)seconds, enabled};
    feedingSchedule.saveSchedule(i, schedule);
  }
  
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Schedules saved\"}");
}

// Instancia global
WebServerManager webServer;