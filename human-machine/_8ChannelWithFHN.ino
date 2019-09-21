#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <stdio.h>

unsigned long time;
unsigned long beginTime;
unsigned long endTime;
unsigned long initialTime4Sensor;
unsigned long finalTime4Sensor;

int Relay1 = 5; //Defined Pin 5 as the Variable

// define Fitzhugh Nagumo parameters and starting values
double f = 0, z = 0.25, a = 1, b = 0, c = 2, eps = 0.08, I_new = 0, I_old = 0, I_diff = 0;
double dt = 0.05; // chosen time step
double g = 0.5; // coupling

// starting equation values
double v = 0, w = -0.5;
double dv = 0, dv_old = 0, dw = 0;
int count = 0;

double previousAccelValue = 0;

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

void setup()
{
  // Set up and begin time counter
  Serial.begin(9600);

  // Setting up pins
  pinMode(Relay1, OUTPUT); //Set Pin6 as output

#ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
#endif

  /* Initialise the sensor */
  if (!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    //Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while (1);
  }

}

void loop() {
  if (count == 0) {
    beginTime = millis();
    initialTime4Sensor = beginTime;
  }

  if (count == 10) {
    finalTime4Sensor = millis();
    /* Get a new sensor event */
    sensors_event_t event;
    accel.getEvent(&event);

    /* Display the results (acceleration is measured in m/s^2) */
    Serial.print("X:\t"); Serial.print(event.acceleration.x); Serial.print("\t");
    Serial.print("Y:\t"); Serial.print(event.acceleration.y); Serial.print("\t");
    Serial.print("Z:\t"); Serial.print(event.acceleration.z); Serial.print("\t\n");

    I_new = (event.acceleration.z - previousAccelValue) / (finalTime4Sensor - initialTime4Sensor);

    I_diff = I_new - I_old;

    // mapping I_diff from (0:1000) to (-1:1)
    //I_diff = ((-I_diff) / 1000.0) * 2;

    /*if (I_diff > 3)
      {
      I_diff = 3;
      }
      if (I_diff < -3)
      {
      I_diff = -3;
      }*/

    I_old = I_new;

    if (v > 0) {
      digitalWrite(Relay1, LOW); //Turn on relay
    }
    else {
      digitalWrite(Relay1, HIGH); //Turn off relay
    }

    //Serial.print("Count: "); Serial.println(count);
    endTime = millis();
    int delayTime = 100 - (endTime - beginTime);

    if (delayTime < 0) {
      delayTime = 0;
    }
    delay(delayTime);

    count = 0;
  } else {
    ++count;
  }

  for (int i = 0; i < 5; i++) {
    // Calculate some Fitzhugh Nagumo stuff
    f = v - (1.0 / 3.0) * (v * v * v);
    dv = dt * (z * (f - w) - w + g * (I_diff));
    dw = dt * eps * (a * v - c * w);

    // Calculate v and w based on rate of change
    v = v + dv;
    w = w + dw;
  }

  Serial.print("V = "); Serial.println(v);


  /* Serial.print("beginTime: "); Serial.println(beginTime);
    Serial.print("endTime: "); Serial.println(endTime);
    Serial.print("\n"); */
}

