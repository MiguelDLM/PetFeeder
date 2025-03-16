#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <Servo.h>
#include <Arduino.h>

class ServoController {
public:
  ServoController();
  
  // Inicializa el servo
  void begin(int pin);
  
  // Activa la alimentación
  void feed(int seconds);
  
  // Comprueba si el servo está activo y lo actualiza
  void update();
  
  // Detiene la alimentación inmediatamente
  void stop();
  
  // Verifica si el servo está actualmente activo
  bool isActive();

private:
  Servo servo;
  int servoPin;
  bool active;
  unsigned long feedStartTime;
  unsigned long feedDuration;
  
  // Posición para dispensar comida
  const int FEED_POSITION = 180;
  
  // Posición de reposo
  const int REST_POSITION = 0;
};

extern ServoController servoController;

#endif // SERVO_CONTROLLER_H