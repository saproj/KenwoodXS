/*
Kenwood XS System Control
See XS-Connection.jpg for pinout.

Based on reverse engineering of the protocol done by Olaf Such (ol...@ihf.rwth-aachen.de).
The following excerpts were taken from de.sci.electronics posts by Olaf from 1997.

"Spezifikationen:
positive Logik, TTL Pegel, Startbit 15ms, 8Bit daten, danach geht
CTRL wieder auf 0V"

"The code consists of: CTRL goes High, 2ms later, SDAT goes High for about
15ms (StartBit), then a Low on SDAT for either approx. 15ms or 7.5ms 
followed by a High level on SDAT for 7.5ms (FrameSignal)
then Low again for 15 or 7.5ms, High for 7.5, etc. 8 times all together.
CTRL then returns to Low."
*/

/*
Here are commands for Kenwood KX-3050 cassette deck:

Commands working in both the power-on mode and stand-by mode (in decimal):
121 - play
112, 113, 115, 117, 122, 123, 125 - stop

Commands working only in the power-on mode (in decimal):
66 - search next track
68 - stop
70 - play if stopped or paused, repeat current song if playing
72 - record
74 - search previous track
76 - pause
*/

enum {
  SDAT = 2,
  CTRL = 3,

  BIT_0 = 15000,
  BIT_1 = 7500,

  PLAY = 121,
  STOP1 = 112,
  STOP2 = 113,
  STOP3 = 115,
  STOP4 = 117,
  STOP5 = 122,
  STOP6 = 123,
  STOP7 = 125,
};

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  // Set-up XS lines
  pinMode(CTRL, OUTPUT);
  pinMode(SDAT, OUTPUT);
  digitalWrite(CTRL, LOW);
  digitalWrite(SDAT, LOW);

  // Usage
  Serial.println("Kenwood KX-3050 commands:");
  Serial.println("  Commands working in both the power-on mode and stand-by mode (in decimal):");
  Serial.println("    121 - play");
  Serial.println("    112, 113, 115, 117, 122, 123, 125 - stop");
  Serial.println("");
  Serial.println("  Commands working only in the power-on mode (in decimal):");
  Serial.println("    66 - search next track");
  Serial.println("    68 - stop");
  Serial.println("    70 - play if stopped or paused, repeat current song if playing");
  Serial.println("    72 - record");
  Serial.println("    74 - search previous track");
  Serial.println("    76 - pause");
  Serial.println("");
  Serial.println("Now type:");
  Serial.println("  value 0-255 to send the corresponding command,");
  Serial.println("  value >255 to start a loop to automatically try all commands with delay of 'value' ms.");
}

void send_byte(byte b) {
  // StartBit
  digitalWrite(SDAT, HIGH);
  delay(15);

  for (byte mask = 0x80; mask; mask >>= 1) {
    // Bit
    digitalWrite(SDAT, LOW);
    if (b & mask)
      delayMicroseconds(BIT_1);
    else
      delayMicroseconds(BIT_0);

    // FrameSignal
    digitalWrite(SDAT, HIGH);
    delayMicroseconds(7500);
  }
}

void send_command(byte cmd) {
  Serial.print("Command ");
  Serial.print(cmd, DEC);
  Serial.print(" / 0x");
  Serial.println(cmd, HEX);

  digitalWrite(CTRL, HIGH);
  delay(2);

  send_byte(cmd);

  // Return to default state
  digitalWrite(CTRL, LOW);
  digitalWrite(SDAT, LOW);
}

void try_all(unsigned long wait) {
  for (unsigned cmd = 0; cmd < 256; cmd++) {
    if(cmd == PLAY ||
      cmd == STOP1 ||
      cmd == STOP2 ||
      cmd == STOP3 ||
      cmd == STOP4 ||
      cmd == STOP5 ||
      cmd == STOP6 ||
      cmd == STOP7) {
      Serial.println("skipping one uninteresting command");
      continue;
    }
    send_command(byte(cmd));
    delay(wait);
  }
}

void loop() {
  while (Serial.available()) {
    const long val = Serial.parseInt();
    if (val < 0)
      Serial.println("wrong input");
    else if (val < 256)
      send_command(byte(val));
    else
      try_all((unsigned long)val);
  }
}
