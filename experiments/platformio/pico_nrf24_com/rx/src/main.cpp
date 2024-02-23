#include <Arduino.h>

/**
 *  @brief Raspberry Pico + nRF24 Receiver code.
 *          The device listens for ESB packets, and prints the received ones via Serial.
 *  @date 23.Feb.2024.
 *  @warning Line: while (!Serial);
 *              will cause the program to hang (wait) until a serial monitor is connected.
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


#define SCK       6
#define MISO      4
#define MOSI      7
#define SS        5
#define CE_PIN    8
#define CSN_PIN   5
RF24 radio(CE_PIN, CSN_PIN);

// the setup routine runs once when you press reset:
void setup()
{
    // initialize the digital pin as an output.
    Serial.begin(115200);
    while (!Serial);    /*! @warning This program will wait at this line until a serial monitor is connected! */
    Serial.printf("Startin Pico Rx\n");

    SPI.setRX(MISO);
    SPI.setCS(CSN_PIN);
    SPI.setSCK(SCK);
    SPI.setTX(MOSI);
    SPI.begin(true);

    radio.begin();
    radio.setChannel(125);
    radio.setDataRate(RF24_2MBPS);
    radio.setPALevel(RF24_PA_HIGH);
    radio.disableCRC();
    radio.openReadingPipe(1, 0x1234567890LL);
    radio.startListening();

    pinMode(LED_BUILTIN, OUTPUT);
}

typedef struct
{
    uint32_t    idx;
    char        text[240];
}data_packet_t;

void loop()
{
    if(radio.available())
    {
        char text[32] = {0};
        data_packet_t data_packet = {0};
        radio.read(&data_packet, sizeof(data_packet));
        Serial.printf("%d ", data_packet.idx);
        Serial.println(data_packet.text);
    }
}
