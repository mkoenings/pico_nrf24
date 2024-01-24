/**
 * @file input.ino
 * @author Phil Schatzmann
 * @brief Input of audio data from the AudioKit microphones
 * @date 2021-12-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "AudioKitHAL.h"
#include "AudioTools.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>

// lyrat
#define SCK 12
#define MISO 13
#define MOSI 14
#define SS 15

#define CE_PIN 23   //SCL
#define CSN_PIN 15  //sda


//olimex
// #define SCK 14
// #define MISO 12
// #define MOSI 13
// #define SS 15

// #define CE_PIN 18
// #define CSN_PIN 15

#define DEFAULT_CHANNELS 1

SPIClass *hspi_class = NULL;
const byte pipeaddress[][6] = { "1Ad", "2Ad", "3Ad" };
RF24 radio;

uint8_t databufr[32];


AudioKit kit;
const int BUFFER_SIZE = 256;
uint8_t buffer[BUFFER_SIZE];


void printBuffer(int len) {
  // by default we get int16_t values on 2 channels = 4 bytes per frame
  int16_t *value_ptr = (int16_t *)buffer;
  for (int j = 0; j < len / 4; j++) {
    Serial.print(*value_ptr++);
    Serial.print(", ");
    Serial.println(*value_ptr++);
  }
}

void setup() {
  Serial.begin(115200);

  hspi_class = new SPIClass(HSPI);
  hspi_class->begin(SCK, MISO, MOSI, SS);
  radio.begin(hspi_class, CE_PIN, CSN_PIN);
  radio.setDataRate(RF24_2MBPS);  // Fast enough.. Better range
  radio.setAutoAck(false);        // no Ack
  radio.setPALevel(RF24_PA_MIN);
  radio.disableCRC();  // no CRC
  radio.setChannel(117);
  delay(50);

  radio.openWritingPipe(pipeaddress[0]);
  radio.stopListening();
  delay(100);


  
  AudioInfo audioInfo;
  audioInfo.channels = 1;
  audioInfo.sample_rate = 32000;
  audioInfo.bits_per_sample = 16;

  auto cfg = kit.defaultConfig(AudioInput);
  cfg.adc_input = AUDIO_HAL_ADC_INPUT_LINE2;  // microphone?
  cfg.sample_rate = AUDIO_HAL_32K_SAMPLES;

  kit.begin(cfg);
}

void loop() {
  size_t len = kit.read(buffer, BUFFER_SIZE);



  radio.openWritingPipe(pipeaddress[1]);
  delayMicroseconds(150);



  // send audio data in 8 packages a 32 byte to the FIFO buffers
  for (int i = 0; i < 8; i++) {
    //Serial.println(i) ;
 

    //printBuffer(8);
    radio.writeFast(&buffer[i * 32], 32);
    delayMicroseconds(90);
  }
}


