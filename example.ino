#include <Arduino.h>
#include <Wire.h>
#include "L_DS3231.h"

// Crear un objeto de la clase DS3231
L_DS3231 rtc;

void setup()
{
  Serial.begin(9600);

  // Inicializar el DS3231 con manejo de errores
  if (!rtc.begin())
  {
    Serial.println("Error: No se pudo inicializar el DS3231");
    while (1)
      ; // Detener ejecución si hay error
  }

  // Configurar la fecha y hora (solo la primera vez o si hay un reset)
  rtc.setDateTime(15, 4, 2025, 12, 30, 0); // día, mes, año, hora, minuto, segundo

  // Activar el pin 32K
  rtc.enable32kHz(true);

  // Configurar el pin SQW/INT para salida de onda cuadrada a 1Hz
  rtc.enableSquareWave(true, 1); // 1Hz

  Serial.println("DS3231 configurado correctamente");
}

void loop()
{

  uint16_t year;
  uint8_t month, date, hour, min, sec;

  // Leer la hora y fecha
  rtc.getDateTime(date, month, year, hour, min, sec);

  // Usar el nuevo método con validación
  if (rtc.getDateTime(date, month, year, hour, min, sec))
  {
    // Solo entra aquí si el año es mayor a 2000
    Serial.print("Fecha válida: ");

    if (date < 10)
      Serial.print("0");
    Serial.print(date);

    Serial.print("/");

    if (month < 10)
      Serial.print("0");
    Serial.print(month);

    Serial.print("/");

    Serial.print(year);

    Serial.print(" ");

    if (hour < 10)
      Serial.print("0");
    Serial.print(hour);

    Serial.print(":");

    if (min < 10)
      Serial.print("0");
    Serial.print(min);

    Serial.print(":");

    if (sec < 10)
      Serial.print("0");
    Serial.print(sec);

    Serial.print("  Día: ");
    Serial.println(rtc.getDayOfWeekStr()); // Día de la semana
  }
  else
  {
    Serial.println("Fecha no válida. Se han perdido los datos");
  }
  delay(1000);
}