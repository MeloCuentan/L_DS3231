#include "L_DS3231.h"

L_DS3231::L_DS3231() {}

bool L_DS3231::begin(TwoWire &wirePort)
{
  _wire = &wirePort;
  _wire->begin();
  for (int i = 0; i < 3; i++)
  {
    _wire->beginTransmission(DS3231_ADDRESS);
    if (_wire->endTransmission() == 0)
      return true;
    delay(10);
  }
  return false;
}

void L_DS3231::setDateTime(int8_t date, int8_t month, int16_t year, int8_t hour, int8_t min, int8_t sec)
{
  uint8_t dayOfWeek = calculateDayOfWeek(date, month, year);

  uint8_t centuryBit = (year >= 2100) ? 0x80 : 0x00;
  uint8_t monthWithCentury = decToBcd(month) | centuryBit;

  uint8_t data[8] = {
      SECONDS_REG,
      decToBcd(sec),
      decToBcd(min),
      decToBcd(hour),
      decToBcd(dayOfWeek),
      decToBcd(date),
      monthWithCentury,
      decToBcd(year % 100)};

  _wire->beginTransmission(DS3231_ADDRESS);
  _wire->write(data, sizeof(data));
  _wire->endTransmission();
}

bool L_DS3231::getDateTime(int8_t &date, int8_t &month, int16_t &year, int8_t &hour, int8_t &min, int8_t &sec)
{
  _wire->beginTransmission(DS3231_ADDRESS);
  _wire->write(SECONDS_REG);
  if (_wire->endTransmission() != 0)
    return false;

  _wire->requestFrom(DS3231_ADDRESS, 7);
  if (_wire->available() < 7)
    return false;

  sec = bcdToDec(_wire->read() & 0x7F);
  min = bcdToDec(_wire->read());
  hour = bcdToDec(_wire->read() & 0x3F);
  _wire->read(); // Ignorar día de semana
  date = bcdToDec(_wire->read());

  uint8_t monthRaw = _wire->read();
  bool centuryBit = monthRaw & 0x80;
  month = bcdToDec(monthRaw & 0x1F);

  uint8_t yearRaw = _wire->read();
  year = bcdToDec(yearRaw) + (centuryBit ? 2100 : 2000);

  // Actualizar variables internas
  dia = date;
  mes = month;
  hora = hour;
  minuto = min;
  segundo = sec;
  anio = year;
  diaSemana = calculateDayOfWeek(date, month, year);

  return true;
}

String L_DS3231::padZero(uint8_t number)
{
  if (number < 10)
  {
    return "0" + String(number);
  }
  return String(number);
}

String L_DS3231::getDateFormatted()
{
  // Formato: "DD/MM/YY"
  return padZero(dia) + "/" + padZero(mes) + "/" + String(anio).substring(2);
}

String L_DS3231::getTimeFormatted()
{
  // Formato: "HH:MM:SS"
  return padZero(hora) + ":" + padZero(minuto) + ":" + padZero(segundo);
}

void L_DS3231::enable32kHz(bool enable)
{
  uint8_t control = readRegister(CONTROL_REG);
  writeRegister(CONTROL_REG, enable ? (control | 0x80) : (control & 0x7F));
}

void L_DS3231::enableSquareWave(bool enable, uint8_t frequency)
{
  uint8_t control = readRegister(CONTROL_REG) & 0xF1;
  if (enable)
    control |= (frequency & 0x03) << 1;
  writeRegister(CONTROL_REG, control);
}

uint8_t L_DS3231::calculateDayOfWeek(uint8_t day, uint8_t month, uint16_t year)
{
  if (month < 3)
  {
    month += 12;
    year--;
  }
  uint8_t century = year / 100;
  uint8_t yearOfCentury = year % 100;
  uint8_t dayOfWeek = (day + (13 * (month + 1)) / 5 + yearOfCentury + (yearOfCentury / 4) + (century / 4) + (5 * century)) % 7;
  return (dayOfWeek == 0) ? 7 : dayOfWeek;
}

void L_DS3231::setDayName(uint8_t dayIndex, const char *newName)
{
  if (dayIndex < 7 && strlen(newName) <= 4)
  { // Máximo 4 caracteres (ej: "Vier")
    strncpy(customDays[dayIndex], newName, 4);
    customDays[dayIndex][4] = '\0'; // Asegura terminación nula
  }
}

void L_DS3231::setMonthName(uint8_t monthIndex, const char *newName)
{
  if (monthIndex < 12 && strlen(newName) <= 4)
  {
    strncpy(customMonths[monthIndex], newName, 4);
    customMonths[monthIndex][4] = '\0';
  }
}

const char *L_DS3231::getDayName(uint8_t dayIndex)
{
  return (dayIndex < 7) ? customDays[dayIndex] : nullptr;
}

const char *L_DS3231::getMonthName(uint8_t monthIndex)
{
  return (monthIndex < 12) ? customMonths[monthIndex] : nullptr;
}

String L_DS3231::getDayOfWeekStr()
{
  uint8_t day = calculateDayOfWeek(dia, mes, anio);
  return (day >= 1 && day <= 7) ? days[day - 1] : "Err";
}

void L_DS3231::setDateTimeFromCompileTime()
{
  const char *compDate = __DATE__;
  const char *compTime = __TIME__;
  char monthStr[4];
  monthStr[0] = compDate[0];
  monthStr[1] = compDate[1];
  monthStr[2] = compDate[2];
  monthStr[3] = '\0';

  uint8_t month = 0;
  for (int i = 0; i < 12; i++)
  {
    if (strcmp(monthStr, months[i]) == 0)
    {
      month = i + 1;
      break;
    }
  }

  setDateTime(
      atoi(&compDate[4]), // Día
      month,              // Mes
      atoi(&compDate[7]), // Año (4 dígitos)
      atoi(&compTime[0]), // Hora
      atoi(&compTime[3]), // Minuto
      atoi(&compTime[6])  // Segundo
  );
}

uint8_t L_DS3231::readRegister(uint8_t reg)
{
  _wire->beginTransmission(DS3231_ADDRESS);
  _wire->write(reg);
  _wire->endTransmission();
  _wire->requestFrom(DS3231_ADDRESS, 1);
  return _wire->read();
}

void L_DS3231::writeRegister(uint8_t reg, uint8_t value)
{
  _wire->beginTransmission(DS3231_ADDRESS);
  _wire->write(reg);
  _wire->write(value);
  _wire->endTransmission();
}
