#include <Servo.h>

// --- PIN CONFIGURATION ---
const int TRIG_PIN  = 9;   // Ultrasonic Sensor Trig
const int ECHO_PIN  = 8;   // Ultrasonic Sensor Echo
const int SERVO_PIN = 10;  // Servo Motor Signal

// --- CALIBRATION VARIABLES ---
const int TRIGGER_DISTANCE = 10;  // Distance in cm to trigger (Adjust as needed)
const int OPEN_ANGLE       = 90;  // Angle to open the valve/pinch the tube
const int CLOSED_ANGLE     = 0;   // Angle to completely close the flow
const unsigned long MAX_DISPENSE_TIME = 5000; // Max time to pour water (5 seconds)

// --- SYSTEM STATES ---
Servo waterServo;
bool isDispensing = false;
unsigned long dispenseStartTime = 0;

void setup() {
  // Initialize Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Initialize Servo
  waterServo.attach(SERVO_PIN);
  waterServo.write(CLOSED_ANGLE); // Start safely closed
  
  // Serial Monitor for Debugging
  Serial.begin(9600);
  Serial.println("--- Touchless Water Dispenser Ready ---");
}

void loop() {
  // 1. Get the current distance from the sensor
  int currentDistance = getDistance();
  
  // 2. Main Logic Control
  if (currentDistance > 0 && currentDistance <= TRIGGER_DISTANCE) {
    
    // If a cup is detected and we aren't pouring yet...
    if (!isDispensing) {
      Serial.println("Cup Detected! Starting water flow...");
      waterServo.write(OPEN_ANGLE);   // Open the valve
      dispenseStartTime = millis();  // Record the exact start time
      isDispensing = true;
    } 
    // If it's already pouring, check if it has been running too long
    else {
      if (millis() - dispenseStartTime >= MAX_DISPENSE_TIME) {
        Serial.println("WARNING: Maximum dispense time reached! Stopping for safety.");
        waterServo.write(CLOSED_ANGLE); // Force close the valve
        // System pauses here until the cup is removed to prevent looping
        while(getDistance() <= TRIGGER_DISTANCE) {
          delay(500); 
        }
        isDispensing = false;
      }
    }
  } 
  else {
    // No cup detected, or cup removed -> Shut off the water
    if (isDispensing) {
      Serial.println("Cup removed. Shutting off flow.");
      waterServo.write(CLOSED_ANGLE);
      isDispensing = false;
    }
  }

  delay(100); // Small delay to keep the sensor readings stable
}

// --- HELPER FUNCTION TO MEASURE DISTANCE ---
int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  int distance = duration * 0.034 / 2;
  
  return distance;
}