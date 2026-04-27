#include <DHT.h>
#include <SoftwareSerial.h>

// ================== SENSOR PINS ==================
#define DHTPIN 2           // DHT11 sensor pin
#define DHTTYPE DHT11
#define IR_PIN A0          // Heartbeat sensor analog pin
#define X_PIN A1           // ADXL335 X-axis
#define Y_PIN A2           // ADXL335 Y-axis
#define Z_PIN A3           // ADXL335 Z-axis

// ================== GSM SETUP ====================
#define GSM_TX 7           // GSM module TX
#define GSM_RX 8           // GSM module RX
SoftwareSerial gsmSerial(GSM_TX, GSM_RX); // RX, TX

// ================== THRESHOLDS ===================
#define HEART_RATE_LOW 50
#define HEART_RATE_HIGH 120
#define TEMP_THRESHOLD 38.0   // Celsius
#define FALL_THRESHOLD 2.0    // Accelerometer g-force threshold

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  gsmSerial.begin(9600);
  dht.begin();
  pinMode(IR_PIN, INPUT);
  
  Serial.println("IoT Health & Safety Monitoring System");
}

// Function to send SMS via GSM
void sendSMS(String message) {
  gsmSerial.println("AT+CMGF=1"); // Set SMS mode
  delay(1000);
  gsmSerial.println("AT+CMGS=\"+91 9786265093\""); // Replace with mobile number
  delay(1000);
  gsmSerial.println(message);
  delay(500);
  gsmSerial.println((char)26); // Ctrl+Z to send
  delay(1000);
}

// Function to read heartbeat (simplified)
int readHeartRate() {
  int sensorValue = analogRead(IR_PIN);
  int bpm = map(sensorValue, 0, 1023, 50, 130); // rough approximation
  return bpm;
}

// Function to read fall detection from ADXL335
bool detectFall() {
  float x = analogRead(X_PIN) * (3.3 / 1023.0);
  float y = analogRead(Y_PIN) * (3.3 / 1023.0);
  float z = analogRead(Z_PIN) * (3.3 / 1023.0);
  
  float gForce = sqrt(sq(x-1.65) + sq(y-1.65) + sq(z-1.65)); // 1.65V ~ 0g
  if(gForce > FALL_THRESHOLD) {
    return true;
  }
  return false;
}

void loop() {
  // ====== READ SENSORS ======
  int heartRate = readHeartRate();
  float temp = dht.readTemperature(); // Celsius
  bool fallDetected = detectFall();

  Serial.print("Heart Rate: "); Serial.println(heartRate);
  Serial.print("Temp: "); Serial.println(temp);
  Serial.print("Fall: "); Serial.println(fallDetected);

  // ====== CHECK FOR EMERGENCIES ======
  if(heartRate < HEART_RATE_LOW || heartRate > HEART_RATE_HIGH) {
    sendSMS("ALERT: Abnormal Heart Rate Detected! BPM: " + String(heartRate));
  }
  
  if(temp >= TEMP_THRESHOLD) {
    sendSMS("ALERT: High Body Temperature! Temp: " + String(temp) + "C");
  }
  
  if(fallDetected) {
    sendSMS("ALERT: Fall Detected!");
  }

  delay(2000); // 2 sec delay before next reading
}