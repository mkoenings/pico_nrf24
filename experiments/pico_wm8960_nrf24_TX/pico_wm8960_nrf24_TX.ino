/**
 * @file streams-generator-wm8990.ino
 * @author Phil Schatzmann
 * @brief Test sketch for wm8990
 * @copyright GPLv3
 */

#include "AudioTools.h"
#include "AudioLibs/WM8960Stream.h"

#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>
#include <SPI.h>


#define SCK 6   //lila
#define MISO 0  //grün
#define MOSI 7  //gelb
#define SS 1    //blau

#define CE_PIN 8    //orange
#define CSN_PIN 5  //blau


//#define DEFAULT_CHANNELS 1


AudioInfo info(44100, 1, 16);
WM8960Stream in;
I2SStream i2sStream; // Access I2S as stream
CsvOutput<int16_t> csvStream(Serial);
NBuffer<uint8_t> ring_buffer(1024,4);
QueueStream<uint8_t> queue(ring_buffer);
//StreamCopy copier(queue, in, 256); // copy i2sStream to csvStream
StreamCopy copier(csvStream, in, 256); // copy i2sStream to csvStream


const byte pipeaddress[][6] = { "1Ad", "2Ad", "3Ad" };
RF24 radio(CE_PIN, CSN_PIN);

// copies sound into i2s
// copied from nrf24 demo
const int BUFFER_SIZE = 256;
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

int16_t sine_wave[256];
//



// Arduino Setup
void setup(void) {
  // Open Serial
  Serial.begin(115200);
  while (!Serial)
    ;
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);

  // setup wire on pins 19 and 21
  Wire.setSCL(17);
  Wire.setSDA(16);
  Wire.begin();
  //Wire.setClock(10000);


 

  // start I2S
  Serial.println("starting I2S...");
  auto config = in.defaultConfig(audio_tools::RX_MODE);
  config.copyFrom(info);
  //config.wire = &Wire;
  // use default i2s pins



  config.pin_bck = 2;
  config.pin_ws = 3;
  config.pin_data = 4;
  //config.pin_data_rx = 5;
  config.buffer_count = 4;
  config.buffer_size = 512;
  config.vs1053_enable_pll = true;
  

Serial.println("starting SPI1...");


  //-----radio--------
  SPI.setRX(MISO);
  SPI.setCS(CSN_PIN);
  SPI.setSCK(SCK);
  SPI.setTX(MOSI);
  //SPI.begin(true);

  radio.begin();
  radio.setDataRate(RF24_2MBPS);  // Fast enough.. Better range
  radio.setAutoAck(false);        // no Ack
  radio.setPALevel(RF24_PA_MIN);
  radio.disableCRC();     // no CRC
  radio.setChannel(125);  // 2.508 Ghz - Above most Wifi Channels

    radio.openWritingPipe(pipeaddress[0]);
  radio.stopListening();


  Serial.println("Radio Configured Status");
  Serial.println();
  delay(500);
  //radio.printDetails();

char buffer[870] = {'\0'};
uint16_t used_chars = radio.sprintfPrettyDetails(buffer);
Serial.println(buffer);
Serial.print(F("strlen = "));
Serial.println(used_chars + 1); // +1 for c-strings' null terminating byte




  Serial.println();
  // //-------------
   



  if (!in.begin(config)) {
    Serial.println("error in config");
    stop();
  }
    csvStream.begin(info);
    //queue.begin();

delay(1000);

  Serial.println("before set volume out...");
  in.setVolume(1);

   Serial.println("after set volume out...");
  // Setup sine wave
  //sineWave.begin(info, N_B4);

  for (int i = 0; i < 256; i++) {
    sine_wave[i] = (int16_t)(sin(2 * PI * i / 256) * 10000);
  }

  Serial.println("started...");
}


void loop() {
 copier.copy();

  size_t len;
  //len =  queue.readBytes(buffer, BUFFER_SIZE);
//Serial.print(len);
//Serial.println("queue copy lenght");

  //if (queue.available()){
      //
  


  //radio.openWritingPipe(pipeaddress[1]);
  //delayMicroseconds(300);



  // send audio data in 8 packages a 32 byte to the FIFO buffers
  for (int i = 0; i < 8; i++) {
    //Serial.println(i) ;
    //
    //radio.writeFast(&sine_wave[i * 32],32);

    //printBuffer(32, i);
    // radio.writeFast(&buffer[i * 32], 32);
    //delayMicroseconds(90);
  }
  
}




void printBuffer(int len,int i) {
  // by default we get int16_t values on 2 channels = 4 bytes per frame
  int16_t *value_ptr = (int16_t *)buffer[i * 32];
  for (int j = 0; j < len / 2; j++) {
    Serial.println(*value_ptr++);
   // Serial.print(", ");
   // Serial.println(*value_ptr++);
  }
}





/////////////////////////////////
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