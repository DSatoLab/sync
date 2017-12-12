#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <stdio.h>

unsigned long time;

int enablePin = 11;
int in1Pin = 10;
int in2Pin = 9;
int ledPin = 6;

// define Fitzhugh Nagumo parameters and starting values
double epsilon = 0.01, alpha = 0.1, gamma = 0.5, Iapp = 0.0;
double v = 0.4, w = 0.5, t = 0;
double dt = 0.01;               // chosen time step
double f = 0.0, dv = 0.0, dw = 0.0;
double I_ext_old = 0.0;

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
  Serial.println("Light Sensor Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!tsl.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  displaySensorDetails();
  
  /* Setup the sensor gain and integration time */
  configureSensor();
  
  /* We're ready to go! */
  Serial.println("");
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
  Serial.println("------------------------------------");
  Serial.print  ("Gain:         "); Serial.println("Auto");
  Serial.print  ("Timing:       "); Serial.println("13 ms");
  Serial.println("------------------------------------");
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
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void loop()
{
  unsigned long beginTime = millis();
  
  /* Get a new sensor event */ 
  sensors_event_t event;
  tsl.getEvent(&event);

  // Calculate some Fitzhugh Nagumo stuff 
  double I_ext = event.light;
  double de = I_ext - I_ext_old;
  Iapp = Iapp + (de / 30.0) - (Iapp / 10.0);
  Iapp = Iapp/50.0;
  I_ext_old = I_ext; 

  f = v * (1 - v) * (v - alpha);
  dv = (dt / epsilon) * (f - w + Iapp + 0.5);
  dw = dt * (v - (gamma * w));

  // Calculate v and w based on rate of change
  t += dt;
  v += dv;
  w += dw;

Serial.print("Voltage: "); Serial.println(v); Serial.print("I_ext value: "); Serial.println(I_ext);
Serial.print("Iapp: "); Serial.println(Iapp);


  // Map voltage (range: -3:1) to new range (0:255)
  double a = ((v + 0.3) / 1.3) * 255;
  
  if (a > 255) 
  {
    a = 255;
  }
  if (a < 0) 
  {
    a = 0;
  }

  Serial.print("a value: "); Serial.println(a);
  //Serial.print("\n");

  digitalWrite(in1Pin, HIGH);
  digitalWrite(in2Pin, LOW);

   
  analogWrite(ledPin, a);
  analogWrite(enablePin, a);


  unsigned long endTime = millis();
  
  Serial.print("beginTime: "); Serial.println(beginTime);
  Serial.print("endTime: "); Serial.println(endTime);
  Serial.print("\n");
  
  delay(70 - (endTime - beginTime)); //NOTE: 70 because max calculation time is about 67ms. If use a number below calc time, delay is negative and LED doesn't blink.
  //NOTE: after 21st iteration of the loop, calculation time evens out to about 18 - 20ms.
}

