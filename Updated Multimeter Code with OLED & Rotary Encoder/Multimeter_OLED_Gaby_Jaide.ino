/*
Course: Electrical Systems II

Circuit Name: Multimeter

Purpose: This circuit allows users to measure resistance, capacitance, or inductance using a rotary encoder and an OLED to display the selections and values.

How it works: Three modes are provided: a ohm meter, capacitance meter, or inductance meter. The modes are selected with a rotary encoder, further interactions can
be done on the resistance meter with the encoder. The OLED screen displays results. If the user wants to return to mode selection, press the reset button. To power on/off use the switch.

Disclaimer: The code does not work the way it was intended to, we hope future classes fix the issues if they choose to tinker with this code.

Authors: Jaide Romero & Gabriela Arevalo

Date: 05/06/2025: LCD replaced by OLED meter
Date: 05/09/2025: IR remote replaced by rotary encoder.
*/

#include <Wire.h> // Use this to find I2C device(OLED) for communication 
#include <Adafruit_SSD1306.h> // Controls OLED screen
#include <Adafruit_GFX.h> // Graphics support library for drawing shapes and text

// OLED Display
#define SCREEN_WIDTH 128 // 128 pixels wide
#define SCREEN_HEIGHT 64 // 64 pixels tall
#define SCREEN_ADDRESS 0x3C // I2C address of OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire); // represents OLED screen in the code

// Rotary Encoder Pins
#define encoder0PinA 2 // First signal pin for rotary encoder
#define encoder0PinB 3 // Second signal pin for rotary encoder
#define encoderBtnPin 4 // push button for rotary encoder

// Encoder Variables
volatile int encoder0Pos = 0; // stores enocder's current position (volatile because it changes unexpectedly due to external interactions)
int lastPos = 0; // tracks last position to determine if encoder has moved
byte menuIndex = 0; // tells us which menu item is currently highlighted
bool clicked = false; // True when encoder button is pressed, false if not pressed

// State
enum Mode { MENU, OHM, CAPACITOR, INDUCTANCE }; // Names the menu options
Mode currentMode = MENU; // start in menu mode by default

// Ohm Meter Variables
int Vread; // Voltage reading for unknown resistor at analog pin A3
float R1, R2, Vrat; // Unknown resistance R1, known resistance R2
int Rpins[] = {8, 7, 6, 5}; // Pins connected to R2 resistors (8, 7, 6, 5)
long Rvals[] = {1000, 10, 1000, 1}; // corresponding values are 1k, 10k, 100k, 1M ohm
char *Units[] = {" ohms", " k ohms", " k ohms", " M ohms"}; // units
int selectedResistor = 0; //Keeps track of what R2 is selected

// Capacitor Meter
int analogPin = A2; // Voltage measurement for capacitor at analog pin A2 
int chargePin = 11; // Charges capacitor using a 2 M ohm resistor
int dischargePin = 10; // Discharges capacitor using a 220 ohm resistor
unsigned long startTime, tau; // Measures how long the capacitor takes to charge
float R = 2000000.0; // Charge resistor(2M ohms)
float C; // Used to hold calculated capacitor
String units; // displays capacitor units

// Inductor Meter
unsigned long pulse; //stores pulse, positive(measures wavelength time)
double frequency, period, capacitance = 2.0, inductance; // Capacitance 2uF, variables will be used to calculate inductance
double fp2 = 4.0 * 3.1415 * 3.1415; //This is a constant 4π² used in LC resonance formula: L = T² / (4π²C)

void setup() { // begin setup
  Serial.begin(9600); // begin serial monitor
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS); //begin OLED 
  display.clearDisplay();// clear OLED screen
  display.display(); // display

  pinMode(encoder0PinA, INPUT_PULLUP);//encoder pins are set as INPUTS by default, and OUTPUT when used
  pinMode(encoder0PinB, INPUT_PULLUP);
  pinMode(encoderBtnPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoder, CHANGE);//detects when encoder is turned

  for (int i = 0; i < 4; i++) pinMode(Rpins[i], INPUT); //R2 resistors are set as inputs for now

  displayMenu(); // display menu on OLED
} // end setup

void loop() { // begin loop
  if (digitalRead(encoderBtnPin) == LOW && !clicked) { //check if encoder button was pressed
    delay(50); // small delat to prevent multiple presses
    clicked = true; // encoder button pressed
    switch (menuIndex) { // select meter
      case 0: currentMode = OHM; break; // ohm meter selected
      case 1: currentMode = CAPACITOR; break; // capacitor meter selected
      case 2: currentMode = INDUCTANCE; break; // inductor meter selected
    } 
    encoder0Pos = 0;// reset encoder position
  } 

  if (digitalRead(encoderBtnPin) == HIGH && clicked) clicked = false; // once encoder button is released

  switch (currentMode) { // handle current mode
    case MENU:
      if (encoder0Pos != lastPos) { // update menu if encoder is rotated
        menuIndex = abs(encoder0Pos) % 3; // limit to the 3 menu items
        displayMenu(); // Update OLED screen
        lastPos = encoder0Pos; // save current position
      }
      break;
    case OHM: resistorMeter(); currentMode = MENU; displayMenu(); break; // run ohm meter code
    case CAPACITOR: capacitanceMeter(); currentMode = MENU; displayMenu(); break; // run capacitor meter code
    case INDUCTANCE: inductanceMeter(); currentMode = MENU; displayMenu(); break; // run inductor meter code
  }
} // end loop

void displayMenu() {
  display.clearDisplay(); // clear screen
  display.setTextSize(2); //text size
  display.setTextColor(WHITE); // text color
  display.setCursor(10, 0); // text location
  display.println("Select:"); // display message
  display.setTextSize(1.99995); // text size
  display.setCursor(10, 20); // text location
  display.println(menuIndex == 0 ? "> Ohm Meter" : "  Ohm Meter"); // display ohm meter
  display.setCursor(10, 30); // text location
  display.println(menuIndex == 1 ? "> Capacitor Meter" : "  Capacitor Meter"); // display capacitor meter
  display.setCursor(10, 40); // text location
  display.println(menuIndex == 2 ? "> Inductance Meter" : "  Inductance Meter"); // display inductor meter
  display.display(); // display
}

void doEncoder() {
  static int lastEncoded = 0; // stores last encoder value
  int MSB = digitalRead(encoder0PinA); // most significant bit
  int LSB = digitalRead(encoder0PinB); // least significant bit
  int encoded = (MSB << 1) | LSB; // combine bits into single number
  int sum = (lastEncoded << 2) | encoded; // creates 4 bit value that combines last and current encoder state

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)// 4-bit combination inidcates clockwise rotation
    encoder0Pos++; // rotate clockwise
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)// 4-bit combination indicates counter-clockwise rotation
    encoder0Pos--; // rotare counter clockwise

  lastEncoded = encoded; // save current encoder state
}

void resistorMeter() { // begin ohm meter code
  int resistorIndex = 0; //Index of R2
  int lastIndex = -1; // detects changes in selection
  bool selecting = true; // Check if we are still selecting an R2 value

  // Reset encoder position
  encoder0Pos = 0; // reset encoder position
  lastPos = 0; // track last position

  display.clearDisplay(); // clear screen
  display.setTextSize(2); // text size
  display.setCursor(0, 0); // text location
  display.println("Ohm Meter Selected"); // display message
  display.display(); // display
  delay(2000); // for 2 seconds

  while (selecting) { // while selecting R2
    // Read encoder manually
    int newPos = encoder0Pos; // Keep track of encoder position
    resistorIndex = abs(newPos) % 4; // limit to 4 R2 values

    if (resistorIndex != lastIndex) { // keep track of R2 index
      display.clearDisplay(); // clear display
      display.setTextSize(2); // text size
      display.setCursor(0, 0); // text position
      display.println("R2 Scale"); // display message

      display.setTextSize(1.9999995); // text size

      for (int i = 0; i < 4; i++) { // 4 resistor values
        display.setCursor(0, 15 + i * 10); // stack R2 values vertically
        if (i == resistorIndex) { // one of the R2 values
          display.print("> "); // highlight current choice
        } else {
          display.print("  "); // other choices are not highlighted
        }
        display.print(Rvals[i]); // display R2 values
        display.print(Units[i]); // display R2 units
      }

      display.display(); // display
      lastIndex = resistorIndex; // detects changes in selection
    }

    // Detect button press
    if (digitalRead(encoderBtnPin) == LOW) { // wait for button to be pressed to confirm selection
      delay(500); // debounce delay
      while (digitalRead(encoderBtnPin) == LOW); // wait for release
      selecting = false; // check if we are still selecting R2 value
    }
  }

  // Set selected resistor and prepare measurement
  selectedResistor = resistorIndex; // store selected R2

  for (int j = 0; j < 4; j++) pinMode(Rpins[j], INPUT);// set all R2 pins as Inputs
  pinMode(Rpins[selectedResistor], OUTPUT); // Set selected R2 value as output
  digitalWrite(Rpins[selectedResistor], LOW); // Allow voltage to flow through selected resistor

  R2 = Rvals[selectedResistor]; // R2 is selected value
  Vread = analogRead(A3); // Read voltage at A3
  Serial.println(Vread);// print A3 voltage in serial monitor
  Vrat = 1023.0 / Vread - 1.0; // calculate R1
  R1 = R2 * Vrat; // R1 estimated value
  Serial.println("R1 = "); // display message
  Serial.print(R1); // display R1 value


  // Display results
  display.clearDisplay(); // clear screen
  display.setCursor(0, 0); // text location
  display.print("Unknown Resistance:"); // display message
  display.setCursor(0, 15); // text location
  display.print(R1, 2); // display R1 value up to two decimal points
  display.print(Units[selectedResistor]); // display units

  display.setCursor(0, 35); // text location
  display.print("R2 Used:"); // display message
  display.setCursor(0, 50); // text location
  display.print(R2); // display R2 value
  display.print(Units[selectedResistor]);// display R2 units
  display.display(); // display
  delay(3000);// for 2 seconds

  display.clearDisplay(); // clear display
  display.setCursor(0, 0); // text location
  if (R1 > R2) { // if R1 is greater than R2
    display.print("Increase Scale"); // display message
  } else {
    display.print("Resistance:"); // display message
    display.print(R1, 2);// display R1 value
  }
  display.display();// display
  delay(2000);// for 2 seconds
}//end ohm meter code


void capacitanceMeter() { // begin capacitor meter code
  display.clearDisplay(); // clear screen
  display.setTextSize(2);// text size
  display.setCursor(0, 0);// text location
  display.println("Capacitor Meter Selected");// print message
  display.display(); // display
  delay(2000);// for 2 seconds
  display.clearDisplay(); // clear display

  pinMode(chargePin, OUTPUT);// set charge resistor to output
  digitalWrite(chargePin, LOW);// make sure capacitor is not being charged

  while(true){  // continously measure capacitor value
    display.setCursor(0, 0);// text location
    display.print("Charging..."); // display message
    digitalWrite(chargePin, HIGH); // start charging capacitor
    display.display(); // display
  //delay(500);

    startTime = micros(); // start timing how long the capacitor takes to charge
    while (analogRead(A2) < 646) {} //wait until voltage reaches 646 bit
    tau = micros() - startTime; // calculate time it takes to reach 646 bit
    C = ((float)tau / R); // calculate capacitor value
    units = " uF"; // units in uF

    if (C < 1) { //convert to nF if needed
      C *= 1000; // divide C by 1000 to achieve this
      units = " nF"; // units in nF
    }

    display.clearDisplay();// clear display
    display.setCursor(0, 0);// text location
    display.print("Capacitor: "); // display message
    display.setCursor(10, 20);// text location
    display.print(C, 2); // capacitor value
    display.print(units); // capacitor units
    display.display();// display
    delay(3000);// display for 3 seconds

    display.clearDisplay();// clear screen
    display.setCursor(0, 0);// text location
    display.print("Discharging...");// display message
    digitalWrite(chargePin, LOW);// stop charging capacitor

   while (analogRead(A2)> 0) {}// wait until capacitor is fully discharged
    delay(2000);// pause before repeating
  }

} // end capacitor code


void inductanceMeter() { // begin inductor meter

  display.clearDisplay();// clear screen
  display.setTextSize(2);// text size
  display.setCursor(0, 0);// text location
  display.println("Inductor Meter Selected");// display message
  display.display();// display
  delay(2000);// for 2 seconds

  pinMode(12, INPUT); // reads oscillation signal
  pinMode(13, OUTPUT);// sends a trigger pusle

  digitalWrite(13, HIGH);// turn on pusle
  delay(2);// small delay
  digitalWrite(13, LOW);// stop reading pulse
  pulse = 10;//pulseIn(12, HIGH, 10000); // this is so circuit/OLED screen are functioning

  display.clearDisplay(); // clear screen
  if (pulse > 1) { //if valid pulse is detected
    period = double(2.0 * pulse); // one full cycle
    inductance = (period * period / (capacitance * fp2)) * 1.65; // calcualte pulse(1.65 interference loss with IR might not be necessary with this code)

    display.setCursor(0, 0); // text location
    display.print("L = "); // display message
    display.print(inductance); // print inductance value
    display.print(" uH"); // units
  } else {
    display.setCursor(0, 0); // text location
    display.print("No signal"); // display message
  }
  display.display();// display
  delay(3000);// for 3 seconds
}

