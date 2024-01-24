/**
 * @file streams-generator-wm8990.ino
 * @author Phil Schatzmann
 * @brief Test sketch for wm8990
 * @copyright GPLv3
 */
 
#include "AudioTools.h"
#include "AudioLibs/WM8960Stream.h"

AudioInfo info(32000, 1, 16);
SineWaveGenerator<int16_t> sineWave(25000);                // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<int16_t> sound(sineWave);             // Stream generated from sine wave
CsvStream<int16_t> csv(256); // for debugging, watch output in Serial Plotter
WM8960Stream out; 
//StreamCopy copier(csv, sound,256);                             // copies sound into i2s
StreamCopy copier(out, sound,256);     
// Arduino Setup
void setup(void) {  
  // Open Serial 
  Serial.begin(115200);
  while(!Serial);

  AudioLogger::instance().begin(Serial, AudioLogger::Warning);

  Wire.setSCL(1);
  Wire.setSDA(0);
  Wire.begin() ;
  //Wire1.setClock(10000);

  // start I2S
  Serial.println("starting I2S...");
  auto config = out.defaultConfig(TX_MODE);
  config.copyFrom(info); 
  //config.wire = &Wire;
  

  config.pin_bck = 2;
  config.pin_ws = 3;
  config.pin_data = 9;
  //config.pin_data_rx = 5;
  config.vs1053_enable_pll = true;
    config.buffer_count = 4;
  config.buffer_size = 256;

  

  if (!out.begin(config)){
    Serial.println("error in config");
    stop();
  
  }

  out.setVolumeOut(.6f);

  // Setup sine wave
  sineWave.begin(info, N_A4);
  csv.begin();
  Serial.println("started...");
}

// Arduino loop - copy sound to out 
void loop() {
  copier.copy();
  delayMicroseconds(200);
}