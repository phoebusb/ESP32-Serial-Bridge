bool debug = false;

#define VERSION "2.00"

// For AP mode:
String ssid;  // You will connect your phone to this Access Point
String pw; // and this is the password

IPAddress ip(192, 168, 4, 1); // From RoboRemo app, connect to this IP
IPAddress netmask(255, 255, 255, 0);

// You must connect the phone to this AP, then:
// menu -> connect -> Internet(TCP) -> 192.168.4.1:8880  for UART0
//                                  -> 192.168.4.1:8881  for UART1
//                                  -> 192.168.4.1:8882  for UART2




#define NUM_COM   3                 // total number of COM Ports
#define DEBUG_COM 0                 // debug output to COM0
/*************************  COM Port 0 *******************************/
#define UART_BAUD0 19200            // Baudrate UART0
#define SERIAL_PARAM0 SERIAL_8N1    // Data/Parity/Stop UART0
#define SERIAL0_RXPIN 21            // receive Pin UART0
#define SERIAL0_TXPIN 1             // transmit Pin UART0
#define SERIAL0_TCP_PORT 2000       // Wifi Port UART0
/*************************  COM Port 1 *******************************/
#define UART_BAUD1 19200            // Baudrate UART1
#define SERIAL_PARAM1 SERIAL_8N1    // Data/Parity/Stop UART1
#define SERIAL1_RXPIN 16            // receive Pin UART1
#define SERIAL1_TXPIN 17            // transmit Pin UART1
#define SERIAL1_TCP_PORT 4352       // Wifi Port UART1
/*************************  COM Port 2 *******************************/
#define UART_BAUD2 19200            // Baudrate UART2
#define SERIAL_PARAM2 SERIAL_8N1    // Data/Parity/Stop UART2
#define SERIAL2_RXPIN 15            // receive Pin UART2
#define SERIAL2_TXPIN 4             // transmit Pin UART2
#define SERIAL2_TCP_PORT 4353       // Wifi Port UART2

#define bufferSize 1024


//////////////////////////////////////////////////////////////////////////

// ESP32 WiFi <-> 3x UART Bridge
// originally by AlphaLima
// www.LK8000.com


// Disclaimer: Don't use  for life support systems
// or any other situations where system failure may affect
// user or environmental safety.

#include <esp_wifi.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <WebConfig.h>
#include <SPIFFS.h>

HardwareSerial Serial_one(1);
HardwareSerial Serial_two(2);
HardwareSerial* COM[NUM_COM] = {&Serial, &Serial_one , &Serial_two};

#define MAX_NMEA_CLIENTS 4
WiFiServer server_0;
WiFiServer server_1;
WiFiServer server_2;
WiFiServer *server[NUM_COM]={&server_0,&server_1,&server_2};
WiFiClient TCPClient[NUM_COM][MAX_NMEA_CLIENTS];

int com0_br, com1_br, com2_br, com0_tcp, com1_tcp, com2_tcp, tx_power;

uint8_t buf1[NUM_COM][bufferSize];
uint16_t i1[NUM_COM]={0,0,0};

uint8_t buf2[NUM_COM][bufferSize];
uint16_t i2[NUM_COM]={0,0,0};

uint8_t BTbuf[bufferSize];
uint16_t iBT =0;


String params = "["
  "{"
  "'name':'ssid',"
  "'label':'Name of WLAN',"
  "'type':"+String(INPUTTEXT)+","
  "'default':'XCSoar'"
  "},"
  "{"
  "'name':'pwd',"
  "'label':'WLAN Password',"
  "'type':"+String(INPUTTEXT)+","
  "'default':'Flightcomputer'"
  "},"
 "{"
  "'name':'com0_br',"
  "'label':'COM0 Baudrate',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':40,'max':115000,"
  "'default':'19200'"
  "}," 
  "{"
  "'name':'com0_tcp_port',"
  "'label':'COM0 TCP Port',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':40,'max':65000,"  
  "'default':'4352'"
  "},"
  "{"
  "'name':'com1_br',"
  "'label':'COM1 Baudrate',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':40,'max':115000,"
  "'default':'19200'"
  "}," 
  "{"
  "'name':'com1_tcp_port',"
  "'label':'COM1 TCP Port',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':40,'max':65000,"
  "'default':'4353'"
  "},"
 "{"
  "'name':'com2_br',"
  "'label':'COM2 Baudrate',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':40,'max':115000,"
  "'default':'19200'"
  "}," 
  "{"
  "'name':'com2_tcp_port',"
  "'label':'COM2 TCP Port',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':40,'max':65000,"
  "'default':'2000'"
  "},"
  "{"
  "'name':'tx_power',"
  "'label':'Tx Power',"
  "'type':"+String(INPUTRANGE)+","
  "'min':40,'max':80,"
  "'default':'50'"
  "},"
  "{"
  "'name':'debug_on',"
  "'label':'Debug on',"
  "'type':"+String(INPUTCHECKBOX)+","
  "'default':'0'"
  "}"  
  "]";

WebConfig conf;

// Set web server port number to 80
WebServer web(80);  // Object of WebServer(HTTP port, 80 is defult)


void setup() {

  delay(500);

  SPIFFS.begin(true);
  conf.setDescription(params);
  conf.readConfig();
  assign_values();

  server_0 = WiFiServer(com0_tcp);
  server_1 = WiFiServer(com1_tcp);
  server_2 = WiFiServer(com2_tcp);

  COM[0]->begin(com0_br, SERIAL_PARAM0, SERIAL0_RXPIN, SERIAL0_TXPIN);
  COM[1]->begin(com1_br, SERIAL_PARAM1, SERIAL1_RXPIN, SERIAL1_TXPIN);
  COM[2]->begin(com2_br, SERIAL_PARAM2, SERIAL2_RXPIN, SERIAL2_TXPIN);
  
  if(debug) COM[DEBUG_COM]->println("\n\nLK8000 WiFi serial bridge V1.00");
  
  if(debug) COM[DEBUG_COM]->println("Open ESP Access Point mode");
  //AP mode (phone connects directly to ESP) (no router)
  WiFi.mode(WIFI_AP);

  if (conf.values[0] != "") {
   COM[DEBUG_COM]->printf("ssid, pw: %s -%s\n",ssid.c_str(), pw.c_str());
  } 
  WiFi.softAP(ssid.c_str(), pw.c_str()); // configure ssid and password for softAP
  delay(2000);
  WiFi.softAPConfig(ip, ip, netmask); // configure ip address for softAP 

  COM[0]->println("Starting TCP Server 1");  
  if(debug) COM[DEBUG_COM]->println("Starting TCP Server 1");  
  server[0]->begin(); // start TCP server 
  server[0]->setNoDelay(true);
  COM[1]->println("Starting TCP Server 2");
  if(debug) COM[DEBUG_COM]->println("Starting TCP Server 2");  
  server[1]->begin(); // start TCP server 
  server[1]->setNoDelay(true);
  COM[2]->println("Starting TCP Server 3");
  if(debug) COM[DEBUG_COM]->println("Starting TCP Server 3");  
  server[2]->begin(); // start TCP server   
  server[2]->setNoDelay(true);

  esp_err_t esp_wifi_set_max_tx_power((uint8_t) tx_power);  //min 40, max 80

  web.on("/", handle_root);
  web.begin();
  if(debug) COM[DEBUG_COM]->println("Starting Web Server"); ;
  delay(100);


}


void loop() 
{  
  
  for(int num= 0; num < NUM_COM ; num++)
  {
    if (server[num]->hasClient())
    {
      for(byte i = 0; i < MAX_NMEA_CLIENTS; i++){
        //find free/disconnected spot
        if (!TCPClient[num][i] || !TCPClient[num][i].connected()){
          if(TCPClient[num][i]) TCPClient[num][i].stop();
          TCPClient[num][i] = server[num]->available();
          if(debug) COM[DEBUG_COM]->print("New client for COM"); 
          if(debug) COM[DEBUG_COM]->print(num); 
          if(debug) COM[DEBUG_COM]->println(i);
          continue;
        }
      }
      //no free/disconnected spot so reject
      WiFiClient TmpserverClient = server[num]->available();
      TmpserverClient.stop();
    }
  }
 
  for(int num= 0; num < NUM_COM ; num++)
  {
    if(COM[num] != NULL)          
    {
      for(byte cln = 0; cln < MAX_NMEA_CLIENTS; cln++)
      {               
        if(TCPClient[num][cln]) 
        {
          while(TCPClient[num][cln].available())
          {
            buf1[num][i1[num]] = TCPClient[num][cln].read(); // read char from client (LK8000 app)
            if(i1[num]<bufferSize-1) i1[num]++;
          } 

          COM[num]->write(buf1[num], i1[num]); // now send to UART(num):
          i1[num] = 0;
        }
      }
  
      if(COM[num]->available())
      {
        while(COM[num]->available())
        {     
          buf2[num][i2[num]] = COM[num]->read(); // read char from UART(num)
          if(i2[num]<bufferSize-1) i2[num]++;
        }
        // now send to WiFi:
        for(byte cln = 0; cln < MAX_NMEA_CLIENTS; cln++)
        {   
          if(TCPClient[num][cln])                     
            TCPClient[num][cln].write(buf2[num], i2[num]);
        }
        i2[num] = 0;
      }
    }    
  }

  web.handleClient();
}


// Handle root url (/)
void handle_root() {
  conf.handleFormRequest(&web);
  if (web.hasArg("SAVE")) {
    uint8_t cnt = conf.getCount();
    COM[DEBUG_COM]->printf("*********** Konfiguration ************, %i elements\n", cnt);
    for (uint8_t i = 0; i<cnt; i++) {
      COM[DEBUG_COM]->print(conf.getName(i));
      COM[DEBUG_COM]->print(" = ");
      COM[DEBUG_COM]->println(conf.values[i]);
    }
    conf.writeConfig();
  }
}

// Handle root url (/)
void assign_values() {
    ssid =     conf.getString("ssid");
    pw =     conf.getString("pwd");
    com0_br = conf.getInt("com0_br");
    com0_tcp = conf.getInt("com0_tcp_port");
    com1_br = conf.getInt("com1_br");
    com1_tcp = conf.getInt("com1_tcp_port");
    com2_br = conf.getInt("com2_br");
    com2_tcp = conf.getInt("com2_tcp_port");
    tx_power = conf.getInt("tx_power");
    debug = conf.getBool("debug");
}