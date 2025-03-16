#include "../include/wifi_manager.h"
#include <storage.h>

// Códigos de error para la conexión WiFi
const char* WiFiManager::getErrorString(int error) {
  switch (error) {
    case WIFI_CONNECT_WRONG_PASSWORD:
      return "Incorrect password";
    case WIFI_CONNECT_AP_NOT_FOUND:
      return "Network not found";
    case WIFI_CONNECT_TIMEOUT:
      return "Connection timeout";
    case WIFI_CONNECT_FAIL:
      return "Connection failed";
    default:
      return "Unknown error";
  }
}

WiFiManager::WiFiManager() {
  lastConnectionError = WIFI_CONNECT_SUCCESS;
  connectionAttempts = 0;
  connectionFailed = false;
}

void WiFiManager::setupAP(const String& ssid, const String& password) {
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_AP);
  
  if (password.length() > 0) {
    WiFi.softAP(ssid.c_str(), password.c_str());
  } else {
    WiFi.softAP(ssid.c_str());
  }
  
  Serial.print("AP Mode Started. SSID: ");
  Serial.println(ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP().toString());
}

int WiFiManager::connectToWiFi(const String& ssid, const String& password) {
  // Si la conexión ya ha fallado con estas credenciales, no intentar de nuevo
  if (connectionFailed && ssid == currentSSID && password == currentPassword) {
    Serial.println("Connection already failed with these credentials. Not trying again.");
    lastConnectionError = WIFI_CONNECT_FAIL;
    return lastConnectionError;
  }
  
  Serial.printf("Connecting to WiFi: %s\n", ssid.c_str());
  
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  // Reiniciar contador si es un nuevo intento de conexión o nuevas credenciales
  if (currentSSID != ssid || currentPassword != password) {
    Serial.println("New credentials detected. Resetting connection attempts.");
    connectionAttempts = 0;
    currentSSID = ssid;
    currentPassword = password;
    connectionFailed = false;  // Resetear estado de fallo con nuevas credenciales
  }
  
  connectionAttempts++;
  Serial.printf("Connection attempt #%d of %d\n", connectionAttempts, MAX_CONNECTION_ATTEMPTS);
  
  // En el método connect() o en la función que maneja los intentos de conexión:
  
  // Después de que falle el tercer intento:
  // Reemplazar el bloque problemático:
  if (connectionAttempts >= MAX_CONNECTION_ATTEMPTS) {
    Serial.println("Connection marked as failed. Won't auto-retry until reset or new credentials.");
    Serial.println("Maximum connection attempts reached: " + String(MAX_CONNECTION_ATTEMPTS));
    
    // Borrar la configuración WiFi para evitar el bucle
    storage.resetWiFiConfig();
    Serial.println("WiFi configuration reset after multiple failed attempts");
    
    // Marcar como fallido para prevenir reintentos
    markConnectionAsFailed();
    
    lastConnectionError = WIFI_CONNECT_FAIL;
    return lastConnectionError;
  }
  
  lastConnectionError = WIFI_CONNECT_SUCCESS;
  return lastConnectionError;
}

int WiFiManager::checkConnectionStatus() {
  wl_status_t status = WiFi.status();
  
  switch (status) {
    case WL_CONNECTED:
      // Conexión exitosa, resetear todo
      connectionAttempts = 0;
      connectionFailed = false;
      lastConnectionError = WIFI_CONNECT_SUCCESS;
      return WIFI_CONNECT_SUCCESS;
      
    case WL_NO_SSID_AVAIL:
      lastConnectionError = WIFI_CONNECT_AP_NOT_FOUND;
      Serial.println("Error: WiFi network not found");
      
      // Si alcanza el máximo de intentos, marcar como fallido
      if (connectionAttempts >= MAX_CONNECTION_ATTEMPTS) {
        markConnectionAsFailed();
      }
      
      return WIFI_CONNECT_AP_NOT_FOUND;
      
    case WL_CONNECT_FAILED:
      lastConnectionError = WIFI_CONNECT_WRONG_PASSWORD;
      Serial.println("Error: WiFi connection failed, possibly wrong password");
      
      // Si alcanza el máximo de intentos, marcar como fallido
      if (connectionAttempts >= MAX_CONNECTION_ATTEMPTS) {
        markConnectionAsFailed();
      }
      
      return WIFI_CONNECT_WRONG_PASSWORD;
      
    default:
      // Aún intentando conectar o error diferente
      return WIFI_CONNECT_IN_PROGRESS;
  }
}

void WiFiManager::markConnectionAsFailed() {
  if (!connectionFailed) {
    connectionFailed = true;
    Serial.println("Connection marked as failed. Won't auto-retry until reset or new credentials.");
  }
}

bool WiFiManager::hasConnectionFailed() {
  return connectionFailed;
}

void WiFiManager::resetFailedState() {
  connectionFailed = false;
  connectionAttempts = 0;
  Serial.println("Connection failed state has been reset. Will attempt to connect again.");
}

bool WiFiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getLocalIP() {
  return WiFi.localIP().toString();
}

String WiFiManager::getAPIP() {
  return WiFi.softAPIP().toString();
}

String WiFiManager::getSSID() {
  return WiFi.SSID();
}

int WiFiManager::getConnectionAttempts() {
  return connectionAttempts;
}

int WiFiManager::getLastError() {
  return lastConnectionError;
}

void WiFiManager::disconnect() {
  WiFi.disconnect(true);
}

void WiFiManager::disconnectAP() {
  WiFi.softAPdisconnect(true);
}

void WiFiManager::reset() {
  disconnect();
  WiFi.mode(WIFI_OFF);
  connectionAttempts = 0;
  connectionFailed = false;
  lastConnectionError = WIFI_CONNECT_SUCCESS;
  delay(100);
}

// Instancia global
WiFiManager wifiManager;