#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef STASSID
#define STASSID "Murty"
#define STAPSK  "anand6301"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

int sensor1 = 12;//D12
int sensor2 = 15;//D10
int sensor3 = 3;//D0

int led=5;//D3 
int led1=4;//D4 
int led2=14;//D5

int smooth;
int LDR;
float beta = 0.65;

int Energy=0;

int switch1=1;
int switching=0;
bool change=true;
//Check if header is present and correct
bool is_authenticated() {
  Serial.println("Enter is_authenticated");
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentication Successful");
      return true;
    }
  }
  Serial.println("Authentication Failed");
  return false;
}

//login page, also called for disconnect
void handleLogin() {
  String msg;
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
    server.send(301);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")) {
    if (server.arg("USERNAME") == "admin" &&  server.arg("PASSWORD") == "admin") {
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
      server.send(301);
      Serial.println("Log in Successful");
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
  }
  String content = "<html><body><H2></H2>Welcome To SMART STREET LIGHT SYSTEM</H2><br>";
  content += "<html><body><H2></H2>                                              </H2><br>";
  content += "<html><body><form action='/login' method='POST'>To Log In, Please enter your Credentiasls<br>";
  content += "<html><body><H2></H2>                                              </H2><br>";
  content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  content += "You also can also check Energy Saved <a href='/inline'>here</a></body></html>";
  server.send(200, "text/html", content);
}

//root page can be accessed only if authentication is ok
void handleRoot() {
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authenticated()) {
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }
  String content = "<html><body><H2>Hello, you successfully Authenticated</H2><br>";
  content += "<html><body><H2></H2>                                              </H2><br>";
  content += "<html><body><H2></H2>Welcome To IOT Based Smart Street Light SYSTEM</H2><br>";
  content += "<html><body><H2></H2>                                              </H2><br>";
  
  
  if(digitalRead(switch1) == 1)
  {
    change=~change;
  }
  
  if(server.arg("mode") == "Automatic" || change == true){
    switching=0;
  }
  if(server.arg("mode") == "Manual" || change == false){
    switching=1;
  }

    if (switching == 0){
       content += "<html><body><H2>Automatic</H2><br>";
       smooth = smooth - (beta * (smooth -  analogRead(A0)));
        LDR = round(((float)smooth / 1023) * 100);
  if (LDR >= 40){
    content += "<html><body><H2></H2>Lights are Not Needed.It's Morning</H2><br>";
    content += "<html><body><H2></H2>IOT Smart Street Light Works During Night</H2><br>";
}
else{
    if (digitalRead(sensor1) == 1 && digitalRead(sensor2) == 1 && digitalRead(sensor3) == 1 )
    {
       content += "<html><body><H2>All Lights are On.</H2><br>";
       }
    if (digitalRead(sensor1) == 1 && digitalRead(sensor2) == 1 && digitalRead(sensor3) == 0 ){
        content += "<html><body><H2>Light 1-On. Light 2-On Light 3-Off</H2><br>";
       }
    if (digitalRead(sensor1) == 1 && digitalRead(sensor2) == 0 && digitalRead(sensor3) == 1 ){
       content += "<html><body><H2>Light 1-On. Light 2-Off Light 3-On</H2><br>";
    }
    if (digitalRead(sensor1) == 1 && digitalRead(sensor2) == 0 && digitalRead(sensor3) == 0 ){
      content += "<html><body><H2>Light 1-On. Light 2-Off Light 3-Off</H2><br>";
       }
    if (digitalRead(sensor1) == 0 && digitalRead(sensor2) == 1 && digitalRead(sensor3) == 1 ){
       content += "<html><body><H2>Light 1-Off. Light 2-On Light 3-On</H2><br>";
    }
    if (digitalRead(sensor1) == 0 && digitalRead(sensor2) == 1 && digitalRead(sensor3) == 0 ){
       content += "<html><body><H2>Light 1-Off. Light 2-On Light 3-Off</H2><br>";
   }
    if (digitalRead(sensor1) == 0 && digitalRead(sensor2) == 0 && digitalRead(sensor3) == 1 ){
       content += "<html><body><H2>Light 1-Off. Light 2-Off Light 3-On</H2><br>";

    }
    if (digitalRead(sensor1) == 0 && digitalRead(sensor2) == 0 && digitalRead(sensor3) == 0 ){
       content += "<html><body><H2>No Vehicles                       </H2><br>";
    }
  }
    }
  if (server.hasHeader("User-Agent")) {
    content += "the user agent used is : " + server.header("User-Agent") + "<br><br>";
  }
  content += "You can choose type of working of system<a href=\"/Manual\">Manual</a></body></html>";
  content += "<html><body><H2></H2>                                          </H2><br>";
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  server.on("/Manual", handleManual);
  server.send(200, "text/html", content);
}

void handleManual(){
  String content = "<html><body><H2></H2>Welcome To Manual STREET LIGHT SYSTEM</H2><br>";
  content += "You can choose type of working of system<a href=\"/Manualon\">All on</a></body></html>";
  content += "\n";
   content += "You can choose type of working of system<a href=\"/Manualoff\">All off</a></body></html>";
   content += "\n";
   content += "You can choose type of working of system<a href=\"/Manual3\">Enter the number</a></body></html>";
  content += "\n";
  content += "<html><body><H2></H2>                                              </H2><br>";
  content += "You can choose type of working of system<a href=\"/Automatic\">Automatic</a></body></html>";
  content += "<html><body><H2></H2>                                          </H2><br>";
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  server.send(200, "text/html", content);
}

void handleManualoff(){
  String content = "<html><body><H2></H2>Welcome To Manual STREET LIGHT SYSTEM</H2><br>";
  content += "<html><body><H2>No Vehicles                       </H2><br>";
  content += "<html><body><H2></H2>                                              </H2><br>";
  content += "You can choose type of working of system<a href=\"/Automatic\">Automatic</a></body></html>";
  content += "<html><body><H2></H2>                                          </H2><br>";
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  server.send(200, "text/html", content);
  digitalWrite(led,LOW);
       digitalWrite(led1,LOW);
       digitalWrite(led2,LOW);
       Energy = Energy+300;
}

void handleManualon(){
  String content = "<html><body><H2></H2>Welcome To Manual STREET LIGHT SYSTEM</H2><br>";
  content += "<html><body><H2>All Lights are On.</H2><br>";
  content += "<html><body><H2></H2>                                              </H2><br>";
  content += "You can choose type of working of system<a href=\"/Automatic\">Automatic</a></body></html>";
  content += "<html><body><H2></H2>                                          </H2><br>";
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  server.send(200, "text/html", content);
  digitalWrite(led,HIGH);
   digitalWrite(led1,HIGH);
   digitalWrite(led2,HIGH);
}

void handleManual3(){
  String content = "<html><body><H2></H2>Welcome To Manual STREET LIGHT SYSTEM</H2><br>";
  content += "<html><body><H2></H2>                                              </H2><br>";
  content += "Enter Light Number:<input type='text' name='light' placeholder='Light'><br>";
        content += "<input type='submit' name='SUBMIT' value='Submit'></form><br>";
    
  content += "You can choose type of working of system<a href=\"/Automatic\">Automatic</a></body></html>";
  content += "<html><body><H2></H2>                                          </H2><br>";
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  server.send(200, "text/html", content);
}

void handleManual4(){
  String content = "<html><body><H2></H2>Welcome To Manual STREET LIGHT SYSTEM</H2><br>";
  content += "<html><body><H2></H2>                                              </H2><br>";
  if (server.hasArg("light")) {
    if (server.arg("light") == "two"){
          digitalWrite(led,HIGH);
          content += "<html><body><H2></H2>Light 1 is on Successfully     </H2><br>";
          server.send(301);
          return;
        }
    if (server.arg("light") == "two"){
          digitalWrite(led1,HIGH);
          content += "<html><body><H2></H2>Light 2 is on Successfully     </H2><br>";
          server.send(301);
          return;
        }
        if (server.arg("light") == "three"){
          digitalWrite(led2,HIGH);
          content += "<html><body><H2></H2>Light 3 is on Successfully     </H2><br>";
          server.send(301);
          return;
   }
  }
  content += "You can choose type of working of system<a href=\"/Automatic\">Automatic</a></body></html>";
  content += "<html><body><H2></H2>                                          </H2><br>";
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  server.send(200, "text/html", content);
}

void handleAutomatic(){
  String content = "<html><body><H2></H2>Welcome To Smart STREET LIGHT SYSTEM</H2><br>";
  content += "<html><body><H2></H2>                                              </H2><br>";
if (LDR >= 40){
    content += "<html><body><H2></H2>Lights are Not Needed.It's Morning</H2><br>";
    content += "<html><body><H2></H2>IOT Smart Street Light Works During Night</H2><br>";
}
else{
    if (digitalRead(sensor1) == 1 && digitalRead(sensor2) == 1 && digitalRead(sensor3) == 1 )
    {
       content += "<html><body><H2>All Lights are On.</H2><br>";
       }
    if (digitalRead(sensor1) == 1 && digitalRead(sensor2) == 1 && digitalRead(sensor3) == 0 ){
        content += "<html><body><H2>Light 1-On. Light 2-On Light 3-Off</H2><br>";
       }
    if (digitalRead(sensor1) == 1 && digitalRead(sensor2) == 0 && digitalRead(sensor3) == 1 ){
       content += "<html><body><H2>Light 1-On. Light 2-Off Light 3-On</H2><br>";
    }
    if (digitalRead(sensor1) == 1 && digitalRead(sensor2) == 0 && digitalRead(sensor3) == 0 ){
      content += "<html><body><H2>Light 1-On. Light 2-Off Light 3-Off</H2><br>";
       }
    if (digitalRead(sensor1) == 0 && digitalRead(sensor2) == 1 && digitalRead(sensor3) == 1 ){
       content += "<html><body><H2>Light 1-Off. Light 2-On Light 3-On</H2><br>";
    }
    if (digitalRead(sensor1) == 0 && digitalRead(sensor2) == 1 && digitalRead(sensor3) == 0 ){
       content += "<html><body><H2>Light 1-Off. Light 2-On Light 3-Off</H2><br>";
   }
    if (digitalRead(sensor1) == 0 && digitalRead(sensor2) == 0 && digitalRead(sensor3) == 1 ){
       content += "<html><body><H2>Light 1-Off. Light 2-Off Light 3-On</H2><br>";

    }
    if (digitalRead(sensor1) == 0 && digitalRead(sensor2) == 0 && digitalRead(sensor3) == 0 ){
       content += "<html><body><H2>No Vehicles                       </H2><br>";
    }
  }
  content += "You can choose type of working of system<a href=\"/Manual\">Manual</a></body></html>";
  content += "<html><body><H2></H2>                                          </H2><br>";
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  server.send(200, "text/html", content);
}
//no need authentication
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
void condition(){
    if (switching == 0){
       smooth = smooth - (beta * (smooth -  analogRead(A0)));
  delay(1000);
  LDR = round(((float)smooth / 1023) * 100);
  if (LDR >= 40){
    digitalWrite(led,LOW);
    digitalWrite(led1,LOW);
    digitalWrite(led2,LOW);
    Energy = Energy+300;
}
else{
    if (digitalRead(sensor1) == 1 && digitalRead(sensor2) == 1 && digitalRead(sensor3) == 1 )
    {
       digitalWrite(led,HIGH);
       digitalWrite(led1,HIGH);
       digitalWrite(led2,HIGH);
       Energy = Energy+100;
       }
    if (digitalRead(sensor1) == 1 && digitalRead(sensor2) == 1 && digitalRead(sensor3) == 0 ){
        digitalWrite(led,HIGH);
       digitalWrite(led1,HIGH);
       digitalWrite(led2,LOW);
       Energy = Energy+200;
       }
    if (digitalRead(sensor1) == 1 && digitalRead(sensor2) == 0 && digitalRead(sensor3) == 1 ){
        digitalWrite(led,HIGH);
        digitalWrite(led1,LOW);
        digitalWrite(led2,HIGH);
        Energy = Energy+200;
      }
    if (digitalRead(sensor1) == 1 && digitalRead(sensor2) == 0 && digitalRead(sensor3) == 0 ){
      digitalWrite(led,HIGH);
       digitalWrite(led1,LOW);
       digitalWrite(led2,LOW);
       Energy = Energy+100;
       }
    if (digitalRead(sensor1) == 0 && digitalRead(sensor2) == 1 && digitalRead(sensor3) == 1 ){
       digitalWrite(led,LOW);
       digitalWrite(led1,HIGH);
       digitalWrite(led2,HIGH);
       Energy = Energy+200;
       }
    if (digitalRead(sensor1) == 0 && digitalRead(sensor2) == 1 && digitalRead(sensor3) == 0 ){
       digitalWrite(led,LOW);
       digitalWrite(led1,HIGH);
       digitalWrite(led2,LOW);
       Energy = Energy+100;    }
    if (digitalRead(sensor1) == 0 && digitalRead(sensor2) == 0 && digitalRead(sensor3) == 1 ){
       digitalWrite(led,LOW);
       digitalWrite(led1,LOW);
       digitalWrite(led2,HIGH);
       Energy = Energy+100;
    }
    if (digitalRead(sensor1) == 0 && digitalRead(sensor2) == 0 && digitalRead(sensor3) == 0 ){
       delay(1000);
       digitalWrite(led,LOW);
       digitalWrite(led1,LOW);
       digitalWrite(led2,LOW);
       Energy = Energy+300;
    }
  }
    }
}

void setup(void) {
  Serial.begin(115200);
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(sensor3, INPUT);
  pinMode(led,OUTPUT);
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(switch1, INPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/inline", []() {
    String Energy1=String(0.00001*Energy);
    server.send(200, "text/plain", "Energy Saving By Smart Streel Light System is "+Energy1+" Watts");
  });
  server.on("/Manual", handleManual);
  server.on("/Manualoff", handleManualoff);
  server.on("/Manualon", handleManualon);
  server.on("/Manual3", handleManual3);
  server.on("/Manual4", handleManual4);
  server.on("/Automatic", handleAutomatic);
  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  condition();
  server.handleClient();
}
