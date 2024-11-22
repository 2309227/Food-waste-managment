/*

The student author of this work has been assessed as having Specific Learning Difficulty (SpLD), 
and this may affect fluent, accurate and concise written expression. 
Markers are advised to check the students individual learning plan for specific guidance on issues to take into account when marking. 
If there are any queries, please contact Advisory Service.

*/
 
#include <Streaming.h> // For serial output (debug)
#include <iostream>

// ---- OLED -----
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED i2c
#define OLED_RESET -1
#define OLED_SCREEN_I2C_ADDRESS 0x3C // I2C address
#define SCREEN_WIDTH 128 // OLED display width (pixels)
#define SCREEN_HEIGHT 64 // OLED display height (pixels)
//Initialize the oled display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);

// ---- Expansion board -----
#include <TM1638plus.h>
//tm(STROBE_TM, CLOCK_TM , DIO_TM, false)
TM1638plus tm(D5, D6, D7, false);

// ---- RTC -----
#include <DS3231.h>
#include <Wire.h>

// ---- Telegram bot -----
#include <UniversalTelegramBot.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Stream.h>

void setup()
  {
    Serial.begin(115200);
    // -- OLED --------------
    //display.begin(...., I2C address of oled screen)
    display.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_I2C_ADDRESS);
    display.clearDisplay();
    display.display();
    delay(2000);

    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(0.8); // - a line is 21 chars in this size
    display.setTextColor(WHITE);

    // For RTC
    Wire.begin();
		//setDateAndTime(); // If need to change time modify values in setDateFunctio
    tm.displayBegin();
    tm.reset();
    delay(100);
  }

// ---- display single line to OLED -----
void displaytext(std::string message)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(message.c_str());
  display.display();

}

// ---- Display clock -----
void clockmode()
{
  // DS3231 uses I2C bus - device ID 0x68
  DS3231 rtc;
  bool century = false;
  bool h12Flag;
  bool pmFlag;
  bool exitbutton = false; // Used as a flag to exit while loop

  //Loop displaying clock then chekcing if button to exit has been pressed.
	while(!exitbutton)
  {
  
    // Store currant date and time in var
    std::string datetime = std::to_string(rtc.getDate()) + "/" + std::to_string(rtc.getMonth(century)) + "/" + std::to_string(rtc.getYear()) + "\n" +
                           std::to_string(rtc.getHour(h12Flag, pmFlag)) + ":" + std::to_string(rtc.getMinute()) + ":" + std::to_string(rtc.getSecond()) + "\n" +
                           "Press 1 to exit\n"; 

    // Send date and time to be displayed
    displaytext(datetime);

    // Get buttons pressed then checking if its button one.
    byte buttonpressed = tm.readButtons();
    if (buttonpressed ==  1)
    {
      exitbutton = true; // If button one pressed set exit Button flag to true
    } 
    else
    {
      exitbutton = false; // If Button one wasnt pressed set exit Button flag to false
    }
  }
}

// ---- Setting time and date -----
/*
void setDateAndTime()
{
		rtc.setClockMode(false); // false = 24hr clock mode
		rtc.setYear(24);
		rtc.setMonth(11);
		rtc.setDate(18);
		rtc.setHour(11);
		rtc.setMinute(44);
		rtc.setSecond(0);
}
*/

// ---- Send message to phone (telegram) -----
void messagetophone() {
  
    // Connect to WiFi
    WiFiClientSecure client;
    client.setInsecure();
    WiFi.mode(WIFI_STA);
    delay(100);
    int timeout = 0; // Timeout timer for connecting to wifi
    bool failed = false; // Flag if wifi is not connected
    // Attempt to connect 
    WiFi.begin("CAMLAP", "Password");
    while (WiFi.status() != WL_CONNECTED && timeout <= 60) {
        // display verbose message to display and increase timeout
        displaytext("Connecting to WiFi");
        delay(500);
        timeout += 1;
        // Set failed to true if timeout
        if (timeout == 60)
        {
          failed = true;
        }
    }

    // Only attempt if failed flag is true 
    if (failed == false)
    {
      displaytext("WiFi connected!"); // Send conformation of connected to screen
      delay(500);

      // Initialize the Telegram bot put bot token in " "
      UniversalTelegramBot bot("7803952243:AAE1FESxRg0VPdnmPZHXLIfRiP0cOmBVQ38" , client);
    
      displaytext("Sending to phone...");

      // Demo for telegrame message
      int milkdate = 8;
      std::string message = "1: milk 10/11/" + std::to_string(milkdate) + "\n"
                            "2: Butter 10/11/12";
                            
      // Send a test message to the phone
      if (bot.sendMessage("7695097447", message.c_str())) {
        displaytext("Success!");
      } else {
        displaytext("Message failed!");
      }
      
    }

    // If failed flag is true then send error message to dipslay
    else
    {
      displaytext("Error: No Wifi");
      delay(1000);
      // Reset failed and timout vairables
    }

  // Disconnect from wifi once message sent
  WiFi.disconnect(true);
  client.stop(); 
}

// ---- Make file -----
void createfile()
{

}

// ---- Display food to display -----
void displayfood(std::vector<std::string> fooddata)
{


}

// ---- Reading in food.txt into vector -----
void readfile()
{

}


// ---- Main menu -----
void mainmenu()
  {
    // Store main menu text
    std::string mainmenutext = "1: View items.\n"
                               "2: Send to phone.\n"
                               "3: Modify item.\n"
                               "4: Clock display.\n";

    displaytext(mainmenutext); // Display main menu text

    // Get user input

    byte button;

    button = tm.readButtons(); // Read the buttons pressed and store into button var

    // Switch statement to call functions
    // for correct button input
    switch(button)
    {
      case 1:
        // View items
        display.clearDisplay();
        display.setCursor(0,0);
        display << "View items" << endl;
        display.display();
        delay(1000);
        return;

      case 2:
        // Send items to phone
        messagetophone();
        delay(1000);
        return;

      case 4:
        // Modify items
        Serial.println("Button 3");
        delay(1000);
        return;

      case 8:
        Serial.println("Button 4");
        clockmode();
        delay(500);
        return;

      case 16:
        readfile();
        delay(1000);

      case 128:
        // Re create csvfile

        Serial.println("Button 8");
        createfile();
        delay(1000);

        return;

    }
  }


void loop(){

  mainmenu();

}