#include <EEPROM.h>
#include <SPI.h>
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "config.h"

Config_t config;
ESP8266WebServer server(80);

String getHeader() {
    String header = "<!doctype html>\n"
"<html lang='en'>\n"
"<head>\n"
"  <meta charset='utf-8'>\n"
"  <meta name='viewport' content='width=device-width, initial-scale=1'>\n"
"  <title>" + WiFi.localIP().toString() + "</title>\n"
"  <link rel='stylesheet' href='https://code.jquery.com/ui/1.12.1/themes/base/jquery-ui.css'>\n"
"  <link href='https://media.karotka.cz/esp/styles.css' rel='stylesheet' type='text/css'>\n"
"  <script src='https://code.jquery.com/jquery-1.12.4.js'></script>\n"
"  <script src='https://code.jquery.com/ui/1.12.1/jquery-ui.js'></script>\n";
    return header;
}

void handleRoot() {

    String ret = getHeader() +
"  <script>\n"
"  $( function() {\n"
"    $( '#slider-vertical' ).slider({\n"
"      orientation: 'vertical',\n"
"      range: 'min',\n"
"      min: 0,\n"
"      max: 100,\n"
"      value: 60,\n"
"      slide: function( event, ui ) {\n"
"        $( '#amount' ).val( ui.value );\n"
"        $.get( '/val', { v: $( '#slider-vertical' ).slider( 'value' ) } );\n"
"      }\n"
"    });\n"
"    $( '#amount' ).val( $( '#slider-vertical' ).slider( 'value' ) );\n"
"  } );\n"
"  </script>\n"
"</head>\n"
"<body>\n"
"<p>\n"
"  <label for='amount'>Volume:</label>\n"
"  <input type='text' id='amount' readonly style='border:0; padding:0px 0px 4px 60px; font-weight:bold;'>\n"
"</p>\n"
"<div id='slider-vertical' style='height:200px;margin-left:20px;'></div>\n"
"</body>\n"
"</html>\n";

    server.setContentLength(ret.length());
    server.send(200, "text/html", ret);
}

void handleSetup() {
    String ret = getHeader() +
"<main>\n"
"  <form>\n"
"    <span>\n"
"      <label for='ssid' class='text-small-uppercase'>SSID</label>\n"
        "      <input class='text-body' id='ssid' name='ssid' type='text' value='" + config.ssid + "'>\n"
"    </span>\n"
"    <span>\n"
"      <label for='password' class='text-small-uppercase'>Password</label>\n"
"      <input class='text-body' id='password' name='password' type='text' value='" + config.password + "'>\n"
"    </span>\n"
"    <span>\n"
"      <label for='ip' class='text-small-uppercase'>IP</label>\n"
"      <input class='text-body' id='ip' name='ip' type='text' value='" + config.ip.toString() + "'>\n"
"    </span>\n"
"    <span>\n"
"      <label for='gw' class='text-small-uppercase'>Gateway</label>\n"
"      <input class='text-body' id='gateway' name='gateway' type='text' value='" + config.gateway.toString() + "'>\n"
"    </span>\n"
"    <span>\n"
"      <label for='nmask' class='text-small-uppercase'>Netmask</label>\n"
"      <input class='text-body' id='subnet' name='subnet' type='text' value='" + config.subnet.toString() + "'>\n"
"    </span>\n"
"    <div>\n"
"    <input class='text-small-uppercase' id='btn' type='button' value='Save'>\n"
"    </div>\n"
"  </form>\n"
"</main>\n"
"<script>\n"
"$('#btn').click(function() {\n"
"  $.get('/ssave', {ip: $('#ip').val(), gateway : $('#gateway').val(), subnet : $('#subnet').val()}).success(\n"
"    function(result, data) {\n"
"        window.location.href='http://' + result + '/';\n"
"    }\n"
")});\n"
"</script>\n"
"</body>\n"
"</html>\n";

    server.setContentLength(ret.length());
    server.send(200, "text/html", ret);
}

void digitalPotWrite(int value) {
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(CS, LOW);
    SPI.transfer(POT_ADDRESS);
    SPI.transfer(value);
    digitalWrite(CS, HIGH);
    delay(10);
    digitalWrite(LED_BUILTIN, HIGH);
}

void handleVal() {
    String v = server.arg("v");
    unsigned int val = map(v.toInt(), 0, 255, 0, 100);
    digitalPotWrite(val);

    server.send(200, "text/html", "OK");
}

void spiBegin() {
    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH);
    SPI.begin();
}

void wifiConnect() {

    // Connect to WiFi network
    Serial.println("");
    Serial.print("Connecting to: ");
    Serial.print(config.ssid);

    WiFi.config(config.ip, config.gateway, config.subnet);
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid, config.password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("WiFi connected: ");
    Serial.print("http://");
    Serial.print(WiFi.localIP().toString());
    Serial.println("/");
}

void handleSSave() {
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    ip.fromString(server.arg("ip"));
    gateway.fromString(server.arg("gateway"));
    subnet.fromString(server.arg("subnet"));

    config.ip = ip;
    config.gateway = gateway;
    config.subnet = subnet;
    server.send(200, "text/plain", server.arg("ip"));
    delay(1000);
    wifiConnect();
}

void setup() {
    Serial.begin(115200);
    delay(10);

    wifiConnect();

    // Start the server
    server.on("/", handleRoot);
    server.on("/val", handleVal);
    server.on("/setup", handleSetup);
    server.on("/ssave", handleSSave);
    server.begin();
    Serial.println("Server started");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    spiBegin();
}

void loop() {
    server.handleClient();
}
