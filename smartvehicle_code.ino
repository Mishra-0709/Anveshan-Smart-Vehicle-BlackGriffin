#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <HardwareSerial.h>

// Define the hardware serial ports for GPS and GSM
HardwareSerial GPS(2); // UART2 for GPS
HardwareSerial GSM(1); // UART1 for GSM

// Create an instance of the MPU6050 sensor
Adafruit_MPU6050 mpu;

// Define the pins for the ultrasonic sensor
#define TRIG_PIN 5
#define ECHO_PIN 18

// Define the pins for the LED and buzzer
#define LED_PIN 4
#define BUZZER_PIN 19

void setup() {
  // Initialize the serial monitor
  Serial.begin(115200);

  // Initialize the GPS and GSM modules
  GPS.begin(9600, SERIAL_8N1, 16, 17); // TX = GPIO16, RX = GPIO17
  GSM.begin(9600, SERIAL_8N1, 2, 15);  // TX = GPIO2, RX = GPIO15

  // Initialize the pins for the ultrasonic sensor, LED, and buzzer
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize the MPU6050 sensor
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  // Configure the MPU6050 sensor settings
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Initialize the GSM module
  GSM.println("AT");
  delay(100);
  GSM.println("AT+CMGF=1"); // Set SMS to text mode
  delay(100);
}

void loop() {
  // Read GPS data
  if (GPS.available() > 0) {
    String gpsData = GPS.readString();
    Serial.println(gpsData);
  }

  // Read ultrasonic sensor data
  long duration, distance;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration / 2) / 29.1;
  if (distance < 20) {
    digitalWrite(LED_PIN, HIGH);  // Turn on LED if object is within 20 cm
    digitalWrite(BUZZER_PIN, HIGH); // Turn on buzzer
    sendAlert("Object detected within 20 cm");
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Read MPU6050 sensor data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  if (abs(a.acceleration.x) > 1 || abs(a.acceleration.y) > 1) {
    digitalWrite(LED_PIN, HIGH); // Turn on LED if sudden acceleration is detected
    digitalWrite(BUZZER_PIN, HIGH); // Turn on buzzer
    sendAlert("Sudden acceleration detected");
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Add a delay to prevent excessive sensor reading
  delay(100);
}

void sendAlert(String message) {
  GSM.println("AT+CMGS=\"+1234567890\""); // Replace with the recipient's phone number
  delay(100);
  GSM.println(message);
  delay(100);
  GSM.write(26); // CTRL+Z to send the SMS
  delay(100);
}