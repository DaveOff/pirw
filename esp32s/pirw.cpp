#include <WebSocketsServer.h>
#include <WiFi.h>

const char* ssid = "SSID"; 
const char* password = "PASSWORD";

const int led = PIN;
const int motionSensor = PIN;

int ledState = LOW;

unsigned long now = millis();
unsigned long lastTrigger = 0;

boolean startTimer = false;
boolean adminSwitch = false;

uint8_t admin = 100;

enum Actions {
    Als,
    Alt,
    Ainvalid
};

#define timeSeconds 3
#define USE_SERIAL Serial

WebSocketsServer webSocket = WebSocketsServer(8081);

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
	const uint8_t* src = (const uint8_t*) mem;
	USE_SERIAL.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) {
		if(i % cols == 0) {
			USE_SERIAL.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		}
		USE_SERIAL.printf("%02X ", *src);
		src++;
	}
	USE_SERIAL.printf("\n");
}

void IRAM_ATTR detectsMovement() {
    startTimer = true;
    lastTrigger = millis();
}

void connectToWifi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(led, HIGH);
    delay(1000);
    digitalWrite(led, LOW);
    Serial.print("Wifi Err: " + String(WiFi.status()));
    Serial.println("");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
    digitalWrite(led, HIGH);
}

Actions resolveAction(std::string input) {
    if( input == "als" ) return Als;
    if( input == "alt" ) return Alt;
    return Ainvalid;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
        USE_SERIAL.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
        {
            admin = num;
            Serial.println(String(num));
            Serial.println(String(admin));
            IPAddress ip = webSocket.remoteIP(num);
            USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            webSocket.sendTXT(num, "Welcome");
        }
        break;
    case WStype_TEXT:
        USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
        char* payloadChar = (char *) payload;
        switch(resolveAction(payloadChar)){
          case Als:
            adminSwitch = adminSwitch ? false : true;
            webSocket.sendTXT(num, "Done");
            break;
          case Alt:
            webSocket.sendTXT(num, (startTimer ? "true" : "false"));
          break;
        }
  }
}

void setup() {
  Serial.begin(115200);
  for(uint8_t t = 4; t > 0; t--) {
      USE_SERIAL.printf("[BOOT]\n", t);
      USE_SERIAL.flush();
      delay(1000);
  }
  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  connectToWifi();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  now = millis();
  if(admin != 100 && adminSwitch == false && startTimer == true){
    if(lastTrigger > 0){
      if(now - lastTrigger > (timeSeconds*1000)){
        webSocket.sendTXT(admin, "1");
        startTimer = false;
      }
    } else {
        webSocket.sendTXT(admin, "1");
        startTimer = false;
      }
  }
  webSocket.loop();
}
