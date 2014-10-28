/*
 Example of reading the dial, return dial, and hook on a rotary phone for a kid's museum.
 By: Nathan Seidle
 SparkFun Electronics
 Date: October 27th, 2014
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 The WOW Kid's museum wanted a phone that rotary phone that automatically played a random track
 when a kid picked up the handset. If they dialed a number it would play the track associated with 
 that number.
 
 There's a shield that was created that goes with this code but you can wire the six wires straight to an Arduino
 if you have the time.
 
 Arduino pin 4 -> One side of the dial return
 5 -> One side of the dial
 6 -> One side of the hook
 
 The other side of the dial return, dial, and hook need to be connected to ground.
 
 10 tracks need to be loaded onto the SD card named TRACK000.WAV, TRACK001.WAV, etc.
 A "TRACK012.WAV" is needed - it is the ring ring track.
 */

#include <SPI.h>           // SPI library
#include <SdFat.h>         // SDFat Library
#include <SdFatUtil.h>     // SDFat Util Library
#include <SFEMP3Shield.h>  // Mp3 Shield Library

SdFat sd; // Create object to handle SD functions

SFEMP3Shield MP3player; // Create Mp3 library object

long lastTime; //Used to blink the status LED

int previousTrack1 = 1; //Used to prevent the playing of sounds twice in a row
int previousTrack2 = 2;

//GPIO declarations
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

byte dialReturn = 4; //Goes to VCC when a person starts dialing
byte dial = 5; //Opens/closes every time a number goes by
byte hook = 6; //Goes to VCC when person picks up the phone

byte statLED = 13;

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup()
{
  Serial.begin(9600);
  Serial.println("Bank phone");
  
  pinMode(A0, INPUT); //Just for a second so we can see the random generator
  randomSeed(analogRead(0)); //For picking random audio tracks

  pinMode(statLED, OUTPUT);

  pinMode(dialReturn, INPUT_PULLUP);
  pinMode(dial, INPUT_PULLUP);
  pinMode(hook, INPUT_PULLUP);

  digitalWrite(statLED, LOW); //Turn off the LED for now

  initSD(); // Initialize the SD card
  initMP3Player(); // Initialize the MP3 Shield
  
  Serial.println("Phone online!");
}

void loop()
{
  //Each second make a reading of cell voltages
  //And blink the status LED
  if(millis() - lastTime > 1000)
  {
    //Toggle stat LED
    if(digitalRead(statLED) == HIGH)
      digitalWrite(statLED, LOW);
    else
      digitalWrite(statLED, HIGH);    
    
    lastTime = millis();
  }
  
  while(offHook() == true)
  {
    Serial.println("Off hook!");
    
    playTrack(12); //Play ring ring track
    
    playTrack(11); //Then pick a random track to play. 11 is the special signal to pick random.
    
    //If user starts dialing, play the track that they just dialed
    byte number = calcNumber(5000); //Wait 5 seconds before giving up
    
    if(number >= 0)
    {
      //Play that track!
      Serial.print("Playing track: ");
      Serial.println(number);
      
      playTrack(number);
    }
    else
    {
      if(number == -1) Serial.println("Hangup");
      if(number == -2) Serial.println("Timeout");
    }
  }
  
  Serial.println("On hook");
  delay(250);
}

//Plays a given track
//11 will get you a random track between 0 and 10
//12 will get you the brrring brrring track
//Returns if user hangs up the phone or starts dialing
void playTrack(int trackNumber)
{
  char track_name[13];

  //Check to see if we need to play a random track
  if(trackNumber == 11)
  {
    trackNumber = previousTrack1;

    while(trackNumber == previousTrack1 || trackNumber == previousTrack2) //Don't play the same track as the last donation
    {
      trackNumber = random(0, 10); //(inclusive min, exclusive max)
    }

    //Update the previous variables
    previousTrack2 = previousTrack1;
    previousTrack1 = trackNumber;
  }

  sprintf(track_name, "TRACK%03d.WAV", trackNumber); //Splice the track number into file name

  //Serial.print("Playing: ");
  //Serial.println(track_name);

  if(MP3player.isPlaying()) MP3player.stopTrack(); //Stop any previous track

  //Not sure how long these functions take
  MP3player.begin();
  MP3player.playMP3(track_name);

  while(MP3player.isPlaying())
  {
    if(offHook() == false) break;
    if(user_is_dialing() == true) break;
  }

  if(MP3player.isPlaying()) MP3player.stopTrack(); //Stop any track that might be playing
}

// initSD() initializes the SD card and checks for an error.
void initSD()
{
  //Initialize the SdCard.
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED))
    sd.initErrorHalt();

  //if(!sd.chdir("/")) 
  //  sd.errorHalt("sd.chdir");
}

// initMP3Player() sets up all of the initialization for the
// MP3 Player Shield. It runs the begin() function, checks
// for errors, applies a patch if found, and sets the volume/
// stero mode.
void initMP3Player()
{
  uint8_t result = MP3player.begin(); // init the mp3 player shield
  if(result != 0) // check result, see readme for error codes.
  {
    // Error checking can go here!
    Serial.println("There's an MP3 error!");
  }

  //Not sure what the handset will need
  MP3player.setVolume(10, 10); // MP3 Player volume 0=max, 255=lowest (off)

  MP3player.setMonoMode(1); // Mono setting: 0=off, 1 = on, 3=max
}

//Returns the number that the user dialed
//Give function the max number of miliseconds before giving up
//Stops counting once user_is_dialing is false
//Returns the number, or -2 for over time error, or -1 for onHook
int calcNumber(int maxWait)
{
  byte numberDialed = 0;
  
  byte debounce_pause = 10; //We need to give some time for the paddle to truely close
  
  Serial.println("Waiting for user to use dial");

  while(user_is_dialing() == false)
  {
    Serial.print("$");
    
    //Wait for user to start dialing
    delay(1);
    maxWait--;
    if(maxWait == 0) return(-2); //Over time error
  }
  
  Serial.println("Starting to count");
  
  //This loop counts the number of high/low transitions of the dial paddle
  while(user_is_dialing() == true)
  {
    //State machine that steps through the opening and closing of the paddle
    
    while(digitalRead(dial) == HIGH)
    {
      if(user_is_dialing() == false) break; //Has the dial returned to home?

      if(offHook() == false) return(-1); //Have you hung up?
    }
    
    delay(debounce_pause);
    
    while(digitalRead(dial) == LOW)
    {
      if(user_is_dialing() == false) break; //Has the dial returned to home?

      if(offHook() == false) return(-1); //Have you hung up?
    }
    
    delay(debounce_pause);
    
    Serial.print(".");
    numberDialed++;
  }
  Serial.println();
  
  numberDialed--; //There is an extra paddle closure every time
  
  if(numberDialed == 10) numberDialed = 0; //Correct for the zero

  return(numberDialed);  
}

//Returns true if user is dialing the rotary dial
//Returns false once dial returns to the home position
boolean user_is_dialing()
{
  if(digitalRead(dialReturn) == LOW) return(true);
  
  return(false);  
}

//Returns true if user picks up the handset
boolean offHook(void)
{
  if(digitalRead(hook) == LOW) return(true);

  return(false);
}

