/*
 * Proyecto UTN - FRBA
 * Sistemas embebidos con aplciacion a la robotica
 */
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <SPI.h>
#include <Ethernet.h>


/****************
 * Configurations
 ****************/
 
/*
 * ETHERNET configuration
 */
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0x00, 0x25, 0x11, 0xF1, 0x8C, 0xA9
};
IPAddress ip(192, 168, 2, 223);
String endpoint = "192.168.2.233:80";

// Initialize the Ethernet server with the IP address and port
EthernetServer server(80);

uint8_t buffer[95];
int bufindex = 0;
char action[11];
char path[65];

/*
 * Hall configuration
 */
int sensor = 38;


/*
 * Keymap configuration
 */
const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns

// Define the Keymap
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 22, 23, 24, 25 };
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 30, 31, 32 }; 
// Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

/*
 * LCD Screen Configuration
 */
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int firstRowCursor = 0;
int secondRowCursor = 0;
int cursors[] = {0, 0};
int FIRST_ROW=0;
int SECOND_ROW=1;

/*
 * Motor variables
 */
int count = 0;

/*
 * Time
 */
int elapsedTime = 0;
unsigned long lastTime = 0;
long velocidad = 0;

/*
 * Actuators variables
 */
int relay = 40;
int laser = 36;
 
/*
 * Free variables
 */
int flag = LOW;

/*
 * special characters
 */
byte smiley[8] = {
  B00000,
  B10001,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000,
};


/****************
 * Functions
 ****************/
 
//cleans the whole screen
//wipes both rows
void cleanScreen(){
  lcd.setCursor(0,0);
  for(int i = 0; i < 16; i++){
    lcd.print(" ");
  }
  lcd.setCursor(0,1);
  for(int i = 0; i < 16; i++){
    lcd.print(" ");
  }
  firstRowCursor=0;
  secondRowCursor=0;
  count=0;
}

//Reads the value entered 
//and acts on that basis
void readFromKeypad(){
  char key = kpd.getKey();
  if(key)  // Check for a valid key.
  {
    switch (key)
    {
      case '#':
        cleanScreen();
        break;
      case '*':
        lcd.setCursor(firstRowCursor,0);
        firstRowCursor = firstRowCursor +1;
        lcd.write(byte(0));
        break;
     case '0':
        break;
      case '2':
        lcd.setCursor(0,0);
        lcd.write("UTN");
        break;
      case '7':
        digitalWrite(relay, HIGH);
        cleanScreen();
        lcd.setCursor(0,0);
        lcd.write("Engine status:");
        lcd.setCursor(0,1);
        lcd.write("Started");
        break;
      case '8':
        digitalWrite(relay, LOW);
        cleanScreen();
        lcd.setCursor(0,0);
        lcd.write("Engine status:");
        lcd.setCursor(0,1);
        lcd.write("Stopped");
        break;
    }
  }
}

//Prints in the LC the given text
//in the given row
void printLCD(int row, String text){
  if(cursors[row] >= 16){
    cursors[row] = 0;
  }
  text.length();
  lcd.setCursor(cursors[row], row);
  cursors[row] = cursors[row] + text.length();
  lcd.print(text);
}

//Analyse if the motor made a spin
void setMotorAttributes(){
    if (digitalRead(sensor) == HIGH)
    {
      if (flag == LOW)
      {
        flag = HIGH;
        count = count + 1;
        elapsedTime = millis() - lastTime;
        velocidad = 1000/elapsedTime;
      }
       lastTime = millis();
    }else {
      flag = LOW;
      }

    digitalWrite(laser, flag);
}

//Http server
//prints HTML on client
void loopEthernet() {
     // listen for incoming clients
    EthernetClient client = server.available();
    if (client) {
        // an http request ends with a blank line
        boolean currentLineIsBlank = true;    
        while (client.connected()) {
            if (client.available()) {
                // Clear action and path strings.
                memset(&buffer, 0, sizeof(buffer));
                memset(&action, 0, sizeof(action));
                memset(&path,   0, sizeof(path));   

                // Set a timeout for reading all the incoming data.
                unsigned long endtime = millis() + 500;
        
                // Read all the incoming data until it can be parsed or the timeout expires.
                bool parsed = false;
                while ( (millis() < endtime) && (bufindex < 95)) {
                    if (client.available()) {
                       buffer[bufindex++] = client.read();
                    }
                    parsed = parseRequest(buffer, bufindex, action, path);
                }   

                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: text/html");
                client.println("Connection: close");  // the connection will be closed after completion of the response
                client.println("Refresh: 0.2");  // refresh the page automatically every 1 sec
                client.println();
                client.println("<!DOCTYPE HTML>");
                client.println("<html>");
                client.println("<h1> Proyecto UTN - FRBA</h1>");
                client.println("<h1>  Sistemas embebidos con aplciacion a la robotica</h1>");
                client.println("<h2> Cantidad de vueltas: ");
                client.println(count);
                client.println("</h2>");
                client.println("<h2> Velocidad media: ");
                client.println(velocidad);
                client.println("</h2>");
                client.println("<form action="+ endpoint+"/start>");
                client.println("  <input type='submit' value='Start engine'>");
                client.println("</form>");
                client.println("<form action="+ endpoint+"/stop>");
                client.println("  <input type='submit' value='Stop engine'>");
                client.println("</form>");
                client.println("</html>");
                
                //analyse the request path  
                //start or stop the engine
                if(path == "/start"){
                    digitalWrite(relay, HIGH);
                }
                if(path == "/stop"){
                    digitalWrite(relay, LOW);
                }
                break;          
            }
        }
    
        // give the web browser time to receive the data
        delay(1);
        // close the connection:
        client.stop();
    }
}

// Parse the action and path from the first line of an HTTP request.
void parseFirstLine(char* line, char* action, char* path) {
    // Parse first word up to whitespace as action.
    char* lineaction = strtok(line, " ");
    if (lineaction != NULL)
    strncpy(action, lineaction, 11);
    // Parse second word up to whitespace as path.
    char* linepath = strtok(NULL, " ");
    if (linepath != NULL)
        strncpy(path, linepath, 65);
}


// Return true if the buffer contains an HTTP request.  
// Also returns the request path and action strings if the request was parsed.  
bool parseRequest(uint8_t* buf, int bufSize, char* action, char* path) {
    // Check if the request ends with \r\n to signal end of first line.
    if (bufSize < 2)
        return false;
    if (buf[bufSize-2] == '\r' && buf[bufSize-1] == '\n') {
        parseFirstLine((char*)buf, action, path);
        return true;
    }
    return false;
}

/**************
 * SETUP
 **************/
void setup(){
    Serial.begin(9600);
    pinMode(relay,OUTPUT);

    lcd.createChar(0, smiley);
    lcd.begin(16, 2);              // start the library
    lcd.setCursor(0,0);
    lcd.print("UTN - FRBA"); // print a simple message

    pinMode(sensor, INPUT);
    pinMode(laser, OUTPUT);
  
    // start the Ethernet connection and the server:
    Ethernet.begin(mac, ip);
    server.begin();
}

/**************
 *  LOOP
 *************/
void loop(){
    setMotorAttributes();
    readFromKeypad();
    loopEthernet();
}

