#include <Arduino.h>
#include "../include/config.h"
#include "../include/wifi_manager.h"
#include "../include/web_server.h"
#include "../include/lcd_ui.h"
#include "../include/state_machine.h"
#include "../include/storage.h"

// Definición del array de menú
const char* MENU_ITEMS[MENU_ITEMS_COUNT] = {
  "WiFi Status",
  "Reset WiFi",
  "Feeding Schedule",
  "Manual Feed",
  "Settings"
};

void setup() {
  Serial.begin(115200);
  Serial.println("\nPet Feeder Initializing...");
  
  // Inicializar componentes
  storage.begin();
  lcdUI.begin();
  
  // Establecer estado inicial
  stateMachine.setState(WELCOME_SCREEN);
}

void loop() {
  // Actualizar la máquina de estados
  stateMachine.update();
  
  // Procesar solicitudes web cuando sea necesario
  if (stateMachine.getState() == AP_MODE_ACTIVE) {
    webServer.handleClient();
  }
  
  delay(10);
}