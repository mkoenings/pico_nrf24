// SimpleTx - the master or the transmitter

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>

#include "AudioTools.h"
#include "AudioLibs/Communication.h"

uint16_t sample_rate = 44100;
uint8_t channels = 1;  // The stream will have 2 channels
SineWaveGenerator<int16_t> sineWave( 32000);  // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<int16_t> sound( sineWave); // Stream generated from sine wave

RingBuffer<uint8_t> ring_buffer(512);
QueueStream<uint8_t> queue(ring_buffer);

StreamCopy copier(queue, sound, 64 );  // copies sound into i2s
const char *peers[] = {"A8:48:FA:0B:93:01"};

#define SCK 26 //I2S DSDIN
#define MISO 35 //ASDOUT 
#define MOSI 25 //LRCK
#define SS 18 //sda

#define CE_PIN   23 //SCL
#define CSN_PIN  18 //sda

const byte slaveAddress[5] = {'R','x','A','A','A'};


RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

char dataToSend[10] = "Message 0";
char txNum = '0';


unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 1; // send once per second


SPIClass * hspi_class = NULL;
byte filter;
byte saw;
byte sqr;
int sine;
int angle;
bool rslt;
void setup() {
  
  queue.begin();
  SPI.begin(SCK, MISO, MOSI, SS);



  // Serial Set-up
  Serial.begin(9600);

   Serial.print("MOSI: ");
  Serial.println(MOSI);
  Serial.print("MISO: ");
  Serial.println(MISO);
  Serial.print("SCK: ");
  Serial.println(SCK);
  Serial.print("SS: ");
  Serial.println(SS);  

  delay(1000);
  printf_begin();
  delay(1000);
  Serial.println("SimpleTx Starting");
  Serial.println();

  // Radio Set-up
  //radio.begin(hspi_class);
  radio.begin();

  Serial.println("Radio Initial Status");
  Serial.println();
  radio.printDetails();
  Serial.println();
  delay(1000);
  radio.setDataRate(RF24_250KBPS);    // Fast enough.. Better range
  radio.setPALevel(RF24_PA_MIN);
  radio.setChannel(108);              // 2.508 Ghz - Above most Wifi Channels
  radio.setRetries(3,15);             // setRetries(delay,count) - max 15 for both delay and count
  // delay: How long to wait between each retry, in multiples of 250us, max is 15. 0 means 250us, 15 means 4000us. 
  // count: How many retries before giving up, max 15 
  radio.openWritingPipe(slaveAddress);
  radio.stopListening();
  Serial.println("Radio Configured Status");
  Serial.println();
  delay(1000);
  radio.printDetails();
  Serial.println();
  delay(1000);
}

//====================

void loop() {
  //copier.copy();
   //sine_gen();
   //Serial.println(sine);
    currentMillis = millis();
    if (currentMillis - prevMillis >= txIntervalMillis) {
       // send();
        
       //rslt = radio.write( &sine, sizeof(sine) );
        prevMillis = millis();
    }
  
}

//====================

void send() {

    bool rslt;
    uint8_t bytes[64];
    if (queue.available())
    {
    queue.readBytes(bytes, 64);
    rslt = radio.writeFast( &bytes, sizeof(bytes) );
        // Always use sizeof() as it gives the size as the number of bytes.
        // For example if dataToSend was an int sizeof() would correctly return 2

    //Serial.print("Data Sent ");
    Serial.print(bytes[0]);
    if (rslt) {
        //Serial.println("Acknowledge received");
        updateMessage();
    }
    else {
        Serial.println("  Tx failed");
    }
    }
}

//================

void updateMessage() {
        // so you can see that new data is being sent
    txNum += 1;
    if (txNum > '9') {
        txNum = '0';
    }
    dataToSend[8] = txNum;
}




void sine_gen() {            //SINE WAVE GEN
  angle = angle + 1;  if (angle > 360) angle = 0;
  //sine = 50 * sin((angle * PI / 180));
  sine = 50 * sin(angle * 0.0174532925);
  //sine = abs(sine);
}