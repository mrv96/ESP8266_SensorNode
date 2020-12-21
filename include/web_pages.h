/*
** ESP8266_SensorNode
** File description:
** web_pages
*/

#pragma once

#include <Arduino.h>

const char SEND_page_OK[] PROGMEM = "<body style=\"background-color:lightgreen\"><a><center><h1> <br><br><br><font size=\"10\">New settings acquired successfully!</font></h1></center> </a></body>";

const char SEND_page_FAILURE[] PROGMEM = "<body style=\"background-color:rgba(255,20,0,0.6)\"><a><center><h1><p> <br><br><br><font size=\"10\">Oops! Something went wrong.</font></p></h1></center> </a></body>";

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head> 
    <title>Change Placeholder alignment</title> 
    <style> 
      
      input[type="text"]::placeholder { 
        text-align: center; 
      } 

      body { 
        text-align:center; 
      }
    </style> 
  </head> 
    
<body style="background-color:powderblue;">

<h1><font size="10">NODE SETTING</font></h1>

<form action="/action_page">
 <b> <font size="4">parameter 1:</font></b>
 <br>
  <input type="number" name="param1"  min="1" max="5" style="cursor:pointer;"  title="meaning of param1"> 
 <br>
 <i>actual value %.2f</i>
  <br>
  <br>
  <br>
  <b><font size="4">parameter 2:</font></b><br>
  <input type="number" name="param2"  min="-10" max="40" style="cursor:pointer;"  title="meaning of param2">
  <br>
  <i>actual value %.2f</i>
  <br>
  <br>
  <br>
  <input type="submit" value="Submit">
  
</form> 
</body>
</html>
)=====";