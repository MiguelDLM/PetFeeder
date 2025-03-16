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
  
private:
  ESP8266WebServer server;
  DNSServer dnsServer;
  
  // Configura las rutas del servidor
  void setupRoutes();
  
  // Maneja la solicitud a la página principal
  void handleRoot();
  
  // Maneja la solicitud de configuración WiFi
  void handleSetupWiFi();
  
  // Maneja la solicitud de escaneo WiFi
  void handleWifiScan();
  
  // Maneja solicitudes no encontradas (para portal cautivo)
  void handleNotFound();
  
  // Contenido HTML en memoria de programa
  static const char INDEX_HTML[] PROGMEM;
  static const char SUCCESS_HTML[] PROGMEM;
};

extern WebServerManager webServer;

#endif // WEB_SERVER_H