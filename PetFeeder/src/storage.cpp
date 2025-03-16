#include "../include/storage.h"

Storage::Storage() {
}

void Storage::begin() {
  EEPROM.begin(EEPROM_SIZE);
}

bool Storage::isWiFiConfigured() {
  return (EEPROM.read(WIFI_CONFIGURED_ADDR) == 1);
}

void Storage::saveWiFiCredentials(const String& ssid, const String& password) {
  // Guardar SSID y contraseña
  writeString(SSID_ADDR, ssid);
  writeString(PASS_ADDR, password);
  
  // Marcar como configurado
  EEPROM.write(WIFI_CONFIGURED_ADDR, 1);
  
  // Confirmar cambios
  commit();
  
  Serial.println("WiFi credentials saved to EEPROM");
}

String Storage::getSSID() {
  return readString(SSID_ADDR, 50);
}

String Storage::getPassword() {
  return readString(PASS_ADDR, 50);
}

void Storage::resetWiFiConfig() {
  // Marcar como no configurado
  EEPROM.write(WIFI_CONFIGURED_ADDR, 0);
  
  // Confirmar cambios
  commit();
  
  Serial.println("WiFi configuration reset");
}

void Storage::commit() {
  EEPROM.commit();
}

String Storage::readString(int startAddr, int maxLength) {
  String result = "";
  
  for (int i = 0; i < maxLength; i++) {
    char c = EEPROM.read(startAddr + i);
    if (c == '\0') break;
    result += c;
  }
  
  return result;
}

void Storage::writeString(int startAddr, const String& value) {
  // Limpiar área de memoria
  for (int i = 0; i < 50; i++) {
    EEPROM.write(startAddr + i, '\0');
  }
  
  // Escribir nueva cadena
  for (unsigned int i = 0; i < value.length(); i++) {
    EEPROM.write(startAddr + i, value[i]);
  }
}

// Añadir este método para verificar si la configuración ha cambiado
bool Storage::hasConfigChanged(const String& oldSSID, const String& oldPassword) {
  String currentSSID = getSSID();
  String currentPassword = getPassword();
  
  return (currentSSID != oldSSID || currentPassword != oldPassword);
}


#define INITIALIZED_ADDR 500
#define INITIALIZED_VALUE 42

bool Storage::isFirstRun() {
  return (EEPROM.read(INITIALIZED_ADDR) != INITIALIZED_VALUE);
}

void Storage::setInitialized() {
  EEPROM.write(INITIALIZED_ADDR, INITIALIZED_VALUE);
  EEPROM.commit();
}
// Instancia global
Storage storage;