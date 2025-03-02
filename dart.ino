#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

GxEPD2_3C<GxEPD2_213_Z98c, GxEPD2_213_Z98c::HEIGHT> display(GxEPD2_213_Z98c(/*CS=5*/ 15, /*DC=*/ 4, /*RES=*/ 5, /*BUSY=*/ 16)); // GDEY0213Z98 122x250, SSD1680

#define ENABLE_GxEPD2_GFX 0

#define SSID "<YOUR SSID>"
#define PASSWORD "<PASSWORD>"

// The hostname can be anything you want, feel free to remove this if you don't care. I want the device to be identifiable on my network.
String wifiHostname = "<HOSTNAME>";
String id = SSID;
String pass = PASSWORD ;

ESP8266WebServer server(80);  

const unsigned char irishrail [] PROGMEM = { /* 0X00,0X01,0X20,0X00,0X1C,0X00, */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0x3f, 0xff, 
	0xff, 0xff, 0xfc, 0xff, 0xfe, 0x1f, 0xff, 0x9f, 0xff, 0xfc, 0xff, 0xfc, 0x0f, 0xff, 0x8f, 0xff, 
	0xfc, 0xff, 0xf8, 0x07, 0xff, 0x87, 0xff, 0xfc, 0xff, 0xf0, 0x03, 0xff, 0x83, 0xff, 0xfc, 0xff, 
	0xe0, 0x01, 0xff, 0x81, 0xff, 0xfc, 0xff, 0xc0, 0x00, 0xff, 0x80, 0xff, 0xfc, 0xff, 0x80, 0x00, 
	0x7f, 0x80, 0x7f, 0xfc, 0xff, 0x00, 0x00, 0x3f, 0x80, 0x3f, 0xfc, 0xfe, 0x00, 0x00, 0x1f, 0x80, 
	0x1f, 0xfc, 0xfc, 0x00, 0x00, 0x0f, 0x80, 0x0f, 0xfc, 0xf8, 0x00, 0x00, 0x07, 0x80, 0x07, 0xfc, 
	0xf0, 0x00, 0x00, 0x03, 0x80, 0x03, 0xfc, 0xe0, 0x00, 0x00, 0x01, 0x80, 0x01, 0xfc, 0xc0, 0x00, 
	0x00, 0x00, 0x80, 0x00, 0xfc, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7c, 0xc0, 0x00, 0x60, 0x00, 0x00, 0x00, 0x7c, 0xe0, 0x00, 0x70, 0x00, 0x00, 0x00, 
	0xfc, 0xf0, 0x00, 0x78, 0x00, 0x00, 0x01, 0xfc, 0xf8, 0x00, 0x7c, 0x00, 0x00, 0x03, 0xfc, 0xfc, 
	0x00, 0x7e, 0x00, 0x00, 0x07, 0xfc, 0xfe, 0x00, 0x7f, 0x00, 0x00, 0x0f, 0xfc, 0xff, 0x00, 0x7f, 
	0x80, 0x00, 0x1f, 0xfc, 0xff, 0x80, 0x7f, 0xc0, 0x00, 0x3f, 0xfc, 0xff, 0xc0, 0x7f, 0xe0, 0x00, 
	0x7f, 0xfc, 0xff, 0xe0, 0x7f, 0xf0, 0x00, 0xff, 0xfc, 0xff, 0xf0, 0x7f, 0xf8, 0x01, 0xff, 0xfc, 
	0xff, 0xf8, 0x7f, 0xfc, 0x03, 0xff, 0xfc, 0xff, 0xfc, 0x7f, 0xfe, 0x07, 0xff, 0xfc, 0xff, 0xfe, 
	0x7f, 0xff, 0x0f, 0xff, 0xfc, 0xff, 0xff, 0x7f, 0xff, 0x9f, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc
};

/**
 * Serve up a static reply when requests are received at the root web directory (http://ESP8266_IP/)
 * This is not required but you may want to tailor it.
 */
void serveHttpGet(){
    StaticJsonDocument<300> JSONData;
    // Use the object just like a javascript object or a python dictionary
    JSONData["data"] = "You must send a POST request to /setMessage, as JSON with a key of 'message'.";
    // You can add more fields
    char data[300];
    // Converts the JSON object to String and stores it in data variable
    serializeJson(JSONData,data);
    // Set content type as application/json and send the data
  server.send(200,"application/json",data);
}

/**
 * Parse JSON sent to the API on the ESP8266 device.
 */
void receiveData(){
    StaticJsonDocument<300> JSONData;
    // Deserialize the JSON document
    String jsonString = server.arg("plain");
    DeserializationError error = deserializeJson(JSONData, jsonString);

    // Test if parsing succeeds.
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        server.send(500,"application/json","Error in parsing request from client.");
        return;
    }else{
        // If the JSON data POSTed to the ESP8266 contains a key "dart" ({"dart": "any value here"}) - then run this section
        if(JSONData.containsKey("dart")){
            // Reply to client
            server.send(200,"application/json", "Received data, sending to epaper display");
            // Call function which will update our ePaper display.
            // This is messy and should be cleaned up.
            display_epaper_text(String(JSONData["dart_destination_1"]), String(JSONData["dart_destination_1_time"]), 
                                String(JSONData["dart_destination_2"]), String(JSONData["dart_destination_2_time"]),
                                String(JSONData["dart_destination_3"]), String(JSONData["dart_destination_3_time"]),
                                String(JSONData["dart_destination_4"]), String(JSONData["dart_destination_4_time"]));
        }
        else{
            server.send(400,"application/json","Bad JSON");
            
        }
    }

}
void setup() {
    Serial.begin(9600);
    // If you removed the hostname above, remove the next line.
    WiFi.hostname(wifiHostname.c_str());
    WiFi.begin(SSID,PASSWORD);
    while(WiFi.status() != WL_CONNECTED){
      Serial.println("Connecting");
    delay(1000);
    }
    Serial.println("Connected to");
    Serial.println(WiFi.localIP());
    delay(500);
    // For all root level requests, reply with static data.
    server.on("/",HTTP_GET,serveHttpGet);
    // For all requests to /setMessage, run receiveData()
    server.on("/setMessage",HTTP_POST,receiveData);

    server.begin();

    // Initialise the ePaper display
    display.init(115200, false, 50, false);


}

/**
 * Parse parameterised data from client requests and display them on the ePaper display.
 * This is messy and should be cleaned up.
 */
void display_epaper_text(String DESTINATION_1, String DESTINATION_1_TIME, 
                          String DESTINATION_2, String DESTINATION_2_TIME,
                          String DESTINATION_3, String DESTINATION_3_TIME, 
                          String DESTINATION_4, String DESTINATION_4_TIME)
{
  // Set device to portrait mode
  display.setRotation(0);
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();              
  display.firstPage();                 
  do                                       
  {
    // Background set to white
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeSans9pt7b);
    // Draw Irish Rail logo bitmap
    display.drawBitmap(0, 0, irishrail, 54, 40, GxEPD_BLACK);

    display.setCursor(55, 10);    
    display.setFont(&FreeMonoBold9pt7b);   
    // Modify this yourself
    display.print("STATION CODE"); 

    // Header - SOUTHBOUND
    display.setCursor(0, 60);    
    display.setFont(&FreeMonoBold9pt7b);   
    display.print("SOUTHBOUND"); 
    display.drawLine(0, 61, 120, 61, GxEPD_BLACK);
    // Destination 1
    display.setCursor(0, 80);    
    display.setFont(&FreeMonoBold9pt7b);   
    display.print(DESTINATION_1);  
    display.setCursor(0, 100);       
    display.setFont(&FreeSans9pt7b);
    display.print(DESTINATION_1_TIME);  
    // Destination 2
    display.setCursor(0, 120);    
    display.setFont(&FreeMonoBold9pt7b);   
    display.print(DESTINATION_2);  
    display.setCursor(0, 140);       
    display.setFont(&FreeSans9pt7b);
    display.print(DESTINATION_2_TIME);  

    // Header - NORTHBOUND
    display.setCursor(0, 160);    
    display.setFont(&FreeMonoBold9pt7b);   
    display.print("NORTHBOUND"); 
    display.drawLine(0, 161, 120, 161, GxEPD_BLACK);
    // Destination 3
    display.setCursor(0, 180);    
    display.setFont(&FreeMonoBold9pt7b);   
    display.print(DESTINATION_3);  
    display.setCursor(0, 200);       
    display.setFont(&FreeSans9pt7b);
    display.print(DESTINATION_3_TIME);  


    // Destination 4
    display.setCursor(0, 220);    
    display.setFont(&FreeMonoBold9pt7b);   
    display.print(DESTINATION_4);  
    display.setCursor(0, 240);       
    display.setFont(&FreeSans9pt7b);
    display.print(DESTINATION_4_TIME);  


  }
  while (display.nextPage());    
}

void loop() {
  server.handleClient();
}