#include "../include/lcd_ui.h"

LCDUI::LCDUI() : 
  lcd(PIN_RS, PIN_EN, PIN_D4, PIN_D5, PIN_D6, PIN_D7),
  menuPosition(0)
{
}

void LCDUI::begin() {
  lcd.begin(16, 2);
  lcd.clear();
}

int LCDUI::handleInput() {
  int key = getKeyID();
  
  if (key == UP_KEY) {
    if (menuPosition > 0) {
      menuPosition--;
    }
  } else if (key == DOWN_KEY) {
    if (menuPosition < MENU_ITEMS_COUNT - 1) {
      menuPosition++;
    }
  }
  
  return key;
}

void LCDUI::displayMessage(const String& line1, const String& line2) {
  lcd.clear();
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void LCDUI::displayMenu(int selectedIndex) {
  lcd.clear();
  lcd.print(">");
  lcd.print(MENU_ITEMS[selectedIndex]);
  
  // Mostrar elemento siguiente si no es el último
  if (selectedIndex < MENU_ITEMS_COUNT - 1) {
    lcd.setCursor(0, 1);
    lcd.print(MENU_ITEMS[selectedIndex + 1]);
  }
}

int LCDUI::getCurrentMenuPosition() {
  return menuPosition;
}

void LCDUI::setMenuPosition(int position) {
  if (position >= 0 && position < MENU_ITEMS_COUNT) {
    menuPosition = position;
  }
}

int LCDUI::getKeyID() {
  int aRead = analogRead(A0);

  if (aRead > 500) return 0;                // no key is pressed
  if (aRead > 420) return SELECT_KEY;       // select key
  if (aRead > 350) return LEFT_KEY;         // left key
  if (aRead > 300) return DOWN_KEY;         // down key
  if (aRead > 150) return UP_KEY;           // up key
  if (aRead < 30)  return RIGHT_KEY;        // right key

  return 0;
}

// Instancia global
LCDUI lcdUI;