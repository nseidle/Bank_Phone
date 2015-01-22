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
 3 -> One side of the hook
 
 The other side of the dial return, dial, and hook need to be connected to ground.
 
 10 tracks need to be loaded onto the SD card named 0.WAV, 1.WAV, etc.
 A "12.WAV" is needed - it is the ring ring track.
 
 WAV files need to be at 700-800kbps. Anything greater will play back oddly.
 
 A Stereo WAV sounds really funky and slow.
 
 This code requires the SD lib from Bill Greiman: https://github.com/greiman/SdFat
 
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
byte hook = 3; //Goes to VCC when person picks up the phone

byte statLED = 13;

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#define ON_HOOK  0
#define PLAY_RING 1
#define DIALING 2
#define PLAY_TRACK 3
#define DEAD_AIR 4
#define RING  12

#define MAIN_VOLUME 5
#define HIGH_VOLUME 0

byte state = ON_HOOK; //Keeps track of where we are at

void setup()
{
  Serial.begin(115200);
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

  //tell the MP3 Shield to play a track
  //MP3player.playTrack(4);
}

void loop()
{
  byte PhoneNumber = 0;

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

    //Print system state every second
    //We don't want to print it every loop because dialing can happen quickly
    //and we don't want to miss it

    Serial.print("State: ");
    if(state == ON_HOOK) Serial.print("On Hook");
    if(state == PLAY_RING) Serial.print("Play Ring");
    if(state == DIALING) Serial.print("Dialing");
    if(state == PLAY_TRACK) Serial.print("Play Track");
    if(state == DEAD_AIR) Serial.print("Dead Air");

    Serial.println();

  }

  if(state == ON_HOOK)
  {
    if(offHook() == true)
    {
      //Start playing the ring
      playTrack(RING);
      state = PLAY_RING;
    }
  }
  else if(state == PLAY_RING)
  {
    if(offHook() == false)
    {
      stopPlaying();
      state = ON_HOOK;
    }
    else if(user_is_dialing() == true)
    {
      stopPlaying(); //Stop playing the ring thing
      state = DIALING;
    }
    else if(!MP3player.isPlaying()) //If we are done playing ring then go to Play Track (random)
    {
      playTrack(99); //This will play a random track
      state = PLAY_TRACK;
    }
  }
  else if(state == PLAY_TRACK)
  {
    if(offHook() == false)
    {
      stopPlaying();
      state = ON_HOOK;
    }
    else if(user_is_dialing() == true)
    {
      stopPlaying(); //Stop playing the ring thing
      state = DIALING;
    }
    else if(!MP3player.isPlaying()) //If we're done playing, goto dead air
    {
      state = DEAD_AIR;
    }
  }
  else if(state == DIALING)
  {
    if(MP3player.isPlaying()) //If we're done playing, goto dead air
    {
      Serial.println("We are already playing a track!");
    }

    PhoneNumber = calcNumber(5000); //Get what the user is doing
    if(PhoneNumber == -1) //User hung up
    {
      state = ON_HOOK;
    }
    else
    {
      playTrack(PhoneNumber);
      state = PLAY_TRACK;
    }
  }
  else if(state == DEAD_AIR)
  {
    if(offHook() == false)
    {
      state = ON_HOOK;
    }
    else if(user_is_dialing() == true)
    {
      state = DIALING;
    }
  }
  
  
  delay(20);
}

void stopPlaying()
{
  if(MP3player.isPlaying()) MP3player.stopTrack(); //Stop any previous track
}
//Plays a given track
//12 will get you the brrring brrring track
//99 will get you a random track between 0 and 10
//Returns if user hangs up the phone or starts dialing
void playTrack(int trackNumber)
{
  char track_name[13];

  //Check to see if we need to play a random track
  if(trackNumber == 99)
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
  
  //Increase the volume for the brrrrring brrrrrring track
  if(trackNumber == 12) MP3player.setVolume(HIGH_VOLUME, HIGH_VOLUME); // MP3 Player volume 0=max, 255=lowest (off)

  sprintf(track_name, "%d.WAV", trackNumber); //Splice the track number into file name

  Serial.print("Playing: ");
  Serial.println(track_name);

  if(MP3player.isPlaying()) MP3player.stopTrack(); //Stop any previous track

    //Not sure how long these functions take
  MP3player.begin();
  int result = MP3player.playMP3(track_name);

  //check result, see readme for error codes.
  if(result != 0) {
    Serial.print(F("Error code: "));
    Serial.print(result);
    Serial.println(F(" when trying to play track"));
  } 

  while(MP3player.isPlaying())
  {
    Serial.print("^");
    if(offHook() == false) break;
    if(user_is_dialing() == true) break;
    delay(50);
  }
  Serial.println();

  //Return volume to normal after brrrrring brrrrrring track
  if(trackNumber == 12) MP3player.setVolume(MAIN_VOLUME, MAIN_VOLUME); // MP3 Player volume 0=max, 255=lowest (off)

  if(MP3player.isPlaying()) MP3player.stopTrack(); //Stop any track that might be playing
}

// initSD() initializes the SD card and checks for an error.
void initSD()
{
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED)) sd.initErrorHalt();
  if(!sd.chdir("/")) sd.errorHalt("sd.chdir");
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
    Serial.print("MP3 error: ");
    Serial.println(result);
  }

  //Not sure what the handset will need
  MP3player.setVolume(MAIN_VOLUME, MAIN_VOLUME); // MP3 Player volume 0=max, 255=lowest (off)

  MP3player.setMonoMode(1); // Mono setting: 0=off, 1 = on, 3=max
}

//Returns the number that the user dialed
//Give function the max number of miliseconds before giving up
//Stops counting once user_is_dialing is false
//Returns the number, or -2 for over time error, or -1 for onHook
int calcNumber(int maxWait)
{
  byte numberDialed = 0;

  byte debounce_pause = 25; //10; //We need to give some time for the paddle to truely close

  //These long print statements cause problems if user dials quickly
  //Limit them or increase serial speed
  //Serial.println("Waiting for user to use dial"); //Removed so that we don't miss quick dialing

  maxWait /= 10;
  while(user_is_dialing() == false)
  {
    Serial.print("$");

    //Wait for user to start dialing
    delay(10);
    maxWait--;
    if(maxWait == 0) return(-2); //Over time error
    if(offHook() == false) return(-1); //Have you hung up?
  }

  //Serial.println("Starting to count"); //Removed so that we don't miss quick dialing

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

