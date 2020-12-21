#include <ESP8266WebServer.h>
#include <WebServer.h>
#include "web_pages.h"

extern ESP8266WebServer server;

float param[2] = {0, 0};

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
  
  char html[sizeof(MAIN_page)+100]; //NOTE: sizeof(html) > sizeof(MAIN_page)
  
  //Serial.println(sizeof(MAIN_page));
  
  snprintf_P(html, sizeof(html), MAIN_page, param[0], param[1]);

   server.send_P(200, "text/html", html);
}

//===============================================================
// This routine is executed when you press submit
//===============================================================
void handleForm() {
  String par1_in = server.arg("param1"); 
  String par2_in = server.arg("param2"); 

 Serial.print("Parameter 1: ");
 Serial.println(param[0]=par1_in.toFloat());

 Serial.print("parameter 2: ");
 Serial.println(param[1]=par2_in.toFloat());
 
 server.send_P(200, "text/html", SEND_page_OK); 

//if (wrong)
// server.send_P(200, "text/html", SEND_page_FAILURE); 
}