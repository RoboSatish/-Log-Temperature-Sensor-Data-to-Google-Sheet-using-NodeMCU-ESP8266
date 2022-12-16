
#include <ESP8266WiFi.h>

#include "HTTPSRedirect.h"

#include "DebugMacros.h"

#include <DHT.h>


#define DHTPIN D4                                                     
#define DHTTYPE DHT11                                                    

DHT dht(DHTPIN, DHTTYPE);


float h;

float t;

String sheetHumid = "";

String sheetTemp = "";


const char* ssid = "satish";               

const char* password = "satish123";        


const char* host = "script.google.com";

const char *GScriptId = "AKfycbxy9wAZKoPIpPq5AvqYTFFn5kkqK_-avacf2NU_w7ycoEtlkuNt"; 

const int httpsPort = 443; //the https port is same



const char* fingerprint = "";




String url = String("/macros/s/") + GScriptId + "/exec?value=Temperature";  



String url2 = String("/macros/s/") + GScriptId + "/exec?cal"; 


String payload_base =  "{\"command\": \"appendRow\", \

                    \"sheet_name\": \"TempSheet\", \

                       \"values\": ";

String payload = "";


HTTPSRedirect* client = nullptr;



void setup() {

  delay(1000);

  Serial.begin(115200);

  dht.begin();     


  Serial.println();

  Serial.print("Connecting to wifi: ");

  Serial.println(ssid);

  

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");

  }

  Serial.println("");

  Serial.println("WiFi connected");

  Serial.println("IP address: ");

  Serial.println(WiFi.localIP());


  

  client = new HTTPSRedirect(httpsPort);

  client->setInsecure();

  client->setPrintResponseBody(true);

  client->setContentTypeHeader("application/json");

  Serial.print("Connecting to ");

  Serial.println(host);      


  bool flag = false;

  for (int i = 0; i < 5; i++) {

    int retval = client->connect(host, httpsPort);

    if (retval == 1) {

      flag = true;

      break;

    }

    else

      Serial.println("Connection failed. Retrying...");

  }


  if (!flag) {

    Serial.print("Could not connect to server: ");

    Serial.println(host);

    Serial.println("Exiting...");

    return;

  }



  Serial.println("\nWrite into cell 'A1'");

  Serial.println("------>");



  client->GET(url, host);

  

  Serial.println("\nGET: Fetch Google Calendar Data:");

  Serial.println("------>");



  client->GET(url2, host);


 Serial.println("\nStart Sending Sensor Data to Google Spreadsheet");


  



  delete client;

  client = nullptr;

}


void loop() {


  h = dht.readHumidity();                                            

  t = dht.readTemperature();                                        

  if (isnan(h) || isnan(t)) {                                            
    Serial.println(F("Failed to read from DHT sensor!"));

    return;

  }

  Serial.print("Humidity: ");  Serial.print(h);

  sheetHumid = String(h) + String("%");                                       

  Serial.print("%  Temperature: ");  Serial.print(t);  Serial.println("Â°C ");

  sheetTemp = String(t) + String("Â°C");


  static int error_count = 0;

  static int connect_count = 0;

  const unsigned int MAX_CONNECT = 20;

  static bool flag = false;


  payload = payload_base + "\"" + sheetTemp + "," + sheetHumid + "\"}";


  if (!flag) {

    client = new HTTPSRedirect(httpsPort);

    client->setInsecure();

    flag = true;

    client->setPrintResponseBody(true);

    client->setContentTypeHeader("application/json");

  }


  if (client != nullptr) {

    if (!client->connected()) {

      client->connect(host, httpsPort);

      client->POST(url2, host, payload, false);

      Serial.print("Sent : ");  Serial.println("Temp and Humid");

    }

  }

  else {

    DPRINTLN("Error creating client object!");

    error_count = 5;

  }


  if (connect_count > MAX_CONNECT) {

    connect_count = 0;

    flag = false;

    delete client;

    return;

  }



  Serial.println("POST or SEND Sensor data to Google Spreadsheet:");

  if (client->POST(url2, host, payload)) {

    ;

  }

  else {

    ++error_count;

    DPRINT("Error-count while connecting: ");

    DPRINTLN(error_count);

  }


  if (error_count > 3) {

    Serial.println("Halting processor...");

    delete client;

    client = nullptr;

    Serial.printf("Final free heap: %u\n", ESP.getFreeHeap());

    Serial.printf("Final stack: %u\n", ESP.getFreeContStack());

    Serial.flush();

    ESP.deepSleep(0);

  }

  

  delay(3000);   

}

