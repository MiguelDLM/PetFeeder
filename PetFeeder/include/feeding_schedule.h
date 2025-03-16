#ifndef FEEDING_SCHEDULE_H
#define FEEDING_SCHEDULE_H

#include <Arduino.h>

#define MAX_SCHEDULES 5
#define SCHEDULE_START_ADDR 200

struct FeedingTime {
  uint8_t hour;        // Hora (0-23)
  uint8_t minute;      // Minuto (0-59)
  uint8_t seconds;     // Duración en segundos (0-255)
  bool enabled;        // Si este horario está activado
};

class FeedingSchedule {
public:
  FeedingSchedule();
  
  // Inicializa el programa de alimentación
  void begin();
  
  // Guarda un horario de alimentación
  void saveSchedule(int index, const FeedingTime& schedule);
  
  // Obtiene un horario específico
  FeedingTime getSchedule(int index);
  
  // Habilita o deshabilita un horario
  void setEnabled(int index, bool enabled);
  
  // Verifica la hora actual y activa la alimentación si coincide con un horario
  void checkSchedule();
  
  // Devuelve el número de horarios configurados y habilitados
  int getEnabledCount();
  
  // Serializa horarios a JSON para la web
  String toJSON();
  
private:
  FeedingTime schedules[MAX_SCHEDULES];
  unsigned long lastCheck;
};

extern FeedingSchedule feedingSchedule;

#endif // FEEDING_SCHEDULE_H