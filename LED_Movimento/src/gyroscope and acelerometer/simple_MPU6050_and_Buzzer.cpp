/**
 * This code demonstrates the use of an MPU6050 gyroscope and accelerometer sensor to control the pitch of a buzzer.
 * 
 * The code includes the following functionality:
 * - Initializes the MPU6050 sensor and configures its settings
 * - Defines a 2D array of musical notes that correspond to different octaves and notes
 * - Implements a `pitch()` function that calculates the current octave and note based on the gyroscope readings
 * - Plays the calculated pitch on the buzzer in the `loop()` function
 * - Prints the accelerometer, gyroscope, and temperature data to the serial monitor
 */
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "pitches.h"

// ESP32 pin GPIO18 connected to piezo buzzer
#define BUZZZER_PIN_1  18 

// defines  Bb Major scale with 6 octaves
int bb_scale[6][8] = {{NOTE_AS1, NOTE_C1, NOTE_D1, NOTE_DS1, NOTE_F1, NOTE_G1, NOTE_A1, SILENCE}, 
                      {NOTE_AS2, NOTE_C2, NOTE_D2, NOTE_DS2, NOTE_F2, NOTE_G2, NOTE_A2, SILENCE}, 
                      {NOTE_AS3, NOTE_C3, NOTE_D3, NOTE_DS3, NOTE_F3, NOTE_G3, NOTE_A3, SILENCE}, 
                      {NOTE_AS4, NOTE_C4, NOTE_D4, NOTE_DS4, NOTE_F4, NOTE_G4, NOTE_A4, SILENCE},
                      {NOTE_AS5, NOTE_C5, NOTE_D5, NOTE_DS5, NOTE_F5, NOTE_G5, NOTE_A5, SILENCE},
                      {NOTE_AS6, NOTE_C6, NOTE_D6, NOTE_DS6, NOTE_F6, NOTE_G6, NOTE_A6, SILENCE}};

int noteDuration[] = {250, 250, 250, 250, 500, 500, 500, 500, 1000, 1000, 1000, 1500};

/**
 * Keeps track of the current octave and note values for the musical scale.
 * The octave value ranges from 0 to 5, and the note value ranges from 0 to 6.
 * These values are used to determine the pitch of the buzzer based on the
 * gyroscope readings.
 */
int octave = 3, note = 7, duration = 3;

// MPU6050 sensor object
Adafruit_MPU6050 mpu;

/**
 * Calculates the current octave and note based on the gyroscope readings, and returns the corresponding pitch value from the Bb Major scale.
 *
 * The octave value ranges from 0 to 5, and the note value ranges from 0 to 6. These values are updated based on the X and Y gyroscope readings, respectively.
 *
 * @param g The gyroscope event data.
 * @return The pitch value from the Bb Major scale corresponding to the current octave and note.
 */
unsigned int pitch(sensors_event_t a, sensors_event_t g){
  float totalSpin = sqrt(g.gyro.x * g.gyro.x + g.gyro.y * g.gyro.y);
  float totalAcc = sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.y * a.acceleration.y);
  Serial.println("totalAcc");
  Serial.println(totalAcc);
  Serial.println("totalSpin");
  Serial.println(totalSpin);

  if (totalAcc < 3) octave -= 1;
  if (totalAcc >= 3) octave += 1;

  if (octave < 0) octave = 5;
  if (octave > 5) octave = 0;
  
  if (totalSpin < 3) note -= random(0, 6);
  if (totalSpin > 4) note += random(0, 6);

  if (note < 0) note = abs(note);
  while (note > 6) note -= 3;

  if(totalAcc < 0.5 || totalSpin < 0.5) note = 7;

  return bb_scale[octave][note];
}

/**
 * @brief Calculates the duration of a note based on accelerometer data.
 *
 * This function takes an accelerometer event as input and calculates the 
 * total acceleration in the x and y directions. Based on the magnitude 
 * of the total acceleration, it returns a duration for a note from a 
 * predefined set of durations.
 *
 * @param a The accelerometer event containing acceleration data.
 * @return The duration of the note based on the total acceleration.
 */
int defineNoteDuration (sensors_event_t acc){
  float totalAcc = sqrt(acc.acceleration.x * acc.acceleration.x + acc.acceleration.y * acc.acceleration.y);
  if (totalAcc > 0.5 and totalAcc < 0.75) return noteDuration[random(8, 11)];
  if (totalAcc > 0.75 and totalAcc < 3) return noteDuration[random(5, 8)];
  return noteDuration[random(0, 5)];
}


/**
 * Configures the MPU6050 sensor with the following settings:
 * - Accelerometer range: ±8g
 * - Gyroscope range: ±500 deg/s
 * - Filter bandwidth: 5 Hz
 * 
 * This function initializes the MPU6050 sensor and checks if it is found. If the sensor is not found, the function will enter an infinite loop.
 */
void setMPUConfigurations(){
  Serial.println("Adafruit MPU6050 test!");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
}

/**
 * Prints the data from the MPU6050 sensor, including the accelerometer, gyroscope, and temperature readings.
 *
 * @param a The accelerometer event data.
 * @param g The gyroscope event data.
 * @param temp The temperature event data.
 */
void printMPUData(sensors_event_t a, sensors_event_t g, sensors_event_t temp){
  Serial.print("AccX:");
  Serial.print(a.acceleration.x);
  Serial.print(",AccY:");
  Serial.print(a.acceleration.y);
  Serial.print(",AccZ:");
  Serial.print(a.acceleration.z);
  Serial.print(",RotX:");
  Serial.print(g.gyro.x * (180 / 3.1415));
  Serial.print(",RotY:");
  Serial.print(g.gyro.y * (180 / 3.1415));
  Serial.print(",RotZ:");
  Serial.print(g.gyro.z * (180 / 3.1415));
  Serial.print(",Temp:");
  Serial.println(temp.temperature);
}


void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  setMPUConfigurations();
  delay(100);
}

void loop() {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /*sets a pitch to the buzzer according to the rotation on the gyroscope*/
  tone(BUZZZER_PIN_1, pitch(a, g));

  //printMPUData(a, g, temp);
  
  delay(defineNoteDuration(a));
  noTone(BUZZZER_PIN_1);
}