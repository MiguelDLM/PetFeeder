#include "../include/servo_controller.h"

ServoController::ServoController() {
  active = false;
  feedStartTime = 0;
  feedDuration = 0;
}

void ServoController::begin(int pin) {
  servoPin = pin;
  servo.attach(servoPin);
  servo.write(REST_POSITION);
  Serial.println("Servo initialized on pin " + String(servoPin));
  delay(500);
}

void ServoController::feed(int seconds) {
  if (active) {
    return;  // Evitar interrumpir una alimentación en curso
  }
  
  // Convertir segundos a milisegundos
  feedDuration = seconds * 1000;
  
  // Iniciar alimentación
  servo.write(FEED_POSITION);
  active = true;
  feedStartTime = millis();
  
  Serial.println("Feeding started for " + String(seconds) + " seconds");
}

void ServoController::update() {
  if (!active) {
    return;
  }
  
  // Verificar si el tiempo de alimentación ha terminado
  if (millis() - feedStartTime >= feedDuration) {
    stop();
  }
}

void ServoController::stop() {
  if (!active) {
    return;
  }
  
  // Detener alimentación
  servo.write(REST_POSITION);
  active = false;
  
  Serial.println("Feeding stopped");
}

bool ServoController::isActive() {
  return active;
}

// Instancia global
ServoController servoController;