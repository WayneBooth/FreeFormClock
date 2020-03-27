#include <RTClib.h>
#include <Wire.h>

#define BIT(n,i) (1 & ( n >> i ))

//#define DEBUG 1

int clockPin = 10;
int latchPin = 9;
int dataPin = 8;
int downMinPin = 14;
int upMinPin = 15;

RTC_DS3231 rtc;

int debounce = 0;
int dim = 0;
byte display[23];
int charCodes[10][6] = { 
    { 7,9,17,18,28,0 }, // 0
    { 2,6,4,12,8,0 },   // 1
    { 7,8,14,2,28,0 },  // 2
    { 7,8,14,16,28,0 }, // 3
    { 1,9,31,8,8,0 },   // 4
    { 7,1,14,16,28,0 }, // 5
    { 2,2,14,18,28,0 }, // 6
    { 7,8,8,8,8,0 },    // 7
    { 7,9,14,18,28,0 }, // 8
    { 7,9,14,8,8,0 }    // 9
};

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif
  Wire.begin();
  rtc.begin();
#ifdef DEBUG
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
#endif

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  digitalWrite(latchPin, HIGH);
  digitalWrite(clockPin, LOW);
  digitalWrite(dataPin, LOW);

  pinMode(downMinPin, INPUT);
  pinMode(upMinPin, INPUT);
  digitalWrite(downMinPin, HIGH); // pullup
  digitalWrite(upMinPin, HIGH); // pullup

}

void changeTime(int changeDirection ) {
  
  DateTime now = rtc.now();
  int hr = now.hour();
  int min = now.minute();
  int day = now.day();
  int month = now.month();
  int yr = now.year();

  min += changeDirection;

  if( min < 0 ) {
    min = 59;
    hr += -1;
  }
  if( min > 59 ) {
    min = 0;
    hr += 1;
  }
  if( hr < 0 ) {
    hr = 23;
  }
  if( hr > 23 ) {
    hr = 0;
  }

  rtc.adjust(DateTime(yr, month, day, hr, min, 0));

}

void loop() {

  digitalWrite(LED_BUILTIN, HIGH);
  DateTime now = rtc.now();
  digitalWrite(LED_BUILTIN, LOW);

  // Check buttons
  if( digitalRead(upMinPin) == LOW || digitalRead(downMinPin) == LOW ) {
    debounce++;
    if(debounce > 35) {
      debounce = 0;
//      if( digitalRead(upMinPin) == LOW && digitalRead(downMinPin) == LOW ) {
//        // Some function when both buttons pressed.
//        // Not sure what that function is just yet.
//      }
//      else 
      if( digitalRead(upMinPin) == LOW ) {
        changeTime(1);
      }
      else if( digitalRead(downMinPin) == LOW ) {
        changeTime(-1);
      }
    }
  }
  else {
   debounce = 0;
  }

  
  // Get the time
  int h = now.hour();
  int m = now.minute();
  int s = now.second();
    
#ifdef DEBUG
  Serial.println();
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
#endif

  // Calculate the digit seperator
  if(s % 2 == 0) {
    s = 1;
  }
  else {
    s = 0;
  }

  int h1 = 0;
  int h2 = 0;
  int m1 = 0;
  int m2 = 0;
  
  // Calculate digits of the hour
  if( h >= 20 ) {
    h1 = 2;
    h2 = h - 20;
  }
  else if( h >= 10 ) {
    h1 = 1;
    h2 = h - 10;
  }
  else {
    h1 = 0;
    h2 = h;
  }


  // Calculate digits of the minute
  int c = 5;
  while( c >= 0 ) {
    if( m >= ( c * 10 ) ) {
      m1 = c;
      m2 = m - ( c * 10 );
      break;
    }
    c--;
  }

  // Calculate codes for each line
  for( int line = 5 ; line >= 0 ; line-- ) {

    display[0] =  BIT( charCodes[ m2 ][ line ], 4);
    display[1] =  BIT( charCodes[ m2 ][ line ], 3);
    display[2] =  BIT( charCodes[ m2 ][ line ], 2);
    display[3] =  BIT( charCodes[ m2 ][ line ], 1);
    display[4] =  BIT( charCodes[ m2 ][ line ], 0);

    display[5] =  BIT( charCodes[ m1 ][ line ], 4);
    display[6] =  BIT( charCodes[ m1 ][ line ], 3);
    // -----------
    display[7] =  0;
    display[8] =  BIT( charCodes[ m1 ][ line ], 2);
    display[9] =  BIT( charCodes[ m1 ][ line ], 1);
    display[10] = BIT( charCodes[ m1 ][ line ], 0);

    display[11] = s == 1 ? 1 : 0;
    
    display[12] = BIT( charCodes[ h2 ][ line ], 4);
    display[13] = BIT( charCodes[ h2 ][ line ], 3);
    display[14] = BIT( charCodes[ h2 ][ line ], 2);
    // -----------
    display[15] = 0;
    display[16] = BIT( charCodes[ h2 ][ line ], 1);
    display[17] = BIT( charCodes[ h2 ][ line ], 0);
   
    display[18] = BIT( charCodes[ h1 ][ line ], 4);
    display[19] = BIT( charCodes[ h1 ][ line ], 3);
    display[20] = BIT( charCodes[ h1 ][ line ], 2);
    display[21] = BIT( charCodes[ h1 ][ line ], 1);
    display[22] = BIT( charCodes[ h1 ][ line ], 0);
    
    setDisplay( line );
    delayMicroseconds(1200);
  }

}

void setDisplay( uint8_t current_line ) {

  // Shift in the row selector
  for( int l = 4 ; l >= 0 ; l-- ) {
    if( l == current_line ) {
      shiftIn(1);
    }
    else {
      shiftIn(0);
    }
  }
  shiftIn(0);

  for( int l = 22 ; l >= 0 ; l-- ) {
    shiftIn( display[l] );
  }
  shiftIn(0);
  
  digitalWrite(latchPin, LOW);
  digitalWrite(latchPin, HIGH);
#ifdef DEBUG
  Serial.println();
#endif
}

void shiftIn(byte d) {
#ifdef DEBUG
  Serial.print(d);
#endif
  digitalWrite(dataPin, d);
  digitalWrite(clockPin, HIGH);
  digitalWrite(clockPin, LOW);
  digitalWrite(dataPin, LOW);
}

