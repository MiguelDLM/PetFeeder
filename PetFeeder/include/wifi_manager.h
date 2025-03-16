#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include <Arduino.h>

// Códigos de error para conexión WiFi
#define WIFI_CONNECT_SUCCESS 0
#define WIFI_CONNECT_IN_PROGRESS 1
#define WIFI_CONNECT_WRONG_PASSWORD 2
#define WIFI_CONNECT_AP_NOT_FOUND 3
#define WIFI_CONNECT_TIMEOUT 4000
#define WIFI_CONNECT_FAIL 5

// Configuración
#define MAX_CONNECTION_ATTEMPTS 3

class WiFiManager {
public:
  WiFiManager();
  
  // Configura el modo AP
  void setupAP(const String& ssid, const String& password = "");
  
  // Intenta conectarse a un WiFi con las credenciales dadas
  int connectToWiFi(const String& ssid, const String& password);
  
  // Verifica el estado actual de la conexión
  int checkConnectionStatus();
  
  // Verifica si está conectado a una red WiFi
  bool isConnected();
  
  // Obtiene la dirección IP local (modo estación)
  String getLocalIP();
  
  // Obtiene la dirección IP del punto de acceso (modo AP)
  String getAPIP();
  
  // Obtiene el SSID de la red conectada
  String getSSID();
  
  // Obtiene el número de intentos de conexión realizados
  int getConnectionAttempts();
  
  // Obtiene el último error de conexión
  int getLastError();
  
  // Obtiene el mensaje de error correspondiente al código
  const char* getErrorString(int error);
  
  // Desconecta y apaga el WiFi
  void disconnect();
  
  // Desconecta el modo AP
  void disconnectAP();
  
  // Reinicia la configuración WiFi
  void reset();

  void handleConnectionSuccess();

  // Marca la configuración WiFi como fallida (no intentar reconexión)
  void markConnectionAsFailed();
  
  // Verifica si la conexión con las credenciales actuales ha fallado
  bool hasConnectionFailed();
  
  // Resetea el estado de fallo cuando hay nuevas credenciales
  void resetFailedState();

  void setupRoutes();
  
private:
  int lastConnectionError;
  int connectionAttempts;
  String currentSSID;
  String currentPassword;
  bool connectionFailed;  // Nuevo flag para indicar que se ha intentado y fallado
};

extern WiFiManager wifiManager;

#endif // WIFI_MANAGER_H