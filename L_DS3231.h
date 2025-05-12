#ifndef L_DS3231_H
#define L_DS3231_H

#include <Wire.h>
#include <Arduino.h>

#define DS3231_ADDRESS 0x68

// Registros
enum
{
  SECONDS_REG = 0x00,
  //    MINUTES_REG = 0x01,
  //    HOURS_REG = 0x02,
  //    DAY_REG = 0x03,
  //    DATE_REG = 0x04,
  //    MONTH_REG = 0x05,
  //    YEAR_REG = 0x06,
  CONTROL_REG = 0x0E
};

class L_DS3231
{
public:
  L_DS3231();

  bool begin(TwoWire &wirePort = Wire); // Inicialización con manejo de errores

  void setDateTime(int8_t date, int8_t month, int16_t year, int8_t hour, int8_t min, int8_t sec);
  bool getDateTime(int8_t &date, int8_t &month, int16_t &year, int8_t &hour, int8_t &min, int8_t &sec);

  // Nuevos métodos para formato string (usando String de Arduino)
  String getDateFormatted(); // Formato "DD/MM/YY"
  String getTimeFormatted(); // Formato "HH:MM:SS"

  void enable32kHz(bool enable);
  void enableSquareWave(bool enable, uint8_t frequency = 1);

  void setDayName(uint8_t dayIndex, const char *newName);
  void setMonthName(uint8_t monthIndex, const char *newName);
  const char *getDayName(uint8_t dayIndex);
  const char *getMonthName(uint8_t monthIndex);

  String getDayOfWeekStr();
  void setDateTimeFromCompileTime();

private:
  int8_t dia, mes, hora, minuto, segundo, diaSemana;
  int16_t anio;

  TwoWire *_wire;
  uint8_t readRegister(uint8_t reg);
  void writeRegister(uint8_t reg, uint8_t value);
  uint8_t calculateDayOfWeek(uint8_t day, uint8_t month, uint16_t year);
  inline uint8_t decToBcd(uint8_t val) { return ((val / 10) << 4) | (val % 10); }
  inline uint8_t bcdToDec(uint8_t val) { return ((val >> 4) * 10) + (val & 0x0F); }

  const char *days[7] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};
  const char *months[12] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
  
  char customDays[7][5] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};
  char customMonths[12][5] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};

  // Función auxiliar para formato
  String padZero(uint8_t number);
};

#endif
