//Using Arduino IoT Cloud App + MYSQL Database

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <DHT.h>
#include <HTTPClient.h>


String URL = "YOUR LOCALHOST DHT11 .PHP";
unsigned long lastDataUploadTime = 0;
const unsigned long dataUploadInterval = 60000; // 60 seconds

void onSwitch1Change();
void onSwitch2Change();
void onSwitch3Change();
void sendDataHTTP(float temperature, float humidity);

const char THING_ID[]           = "YOUR THING ID";
const char DEVICE_LOGIN_NAME[]  = "YOUR DEVICE LOGIN NAME";
const char SSID[]               = "YOUR WIFI SSID";
const char PASS[]               = "YOUR PASSWORD WIFI";
const char DEVICE_KEY[]         = "YOUR DEVICE KEY";

#define DHTPIN 16
#define RelayPin1 23
#define RelayPin2 22
#define RelayPin3 21
#define SwitchPin1 13
#define SwitchPin2 12
#define SwitchPin3 14
#define wifiLed    2

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

int toggleState_1 = 0;
int toggleState_2 = 0;
int toggleState_3 = 0;

float temperature1 = 0;
float humidity1   = 0;
int   reconnectFlag = 0;

CloudSwitch switch1;
CloudSwitch switch2;
CloudSwitch switch3;
CloudTemperatureSensor temperature;

void initProperties(){
  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(switch1, READWRITE, ON_CHANGE, onSwitch1Change);
  ArduinoCloud.addProperty(switch2, READWRITE, ON_CHANGE, onSwitch2Change);
  ArduinoCloud.addProperty(switch3, READWRITE, ON_CHANGE, onSwitch3Change);
  ArduinoCloud.addProperty(temperature, READ, 60 * SECONDS, NULL);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

void readSensor(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  } else {
    humidity1 = h;
    temperature1 = t;
  }
}

void sendSensor(){
  if (millis() - lastDataUploadTime >= dataUploadInterval) {
    readSensor();

    // Update Arduino Cloud properties
    temperature = temperature1;
    switch1 = toggleState_1;
    switch2 = toggleState_2;
    switch3 = toggleState_3;

    // Send data to MySQL
    sendDataHTTP(temperature1, humidity1);
    
    lastDataUploadTime = millis();
  }
}

void relayOnOff(int relay) {
  switch (relay) {
    case 1:
      if (toggleState_1 == 0) {
        digitalWrite(RelayPin1, LOW); // turn on relay 1
        toggleState_1 = 1;
        Serial.println("Device1 ON");
      }
      else {
        digitalWrite(RelayPin1, HIGH); // turn off relay 1
        toggleState_1 = 0;
        Serial.println("Device1 OFF");
      }
      delay(100);
      break;
    case 2:
    if (toggleState_2 == 0) {
        digitalWrite(RelayPin2, LOW); // turn on relay 2
        toggleState_2 = 1;
        Serial.println("Device2 ON");
      }
      else {
        digitalWrite(RelayPin2, HIGH); // turn off relay 2
        toggleState_2 = 0;
        Serial.println("Device2 OFF");
      }
      delay(100);
      break;
    case 3:
    if (toggleState_3 == 0) {
        digitalWrite(RelayPin3, LOW); // turn on relay 3
        toggleState_3 = 1;
        Serial.println("Device3 ON");
      } else {
        digitalWrite(RelayPin3, HIGH); // turn off relay 3
        toggleState_3 = 0;
        Serial.println("Device3 OFF");
      }
      delay(100);
      break;
    default : break;
  }
}

void manual_control() {
    if (digitalRead(SwitchPin1) == LOW) {
    delay(200);
    relayOnOff(1);
    switch1 = toggleState_1;
  } else if (digitalRead(SwitchPin2) == LOW) {
    delay(200);
    relayOnOff(2);
    switch2 = toggleState_2;
  } else if (digitalRead(SwitchPin3) == LOW) {
    delay(200);
    relayOnOff(3);
    switch3 = toggleState_3;
  }
}

void doThisOnConnect(){
  Serial.println("Board successfully connected to Arduino IoT Cloud");
  digitalWrite(wifiLed, HIGH);
}

void doThisOnSync(){
  Serial.println("Thing Properties synchronized");
}

void doThisOnDisconnect(){
  Serial.println("Board disconnected from Arduino IoT Cloud");
  digitalWrite(wifiLed, LOW);
}

void setup() {
  Serial.begin(9600);
  delay(1500);

  initProperties();
  dht.begin();

  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::CONNECT, doThisOnConnect);
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::SYNC, doThisOnSync);
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::DISCONNECT, doThisOnDisconnect);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(RelayPin3, OUTPUT);

  pinMode(wifiLed, OUTPUT);

  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);
  pinMode(SwitchPin3, INPUT_PULLUP);

  digitalWrite(RelayPin1, HIGH);
  digitalWrite(RelayPin2, HIGH);
  digitalWrite(RelayPin3, HIGH);
}

void loop() {
  ArduinoCloud.update();
  manual_control();
  sendSensor();
}

void onSwitch1Change() {
  if (switch1 == 1) {
    digitalWrite(RelayPin1, LOW);
    toggleState_1 = 1;
  } else {
    digitalWrite(RelayPin1, HIGH);
    toggleState_1 = 0;
  }
}

void onSwitch2Change() {
  if (switch2 == 1) {
    digitalWrite(RelayPin2, LOW);
    toggleState_2 = 1;
  } else {
    digitalWrite(RelayPin2, HIGH);
    toggleState_2 = 0;
  }
}

void onSwitch3Change() {
  if (switch3 == 1) {
    digitalWrite(RelayPin3, LOW);
    toggleState_3 = 1;
  } else {
    digitalWrite(RelayPin3, HIGH);
    toggleState_3 = 0;
  }
}

void sendDataHTTP(float temperature, float humidity) {
  HTTPClient http;

  http.begin(URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String data = "Temperature=" + String(temperature) + "&Humidity=" + String(humidity);

  int httpResponseCode = http.POST(data);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.print("HTTP Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
