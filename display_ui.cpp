#include "display_ui.h"
#include "globals.h"

void ui_lcdPrintFloatOneDecimal(float x) {
  int x10 = (int)(x * 10.0f + (x >= 0.0f ? 0.5f : -0.5f));
  if (x10 < 0) {
    lcd.print('-');
    x10 = -x10;
  }
  lcd.print(x10 / 10);
  lcd.print('.');
  lcd.print(x10 % 10);
}

int ui_modeFromCurrent(float raw_mA) {
  if (raw_mA < -5.0f) {
    return 0;
  }
  if (raw_mA > 5.0f) {
    return 1;
  }
  return 2;
}

const char *ui_modeLabel(int m) {
  switch (m) {
    case 0:
      return "Dischg";
    case 1:
      return "Charge";
    default:
      return "Idle";
  }
}

void ui_render(float percent, float current, float tempDisp, int modeDisplay, bool stopChargeWarn) {
  lcd.setCursor(0, 0);
  lcd.print("P:");
  ui_lcdPrintFloatOneDecimal(percent);
  lcd.print("% ");
  lcd.print("I:");
  if (current >= 100.0f || current <= -100.0f) {
    lcd.print((int)(current + (current >= 0.0f ? 0.5f : -0.5f)));
  } else {
    ui_lcdPrintFloatOneDecimal(current);
  }
  lcd.print("mA ");
  lcd.print("   ");

  lcd.setCursor(0, 1);
  if (stopChargeWarn) {
    lcd.print("RELAY CUT TEMP! ");
  } else {
    lcd.print(ui_modeLabel(modeDisplay));
    lcd.print(" ");
    ui_lcdPrintFloatOneDecimal(tempDisp);
    lcd.print("C   ");
  }
}
