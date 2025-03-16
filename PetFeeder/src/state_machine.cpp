#include "../include/state_machine.h"
#include "../include/lcd_ui.h"
#include "../include/wifi_manager.h"
#include "../include/web_server.h"
#include "../include/storage.h"

StateMachine::StateMachine() : 
  currentState(WELCOME_SCREEN),
  stateStartTime(0)
{
}

AppState StateMachine::getState() {
  return currentState;
}

void StateMachine::setState(AppState newState) {
  if (newState != currentState) {
    currentState = newState;
    stateStartTime = millis();
    onEnterState();
  }
}

void StateMachine::update() {
  switch (currentState) {
    case WELCOME_SCREEN:
      handleWelcomeScreen();
      break;
      
    case AP_MODE_ACTIVE:
      handleAPMode();
      break;
      
    case CONNECTING_WIFI:
      handleConnectingWiFi();
      break;
      
    case WIFI_CONNECTED:
      handleWiFiConnected();
      break;
      
    case MAIN_MENU:
      handleMainMenu();
      break;
  }
}

void StateMachine::onEnterState() {
  switch (currentState) {
    case WELCOME_SCREEN:
      lcdUI.displayMessage("Pet Feeder", "Welcome!");
      Serial.println("State: WELCOME_SCREEN");
      break;
      
    case AP_MODE_ACTIVE:
      Serial.println("State: AP_MODE_ACTIVE");
      // Realizar escaneo WiFi antes de iniciar el modo AP para evitar interferencias
      Serial.println("Scanning WiFi networks before AP setup...");
      webServer.scanWifiNetworks();
      
      // Configurar modo AP
      Serial.println("Setting up AP mode with SSID: " + String(AP_SSID));
      wifiManager.setupAP(AP_SSID);
      webServer.begin();
      webServer.setupCaptivePortal(WiFi.softAPIP());
      
      // Resetear variables estáticas para este estado
      static_cast<void>(handleAPMode());
      break;
      
    case CONNECTING_WIFI:
      Serial.println("State: CONNECTING_WIFI");
      // Iniciar conexión WiFi solo si no hemos excedido los intentos
      if (wifiManager.getConnectionAttempts() < MAX_CONNECTION_ATTEMPTS) {
        Serial.println("Starting WiFi connection to: " + storage.getSSID());
        wifiManager.connectToWiFi(storage.getSSID(), storage.getPassword());
      } else {
        Serial.println("Not attempting connection - max attempts reached");
        setState(AP_MODE_ACTIVE); // Cambiar inmediatamente a modo AP
      }
      
      // Resetear variables estáticas para este estado
      static_cast<void>(handleConnectingWiFi());
      break;
      
    case WIFI_CONNECTED:
      Serial.println("State: WIFI_CONNECTED");
      lcdUI.displayMessage("WiFi Connected!", wifiManager.getLocalIP());
      break;
      
    case MAIN_MENU:
      Serial.println("State: MAIN_MENU");
      lcdUI.displayMenu(lcdUI.getCurrentMenuPosition());
      break;
  }
}

void StateMachine::handleWelcomeScreen() {
  if (millis() - stateStartTime >= WELCOME_SCREEN_DURATION) {
    if (storage.isWiFiConfigured()) {
      setState(CONNECTING_WIFI);
    } else {
      setState(AP_MODE_ACTIVE);
    }
  }
}

void StateMachine::handleConnectingWiFi() {
  static unsigned long lastCheck = 0;
  static bool errorDisplayed = false;
  
  // Si es la primera vez que entramos a esta función desde un cambio de estado,
  // reiniciar las variables estáticas
  if (millis() - stateStartTime < 100) {
    lastCheck = 0;
    errorDisplayed = false;
  }
  
  // Verificar primero si ya alcanzamos el máximo de intentos
  if (wifiManager.getConnectionAttempts() >= MAX_CONNECTION_ATTEMPTS) {
    if (!errorDisplayed) {
      lcdUI.displayMessage("WiFi Error:", "Max attempts");
      Serial.println("Maximum connection attempts reached: " + String(MAX_CONNECTION_ATTEMPTS));
      Serial.println("Switching to AP mode after delay");
      errorDisplayed = true;
      
      // Esperar 3 segundos para mostrar el mensaje
      delay(3000);
      
      // Forzar reset de WiFi y volver a modo AP
      wifiManager.reset();
      setState(AP_MODE_ACTIVE);
    }
    return;
  }
  
  // Limitar frecuencia de verificación para no sobrecargar
  if (millis() - lastCheck < 1000) {
    return;
  }
  lastCheck = millis();
  
  // Verificar estado de la conexión
  int status = wifiManager.checkConnectionStatus();
  
  switch (status) {
    case WIFI_CONNECT_SUCCESS:
      Serial.println("WiFi connected successfully!");
      Serial.print("IP address: ");
      Serial.println(wifiManager.getLocalIP());
      Serial.print("Network: ");
      Serial.println(wifiManager.getSSID());
      setState(WIFI_CONNECTED);
      break;
      
    case WIFI_CONNECT_AP_NOT_FOUND:
    case WIFI_CONNECT_WRONG_PASSWORD:
      // Si ya mostró el error, verifica si debemos reintentar o cambiar a modo AP
      if (!errorDisplayed) {
        // Mostrar mensaje de error específico en LCD
        const char* errorMsg = wifiManager.getErrorString(status);
        lcdUI.displayMessage("WiFi Error:", errorMsg);
        Serial.printf("WiFi connection error: %s\n", errorMsg);
        errorDisplayed = true;
      }
      
      // Después de mostrar el error por 5 segundos
      if (millis() - stateStartTime >= 5000) {
        // Si aún no alcanzamos el máximo de intentos, reintentar
        if (wifiManager.getConnectionAttempts() < MAX_CONNECTION_ATTEMPTS) {
          Serial.printf("Retrying WiFi connection (attempt %d of %d)...\n", 
                        wifiManager.getConnectionAttempts() + 1, MAX_CONNECTION_ATTEMPTS);
          wifiManager.connectToWiFi(storage.getSSID(), storage.getPassword());
          stateStartTime = millis();
          errorDisplayed = false;
        }
        // Si ya alcanzamos el máximo, cambiar a modo AP
        else {
          lcdUI.displayMessage("Connection failed", "Switching to AP");
          Serial.println("Max connection attempts reached. Returning to AP mode.");
          delay(2000);
          
          // Forzar reset de WiFi y volver a modo AP
          wifiManager.reset();
          setState(AP_MODE_ACTIVE);
        }
      }
      break;
      
    case WIFI_CONNECT_IN_PROGRESS:
      // Aún intentando conectar
      if ((millis() / 500) % 2 == 0) {
        lcdUI.displayMessage("Connecting to", storage.getSSID());
      } else {
        lcdUI.displayMessage("Attempt " + String(wifiManager.getConnectionAttempts()), 
                            "of " + String(MAX_CONNECTION_ATTEMPTS));
      }
      
      // Timeout de conexión
      if (millis() - stateStartTime >= WIFI_CONNECT_TIMEOUT) {
        Serial.println("WiFi connection timeout after " + String(WIFI_CONNECT_TIMEOUT) + "ms");
        
        // Si ya alcanzamos el máximo, cambiar a modo AP
        if (wifiManager.getConnectionAttempts() >= MAX_CONNECTION_ATTEMPTS) {
          lcdUI.displayMessage("Timeout", "Switching to AP");
          Serial.println("Max connection attempts reached. Returning to AP mode.");
          delay(2000);
          
          // Forzar reset de WiFi y volver a modo AP
          wifiManager.reset();
          setState(AP_MODE_ACTIVE);
        } 
        // Si no, reintentar
        else {
          Serial.printf("Connection timeout. Retrying (%d of %d)...\n", 
                       wifiManager.getConnectionAttempts() + 1, MAX_CONNECTION_ATTEMPTS);
          wifiManager.connectToWiFi(storage.getSSID(), storage.getPassword());
          stateStartTime = millis();
        }
      }
      break;
  }
}

void StateMachine::handleAPMode() {
  static unsigned long lastToggle = 0;
  static bool showingIP = false;
  static unsigned long configTime = 0;
  static bool configChanged = false;
  
  // Si es la primera vez que entramos a esta función desde un cambio de estado
  if (millis() - stateStartTime < 100) {
    lastToggle = 0;
    showingIP = false;
    configTime = 0;
    configChanged = false;
  }
  
  bool failedConnection = wifiManager.hasConnectionFailed();
  
  // Alternar entre mensajes dependiendo del estado
  if (millis() - lastToggle > 3000) {
    showingIP = !showingIP;
    lastToggle = millis();
    
    if (failedConnection) {
      // Si venimos de una conexión fallida, mostrar mensaje de error
      if (showingIP) {
        lcdUI.displayMessage("WiFi Failed!", "Reconfigure WiFi");
      } else {
        lcdUI.displayMessage("Connect to AP:", AP_SSID);
      }
    } else {
      // Mostrar mensajes normales de configuración
      if (showingIP) {
        lcdUI.displayMessage("Connect to WiFi:", AP_SSID);
      } else {
        lcdUI.displayMessage("Open in browser:", wifiManager.getAPIP());
      }
    }
  }
  
  // Verificar si hay nuevos datos de configuración
  String newSSID = storage.getSSID();
  String oldSSID = failedConnection ? wifiManager.getSSID() : "";
  
  // Verificar si se configuró WiFi (solo si no venimos de un error o si las credenciales cambiaron)
  if (storage.isWiFiConfigured() && !configChanged && 
      (!failedConnection || (newSSID != oldSSID))) {
    
    if (failedConnection) {
      Serial.println("Detected new WiFi credentials after failure - will try to connect");
      wifiManager.resetFailedState();  // Resetear el estado fallido para intentar de nuevo
    }
    
    configChanged = true;
    configTime = millis();
    lcdUI.displayMessage("WiFi Configured", "Connecting soon...");
    Serial.println("WiFi configuration detected. Will connect in 8 seconds.");
  }
  
  // Esperar 8 segundos después de guardar la configuración antes de desconectar
  if (configChanged && (millis() - configTime > 8000)) {
    Serial.println("Timeout reached. Closing AP mode and attempting connection.");
    webServer.stop();
    wifiManager.disconnectAP();
    setState(CONNECTING_WIFI);
    configChanged = false;
  }
  
  // Verificar pulsaciones de botones para salir del modo AP manualmente
  int key = lcdUI.handleInput();
  if (key == SELECT_KEY) {
    if (failedConnection) {
      // Si venimos de un error, confirmar reintentar la conexión
      lcdUI.displayMessage("Retry WiFi?", "SELECT to confirm");
      delay(1500);
      
      // Esperar confirmación
      unsigned long startWait = millis();
      while (millis() - startWait < 3000) {
        if (lcdUI.handleInput() == SELECT_KEY) {
          lcdUI.displayMessage("Retrying WiFi", "Please wait...");
          delay(1000);
          wifiManager.resetFailedState();  // Resetear el estado fallido
          setState(CONNECTING_WIFI);
          return;
        }
      }
      lcdUI.displayMessage("Cancelled", "Staying in AP mode");
      delay(1500);
    } else {
      lcdUI.displayMessage("Exiting AP Mode", "Please wait...");
      Serial.println("User requested to exit AP mode");
      delay(1500);
      webServer.stop();
      wifiManager.disconnectAP();
      
      if (storage.isWiFiConfigured()) {
        setState(CONNECTING_WIFI);
      } else {
        setState(WELCOME_SCREEN);
      }
    }
  }
}

void StateMachine::handleWiFiConnected() {
  if (millis() - stateStartTime >= 3000) {
    setState(MAIN_MENU);
  }
}

void StateMachine::handleMainMenu() {
  int key = lcdUI.handleInput();
  int menuPos = lcdUI.getCurrentMenuPosition();
  
  lcdUI.displayMenu(menuPos);
  
  if (key == SELECT_KEY) {
    handleMenuSelection(menuPos);
  } else if (key == RIGHT_KEY) {
    // Opción para forzar el modo AP con botón derecho
    lcdUI.displayMessage("Force WiFi Setup?", "SELECT to confirm");
    
    // Esperar confirmación por 3 segundos
    unsigned long startWait = millis();
    while (millis() - startWait < 3000) {
      delay(100);
      if (lcdUI.handleInput() == SELECT_KEY) {
        // Resetear WiFi y forzar modo AP
        Serial.println("User forced WiFi setup mode");
        lcdUI.displayMessage("Entering", "Setup Mode...");
        delay(1000);
        wifiManager.reset();
        setState(AP_MODE_ACTIVE);
        return;
      }
    }
    
    // Si no se confirmó, volver al menú
    lcdUI.displayMenu(menuPos);
  }
}

void StateMachine::handleMenuSelection(int menuIndex) {
  switch (menuIndex) {
    case 0: { // WiFi Status
      if (wifiManager.isConnected()) {
        lcdUI.displayMessage("WiFi: " + wifiManager.getSSID(), wifiManager.getLocalIP());
        Serial.println("Showing WiFi status. Connected to: " + wifiManager.getSSID());
      } else {
        lcdUI.displayMessage("WiFi Status:", "Not connected");
        Serial.println("Showing WiFi status. Not connected.");
      }
      delay(2000); // Mostrar por 2 segundos
      lcdUI.displayMenu(lcdUI.getCurrentMenuPosition());
      break;
    }
      
    case 1: { // Reset WiFi
      lcdUI.displayMessage("Reset WiFi?", "SELECT to confirm");
      Serial.println("WiFi reset requested. Waiting for confirmation...");
      
      // Esperar confirmación por 3 segundos
      unsigned long startWait = millis();
      while (millis() - startWait < 3000) {
        delay(100);
        if (lcdUI.handleInput() == SELECT_KEY) {
          lcdUI.displayMessage("Resetting WiFi", "Please wait...");
          Serial.println("WiFi reset confirmed. Clearing settings.");
          storage.resetWiFiConfig();
          wifiManager.reset();
          delay(1500);
          setState(AP_MODE_ACTIVE);
          return;
        }
      }
      
      // Si no se confirmó, volver al menú
      Serial.println("WiFi reset not confirmed. Returning to menu.");
      lcdUI.displayMenu(lcdUI.getCurrentMenuPosition());
      break;
    }
      
    case 2: { // Feeding Schedule
      // Implementar la lógica del horario de alimentación
      lcdUI.displayMessage("Feeding Schedule", "Not implemented");
      Serial.println("Feeding Schedule selected (not implemented)");
      delay(2000);
      lcdUI.displayMenu(lcdUI.getCurrentMenuPosition());
      break;
    }
      
    case 3: { // Manual Feed
      // Implementar la lógica de alimentación manual
      lcdUI.displayMessage("Manual Feed", "Not implemented");
      Serial.println("Manual Feed selected (not implemented)");
      delay(2000);
      lcdUI.displayMenu(lcdUI.getCurrentMenuPosition());
      break;
    }
      
    case 4: { // Settings
      // Implementar la lógica de configuración
      lcdUI.displayMessage("Settings", "Not implemented");
      Serial.println("Settings selected (not implemented)");
      delay(2000);
      lcdUI.displayMenu(lcdUI.getCurrentMenuPosition());
      break;
    }
  }
}

// Instancia global
StateMachine stateMachine;