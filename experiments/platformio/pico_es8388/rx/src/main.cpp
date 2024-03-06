#include <Arduino.h>

#include <string.h>

void setup()
{
    Serial.begin(115200);
    while (!Serial);    /*! @warning This program will wait at this line until a serial monitor is connected! */
    Serial.printf("Startin Pico Tx\n");

    pinMode(LED_BUILTIN, OUTPUT);
}


void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}
