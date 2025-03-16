#include <Arduino.h>
#include "../include/config.h"
#include "../include/wifi_manager.h"
#include "../include/web_server.h"
#include "../include/lcd_ui.h"
#include "../include/state_machine.h"
#include "../include/storage.h"
#include "../include/servo_controller.h"
#include "../include/feeding_schedule.h"

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
  
  // Inicializar el servo en pin D3
  servoController.begin(D3);
  
  // Inicializar horarios de alimentación
  feedingSchedule.begin();
  
  // Configurar NTP para obtener la hora actual cuando se conecte a WiFi
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  
  // Establecer estado inicial
  stateMachine.setState(WELCOME_SCREEN);
}

// Modificar la función loop()
void loop() {
  // Actualizar la máquina de estados
  stateMachine.update();
  
  // Procesar solicitudes web cuando sea necesario
  if (stateMachine.getState() == AP_MODE_ACTIVE || 
      stateMachine.getState() == WIFI_CONNECTED) {
    webServer.handleClient();
  }
  
  // Verificar si hay alimentación programada
  if (stateMachine.getState() == WIFI_CONNECTED) {
    feedingSchedule.checkSchedule();
  }
  
  // Actualizar el servo si está activo
  servoController.update();
  
  delay(10);
}