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
#define MISO 4  //grün
#define MOSI 7  //gelb
#define SS 5    //blau

#define CE_PIN 8    //orange
#define CSN_PIN 5  //blau


//#define DEFAULT_CHANNELS 1


AudioInfo info(32000, 1, 16);
SineWaveGenerator<int16_t> sineWave(32000);     // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<int16_t> sound(sineWave);  // Stream generated from sine wave
WM8960Stream out;
StreamCopy copier(out, sound);


const byte pipeaddress[][6] = { "1Ad", "2Ad", "3Ad" };
RF24 radio(CE_PIN, CSN_PIN);

// copies sound into i2s
// copied from nrf24 demo
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


//



// Arduino Setup
void setup(void) {
  // Open Serial
  Serial.begin(115200);
  while (!Serial)
    ;
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);

  // setup wire on pins 19 and 21
  Wire.setSCL(1);
  Wire.setSDA(0);
  Wire.begin();
  Wire.setClock(10000);


  // start I2S
  Serial.println("starting I2S...");
  auto config = out.defaultConfig(TX_MODE);
  config.copyFrom(info);
  config.wire = &Wire;
  // use default i2s pins

  config.pin_bck = 2;
  config.pin_ws = 3;
  config.pin_data = 9;
  config.buffer_count = 4;
  config.buffer_size = 256;
  config.is_master
  config.pin_mck = 10;
  config.mck_multiplier = 256;
  //config.pin_data_rx = 5;
  config.vs1053_enable_pll = true;

Serial.println("starting SPI1...");


  //-----radio--------
  SPI.setRX(MISO);
  SPI.setCS(CSN_PIN);
  SPI.setSCK(SCK);
  SPI.setTX(MOSI);
  SPI.begin(true);

  radio.begin();
  radio.setDataRate(RF24_2MBPS);  // Fast enough.. Better range
  radio.setAutoAck(false);        // no Ack
  radio.setPALevel(RF24_PA_MIN);
  radio.disableCRC();     // no CRC
  radio.setChannel(125);  // 2.508 Ghz - Above most Wifi Channels
  radio.openReadingPipe(1, pipeaddress[1]);
  radio.startListening();
  Serial.println("Radio Configured Status");
  Serial.println();
  delay(500);
  radio.printDetails();
  Serial.println();
  //-------------
   



  if (!out.begin(config)) {
    Serial.println("error in config");
    stop();
  }
delay(1000);

  Serial.println("before set volume out...");
  //out.setVolumeOut(.25f);
   Serial.println("after set volume out...");
  // Setup sine wave
  //sineWave.begin(info, N_B4);
  Serial.println("started...");
}

// Arduino loop - copy sound to out
void loop() {
  //copier.copy();

  

getData();
showData();


}



void getData() {
    if ( radio.available() ) {
        radio.read(&dataReceived, sizeof(dataReceived) );
         Serial.println("data available");
        //processMonoToStereo(dataReceived, stereoData, PAYLOAD_SIZE);
        
        ringBufferWrite(dataReceived);
        newData = true;
       // Serial.println("data received and written to rbuffer");
    }
    else
    {Serial.println("no data");
    }
     
}
void printBuffer(int len) {
  // by default we get int16_t values on 2 channels = 4 bytes per frame
  int16_t *value_ptr = (int16_t *)dataReceived;
  for (int j = 0; j < len / 2; j++) {
    Serial.println(*value_ptr++);
   // Serial.print(", ");
   // Serial.println(*value_ptr++);
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
        printBuffer(2);
         out.write(data, sizeof(data));
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