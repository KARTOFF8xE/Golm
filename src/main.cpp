#include <Arduino.h>
#include <WiFiS3.h>

const char* ssid = "Arduino_AP";  // ssid
const char* password = "12345678"; // password

WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    Serial.println("Starte Access Point...");

    // create own wifi
    WiFi.beginAP(ssid, password);
    while (WiFi.status() != WL_AP_LISTENING) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nAccess Point");
    Serial.print("connect to: ");
    Serial.println(ssid);
    Serial.print("open in browser: http://");
    Serial.println(WiFi.localIP());

    server.begin();
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
            if (c == '\n') break;
        }
    }

    // search for taken int-value
    int value = -1;
    int pos = request.indexOf("GET /submit?value=");
    if (pos != -1) {
        String valStr = request.substring(pos + 18);  // 18 characters for "GET /submit?value="
        valStr = valStr.substring(0, valStr.indexOf(" "));
        value = valStr.toInt();
        Serial.print("Typed number: ");
        Serial.println(value);
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<!DOCTYPE html>");
    client.println("<html lang='de'><head><meta charset='UTF-8'>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    client.println("<title>Arduino Eingabe</title>");
    client.println("</head><body style='font-family: Arial, sans-serif;'>");
    client.println("<h1>UNO R4 WiFi Access Point</h1>");
    client.println("<p>Gib eine Zahl ein:</p>");
    client.println("<form action='/submit' method='GET'>");
    client.println("<input type='number' name='value' required>");
    client.println("<input type='submit' value='Senden'>");
    client.println("</form>");
    client.println("</body></html>");
    
    client.stop();
}
