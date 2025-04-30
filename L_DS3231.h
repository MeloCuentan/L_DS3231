#ifndef L_DS3231_H
#define L_DS3231_H

#include <Wire.h>
#include <Arduino.h>

#define DS3231_ADDRESS 0x68

// Registros
enum {
    SECONDS_REG = 0x00,
//    MINUTES_REG = 0x01,
//    HOURS_REG = 0x02,
//    DAY_REG = 0x03,
//    DATE_REG = 0x04,
//    MONTH_REG = 0x05,
//    YEAR_REG = 0x06,
    CONTROL_REG = 0x0E
};

class L_DS3231 {
public:
    L_DS3231();

    bool begin(TwoWire &wirePort = Wire);  // Inicialización con manejo de errores

    void setDateTime(uint8_t date, uint8_t month, uint16_t year, uint8_t hour, uint8_t min, uint8_t sec);
    bool getDateTime(uint8_t &date, uint8_t &month, uint16_t &year, uint8_t &hour, uint8_t &min, uint8_t &sec);

    // Nuevos métodos para formato string (usando String de Arduino)
    String getDateFormatted();  // Formato "DD/MM/YY"
    String getTimeFormatted();  // Formato "HH:MM:SS"

    void enable32kHz(bool enable);
    void enableSquareWave(bool enable, uint8_t frequency = 1);

    String getDayOfWeekStr();
    void setDateTimeFromCompileTime();

private:
    uint8_t dia, mes, hora, minuto, segundo, diaSemana;
    uint16_t anio; 
   
    TwoWire *_wire;
    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t calculateDayOfWeek(uint8_t day, uint8_t month, uint16_t year);
    inline uint8_t decToBcd(uint8_t val) { return ((val / 10) << 4) | (val % 10); }
    inline uint8_t bcdToDec(uint8_t val) { return ((val >> 4) * 10) + (val & 0x0F); }
    
    // Función auxiliar para formato
    String padZero(uint8_t number);
};

#endif