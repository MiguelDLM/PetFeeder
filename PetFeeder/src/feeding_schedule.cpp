#include "../include/feeding_schedule.h"
#include "../include/storage.h"
#include "../include/servo_controller.h"
#include <time.h>

FeedingSchedule::FeedingSchedule() {
  lastCheck = 0;
}

void FeedingSchedule::begin() {
  // Leer horarios guardados en EEPROM
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    schedules[i] = getSchedule(i);
  }
  
  // Inicializar con un horario predeterminado vacío si es la primera vez
  if (storage.isFirstRun()) {
    for (int i = 0; i < MAX_SCHEDULES; i++) {
      FeedingTime emptySchedule = {0, 0, 0, false};
      saveSchedule(i, emptySchedule);
    }
  }
  
  Serial.println("Feeding schedule initialized");
}

void FeedingSchedule::saveSchedule(int index, const FeedingTime& schedule) {
  if (index < 0 || index >= MAX_SCHEDULES) {
    return;
  }
  
  schedules[index] = schedule;
  
  // Guardar en EEPROM
  int addr = SCHEDULE_START_ADDR + (index * sizeof(FeedingTime));
  EEPROM.put(addr, schedule);
  EEPROM.commit();
  
  Serial.println("Feeding schedule saved at index " + String(index));
}

FeedingTime FeedingSchedule::getSchedule(int index) {
  if (index < 0 || index >= MAX_SCHEDULES) {
    FeedingTime emptySchedule = {0, 0, 0, false};
    return emptySchedule;
  }
  
  // Leer de EEPROM
  FeedingTime schedule;
  int addr = SCHEDULE_START_ADDR + (index * sizeof(FeedingTime));
  EEPROM.get(addr, schedule);
  
  return schedule;
}

void FeedingSchedule::setEnabled(int index, bool enabled) {
  if (index < 0 || index >= MAX_SCHEDULES) {
    return;
  }
  
  schedules[index].enabled = enabled;
  saveSchedule(index, schedules[index]);
}

void FeedingSchedule::checkSchedule() {
  // Limitar la frecuencia de verificación a una vez por minuto
  if (millis() - lastCheck < 60000) {
    return;
  }
  lastCheck = millis();
  
  // Obtener la hora actual
  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);
  
  int currentHour = timeinfo->tm_hour;
  int currentMinute = timeinfo->tm_min;
  
  // Verificar cada horario habilitado
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    if (schedules[i].enabled && 
        schedules[i].hour == currentHour && 
        schedules[i].minute == currentMinute) {
      
      // Si el servo no está activo, iniciar la alimentación
      if (!servoController.isActive()) {
        servoController.feed(schedules[i].seconds);
        Serial.println("Scheduled feeding triggered at " + 
                      String(currentHour) + ":" + String(currentMinute) + 
                      " for " + String(schedules[i].seconds) + " seconds");
      }
    }
  }
}

int FeedingSchedule::getEnabledCount() {
  int count = 0;
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    if (schedules[i].enabled) {
      count++;
    }
  }
  return count;
}

String FeedingSchedule::toJSON() {
  String json = "{\"schedules\":[";
  
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    if (i > 0) json += ",";
    
    json += "{";
    json += "\"index\":" + String(i) + ",";
    json += "\"hour\":" + String(schedules[i].hour) + ",";
    json += "\"minute\":" + String(schedules[i].minute) + ",";
    json += "\"seconds\":" + String(schedules[i].seconds) + ",";
    json += "\"enabled\":" + String(schedules[i].enabled ? "true" : "false");
    json += "}";
  }
  
  json += "]}";
  return json;
}

// Instancia global
FeedingSchedule feedingSchedule;