#include <Arduino.h>

/**
 *  @brief Raspberry Pico + nRF24 Tx code.
 *          The device should blink onboard LED and send data periodically.
 *  @date 23.Feb.2024.
 *  @warning Line: while (!Serial);
 *              will cause the program to hang (wait) until a serial monitor is connected.
 */

#include <string.h>
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

/** @brief A custom nRF24 data structure. */
typedef struct
{
    uint32_t    idx;
    uint8_t     text[240];
}data_packet_t;

data_packet_t data_packet;

static void tx_payload_init(data_packet_t* p_data)
{
    p_data->idx = 0;
    memcpy(p_data->text, "Test", sizeof("Test"));
}

// the setup routine runs once when you press reset:
void setup()
{
    tx_payload_init(&data_packet);
    Serial.begin(115200);
    while (!Serial);    /*! @warning This program will wait at this line until a serial monitor is connected! */
    Serial.printf("Startin Pico Tx\n");

    SPI.setRX(MISO);
    SPI.setCS(CSN_PIN);
    SPI.setSCK(SCK);
    SPI.setTX(MOSI);
    SPI.begin(true);

    radio.begin();
    radio.setPayloadSize(250);
    Serial.printf("Max payload size ");
    Serial.println(radio.getPayloadSize());
    radio.setChannel(125);
    radio.setDataRate(RF24_2MBPS);
    radio.setPALevel(RF24_PA_HIGH);
    radio.disableCRC();
    radio.enableAckPayload();
    radio.openWritingPipe(0x1234567890LL);

    pinMode(LED_BUILTIN, OUTPUT);
}


// the loop routine runs over and over again forever:
void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(500);               // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(500);               // wait for a second
    radio.write(&data_packet, sizeof(data_packet));
    data_packet.idx++;
}
