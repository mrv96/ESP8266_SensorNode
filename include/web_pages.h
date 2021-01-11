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
    <title>Node Setup</title> 
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
  
    <br>
  <br>

<form action="/action_page">
  <font size="3">Actual IP address of the node %s:</font>
  <br>
   <br>
   
     <script type="text/javascript">
    function EnableDisableTextBox(checkIP) {
        
        ip.disabled = checkIP.checked ? true : false;
        
        if (!ip.disabled) {
            ip.focus();
        }
    }
</script>

<font size="3">Set your own IP &emsp;  <input id="ip" name="nodeAddress" disabled value="%s" />       
<label for="checkIP"> &emsp;  &emsp; 
    <input type="checkbox" id="checkIP" name="ip_changed" value="false" onclick="EnableDisableTextBox(this)" checked/>
    Use IP assigned by DHCP
</label>
  <br>  
   <br>
     <br>
     <font size="1"> <em>Hover the mouse over the boxex for tooltips</em></font>
  <br>
  <br>
 <b> <font size="4">ThingSpeak channel number:</font></b>
  <input type="number" name="myChannelNumber"     title="Insert here the channel number provided by ThingSpeak platoform">
    <br>
   <b> &emsp; &emsp; &emsp;<font size="4">ThingSpeak API Key:</font></b> 
  <input type="number" name="myWriteAPIKey"     title="Insert here the key provided by ThingSpeak platoform"> 
     <br>
  <br>
  <br>
 <b> <font size="4">Sample interval:</font></b>
  <input type="number" name="sampleTime"  min="1" max="50" style="cursor:pointer;"  title="Set how often the node will sample the environmental parameters"> 
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
  
  

  <br>
  <br>
  <br>
  
  <input  type="submit" value="Submit">
  
</form> 
</body>
</html>
)=====";