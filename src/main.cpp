#include <Arduino.h>
#include <seesaw_neopixel.h>
#include "Adafruit_seesaw.h"

#include <WiFiS3.h>

#include "config.cpp"
#include "csd.cpp"

/***Seesaw***/
Adafruit_seesaw ss;
seesaw_NeoPixel sspixel = seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800);
int32_t encoder_position;

/***WiFi***/
const char* ssid = "Arduino_AP";  // ssid
const char* password = "12345678";  // pwd
WiFiServer server(80);
bool switchState = false;   // switch state, false is counterClockwise, true is Clockwise
float inputVel = 0;         // velocity

/***Cordless ScrewDriver***/
CSD* csd;

void setupRotaryEncoder() {
    Serial.println("Looking for seesaw!");
    if (!ss.begin(SEESAW_ADDR) || !sspixel.begin(SEESAW_ADDR)) {
        Serial.println("Couldn't find seesaw!");
        while(1) delay(10);
    }
    Serial.println("Seesaw started");

    uint32_t version = ((ss.getVersion() >> 16) & 0xFFFF);
    Serial.print("Seesaw Version: ");
    Serial.println(version);
    
    if (version != 4991) {
        Serial.println("Wrong firmware!");
        while(1) delay(10);
    }

    sspixel.setBrightness(20);
    sspixel.show();

    ss.pinMode(SS_SWITCH, INPUT_PULLUP);
    encoder_position = ss.getEncoderPosition();
    Serial.println("Turning on interrupts");
    ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
    ss.enableEncoderInterrupt();
}

void setupCSD() {
    pinMode(clockwise, OUTPUT);
    pinMode(counterClockwise, OUTPUT);
    pinMode(dacPin, OUTPUT);

    csd = new CSD(dacPin);
    csd->setDirection(clockwise);
    csd->setSpeed(0);
}

void setupWiFi() {
    WiFi.beginAP(ssid, password);
    while (WiFi.status() != WL_AP_LISTENING) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nAccess Point online!");
    Serial.print("Connect to: ");
    Serial.println(ssid);
    Serial.print("Open: http://");
    Serial.println(WiFi.localIP());

    server.begin();
}

volatile unsigned long lastInterruptTime = 0;
void emergenyStop() {
    unsigned long currentTime = millis();
    if (currentTime - lastInterruptTime > 50) {
        lastInterruptTime = currentTime;
        
        // attachInterrupt(digitalPinToInterrupt(1), emergenyStop, RISING);
        attachInterrupt(digitalPinToInterrupt(2), emergenyStop, RISING);
        csd->breakSpeed();
        // Serial.println("Foo");
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    setupRotaryEncoder();
    setupCSD();
    setupWiFi();

    pinMode(1, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(2), emergenyStop, FALLING);
}

void loop() {
    WiFiClient client = server.available();
    if (!client) {
        return;
    }

    String request = "";
    
    while (client.connected()) {
        if (client.available()) {
            char c = client.read();
            request += c;
            if (c == '\n' && request.endsWith("\r\n\r\n")) {
                break;
            }
        }
    }

    // process switch-toggle
    if (request.indexOf("GET /toggle") != -1) {
        int pos = request.indexOf("state=");
        if (pos != -1) {
            int start = pos + 6;
            String stateStr = request.substring(start, start + 1);
            switchState = (stateStr == "1");
        }
        Serial.print("Direction: ");
        Serial.println(switchState ? "clockwise" : "counterClockwise");
        csd->setDirection(switchState ? clockwise : counterClockwise);

        // response to client
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.println();
        client.println(switchState ? "clockwise" : "counterClockwise");
        client.stop();
        return;
    }

    // process number
    if (request.indexOf("GET /setnum") != -1) {
        int pos = request.indexOf("num=");
        if (pos != -1) {
            int start = pos + 4;
            String numStr = request.substring(start);
            numStr = numStr.substring(0, numStr.indexOf(" "));
            inputVel = numStr.toFloat();
        }
        Serial.print("Input: ");
        Serial.println(inputVel);
        csd->accelerateToVel(inputVel, ss);

        // response number
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.println();
        client.println(String(inputVel));
        client.stop();
        return;
    }

    if (request.indexOf("GET /brake") != -1) {
        csd->breakSpeed();
        
        // response to client
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.stop();
        return;
    }

    // Process stop request
    if (request.indexOf("GET /stop") != -1) {
        csd->setSpeed(0);
        
        // response to client
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.stop();
        return;
    }

    // html page
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<!DOCTYPE html>");
    client.println("<html lang='de'><head><meta charset='UTF-8'>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    client.println("<title>Arduino Steuerung</title>");
    client.println("<style>");
    client.println("body { font-family: Arial, sans-serif; text-align: center; }");
    client.println(".switch { position: relative; display: inline-block; width: 60px; height: 34px; }");
    client.println(".switch input { opacity: 0; width: 0; height: 0; }");
    client.println(".slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0;");
    client.println(" background-color: #ccc; transition: .4s; border-radius: 34px; }");
    client.println(".slider:before { position: absolute; content: ''; height: 26px; width: 26px;");
    client.println(" left: 4px; bottom: 4px; background-color: white; transition: .4s; border-radius: 50%; }");
    client.println("input:checked + .slider { background-color: #4CAF50; }");
    client.println("input:checked + .slider:before { transform: translateX(26px); }");
    client.println("input, button { margin: 10px; padding: 10px; font-size: 16px; display: block; width: 200px; margin-left: auto; margin-right: auto; }");
    client.println(".stopButton { background-color: red; color: white; }");
    client.println("</style>");
    client.println("</head><body>");
    client.println("<h1>UNO R4 WiFi Steuerung</h1>");

    // toggle on/off
    client.println("<p>Switch Status: <strong id='switchStateText'>" + String(switchState ? "clockwise" : "counterClockwise") + "</strong></p>");
    client.println("<label class='switch'>");
    client.println("<input type='checkbox' id='toggleSwitch' " + String(switchState ? "checked" : "") + ">");
    client.println("<span class='slider'></span>");
    client.println("</label>");

    // input number
    client.println("<h2>Steuerung</h2>");
    client.println("<p>Current Vel: <strong id='numberDisplay'>" + String(inputVel) + "</strong></p>");
    client.println("<input type='number' id='numberInput' value='" + String(inputVel) + "'>");
    client.println("<button onclick='sendNumber()'>Start</button>");

    // break and stop Buttons
    // client.println("<button onclick='sendBreak()'>Auf 0 Bremsen</button>");
    client.println("<button onclick='sendBrake()'>Abbremsen auf 0</button>");
    client.println("<button class='stopButton' onclick='sendStop()'>Notstop</button>");

    // JavaScript for asynchronous instrumentation
    client.println("<script>");
    
    // Switch-Handling
    client.println("document.getElementById('toggleSwitch').addEventListener('change', function() {");
    client.println("  let state = this.checked ? '1' : '0';");
    client.println("  fetch('/toggle?state=' + state)"); 
    client.println("    .then(response => response.text())");  
    client.println("    .then(data => { document.getElementById('switchStateText').innerText = data; });");
    client.println("});");

    // response number
    client.println("function sendNumber() {");
    client.println("  let num = document.getElementById('numberInput').value;");
    client.println("  fetch('/setnum?num=' + num)"); 
    client.println("    .then(response => response.text())");  
    client.println("    .then(data => { document.getElementById('numberDisplay').innerText = data; });");
    client.println("}");

    // Brake Button Handling
    client.println("function sendBrake() {");
    client.println("  fetch('/brake')"); // Sende eine Anfrage zum Bremsen
    client.println("    .then(response => response.text())");
    client.println("}");

    // Stop Button Handling
    client.println("function sendStop() {");
    client.println("  fetch('/stop')"); // Sende eine Anfrage zum Stoppen
    client.println("    .then(response => response.text())");
    client.println("}");
    
    client.println("</script>");

    client.println("</body></html>");
    client.stop();
}
