/**
 * @file headphone.ino
 * @author Phil Schatzmann
 * @brief Headphone Detection Demo
 * @date 2021-12-10
 * 
 * @copyright Copyright (c) 2021
 */
#include "AudioKitHAL.h"
#include "SineWaveGenerator.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>


#define SCK 14
#define MISO 12
#define MOSI 13
#define SS 15




#define CE_PIN   18
#define CSN_PIN 15

#define DEFAULT_CHANNELS 1

SPIClass * hspi_class = NULL;
const byte pipeaddress[][6] = { "1Ad", "2Ad", "3Ad" };
RF24 radio;

AudioKit kit;
SineWaveGenerator wave;
const int BUFFER_SIZE = 32;
uint8_t buffer[BUFFER_SIZE];


#define RINGBUFFER_SIZE 1024 // Adjust as needed
#define PAYLOAD_SIZE 32
#define INITIAL_FILL 8
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


void setup() {
  LOGLEVEL_AUDIOKIT = AudioKitInfo; 
  Serial.begin(115200);

  ///
  hspi_class = new SPIClass(HSPI);
  hspi_class->begin(SCK, MISO, MOSI,SS);
    radio.begin(hspi_class,CE_PIN, CSN_PIN);
    radio.setDataRate(RF24_2MBPS);    // Fast enough.. Better range
  radio.setAutoAck(false); // no Ack
  radio.setPALevel(RF24_PA_MIN);
  radio.disableCRC(); // no CRC
  radio.setChannel(117);              // 2.508 Ghz - Above most Wifi Channels
   radio.openReadingPipe(1, pipeaddress[1]);
  radio.startListening();
  Serial.println("Radio Configured Status");
  Serial.println();
  delay(500);
  radio.printDetails();
  Serial.println();
  ///


  // open in write mode
  auto cfg = kit.defaultConfig();

  cfg.buffer_size = 32;
  cfg.buffer_count = 20;
  cfg.sample_rate = AUDIO_HAL_32K_SAMPLES;
   // cfg.setAudioInfo(audioInfo);

  
  kit.begin(cfg);
  kit.setVolume(50);

  // 1000 hz
  wave.setFrequency(1000);
  wave.setSampleRate(cfg.sampleRate());
   preFillBuffer(); // Pre-fill the buffer before starting the main loop
}

void loop() {

  /////
  //size_t l = wave.read(buffer, BUFFER_SIZE);
  //Serial.println(l );

  //kit.write(buffer, l);
////



getData();
showData();

}
void processMonoToStereo(uint8_t *monoData, uint8_t *stereoData, int monoDataSize) {
    for (int i = 0; i < monoDataSize; i++) {
        // Duplicate each mono sample for left and right channels
        stereoData[2 * i] = monoData[i];     // Left channel
        stereoData[2 * i + 1] = monoData[i]; // Right channel
    }
}


    // Buffer for stereo data, twice the size of mono data
    uint8_t stereoData[PAYLOAD_SIZE * 2];

void getData() {
    if ( radio.available() ) {
        radio.read(&dataReceived, sizeof(dataReceived) );
        //processMonoToStereo(dataReceived, stereoData, PAYLOAD_SIZE);
        
        ringBufferWrite(dataReceived);
        newData = true;
        //Serial.println("data received and written to rbuffer");
    }
    //else
    //Serial.println("no data");
     
}
void printBuffer(int len) {
  // by default we get int16_t values on 2 channels = 4 bytes per frame
  int16_t *value_ptr = (int16_t *)dataReceived;
  for (int j = 0; j < len / 4; j++) {
    Serial.print(*value_ptr++);
    Serial.print(", ");
    Serial.println(*value_ptr++);
  }
}

void showData() {
  size_t      bytes_written;
    if (newData == true) {
        //Serial.print("Data received ");
        //i2s_write(I2S_NUM, dataReceived, ESPNOW_MSG_LEN, &bytes_written, portNO_WAIT);
        //for (int i = 0; i < 10; i++) {
          //Serial.print(dataReceived[i], HEX);
        //}
        //Serial.print(ringBufferSize());
        //Serial.println(" ringBufferSize");
       // if (ringBufferSize() > READ_THRESHOLD) {
        uint8_t data[PAYLOAD_SIZE];
        ringBufferRead(data);
        //printBuffer(32);
         kit.write(data, sizeof(data));
        //Serial.println();
        newData = false;
    //}

       
    }
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
            // Handle buffer overflow (e.g., advance tail or error handling)
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



