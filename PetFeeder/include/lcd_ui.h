#ifndef LCD_UI_H
#define LCD_UI_H

#include <LiquidCrystal.h>
#include <Arduino.h>
#include "config.h"

class LCDUI {
public:
  LCDUI();
  
  // Inicializa la pantalla LCD
  void begin();
  
  // Maneja la entrada del usuario (botones)
  int handleInput();
  
  // Muestra un mensaje en el LCD
  void displayMessage(const String& line1, const String& line2);
  
  // Muestra el menú principal
  void displayMenu(int selectedIndex);
  
  // Obtiene el índice de menú seleccionado actualmente
  int getCurrentMenuPosition();
  
  // Establece la posición actual del menú
  void setMenuPosition(int position);
  
private:
  LiquidCrystal lcd;
  int menuPosition;
  
  // Lee la entrada del teclado
  int getKeyID();
};

extern LCDUI lcdUI;

#endif // LCD_UI_H