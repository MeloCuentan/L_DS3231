#include "L_DS3231.h"

L_DS3231::L_DS3231() {}

bool L_DS3231::begin(TwoWire &wirePort) {
    _wire = &wirePort;
    _wire->begin();
    for (int i = 0; i < 3; i++) {
        _wire->beginTransmission(DS3231_ADDRESS);
        if (_wire->endTransmission() == 0) return true;
        delay(10);
    }
    return false;
}

void L_DS3231::setDateTime(int8_t date, int8_t month, int16_t year, int8_t hour, int8_t min, int8_t sec) {
  uint8_t dayOfWeek = calculateDayOfWeek(date, month, year);
    uint8_t data[8] = {
        SECONDS_REG,
        decToBcd(sec),
        decToBcd(min),
        decToBcd(hour),
        decToBcd(dayOfWeek),
        decToBcd(date),
        decToBcd(month),
        decToBcd(year - 2000)};
    _wire->beginTransmission(DS3231_ADDRESS);
    _wire->write(data, sizeof(data));
    _wire->endTransmission();
}

bool L_DS3231::getDateTime(int8_t &date, int8_t &month, int16_t &year, int8_t &hour, int8_t &min, int8_t &sec) {
    _wire->beginTransmission(DS3231_ADDRESS);
    _wire->write(SECONDS_REG);
    if (_wire->endTransmission() != 0) return false;

    _wire->requestFrom(DS3231_ADDRESS, 7);
    if (_wire->available() < 7) return false;

    sec = bcdToDec(_wire->read() & 0x7F);
    min = bcdToDec(_wire->read());
    hour = bcdToDec(_wire->read() & 0x3F);
    _wire->read(); // Ignorar día de semana
    date = bcdToDec(_wire->read());
    month = bcdToDec(_wire->read());
    year = bcdToDec(_wire->read()) + 2000;

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

String L_DS3231::padZero(uint8_t number) {
    if (number < 10) {
        return "0" + String(number);
    }
    return String(number);
}

String L_DS3231::getDateFormatted() {
    // Formato: "DD/MM/YY"
    return padZero(dia) + "/" + padZero(mes) + "/" + String(anio).substring(2);
}

String L_DS3231::getTimeFormatted() {
    // Formato: "HH:MM:SS"
    return padZero(hora) + ":" + padZero(minuto) + ":" + padZero(segundo);
}

void L_DS3231::enable32kHz(bool enable) {
    uint8_t control = readRegister(CONTROL_REG);
    writeRegister(CONTROL_REG, enable ? (control | 0x80) : (control & 0x7F));
}

void L_DS3231::enableSquareWave(bool enable, uint8_t frequency) {
    uint8_t control = readRegister(CONTROL_REG) & 0xF1;
    if (enable) control |= (frequency & 0x03) << 1;
    writeRegister(CONTROL_REG, control);
}

uint8_t L_DS3231::calculateDayOfWeek(uint8_t day, uint8_t month, uint16_t year) {
    if (month < 3) { month += 12; year--; }
    uint8_t century = year / 100;
    uint8_t yearOfCentury = year % 100;
    uint8_t dayOfWeek = (day + (13 * (month + 1)) / 5 + yearOfCentury + (yearOfCentury / 4) + (century / 4) + (5 * century)) % 7;
    return (dayOfWeek == 0) ? 7 : dayOfWeek;
}

String L_DS3231::getDayOfWeekStr() {
    static String days[7] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};
    uint8_t day = calculateDayOfWeek(dia, mes, anio);
    return (day >= 1 && day <= 7) ? days[day - 1] : "Err";
}

void L_DS3231::setDateTimeFromCompileTime() {
    const char* compDate = __DATE__;
    const char* compTime = __TIME__;
    char monthStr[4];
    monthStr[0] = compDate[0];
    monthStr[1] = compDate[1];
    monthStr[2] = compDate[2];
    monthStr[3] = '\0';

    const char* months[] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
    uint8_t month = 0;
    for (int i = 0; i < 12; i++) {
        if (strcmp(monthStr, months[i]) == 0) {
            month = i + 1;
            break;
        }
    }

    setDateTime(
        atoi(&compDate[4]),  // Día
        month,               // Mes
        atoi(&compDate[7]),  // Año (4 dígitos)
        atoi(&compTime[0]),  // Hora
        atoi(&compTime[3]),  // Minuto
        atoi(&compTime[6])   // Segundo
    );
}

uint8_t L_DS3231::readRegister(uint8_t reg) {
    _wire->beginTransmission(DS3231_ADDRESS);
    _wire->write(reg);
    _wire->endTransmission();
    _wire->requestFrom(DS3231_ADDRESS, 1);
    return _wire->read();
}

void L_DS3231::writeRegister(uint8_t reg, uint8_t value) {
    _wire->beginTransmission(DS3231_ADDRESS);
    _wire->write(reg);
    _wire->write(value);
    _wire->endTransmission();
}
