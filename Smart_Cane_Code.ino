#include <SoftwareSerial.h>

// Hardware Pin Configuration
#define TRIG 2
#define ECHO 3
#define BUZZER 4
#define BUTTON 5
#define TX_GPS 6
#define RX_GPS 7

// Software Serial Setup for GPS (Pins 6 and 7)
SoftwareSerial gpsSerial(TX_GPS, RX_GPS);

// Global Variables
String latitude = "";
String longitude = "";
bool lastState = HIGH;

void setup() {
  // Initialize Serial Connections
  Serial.begin(9600);     // For HC-05 Bluetooth communication
  gpsSerial.begin(9600);  // For NEO-6M GPS data extraction
  
  // Pin Modes
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP); // Using internal pull-up for the switch
  
  Serial.println("Smart Cane System Initialized Successfully!");
}

void loop() {
  // 1. Obstacle Detection Logic
  long distance = getDistance();
  triggerBuzzer(distance);
  
  // 2. Continuous NMEA Parsing from GPS
  while (gpsSerial.available() > 0) {
    String nmeaSentence = gpsSerial.readStringUntil('\n');
    processGPS(nmeaSentence);
  }
  
  // 3. Emergency Push Button Polling
  bool currentState = digitalRead(BUTTON);
  if (lastState == HIGH && currentState == LOW) { // Button Pressed
    sendLocation();
    delay(500); // Debounce delay
  }
  lastState = currentState;
}

// ================= 1. Obstacle Detection =================
long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  long duration = pulseIn(ECHO, HIGH);
  long distanceCm = duration * 0.034 / 2;
  return distanceCm;
}

// ================= 2. Auditory Feedback =================
void triggerBuzzer(long distance) {
  if (distance > 0 && distance <= 30) {
    // High-frequency urgent alert for close obstacles
    digitalWrite(BUZZER, HIGH);
    delay(100);
    digitalWrite(BUZZER, LOW);
    delay(100);
  } 
  else if (distance > 30 && distance <= 70) {
    // Moderate frequency tone for medium distance obstacles
    digitalWrite(BUZZER, HIGH);
    delay(300);
    digitalWrite(BUZZER, LOW);
    delay(300);
  } 
  else {
    // No obstacle detected in the safety zone
    digitalWrite(BUZZER, LOW);
  }
}

// ================= 3. GPS NMEA Parsing =================
void processGPS(String s) {
  // Check for Recommended Minimum Specific GPS Data sentence ($GPRMC) and active status
  if (s.indexOf("$GPRMC") != -1 && s.indexOf(",A Close,") == -1) {
    int commas[12];
    int idx = 0; 
    
    for (int i = 0; i < s.length(); i++) {
      if (s[i] == ',') {
        commas[idx++] = i;
      }
    }
    
    if (idx > 6) {
      String lat = s.substring(commas[2] + 1, commas[3]);
      String latDir = s.substring(commas[3] + 1, commas[4]);
      String lon = s.substring(commas[4] + 1, commas[5]);
      String lonDir = s.substring(commas[5] + 1, commas[6]);
      
      latitude = convert(lat, latDir);
      longitude = convert(lon, lonDir);
      
      Serial.println("Location Updated via NMEA Processing!");
      Serial.println("Lat/Lon: " + latitude + " , " + longitude);
    }
  }
}

// ================= 4. Coordinate Conversion =================
String convert(String raw, String dir) {
  float val = raw.toFloat();
  int degrees = (int)(val / 100);
  float minutes = val - (degrees * 100);
  float decimalDegrees = degrees + (minutes / 60);
  
  if (dir == "S" || dir == "W") {
    decimalDegrees = -decimalDegrees;
  }
  
  return String(decimalDegrees, 6); // 6-decimal point precision
}

// ================= 5. Emergency Data Stream =================
void sendLocation() {
  /*
  if (latitude != "" && longitude != "") {
    // Constructs and streams a dynamic Google Maps URL over the Bluetooth TX Buffer
    Serial.print("HELP! I am in emergency. My location: ");
    Serial.print("https://maps.google.com/?q=");
    Serial.print(latitude);
    Serial.print(",");
    Serial.println(longitude);
  } else {
    Serial.println("HELP! GPS tracking is searching for a valid satellite fix...");
  }
*/
}
