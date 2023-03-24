#include <Arduino.h>
//#include <WiFi.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPSRedirect.h>

#include<Wire.h>
#include<LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
byte tick[8]={
  0b00000,
  0b00000,
  0b00000,
  0b00001,
  0b00010,
  0b10100,
  0b01000,
  0b00000
};
//---------------------------------------------------------------------------------------------------------
// Enter Google Script Deployment ID:
const char *GScriptId = "AKfycbwMqNrQVboqxQq-IzdYJegzNaLCMyIV0DVLqfHiagwunMaL3olWQSS3UihuiTI4DBFJJA";
//---------------------------------------------------------------------------------------------------------
// Enter network credentials:
const char* ssid     = "GalaxyA71";
const char* password = "wert2345";
//---------------------------------------------------------------------------------------------------------
// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";
//---------------------------------------------------------------------------------------------------------
// Google Sheets setup (do not edit)
const char* host        = "script.google.com";
const int   httpsPort   = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;
//------------------------------------------------------------
// Declare variables that will be published to Google Sheets
String student_id;
//------------------------------------------------------------
int blocks[] = {4,5,6,8};
#define total_blocks  (sizeof(blocks) / sizeof(blocks[0]))
//------------------------------------------------------------
#define RST_PIN  D3
#define SS_PIN   D4 
#define BUZZER   D8  
//#define button   D10
//------------------------------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;
//------------------------------------------------------------
/* Be aware of Sector Trailer Blocks */
int blockNum = 2;  
/* Create another array to read data from Block */
/* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];
int pubflag=0;
int buttonstate=0;

//------------------------------------------------------------

/****************************************************************************************************
 * setup Function
****************************************************************************************************/
void setup() {
  //----------------------------------------------------------
  Serial.begin(115200);        
  delay(10);
  Serial.println('\n');
   pinMode(BUZZER, OUTPUT);
   //pinMode(button, INPUT_PULLUP);
  //----------------------------------------------------------
  SPI.begin();
  //----------------------------------------------------------
  //initialize lcd screen
// lcd.init();
  lcd.begin();
  // turn on the backlight
  lcd.backlight();
  lcd.createChar(1,tick);
  lcd.clear();
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("Connecting to");
  lcd.setCursor(0,1); //col=0 row=0
  lcd.print("WiFi...");
  //----------------------------------------------------------
  // Connect to WiFi
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  lcd.print("Done!");
  delay(500);
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  //----------------------------------------------------------
  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  //----------------------------------------------------------
  lcd.clear();
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("Connecting to");
  lcd.setCursor(0,1); //col=0 row=0
  lcd.print("Google... ");
  //delay(5000);
  //----------------------------------------------------------
  Serial.print("Connecting to ");
  Serial.println(host);
  //----------------------------------------------------------
  // Try to connect for a maximum of 5 times
  bool flag = false;
  for(int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    //*************************************************
    if (retval == 1){
      flag = true;
      String msg = "Connected. OK";
      Serial.println(msg);
      lcd.clear();
      lcd.setCursor(0,0); //col=0 row=0
      lcd.print(msg);
      delay(1000);
      break;
    }
    //*************************************************
    else
      Serial.println("Connection failed. Retrying...");
    //*************************************************
  }
  //----------------------------------------------------------
  if (!flag){
    //____________________________________________
    lcd.clear();
    lcd.setCursor(0,0); //col=0 row=0
    lcd.print("Connection fail");
    //____________________________________________
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    delay(1000);
    return;
    //____________________________________________
  }
  //----------------------------------------------------------
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object
  //----------------------------------------------------------
 
}

/****************************************************************************************************
 * loop Function
****************************************************************************************************/
void loop() {
/*buttonstate =  digitalRead(button);
//Serial.println("BUTTON = " + buttonstate);
delay(500);
 if(buttonstate==1)*/
 scan(); 

/*else
{
  lcd.clear();
 lcd.print("PRESS BUTTON!");*/
//  }

}
/****************************************************************************************************
 * 
****************************************************************************************************/
/****************************************************************************************************
 * ReadDataFromBlock() function
 ****************************************************************************************************/
 void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{ 
  //----------------------------------------------------------------------------
  /* Prepare the ksy for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  //----------------------------------------------------------------------------
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  //----------------------------------------------------------------------------s
  if (status != MFRC522::STATUS_OK){
     Serial.print("Authentication failed for Read: ");
      /*lcd.clear();
      lcd.setCursor(1,0);
      lcd.print("AUTH FAILURE!");
      lcd.setCursor(1,1);
      lcd.print("TRY AGAIN...");
      //lcd.print("RESTARTING...");*/
     // delay(1000);
     error_auth();
     Serial.println(mfrc522.GetStatusCodeName(status));
     
   //ESP.restart();
   return;
  }
  //----------------------------------------------------------------------------
  else {
    pubflag=0;
    Serial.println("Authentication success");
  }
  //----------------------------------------------------------------------------
  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    lcd.clear();
      lcd.setCursor(4,0);
      lcd.print("Error!");
      Serial.println(mfrc522.GetStatusCodeName(status));
    delay(1000);

  }
  //----------------------------------------------------------------------------
  else {
    readBlockData[16] = ' ';
    readBlockData[17] = ' ';
    Serial.println("Block was read successfully");  

    
  }
  //----------------------------------------------------------------------------
}
 void scan(){
  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    if (!client->connected())
      {client->connect(host, httpsPort);}
  }
  else{Serial.println("Error creating client object!");}
  //----------------------------------------------------------------
  lcd.clear();
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("READY TO SCAN...");
   //Serial.print("READY TO SCAN...");
  
  /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  if ( ! mfrc522.PICC_IsNewCardPresent()) {return;}
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) {return;}
  /* Read data from the same block */
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));  
   digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(200);
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);

  //----------------------------------------------------------------
  String values = "", data;
  /*
  
  //----------------------------------------------------------------*/
  //creating payload - method 2 - More efficient
  for (byte i = 0; i < total_blocks; i++) {
   ReadDataFromBlock(blocks[i], readBlockData);
  //*************************************************
    if(i == 0){
      data = String((char*)readBlockData);
      data.trim();
      student_id = data;
      values = "\"" + data + ",";
    }
    //*************************************************
    else if(i == total_blocks-1){
      data = String((char*)readBlockData);
      data.trim();
      values += data + "\"}";
    }
    //*************************************************
    else{
      data = String((char*)readBlockData);
      data.trim();
      values += data + ",";
    }
  }
  
  //----------------------------------------------------------------
  // Create json object string to send to Google Sheets
  // values = "\"" + value0 + "," + value1 + "," + value2 + "\"}"
payload = payload_base + values;
  //----------------------------------------------------------------
  if(pubflag==0)
  publish();
  else
 no_publish();
 }
 void no_publish(){}
 void publish(){
  lcd.clear();
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("UPLOADING DATA");
    Serial.println("UPLOADING DATA");
  lcd.setCursor(1,1); //col=0 row=0
  lcd.print("PLEASE WAIT...");
  Serial.println("PLEASE WAIT...");
  //----------------------------------------------------------------
  // Publish data to Google Sheets
  Serial.println("Publishing data...");
  Serial.println(payload);
  if(client->POST(url, host, payload)){ 
    // do stuff here if publish was successful
    lcd.clear();
    lcd.setCursor(0,0); //col=0 row=0
     Serial.println("ROLL NO: "+student_id);
    lcd.print("ROLL NO: "+student_id);
    lcd.setCursor(0,1); //col=0 row=0
    Serial.print("PRESENT ");
    lcd.print("PRESENT ");
    lcd.write(1);
    delay(2000);
   
  }
 else
  error_publish();
 }
  //----------------------------------------------------------------
  void error_publish(){
    // do stuff here if publish was not successful
    Serial.println("Error while connecting");
    lcd.clear();
    lcd.setCursor(0,0); //col=0 row=0
    lcd.print("SCAN FAILED!");
    lcd.setCursor(0,1); //col=0 row=0
    lcd.print("CHECK WiFi..");
      delay(3000);
     return;
 
  }
   void error_auth()
   { pubflag=1;
     lcd.clear();
      lcd.setCursor(1,0);
      lcd.print("AUTH FAILURE!");
      lcd.setCursor(1,1);
      lcd.print("TRY AGAIN...");
      //lcd.print("RESTARTING...");
      delay(1000);
    
   }
  //-------------------
//  ---------------------------------------------
  // a delay of several seconds is required before publishing again    

 
 
