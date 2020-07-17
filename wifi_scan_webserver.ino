#include <stdio.h>
#include <string.h>
#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
//#include "HTTPSRedirect.h"
//#include "sscanf.h"
#include <Ticker.h>

Ticker secondTick;

//*-- Hardware Serial
#define _baudrate	115200
String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete

//Ryan begin
#define ledPin 2 // GPIO2


#define Gladoor 15 
#define Anjin_UP 16
#define Anjin_OFF 14
#define Anjin_STOP 12
#define Anjin_DOWN 13

#define Gladoor_Button_UP    HIGH// LOW
#define Gladoor_Button_DOWN  LOW//HIGH

#define Anjin_Button_UP      HIGH
#define Anjin_Button_DOWN    LOW

enum rolling_door_button
{
 rolling_door_Gladoor=1, 
 rolling_door_Anjin_UP, 
 rolling_door_Anjin_OFF, 
 rolling_door_Anjin_STOP, 
 rolling_door_Anjin_DOWN 
};


int led_blink = 0;
int LED_Status=0;

int save_flag = 0;
int reset_GPIO__flag = 0;

char inChar;
bool WiFi_connected=true;

int data_Gladoor=0;
int data_Anjin_UP=0;
int data_Anjin_OFF=0;
int data_Anjin_STOP=0;
int data_Anjin_DOWN=0;

typedef struct
{
  byte date;	// 1 byte
  byte spraytime;	// 1 byte 
  int boot_count;	// 2 byte 
  char eep_ssid[32];
  char eep_pass[64];
  unsigned int id;
  int eep_intData;
} TDIC_SystemDictionary;								// 6 bytes

TDIC_SystemDictionary  SystemData;
int eeAddress = 0; //EEPROM address to start reading from
//Ryan end

WebServer server(80);

/* Put your SSID & Password */
const char* ssid = "ESP32_AP";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

uint8_t LED1pin = 4;
bool LED1status = LOW;

uint8_t LED2pin = 5;
bool LED2status = LOW;


String st;
String content;
int statusCode;


void setup() {
  Serial.begin(_baudrate);	
  delay(100);
  
  pinMode(ledPin, OUTPUT);	

  pinMode(Gladoor, OUTPUT);	
  pinMode(Anjin_UP, OUTPUT);	
  pinMode(Anjin_OFF, OUTPUT);	
  pinMode(Anjin_STOP, OUTPUT);	
  pinMode(Anjin_DOWN, OUTPUT);	

  reset_GPIO__flag = 1;
  remote_door_GPIO_Reset();

  
  //pinMode(buttonPin, INPUT);	 
  secondTick.attach(1,Timer0_callback_1s);
  

  led_blink = 1;
  WiFi_connected=true;
	
  save_flag = 0;

  
  EEPROM.begin(512);
  delay(10);
  
  Serial.println("Reading EEPROM ");
  EEPROM.get(eeAddress, SystemData);	  

  
  Serial.println();
  Serial.print("Hardware ID =");Serial.println(SystemData.id);
  if (SystemData.id>1000)
  {
	  SystemData.id = 2;
	  Serial.print("Hardware ID =");Serial.println(SystemData.id);
  }

  if (true==wifi_connect())
  {

	  launchWeb(0);
      return;	  
  }
  
  WiFi.disconnect();

  if (true==wifi_connect())
  {

	  launchWeb(0);
      return;	  
  }  
  
  WiFi_connected = false;
  setupAP();

 
}
//====================================================================================================================
bool wifi_connect(void)
{
  String esid(SystemData.eep_ssid);// = (String)SystemData.eep_ssid;
  Serial.print("SSID: ");
  Serial.println(esid);
  
  String epass(SystemData.eep_pass);// = (String)SystemData.eep_pass;
  Serial.print("PASS: ");
  Serial.println(epass);  

   if ( esid.length() > 1 ) 
   {
      WiFi.begin(esid.c_str(), epass.c_str());
      return testWifi();     
   } 
	
}
//====================================================================================================================
bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect");  
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) { return true; } 
    delay(1000);
    Serial.print(WiFi.status());    
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
} 

void launchWeb(int webtype) {
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer(webtype);
  
  // Start the server
  server.begin();
  Serial.println("Server started"); 
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
     {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
     }
  }
  Serial.println(""); 
  st = "<ol>";
  for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      st += "<li>";
      st += WiFi.SSID(i);
      st += " (";
      st += WiFi.RSSI(i);
      st += ")";
      st += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*";
      st += "</li>";
    }
  st += "</ol>";
  delay(100);
  
  //WiFi.softAP(ssid, password, 6);
  WiFi.softAP(ssid);
  Serial.println("softap");
  launchWeb(1);
  Serial.println("over");
}

void createWebServer(int webtype)
{
  if ( webtype == 1 ) {
    server.on("/", []() {
        IPAddress ip = WiFi.softAPIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP32 at ";
        content += ipStr;
        content += "<p>";
        content += st;
        content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
        content += "</html>";
        server.send(200, "text/html", content);  
    });
    server.on("/setting", []() {
        String qsid = server.arg("ssid");
        String qpass = server.arg("pass");
        if (qsid.length() > 0 && qpass.length() > 0) {
          Serial.println("clearing eeprom");
          //for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
		  EEPROM_clean_network();
          Serial.println(qsid);
          Serial.println("");
          Serial.println(qpass);
          Serial.println("");
            
          Serial.println("writing eeprom ssid:");
		  strcpy(SystemData.eep_ssid, qsid.c_str());
		  strcpy(SystemData.eep_pass, qpass.c_str());
		  EEPROM.put(eeAddress, SystemData);
		  /*
          for (int i = 0; i < qsid.length(); ++i)
            {
              EEPROM.write(i, qsid[i]);
              Serial.print("Wrote: ");
              Serial.println(qsid[i]); 
            }
          Serial.println("writing eeprom pass:"); 
          for (int i = 0; i < qpass.length(); ++i)
            {
              EEPROM.write(32+i, qpass[i]);
              Serial.print("Wrote: ");
              Serial.println(qpass[i]); 
            }
          */			
          EEPROM.commit();
          content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
          statusCode = 200;
        } else {
          content = "{\"Error\":\"404 not found\"}";
          statusCode = 404;
          Serial.println("Sending 404");
        }
        server.send(statusCode, "application/json", content);
    });
  } else if (webtype == 0) {
    server.on("/", []() {
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      server.send(200, "application/json", "{\"IP\":\"" + ipStr + "\"}");
    });
    server.on("/cleareeprom", []() {
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>Clearing the EEPROM</p></html>";
      server.send(200, "text/html", content);
      Serial.println("clearing eeprom");
      //for (int i = 0; i < sizeof(TDIC_SystemDictionary); ++i) { EEPROM.write(i, 0); }
	  EEPROM_clean_network();
      //EEPROM.commit();
    });
  }
}

void loop() {
    server.handleClient();
	uart_command();
    remote_door_GPIO_Reset();
	EEP_save();
  
}

//======================================================================================================================
void Timer0_callback_1s()
{
	
     if (led_blink>0) led_blink--;  

     if (0==led_blink)
     {
         if (false==WiFi_connected)
         {
             led_blink = 2;
         }
         else
         {
             led_blink = 1;
         }
         
         if (0==LED_Status)
         {
             LED_Status = 1;
         }
         else
         {
             LED_Status = 0;
         }
     }
	
	
	digitalWrite(ledPin, LED_Status);	

}
//======================================================================================================================
void EEPROM_clean_network()
{
	//Serial.println("EEPROM_clean_network...");
	memset (SystemData.eep_ssid, 0, sizeof(SystemData.eep_ssid));
	memset (SystemData.eep_pass, 0, sizeof(SystemData.eep_pass));
    //Serial.println(SystemData.eep_ssid);
	//Serial.println(SystemData.eep_pass);	
	EEPROM.put(eeAddress, SystemData);
	EEPROM.commit();
}
//====================================================================================================================
void uart_command()
{
  char command[50];
  char ch;
  unsigned int id;
  
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:

    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\r') 
	{
        stringComplete = true;
        Serial.println();
    }
	else
	{
		Serial.print(inChar);
		inputString += inChar;		
	}
	
  }
  
    if (stringComplete) 
    {
        stringComplete = false;
		
		strcpy(command, inputString.c_str());
        inputString = "";		

		switch (command[0])
		{
			case 'u':
                    Serial.println("Button UP");
				if ((sscanf(command, "%s %d", &ch,&id))==2)
				{
				    switch (id)
                        {
                            case rolling_door_Gladoor:
                                Serial.println("Gladoor Button UP");
                                digitalWrite(Gladoor, Gladoor_Button_UP);
                            break;
                            
                            case rolling_door_Anjin_UP:
                                 Serial.println("Anjin_UP Button UP");
                                 digitalWrite(Anjin_UP, Anjin_Button_UP);
                            break;
                            
                            case rolling_door_Anjin_OFF:
                                Serial.println("Anjin_OFF Button UP");
                                digitalWrite(Anjin_OFF, Anjin_Button_UP);
                            break;
                            
                            case rolling_door_Anjin_STOP:
                                Serial.println("Anjin_STOP Button UP");
                                digitalWrite(Anjin_STOP, Anjin_Button_UP);
                            break;
                            
                            case rolling_door_Anjin_DOWN:
                                Serial.println("Anjin_DOWN Button UP");
                                digitalWrite(Anjin_DOWN, Anjin_Button_UP);
                            break;

                            default:
                                Serial.println("All Button UP");
                                reset_GPIO__flag = 1;
                                remote_door_GPIO_Reset();                       
                            break;

                         }
                    }                
           
			break;

			case 'd':
                     if ((sscanf(command, "%s %d", &ch,&id))==2)
                     {
                         switch (id)
                             {
                                 case rolling_door_Gladoor:
                                     Serial.println("Gladoor Button down");
                                     digitalWrite(Gladoor, Gladoor_Button_DOWN);
                                 break;
                                 
                                 case rolling_door_Anjin_UP:
                                      Serial.println("Anjin_UP Button down");
                                      digitalWrite(Anjin_UP, Anjin_Button_DOWN);
                                 break;
                                 
                                 case rolling_door_Anjin_OFF:
                                     Serial.println("Anjin_OFF Button down");
                                     digitalWrite(Anjin_OFF, Anjin_Button_DOWN);
                                 break;
                                 
                                 case rolling_door_Anjin_STOP:
                                     Serial.println("Anjin_STOP Button down");
                                     digitalWrite(Anjin_STOP, Anjin_Button_DOWN);
                                 break;
                                 
                                 case rolling_door_Anjin_DOWN:
                                     Serial.println("Anjin_DOWN Button down");
                                     digitalWrite(Anjin_DOWN, Anjin_Button_DOWN);
                                 break;
                     
                                 default:
                                      Serial.println("All Button down");
                                     digitalWrite(Gladoor, Gladoor_Button_DOWN);
                                     digitalWrite(Anjin_UP, Anjin_Button_DOWN);
                                     digitalWrite(Anjin_OFF, Anjin_Button_DOWN);
                                     digitalWrite(Anjin_STOP, Anjin_Button_DOWN);
                                     digitalWrite(Anjin_DOWN, Anjin_Button_DOWN);
                                 break;
                     
                              }
                         }                 

			break;
					
			case 'h':
				 Serial.println("High");
				 //digitalWrite(Spray_Enable_Pin, LOW);		
				 digitalWrite(Gladoor, HIGH);		
			break;

			case 'l':
				 Serial.println("Low");
				 //digitalWrite(Spray_Enable_Pin, HIGH);	
				 digitalWrite(Gladoor, LOW);		
			break;
			
			case 's':
				Serial.println("spray");
				//digitalWrite(Spray_Enable_Pin, LOW);	        			
			break;
			
			case 'c':
				Serial.println("clean");
				SystemData.date = 0;
				SystemData.spraytime = 0;
				SystemData.boot_count = 0;
				EEPROM.put(eeAddress, SystemData);
				EEPROM.commit();			
			break;
			
			case 'n':
				if ((sscanf(command, "%s %d", &ch,&id))==2)
				{
					SystemData.id = id;
					save_flag = 1;
					Serial.print("New HW ID = "); Serial.println(SystemData.id);
				}
					
			break;
			default:
                    Serial.println("u : Button up");
				Serial.println("d : Button down");
                
			     Serial.println("h : Spray_Enable_Pin = high");
				Serial.println("l : Spray_Enable_Pin = low");
				Serial.println("s : Spray now");
				Serial.println("c : clean spay data ");
				Serial.println("n id : set HW ID ");
			break;
			
		}


    }  

	
}
//====================================================================================================================
void remote_door_GPIO_Reset(void)
{
  if (1==reset_GPIO__flag)
  {
      Serial.println("remote_door_GPIO_Reset!!!!!!!!!!!");
      reset_GPIO__flag  = 0;
      digitalWrite(Gladoor, Gladoor_Button_UP);
      digitalWrite(Anjin_UP, Anjin_Button_UP);
      digitalWrite(Anjin_OFF, Anjin_Button_UP);
      digitalWrite(Anjin_STOP, Anjin_Button_UP);
      digitalWrite(Anjin_DOWN, Anjin_Button_UP);
  }	
}

//====================================================================================================================
void EEP_save(void)
{
    if (1==save_flag)
	{
		save_flag = 0;
		EEPROM.put(eeAddress, SystemData);
		EEPROM.commit();
	}		
}
//====================================================================================================================
