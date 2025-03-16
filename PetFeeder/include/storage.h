#ifndef STORAGE_H
#define STORAGE_H

#include <EEPROM.h>
#include <Arduino.h>
#include "config.h"

class Storage {
public:
  Storage();
  
  // Inicializa la EEPROM
  void begin();
  
  // Verifica si las credenciales WiFi están configuradas
  bool isWiFiConfigured();

  // En la clase Storage, añadir:
  bool hasConfigChanged(const String& oldSSID, const String& oldPassword);
  
  // Guarda las credenciales WiFi
  void saveWiFiCredentials(const String& ssid, const String& password);
  
  // Carga el SSID almacenado
  String getSSID();
  
  // Carga la contraseña almacenada
  String getPassword();
  
  // Elimina la configuración WiFi
  void resetWiFiConfig();
  
  // Guarda el byte de configuración
  void commit();
  
  // Verifica si es primera ejecución (para inicializar valores por defecto)
  bool isFirstRun();
  
  // Marca que ya no es primera ejecución
  void setInitialized();
private:
  // Lee una cadena desde la EEPROM
  String readString(int startAddr, int maxLength);
  
  // Escribe una cadena en la EEPROM
  void writeString(int startAddr, const String& value);
};

extern Storage storage;

#endif // STORAGE_H