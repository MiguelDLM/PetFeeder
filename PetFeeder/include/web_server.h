#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <Arduino.h>

class WebServerManager {
public:
  WebServerManager();
  
  // Inicia el servidor web
  void begin();
  
  // Maneja las solicitudes de clientes
  void handleClient();
  
  // Configura el portal cautivo
  void setupCaptivePortal(IPAddress apIP);
  
  // Detiene el servidor web
  void stop();
  
  // Escanea las redes WiFi disponibles
  void scanWifiNetworks();
  
  // Configura las rutas del servidor
  void setupRoutes();
  
private:
  ESP8266WebServer server;
  DNSServer dnsServer;
  
  
  // Maneja la solicitud a la página principal
  void handleRoot();
  
  // Maneja la solicitud de configuración WiFi
  void handleSetupWiFi();
  
  // Maneja la solicitud de escaneo WiFi
  void handleWifiScan();
  
  // Maneja solicitudes no encontradas (para portal cautivo)
  void handleNotFound();

  void handleConnectionSuccess();
  
  // Contenido HTML en memoria de programa
  static const char INDEX_HTML[] PROGMEM;
  static const char SUCCESS_HTML[] PROGMEM;
  
  // Maneja la página de horarios de alimentación
  void handleFeedingSchedule();
  
  // Maneja la API para guardar horarios
  void handleSaveSchedule();
  
  // Maneja la API para obtener horarios
  void handleGetSchedules();
  
  // Añadir este campo privado
  static const char SCHEDULE_HTML[] PROGMEM;
};

extern WebServerManager webServer;

#endif // WEB_SERVER_H