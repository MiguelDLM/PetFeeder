#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Definiciones de pines
#define PIN_RS D1
#define PIN_EN D2
#define PIN_D4 D4
#define PIN_D5 D5
#define PIN_D6 D6
#define PIN_D7 D7

// Definiciones de teclas
#define SELECT_KEY 1
#define LEFT_KEY   2
#define DOWN_KEY   3
#define UP_KEY     4
#define RIGHT_KEY  5

// Configuración EEPROM
#define EEPROM_SIZE 512
#define SSID_ADDR 0
#define PASS_ADDR 100
#define WIFI_CONFIGURED_ADDR 400

// Estados de la aplicación
enum AppState {
  WELCOME_SCREEN,
  AP_MODE_ACTIVE,
  CONNECTING_WIFI,
  WIFI_CONNECTED,
  MAIN_MENU
};

// Nombre de la red AP para configuración
#define AP_SSID "PetFeeder-Setup"

// Tiempo de visualización de la pantalla de bienvenida (ms)
#define WELCOME_SCREEN_DURATION 3000

// Número de elementos en el menú principal
#define MENU_ITEMS_COUNT 5

// Elementos del menú principal
extern const char* MENU_ITEMS[MENU_ITEMS_COUNT];

#endif // CONFIG_H