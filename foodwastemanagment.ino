#include <Streaming.h> // For serial output (debug)
#include <iostream>
#include <sstream>
#include <LittleFS.h>
#include <tuple>

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

// ---- display string to OLED -----
void displaytext(std::string message, int textsize)
{
  display.clearDisplay(); // Clear display from last 
  display.setCursor(0, 0);
  display.setTextSize(textsize); // Set user dedined text size
  display.print(message.c_str());
  display.display(); 
}

// ---- Show cross on screen for error -----
void drawcross()
{
  display.clearDisplay();
  // Draw both lines of the cross
  display.drawLine(43, 22, 86, 43, WHITE);
  display.drawLine(84, 22, 43, 43, WHITE);
  display.display();
  // Delay for 1 sec
  delay(1000);
}

// ---- Beeping the buzzer -----
void beep(int times, int timer)
{
  // Loop for the amount of times we need to beep
  for (int i = 0; i < times; i++)
  { // Cycle settting D8 to high and low with custom delay
    digitalWrite(D8, HIGH); 
    delay(timer); 
    digitalWrite(D8, LOW); 
    delay(timer);
  }
}

// ---- Calculate if currant year is a leep year -----
bool leepyear()
{
  // Get and store currant year
  DS3231 rtc;
  short int year = 20;
  bool leap;
  year += rtc.getYear();

  // Check if year is a leap year
  if ((year % 4 == 0 and year % 100 != 0) or year % 400 == 0)
  {
    // If true currant year is a leap year
    leap = true;
  }
  else
  {
    // If false not a leap year 
    leap = false;
  }

  return leap;
}

// ---- Read to file -----
std::vector<std::string> readfile()
{ 
  // Open txt file
  File fooddate = LittleFS.open("/food.txt", "r");

  // Display/sound error if file not open
  if(!fooddate)
  {
    displaytext("No file",1);
    Serial << "No file\n";
    delay(1000);
    beep(3,100);
  }

  // Store data from file into a string var
  std::string indata = "";
  while(fooddate.available())
  {
    indata += fooddate.read();
  }

  // Close file
  fooddate.close();

  // Splitstring and put into vector
  std::vector<std::string> dataout = {};
  char delimeter = ',';
  std::string word = "";
  std::stringstream ss(indata);

  // Store all data into vector
  while(getline(ss,word,delimeter))
  {
    dataout.push_back(word);
  }

  return dataout;
}

// ---- Write to file -----
void writefile(std::vector<std::string> datain)
{
  // Store new contents into one string
  std::string stringtofile = "";
  for (int i = 0; i < datain.size(); i++)
  {
    stringtofile += datain[i] + ",";
  }

  // Open txt file
  File fooddata = LittleFS.open("/food.txt", "w");

  // Display/sound error if file not open  
  if(!fooddata)
  {
    displaytext("No file",1);
    Serial << "No file\n";
    beep(3,100);
    delay(1000);
  }
  
  // Print data back into file
  fooddata.print(stringtofile.c_str());

  // Close file
  fooddata.close();
}

// ---- change the range to get a new value -----
int remapvalues(int ovalue, int omin, int omax, int nmin, int nmax)
{
  /*
  The following formula is taken and modified to work in a function from
  https://stackoverflow.com/questions/929103/convert-a-number-range-to-another-range-maintaining-ratio 
  */
  // Using the formula to get a new value
  int newvalue = (ovalue - omin) * (nmax - nmin) / (omax - omin) + nmin;
  return newvalue;
}

// ---- Display clock -----
void clockmode()
{
  // DS3231 uses I2C bus - device ID 0x68
  // RTC is connect to SDA (D2) and SCL (D1) pins
  DS3231 rtc;
  bool century = false;
  bool h12Flag;
  bool pmFlag;
  bool exitbutton = false; // Used as a flag to exit while loop

  //Loop displaying clock then chekcing if button to exit has been pressed.
	while(!exitbutton)
  {
    // Store currant date and time in var
    std::string datetime = std::to_string(rtc.getHour(h12Flag, pmFlag)) + ":" + std::to_string(rtc.getMinute()) + ":" + std::to_string(rtc.getSecond()) + "\n" +
                           std::to_string(rtc.getDate()) + "/" + std::to_string(rtc.getMonth(century)) + "/" + std::to_string(rtc.getYear()) + "\n" +
                           "8 to exit\n"; 

    // Send date and time to be displayed
    displaytext(datetime,2);

    // Get buttons pressed then checking if its button one.
    byte buttonpressed = tm.readButtons();
    if (buttonpressed ==  128)
    {
      exitbutton = true; // If button one pressed set exit Button flag to true
    } 
    else
    {
      exitbutton = false; // If Button one wasnt pressed set exit Button flag to false
    }
  }
}

// ---- Set LEDS to days left -----
void setLEDS(int daysleft, bool clearLEDS)
{
  // Switch off all leds
  for (int i = 0; i < 8; i++)
  {
    tm.setLED(i,0);
  }

 if(!clearLEDS & daysleft <= 8)
 {
  // Loop and switch on all leds up to 8
  for (int i = 0; i < daysleft; i++)
  {
    // Set currant index pos led on
    tm.setLED(i,1);
  }
 }
}

// ---- Get current date and month -----
std::tuple<int,int> getDateMonth()
{
  // DS3231 uses I2C bus - device ID 0x68
  // rtc is connected to SDA (D2) and SCL (D1) pins
  DS3231 rtc;
  bool century = false;

  // Store date and month in vars
  short int date = rtc.getDate();
  short int month = rtc.getMonth(century);

  // return month and date a tuple
  return std::make_tuple(month, date);
}

// ---- Seperate string date into integers -----
std::tuple<int,int> seperatedate(std::string strdate)
{
  /*
    The code to convert and seperate the date into indivual elements and convert them into integers is modified from:
    https://www.upgrad.com/tutorials/software-engineering/cpp-tutorial/stringstream-in-cpp/?form=MG0AV3
  */

  // vars to store date in
  short int month = 0;
  short int day = 0;
  char delimeter;

  // Seperate date 
  std::stringstream ss(strdate);
  ss >> month >> delimeter >> day;

  // Return date as tuple
  return std::make_tuple(month, day);
}

// ---- Calculate the amount of days left on a item -----
int calculatedays(std::string date)
{
  // Store vars
  short int daysleft = 0;
  // Get the food exp month and date
  short int foodmonth = 0;
  short int fooddate = 0;
  std::tie(foodmonth, fooddate) = seperatedate(date);

  // Get the currant month and day
  short int currmonth = 0;
  short int currdate = 0;
  // Seperate curr month and currdate from tuple
  std::tie(currmonth, currdate) = getDateMonth();

  // Integer array to store all the valid days in each month
  short int validdays[12] = { 31,28,31,30,31,30,31,31,30,31,30,31}; // 13th number is for is we are in a leap year

  // Calculate amount of days left
  // If the currant month and date are the same
  if (foodmonth == currmonth)
  {
    // If they are in the same month
    if (fooddate <= currdate)
    {
      daysleft = 0;
    }
    else
    {
      daysleft = fooddate - currdate;
    }
  }
  
  // If the fooddate is the next month
  else if ((foodmonth - currmonth) == 1)
  {
    // Add days left in currant month and then the amount in the next month
    daysleft = ((validdays[currmonth - 1] - currdate) + fooddate);
  }
  // If there is more than a month differnce
  else if ((foodmonth - currmonth) >= 2)
  {
    // Add the days in the months between
    daysleft = ((validdays[currmonth - 1] - currdate) + fooddate);
    // Add remaning days
    for (int i = currmonth; i < (foodmonth - 1); i++)
    {
      daysleft += validdays[i];
    }
  }
  // If months is in the next year 
  else if (foodmonth < currmonth)
  {
    daysleft = ((validdays[currmonth - 1] - currdate) + fooddate);

    // Add the remain days left in the year
    for (int i = currmonth; i < 12; i++)
    {
      daysleft += validdays[i];
    }

    // Add all the days in the next year
    for (int i = 0; i < foodmonth - 1; i++) 
    {
      daysleft += validdays[i];
    }
  }
  return daysleft;
}

// ---- Send message to phone (telegram) -----
void messagetophone() 
{
  // Connect to WiFi
  WiFiClientSecure client;
  client.setInsecure();
  WiFi.mode(WIFI_STA);
  delay(100);
  short int timeout = 0; // Timeout timer 
  bool failed = false; // connected flag

  // Attempt to connect to wifi with follow creds
  WiFi.begin("SSID", "Password");
  while (WiFi.status() != WL_CONNECTED && timeout <= 60) 
  {
    // Display connected 
    displaytext("Connecting to WiFi...",1);
    delay(500);
    timeout += 1;

    // Set failed to true if timeout
    if (timeout == 60)
    {
      failed = true;
    }
  }

  // Only attempt if failed flag is true 
  if (!failed)
  {
    // Send conformation of connected to screen
    displaytext("WiFi connected!",1);
    delay(500);

    // Initialize the Telegram bot put bot token in " "
    UniversalTelegramBot bot("BOT-TOKEN" , client);
    
    displaytext("Sending to phone...",1);

    // Put vector of data into string array to be sent
    std::vector<std::string> data = readfile();
    std::vector<std::string> daysleft = {};
    std::string strdata = std::string("Here is your Items and expiry date and days left: ") + "\n"  + "\n"; // string to store data

    // Calcualte the amount of days left for each item
    for (int i = 0; i < data.size(); i += 2)
    {
      daysleft.push_back(std::to_string(calculatedays(data[i+1])));
    }

    // Loop for every pair in data vector and add days left
    for (int i = 0; i < data.size(); i += 2)
    {
      strdata += "Item: " + data[i] + "\n" +"Date: " + data[i+1] + "\n" + "Days left: " + daysleft[i / 2] + "\n" +"\n";
    }
                             
    // Send a test message to the phone
    if (bot.sendMessage("CHAT-ID", strdata.c_str())) 
    {
      // Display success with success beep
      displaytext("Success!",1);
      beep(1,50);
    } 
    else 
    {
      // Display error and make error beep
      displaytext("Message failed!",1);
      Serial << "Message failed!\n";
      beep(3,100);
      drawcross();
    }    
  }

  // If failed flag is true then send error message to dipslay
  else
  {
    displaytext("Error: No Wifi",1);
    Serial << "Error: No Wifi\n";
    beep(3,100);
    drawcross();
    delay(1000);
  }

  // Disconnect from wifi once message sent
  WiFi.disconnect(true);
  client.stop(); 
  delay(1000);
}

// ---- Display food to display -----
void displayfood()
{
  // Store vars and read in food data
  bool viewexit = false;
  short int pos = 0;
  std::vector<std::string> fooddata = readfile();

  // Loop until user presses exit button
  while(!viewexit)
  {
    delay(250);
    byte move = tm.readButtons();

    // To increase through the list
    if (move == 1)
    {
      pos += 2;
      // Checking to see if we are at the end of the list
      if (pos + 1 > fooddata.size())
      {
        // Chage to go back to start
        pos -= 2;
        setLEDS(1,1);
      }
    }
    // To decrease through the list
    else if (move == 2)
    {
      pos -= 2;
      // Check to see that we are at the botttom of the list
      if (pos < 0)
      {
        // Change to go back to the end of the list
        pos = 0;
      }
      setLEDS(1,1);
    }
    // Modify food item
    else if (move == 4)
    {
      // replacing the date for currant item with the output of modifydate
      fooddata[pos + 1] = modifydate(fooddata[pos + 1]);
      delay(250);
    }
    // Exit button
    else if (move == 128)
    {
      viewexit = true; // Set viewexit flag to true
    }

    // Store amount of days left on the selected item
    short int daysleft = calculatedays(fooddata[pos+1]);

    // Store and outpu the currant item and the menu choices
    std::string strfooddata = fooddata[pos] + " : " + fooddata[pos+1] + "\n" +
                              "Days left: " + std::to_string(daysleft) + "\n" 
                              "Press:\n" +  "1 to + \n" + "2 to - \n" 
                              "3 to modify \n" "8 to exit \n";
    displaytext(strfooddata,1);
    setLEDS(daysleft, 0);
  }

  // Re-write to file
  writefile(fooddata);
  setLEDS(1,1);
}

// ---- Modidy date -----
std::string modifydate(std::string currantdate)
{
  // Integer array to store all the valid days in each month
  short int validdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

  // Detect if leep year then set feb max days to 29
  bool curryear = leepyear();
  if (curryear == true)
  {
    validdays[1] = 29;
  }

  // Set vars to store the new month, date
  short int month = 0;
  short int day = 0;
  std::tie(month,day) = seperatedate(currantdate);

  // Get user input for new date
  // Modify month
  bool confirm = false;
  
  while (!confirm)
  {
    byte button = tm.readButtons();
    short int pot = analogRead(A0); // Get anaolog input
    short int dialpos = remapvalues(pot,14,1024,1,12); // Change the range of the dial to a suiteable one

    // Message to be output to screen
    std::string output = "Select month: " + std::to_string(dialpos) + "\n"
                         "Use dial to select\n"
                         "Press 8 to save\n";
    displaytext(output,1);

    // Check if user wants to confrim
    if (button == 128)
    {
      confirm = true;
      month = dialpos;
    } 
  }

  // Reset confirm and delay for next day selection
  confirm = false;
  delay(500);

  // Modify the date
  while (!confirm)
  {
    // Read button and potentiometer
    byte button = tm.readButtons();
    short int pot = analogRead(A0);
    short int dialpos = remapvalues(pot,14,1024,1,validdays[month - 1]); // Change the range to suitable one

    // Message to be output to screen
    std::string output = "Select day: " + std::to_string(dialpos) + "\n"
                         "Use dial to select\n"
                         "Press 8 to save\n";
    displaytext(output,1);

    // Check if user wants to confirm
    if (button == 128)
    {
      confirm = true;
      day = dialpos;
    }     
  }

  confirm = false;

  // Asking user to confirm new date
  while (!confirm)
  {
    std::string output = "New date is " + std::to_string(month) + "/" + std::to_string(day) + "\n"
                          "1 to confirm save\n"
                          "2 to re-enter date\n";

    displaytext(output,1);
    byte modifier = tm.readButtons();
    yield();

    // If user confirms new date
    if (modifier == 1)
    {
      // exit loop
      confirm = true;
      // Beep once to confirm selection
      beep(1,50);
    }
    // If user wants to re-enter date 
    else if (modifier == 2)
    {
      // Restart the date selection
      return modifydate(currantdate);
    }
    delay(500);
  }

  // Using string concatination to store new date
  std::string newdate = std::to_string(month) + "/" + std::to_string(day);
  delay(500);

  return newdate;
}

// ---- Main menu -----
void mainmenu()
{
  // Store main menu text and display it
  std::string mainmenutext = "1: View items.\n"
                             "2: Send to phone.\n"
                             "3: Clock display.\n";
  displaytext(mainmenutext,1);

  // Get user input
  byte button;
  button = tm.readButtons(); // Read the buttons pressed and store into button var

  // Switch statement to call functions
  // for correct button input
  switch(button)
  {
    case 1:
      // Display food items
      delay(400);
      displayfood();
      return;

    case 2:
      // Send items to phone
      messagetophone();
      return;

    case 4:
      // Enter clock mode
      clockmode();
      return;
  }
}

// ---- Ardino setup function -----
void setup()
{
  // Serial comunication
  Serial.begin(115200);

  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_I2C_ADDRESS);
  display.clearDisplay();
  display.display();
  delay(2000);
  display.setCursor(0,0);
  display.setTextSize(0.8); // - a line is 21 chars in this size
  display.setTextColor(WHITE);

  // For RTC
  Wire.begin();
  tm.displayBegin();
  tm.reset();
  delay(100);

  // Starting littleFS 
  if(!LittleFS.begin()) 
  {
    // Print error if not started
    displaytext("Error: LittleFS",1);
    Serial << "Error: LittleFS";
    drawcross();
    delay(1000);
  }

  // For buzzer
  // Buzzer is connected to D3 on the wemos.
  pinMode(D8, OUTPUT);
}

// ---- Main loop -----
void loop()
{
  mainmenu();
}