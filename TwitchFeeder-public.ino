/*******************************************************************
    Connect to Twtich Chat with a Bot
   Created with code from TheOtherLoneStar (https://www.twitch.tv/theotherlonestar)
   Hackaday IO: https://hackaday.io/otherlonestar
   By Brian Lough (https://www.twitch.tv/brianlough)
   YouTube: https://www.youtube.com/channel/UCezJOfu7OtqGzd5xrP3q6WA
Created with code from noycebru www.twitch.tv/noycebru
 *******************************************************************/
 
#include <WiFi.h>          
#include <IRCClient.h>
#include <ESP32Servo.h>
#include <Preferences.h>


Servo servo; 

//define your default values here, if there are different values in config.json, they are overwritten.
#define secret_ssid "" 
#define IRC_SERVER   "irc.chat.twitch.tv"
#define IRC_PORT     6667
 
//------- Replace the following! ------
char ssid[] = "";       // your network SSID (name)
char password[] = "";  // your network key
 
//The name of the channel that you want the bot to join
const String twitchChannelName = ""; //this is case sensitive, must be all lower case!
 
//The name that you want the bot to have
#define TWITCH_BOT_NAME "" //make this your channel name, mixed case as you see fit
 
//OAuth Key for your twitch bot
// https://twitchapps.com/tmi/
#define TWITCH_OAUTH_TOKEN ""
 
 
//------------------------------
 

int Feeder = 4;  //this is the pin the servo connects to 
int feedcount = 0;
int allfeed = 0;
int yesterfeed = 0;
String ircChannel = ""; //leave this line blank.

WiFiClient wiFiClient;
IRCClient client(IRC_SERVER, IRC_PORT, wiFiClient);
Preferences prefs;


   
// put your setup code here, to run once:
void setup() {
 
  pinMode(led, OUTPUT);
  pinMode(Feeder, OUTPUT);
  servo.attach(Feeder);
  servo.write(90); //stop on a continuous servo
  delay(100);
  servo.detach();
  delay(2000);
  Serial.begin(115200);
  Serial.println();

  prefs.begin("counters", false);
  allfeed = prefs.getInt("allfeed", 0); 
  yesterfeed = prefs.getInt("yesterfeed", 0);
  
  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
 
  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
 
  ircChannel = "#" + twitchChannelName;
 
  client.setCallback(callback);

  
}
 
void loop() {
 
  // Try to connect to chat. If it loses connection try again
  if (!client.connected()) {
    Serial.println("Attempting to connect to " + ircChannel );
    // Attempt to connect
    // Second param is not needed by Twtich
    if (client.connect(TWITCH_BOT_NAME, "", TWITCH_OAUTH_TOKEN)) {
      client.sendRaw("JOIN " + ircChannel);
      Serial.println("connected and ready to rock");
      sendTwitchMessage("Ready to go Boss!");
    } else {
      Serial.println("failed... try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
    return;
  }
  client.loop();
}
 
void sendTwitchMessage(String message) {
  client.sendMessage(ircChannel, message);
}
 
 
void callback(IRCMessage ircMessage) {
  //Serial.println("In CallBack");
 
  if (ircMessage.command == "PRIVMSG" && ircMessage.text[0] != '\001') {
    //Serial.println("Passed private message.");
   
    ircMessage.nick.toUpperCase();
 
    String message("<" + ircMessage.nick + "> " + ircMessage.text);
 
    //prints chat to serial
    Serial.println(message);

//this is where you would replace these elements to match your streaming configureation. All of the LED commands are commented out since I'm not using them. Uncomment if you want to add lights!
/* if (ircMessage.text.indexOf("subscribed") > -1 && ircMessage.nick == "STREAMELEMENTS")
      {
     
      digitalWrite(led, HIGH);
      delay(10000);
      digitalWrite(led, LOW);
      delay(25);
   
    }

    if (ircMessage.text.indexOf("streaming") > -1 && ircMessage.nick == "STREAMELEMENTS")
      {
     
      digitalWrite(led, HIGH);
      delay(10000);
      digitalWrite(led, LOW);
      delay(25);
   
    }

   if (ircMessage.text.indexOf("offline") > -1 && ircMessage.nick == "STREAMELEMENTS")
      {
     
      digitalWrite(led, HIGH);
      delay(10000);
      digitalWrite(led, LOW);
      delay(25);
   
    }
*/
    //servo control
if (ircMessage.text.indexOf("thanks") > -1 && ircMessage.nick == "STREAMELEMENTS")
{
feedcount++;
allfeed++;

sendTwitchMessage("Chickens fed " + String(feedcount) + " times!");
servo.attach(Feeder);
delay(100);
servo.write(45);
delay (750);
servo.write(90);
delay(100);
servo.detach();
Serial.println("Fed birds "+ String(feedcount) +" times");
}
      
if (ircMessage.text.indexOf("is now live") > -1 && ircMessage.nick == "STREAMELEMENTS") //
{
  
sendTwitchMessage("Going live, reset feed counter to 0! Use !feedcount for details");
prefs.putInt("yesterfeed", feedcount);
prefs.putInt("allfeed", allfeed);
feedcount = 0;
Serial.println("Reset counter");
}

if (ircMessage.text.indexOf("!feedcount") == 0)
{
yesterfeed = prefs.getInt("yesterfeed");
sendTwitchMessage("Chickens were fed " + String(yesterfeed) + " times yesterday, and " + String(allfeed) + " times in total. Thanks for making sure they grow up big and strong!");
}
if (ircMessage.text.indexOf("!testcounter") == 0 && ircMessage.nick == "KREDAL")
{
feedcount++;
allfeed++;
sendTwitchMessage("Chickens fed " + String(feedcount) + " times! (but not really, this is just a test)");
}
if (ircMessage.text.indexOf("!reset") == 0 && ircMessage.nick == "KREDAL")
{
sendTwitchMessage("Reset feed counter to 0! Use !feedcount for details");
prefs.putInt("yesterfeed", feedcount);
prefs.putInt("allfeed", allfeed);
feedcount = 0;
Serial.println("Reset counter");
}
 
    return;
  }
}
