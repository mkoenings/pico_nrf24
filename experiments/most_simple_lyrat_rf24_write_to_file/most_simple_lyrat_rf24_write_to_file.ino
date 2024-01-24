/**
 * @file headphone.ino
 * @author Phil Schatzmann
 * @brief Headphone Detection Demo
 * @date 2021-12-10
 * 
 * @copyright Copyright (c) 2021
 */

#include "AudioKitHAL.h"

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>
#include <esp32-hal-timer.h>


#define SCK 14
#define MISO 12
#define MOSI 13
#define SS 15



// #define SCK 12
// #define MISO 13 
// #define MOSI 14
// #define SS 15 


#define CE_PIN   18
#define CSN_PIN 15


#define DEBUG_PIN 33

#define DEFAULT_CHANNELS 1

SPIClass * hspi_class = NULL;
const byte pipeaddress[][6] = { "1Ad", "2Ad", "3Ad" };
RF24 radio;

AudioKit kit;
const int BUFFER_SIZE = 32;
uint8_t buffer[BUFFER_SIZE];
uint8_t stereobuffer[BUFFER_SIZE *2];

#define RINGBUFFER_SIZE 512 // Adjust as needed
#define PAYLOAD_SIZE 32
#define INITIAL_FILL 16
#define READ_THRESHOLD 128

     
    
typedef struct {
    uint8_t buffer[RINGBUFFER_SIZE];
    int head;
    int tail;
} LRingBuffer;

LRingBuffer ringBuffer;


byte dataReceived[32]; // this must match dataToSend in the TX
//int dataReceived;
bool newData = false;


//////



//////


void setup() {
  //LOGLEVEL_AUDIOKIT = AudioKitInfo; 
  Serial.begin(115200);

  ///
  hspi_class = new SPIClass(HSPI);
  hspi_class->begin(SCK, MISO, MOSI,SS);
    radio.begin(hspi_class,CE_PIN, CSN_PIN);
    radio.setDataRate(RF24_2MBPS);    // Fast enough.. Better range
  radio.setAutoAck(false); // no Ack
  radio.setPALevel(RF24_PA_MAX);
  radio.setCRCLength(RF24_CRC_8);
  radio.disableCRC(); // no CRC
  radio.setChannel(125);            // 2.508 Ghz - Above most Wifi Channels
  radio.openReadingPipe(1, pipeaddress[1]);
  radio.startListening();
  Serial.println("Radio Configured Status");
  Serial.println();
  delay(500);
  radio.printDetails();
  Serial.println();


  ///////





  //////
  ///


  // open in write mode
 auto cfg = kit.defaultConfig(audiokit::AudioOutput);

  cfg.buffer_size = 32;
  cfg.buffer_count = 4;
  cfg.sample_rate = AUDIO_HAL_32K_SAMPLES;
  //cfg.sd_active = false;
  
  

 
  
  kit.begin(cfg);
   kit.setVolume(50);
   //pinMode(DEBUG_PIN, OUTPUT);
   // digitalWrite(DEBUG_PIN, LOW);
   preFillBuffer(); // Pre-fill the buffer before starting the main loop
}
    // Buffer for stereo data, twice the size of mono data
    uint8_t stereoData[PAYLOAD_SIZE];

void loop() {


getData();
showData();
}

long lastpackagetime = 0; 

void getData() {
    if ( radio.available() ) {
        //digitalWrite(DEBUG_PIN, HIGH );
        radio.read(&dataReceived, sizeof(dataReceived) );
        lastpackagetime = micros();
        //digitalWrite(DEBUG_PIN, LOW);
        //processMonoToStereo(dataReceived, stereoData, PAYLOAD_SIZE);
        //monoToStereo(dataReceived, stereobuffer, 32);

        ringBufferWrite(dataReceived);
           //Serial.println(ringBufferSize());
        
        newData = true;
        //Serial.println("data received and written to rbuffer");
    }
    //else
    //Serial.println("no data");
     
}

size_t      bytes_written;
uint8_t last_sample[2];
uint8_t data[PAYLOAD_SIZE];
void showData() {
  
    if (newData == true) {
        //Serial.print("Data received ");

        //for (int i = 0; i < 10; i++) {
          //Serial.print(dataReceived[i], HEX);
        //}
        ///Serial.print(ringBufferSize());
        //Serial.println(" ringBufferSize");
       // if (ringBufferSize() > READ_THRESHOLD) {
        
        ringBufferRead(data);

        //digitalWrite(DEBUG_PIN, LOW);


         kit.write(data, sizeof(data));
         last_sample[0] = data[30] ;
         last_sample[1] = data[31] ;
        delayMicroseconds(50);
        //Serial.println();
        newData = false;
    }
    //else
    //  if (micros() - lastpackagetime > 300)
    //  {
    //  // we are late, so we create a artificial buffer
    //  uint8_t fake_data[PAYLOAD_SIZE];
    // for (uint8_t i = 0; i<16; i+=2)
    // {
    //     fake_data[i]= last_sample[0];
    //     fake_data[i+1]= last_sample[1];

    // }
    //     digitalWrite(DEBUG_PIN, HIGH);
    //     kit.write(fake_data, sizeof(fake_data));
    //     lastpackagetime = micros();
    //  }
    
}


int ringBufferSize() {
    int size = ringBuffer.head - ringBuffer.tail;
    if (size < 0) {
        size += RINGBUFFER_SIZE;
    }
    return size;
}

void ringBufferWrite(uint8_t *data) {
    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        ringBuffer.buffer[ringBuffer.head] = data[i];
        ringBuffer.head = (ringBuffer.head + 1) % RINGBUFFER_SIZE;
        
        // Optional: Check for buffer overflow
        if (ringBuffer.head == ringBuffer.tail) {
           Serial.println("buffer full!");
        }
    }
}

void ringBufferRead(uint8_t *data) {
    if (isRingBufferEmpty()) {
        // Handle empty buffer case
        return;
    }

    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        data[i] = ringBuffer.buffer[ringBuffer.tail];
        ringBuffer.tail = (ringBuffer.tail + 1) % RINGBUFFER_SIZE;
    }
}
bool isRingBufferEmpty() {
    return ringBuffer.head == ringBuffer.tail;
}
void preFillBuffer() {
    uint8_t initialData[PAYLOAD_SIZE];

    for (int i=0; i < PAYLOAD_SIZE; i++)
    {
      initialData[i] = 0;
    }
    // Fill initialData with actual or dummy data

    for (int i = 0; i < INITIAL_FILL; i++) {
        ringBufferWrite(initialData);
    }
}


