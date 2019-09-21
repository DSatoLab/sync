#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <stdio.h>

unsigned long time;
unsigned long beginTime;
unsigned long endTime;
unsigned long t1;
unsigned long t2;

int enablePin = 11; // to enable the motor
int in1Pin = 10;
int in2Pin = 9;
int ledPin = 6;

// define Fitzhugh Nagumo parameters and starting values
double f = 0, z = 0.25, a = 1, b = 0, c = 2, eps = 0.08, I_new = 0, I_old = 0, I_diff = 0;
double dt = 0.05; // chosen time step
double g = 0.5; // coupling

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
  if (count == 0) {
     beginTime = millis();
  }

  // want to only get new sensor event and do new calculations every 10 interations
  if (count == 10) {
    
    /* Get a new sensor event */ 
    sensors_event_t event;
    tsl.getEvent(&event);
    I_new = event.light;
  
    I_diff = I_new - I_old;
    
    if (I_diff > 3) {
      I_diff = 3;
    }
    if (I_diff < -3) {
      I_diff = -3;
    }
  
    I_old = I_new;
  
    // Map voltage (range: -1:1) to new range (70:255) for motor output
    double mappedVoltage = (((v + 1) / 2) * 185) + 70;
    
    if (mappedVoltage > 255) {
      mappedVoltage = 255;
    }
    if (mappedVoltage < 70) {
      mappedVoltage = 70;
    }
  
    // map voltage (range: -0.5:1) to new range (0:255)for LED output
    double LEDmap = ((v + 0.5) / 1.5) * 255;
  
    if (LEDmap > 255) {
      LEDmap = 255;
    }
    if (LEDmap < 0) {
      LEDmap = 0;
    }
  
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
     
    analogWrite(ledPin, LEDmap);
    analogWrite(enablePin, mappedVoltage);
  
    endTime = millis();
    int delayTime = 120 - (endTime - beginTime);
       
    if (delayTime < 0) {
      delayTime = 0;
    }
    
    delay(delayTime);
    count = 0;
  
  } 
  else {
    ++count;
  }
  
  // Calculate Fitzhugh Nagumo stuff
  f = v_old - (1.0/3.0) * (v_old * v_old * v_old);
  dv = dt * (z * (f - w_old) - w_old + g*(I_diff));
  dw = dt * eps * (a * v_old - c * w_old);

  double left = (z * (f - w_old) - w_old);
  double right = g*I_diff;

  // Calculate v and w based on rate of change
  v = v_old + dv;
  w = w_old + dw;

  // replace old variables
  v_old = v;
  w_old = w;
}

