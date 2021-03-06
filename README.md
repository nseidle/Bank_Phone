Bank Phone
=========================

Bank Phone is an interactive exhibit at the [WOW kid's museum](http://www.wowchildrensmuseum.org/) in Lafayette, CO. When the user picks up the phone they hear a classic 'ring' tone, then one of 10 random tracks in English and Spanish. Dialing the dial causes that particular track (0 to 9) play.

![Bank Phone Installed](https://raw.githubusercontent.com/nseidle/Bank_Phone/master/Bank-Phone.jpg)

The WOW Kid's museum wanted a rotary phone that automatically played a random track when a kid picked up the handset. If they dialed a number it would play the track associated with that number.

10 kid-friendly audio tracks (plus a 'ring ring' track) are loaded onto the microSD card. Power is provided from a 5V wall wart. The phone is screwed to a desk. 

![Cracked Handset](https://raw.githubusercontent.com/nseidle/Bank_Phone/master/Cracked-Handset.jpg)

This is [bakelite](https://en.wikipedia.org/wiki/Bakelite)! And yet 32,000 hands a month managed to destroy the handset. The replacement has been working well for over 2 years.

**Note:** The audio tracks must be converted to mono for this project. Use http://audio.online-convert.com/convert-to-wav to convert to mono.

[![Bank Phone Layout](https://raw.githubusercontent.com/nseidle/Bank_Phone/master/Bank%20Phone%20Layout.png)](https://raw.githubusercontent.com/nseidle/Bank_Phone/master/Bank%20Phone%20Layout.png)

This is the Arduino shield that connects to the various phone hook, dial, and return switch. There are ports for future expansion including an active ringer circuit and battery monitoring. Not needed for this exhibit.

Repository Contents
-------------------

* **firmware** - Contains the firmware that runs the phone
* **hardware** - Schematic and PCB layout for the shield that connects the dial and hook

License Information
-------------------
The hardware is released under [CERN Open Hardware License 1.2](http://www.ohwr.org/attachments/2388/cern_ohl_v_1_2.txt).
The code is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
