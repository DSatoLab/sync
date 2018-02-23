#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <stdio.h>

unsigned long time;
unsigned long beginTime;
unsigned long endTime;

int enablePin = 11;
int in1Pin = 10;
int in2Pin = 9;
int ledPin = 6;

// define Fitzhugh Nagumo parameters and starting values
double f = 0, z = 0.25, a = 1, b = 0, c = 2, eps = 0.1, t = 0, I_new = 0, I_old = 0, I_diff = 0;
double dt = 0.05; // chosen time step
double g = 0.1; // coupling

// starting equation values
double v = 0, w = -0.5;
double v_old = 0, w_old = -0.5;
double dv = 0, dv_old = 0, dw = 0;
int count = 0;

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

void setup()
{ 
  // Set up and begin time counter
  Serial.begin(9600);
  
  // Setting up pins
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // Setting up sensor
  Serial.begin(9600);
  //Serial.println("Light Sensor Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!tsl.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    //Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  //displaySensorDetails();
  
  /* Setup the sensor gain and integration time */
  configureSensor();
  
  /* We're ready to go! */
  //Serial.println("");

  sensors_event_t event;
  tsl.getEvent(&event);
  I_new = event.light;
  I_old = I_new;
}

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2561
*/
/**************************************************************************/
void configureSensor(void)
{
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
  
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  /* Update these values depending on what you've set above! */  
  /*Serial.println("------------------------------------");
  Serial.print  ("Gain:         "); Serial.println("Auto");
  Serial.print  ("Timing:       "); Serial.println("13 ms");
  Serial.println("------------------------------------");*/
}

/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
void displaySensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
 /* Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500); */
}

void loop(){
  //NOTE: THIS THING IS STOPPING AFTER COUNT = 4 AND WON'T DO ANYTHING. MAYBE MAPPED I_DIFF WRONG? RUN AND FIX IT. //
  if (count == 0) {
     beginTime = millis();
  }

  if (count == 4) {
  /* Get a new sensor event */ 
  sensors_event_t event;
  tsl.getEvent(&event);
  I_new = event.light;
  }

  // Calculate some Fitzhugh Nagumo stuff 
  Serial.print("I_new = "); Serial.println(I_new);
  Serial.print("I_old = "); Serial.println(I_old);
  I_diff = I_new - I_old;

  // mapping I_diff from (0:3) to (-1:1)
  I_diff = ((I_diff) / 3.0) * 2;
  
  if (I_diff > 1) 
  {
    I_diff = 1;
  }
  if (I_diff < -1) 
  {
    I_diff = -1;
  }
  
  Serial.print("I_diff = "); Serial.println(I_diff);
  f = v_old - (1.0/3.0) * (v_old * v_old * v_old);
  dv = dt * (z * (f - w_old) - w_old + g*(I_diff));
  dw = dt * eps * (a * v_old - c * w_old);

  // Calculate v and w based on rate of change
  t += dt;
  v = v_old + dv;
  w = w_old + dw;

  // replace old variables
  v_old = v;
  w_old = w;

  if (count == 4) {
    I_old = I_new;
  }

  // Map voltage (range: -1:1) to new range (0:255)
  double mappedVoltage = ((v + 1) / 2) * 255;
  
  if (mappedVoltage > 255) 
  {
    mappedVoltage = 255;
  }
  if (mappedVoltage < 0) 
  {
    mappedVoltage = 0;
  }

  // map voltage (range: -0.5:1) to new range (0:255)
  double LEDmap = ((v + 0.5) / 1.5) * 255;

  if (LEDmap > 255) 
  {
    LEDmap = 255;
  }
  if (LEDmap < 0) 
  {
    LEDmap = 0;
  }
  
/*
  Serial.print("v value: "); Serial.println(v);
  Serial.print("mappedVoltage value: "); Serial.println(mappedVoltage);
  Serial.print("\n");  */

  digitalWrite(in1Pin, HIGH);
  digitalWrite(in2Pin, LOW);

   
  analogWrite(ledPin, LEDmap);
  analogWrite(enablePin, mappedVoltage);

  
  /* Serial.print("beginTime: "); Serial.println(beginTime);
  Serial.print("endTime: "); Serial.println(endTime);
  Serial.print("\n"); */

 Serial.print("Count: "); Serial.println(count);
  if (count == 4) {
    endTime = millis();
    //Serial.print("beginTime: "); Serial.println(beginTime);
  //Serial.print("endTime: "); Serial.println(endTime);
  int delayTime = 60 - (endTime - beginTime);
  delay(delayTime);
  Serial.print("delay time: "); Serial.println(delayTime);
  //Serial.print("\n");
  count = 0;
  }
  else {
    ++count;
  }
}

