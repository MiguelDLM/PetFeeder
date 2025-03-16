#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "config.h"

class StateMachine {
public:
  StateMachine();
  
  // Obtiene el estado actual
  AppState getState();
  
  // Establece un nuevo estado
  void setState(AppState newState);
  
  // Actualiza la máquina de estados
  void update();
  
  // Ejecuta acciones al entrar en un nuevo estado
  void onEnterState();
  
  // Maneja la selección del menú principal
  void handleMenuSelection(int menuIndex);
  
private:
  AppState currentState;
  unsigned long stateStartTime;
  
  // Maneja el estado de la pantalla de bienvenida
  void handleWelcomeScreen();
  
  // Maneja el estado del modo AP
  void handleAPMode();
  
  // Maneja el estado de conexión WiFi
  void handleConnectingWiFi();
  
  // Maneja el estado de WiFi conectado
  void handleWiFiConnected();
  
  // Maneja el estado del menú principal
  void handleMainMenu();
};

extern StateMachine stateMachine;

#endif // STATE_MACHINE_H