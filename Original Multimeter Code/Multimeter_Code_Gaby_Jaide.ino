/*
Course: Electrical Systems II

Circuit Name: Multimeter

Purpose: This circuit allows users to measure resistance, capacitance, or inductance using an IR remote.

How it works: Three modes are provided: a ohm meter, capacitance meter, or inductance meter. The modes are selected with an IR remote, further interactions can
be done on the resistance meter with the remote. The LCD displays results. If the user wants to return to mode selection, press the reset button. To power on/off use the switch.

How the code was put together: We combined our previous ohm meter code, capacitance meter code, and inductance meter code. Added code that would allow the user to choose
between the meters using the IR remote.

Authors: Jaide Romero & Gabriela Arevalo

Date: 03/27/2025: Ohm & Capacitance Meter
Date: 04/17/2025: Inductance Meter added
Date: 04/24/2025: Fixed Inductance Meter code
*/

#include <LiquidCrystal.h> // Include LCD library
#include <IRremote.h>  // Include IR library


// LCD setup
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); //  pins used for LCD


// IR Receiver setup
IRrecv irrecv(2); // Pin 2 is used for IR receiver, greater distance from inductor output
decode_results results; // Decodes resuslts


// Variables for ohm meter
int Vread; // Stores voltage reading from sensor
float R1, R2, Vrat; // R1 is unknown resistance, R2 is known resistance
int Rpins[] = {15, 16, 17, 18};  // Pins connected to resistors (A1, A2, A3, A4)
int Rvals[] = {1000, 10, 100, 1};  // Resistor values corresponding to the pins
char *Units[] = {" ohms", " k ohms", " k ohms", " M ohms"};  // Units for each scale


// Variables for capacitance meter
int analogPin = A0;          // Analog pin for measuring capacitor voltage
int chargePin = 5;           // Pin to charge the capacitor
int dischargePin = 6;        // Pin to discharge the capacitor
unsigned long startTime, tau; // unsigned long value for tau(time)
float R = 2000000.0;         // Resistor value in ohms
float C;                     // Capacitor can be a decimal
String units;                // Units for capacitance (nano or micro Farads)

// Variables for inductance meter
unsigned long pulse; // stores pulse, positive(measures wavelength time)
double frequency, period, capacitance = 2.0, inductance; //capacitance 2 uF, frequency = Pulses/Period(T) (wavelengths/time to complete one wavelength) or 1/T
double fp2 = 4.0*3.1415*3.1415;// 4*pi*pi


void setup() { // begin setup
  Serial.begin(9600);//begins serial monitor
  // Initialize the LCD
  lcd.begin(16, 2); // Lcd has 16 columns and two rows
  lcd.print("Choose meter:"); // Prints "Choose meter" message on LCD
  delay(2000); // for 2 seconds

  lcd.clear(); // clears LCD screen
  lcd.print("UP for: "); // Prints message
  lcd.setCursor(0, 1); // Second row
  lcd.print("Ohm Meter"); // Prints message
  delay(3000);

  lcd.clear(); // clears LCD screen
  lcd.print("DOWN for: "); // Prints message
  lcd.setCursor(0, 1); // Second row
  lcd.print("Capacitator Meter"); // Prints message
  delay(3000); // for 3 seconds

  lcd.clear(); // clears LCD screen
  lcd.print("PAUSE for: "); // Prints message
  lcd.setCursor(0, 1); // second row
  lcd.print("Inductance Meter");//Prints message
  delay(3000);// 3 seconds

  irrecv.enableIRIn(); // enables IR receiver
} // end setup


void loop() { // begin loop
  if (irrecv.decode(&results)) { // Check if IR siganl was recieved
    unsigned long value = results.value; // Get the value of signal
    irrecv.resume(); // Ready for next signal

    if (value == 0xFF906F) {  // UP button on remote (This button takes you to Ohm meter)
      lcd.clear(); // clear screen
      lcd.print("Ohm Meter"); // print Resistor Meter
      delay(2000); // for 2 seconds
      while (true) { // while the resistor meter is chosen loop
        resistorMeter(); // code for resistor meter
      } //end loop for Ohm meter
    } else if (value == 0xFFE01F) {  // DOWN button on remote (This button takes you to Capacitor meter)
      lcd.clear(); // clear screen
      lcd.print("Capacitor Meter"); // Print Capacitor Meter
      delay(2000); // for 2 seconds
      while (true) { // while capacitor meter is selected loop
        capacitanceMeter(); // capacitor meter code
      } // end loop for capacitor meter
    }
    else if (value == 0xFF02FD){ // PAUSE button on remote (This button takes you to Inductance Meter) 
      lcd.clear(); // clear screen
      lcd.print("Inductance Meter");//Print Inductance Meter
      delay(2000);// for 2 seconds
      while(true){ // while inductance meter is selected loop
        inductanceMeter(); // inductance meter code
      }// end loop for inductance meter
    }
  } // ends IR signal check for meter mode
} // ends loop


void resistorMeter() { // begin code for ohm meter
  lcd.clear(); // clear screen
  lcd.setCursor(0, 0); // first row and first column
  lcd.print("Please PICK:"); // print message
  lcd.setCursor(0, 1); // second row
  lcd.print("1, 2, 3 or 4"); // print message
  delay(3500); // for 3.5 seconds

  irrecv.enableIRIn(); // enable IR signal 

  for (int pin = 0; pin < 4; pin++) { // for all pin resistors
    pinMode(Rpins[pin], INPUT); // set resistor pins as inputs
  }

  int pin = -1; // No resistor value has been chosen (acts like a placeholder until a valid value is selected)
  while (pin == -1) {  // Keep checking until valid input
    if (irrecv.decode(&results)) { // receive IR signal
      unsigned int value = results.value; // receive the value of IR signal
      switch (value) { // Use switch case to go to specified value
        case 12495: pin = 0; break; // pressing button 1 takes you to 1k resistor value
        case 6375: pin = 1; break; // pressing button 2 takes you to 10k resistor value
        case 31365: pin = 2; break; // pressing button 3 takes you to 100k resistor value
        case 4335: pin = 3; break; // pressing button 4 takes you to 1M resistor value
      } // end switch case
      irrecv.resume(); // ready for next IR signal
    } // end IR siganl
    delay(1000); // wait 1 second
  } // end while loop

  for (int j = 0; j < 4; j++) { // loop through all 4 resistor pins (0 to 3)
    pinMode(Rpins[j], INPUT); // set each pin to input
  }
  pinMode(Rpins[pin], OUTPUT); // Set selected pin value to output
  digitalWrite(Rpins[pin], LOW); // Set output of selected pin to LOW(ground)

  R2 = Rvals[pin]; // Known resistor value
  Vread = analogRead(A5); // read voltage at pin A1
  Vrat = 1023.0 / Vread - 1.0; // calculate voltage ratio
  R1 = R2 * Vrat; // calculate unknown resistance (R1)

  lcd.clear(); // clear screen
  lcd.setCursor(0, 0); // first row first column
  lcd.print("R1 = "); // print message
  lcd.print(R1, 2); // print unknown resistance value, two decimals
  lcd.setCursor(10, 0); // 10th column on first row
  lcd.print(Units[pin]); // print resistor units
  lcd.setCursor(0, 1); // second row
  lcd.print("R2 = "); // print known resistance
  lcd.print(R2); // print known reistor value
  lcd.setCursor(10, 1); // 10th column on second row
  lcd.print(Units[pin]); // print units
  delay(3000); // for 3 seconds

  if (R1 > R2) { // if unknown resistance is greater than known resistance
    lcd.clear();  // clear screen
    lcd.setCursor(0, 0); // first row, first column
    lcd.print("Increase scale"); // print message
    delay(2000); // for 2 seconds
  } else { // if known resistance is greater than unknown resistance
    lcd.clear(); // clear screen
    lcd.setCursor(0, 0); // first row, first column
    lcd.print("Resistance: "); // print message
    lcd.setCursor(0, 1); // second row
    lcd.print(R1, 2); // print unknown resistance, two decimal points
    lcd.print(Units[pin]); // print units
    delay(3000); // for 3 seconds
  } // end else
} // end resistor code


void capacitanceMeter() { // begin capacitor meter code
  lcd.clear(); // clear screen
  lcd.print("Capacitor Meter"); // print message
  delay(2000); // for 2 seconds
  lcd.clear(); // clear screen

  pinMode(5, OUTPUT); // set charge resistor to output
  digitalWrite(5, LOW); // set charge resistor to ground so no voltage is applied initially

  while (true) { // begin loop to continuously charge and measure capacitor
    lcd.setCursor(0, 0); // first row, first column
    lcd.print("Charging..."); // print message
    digitalWrite(5, HIGH); // set charge resistor to high so capacitor can start charging
    //delay(3000); // for 3 seconds (this delay made us get really low tau values, giving us nonsense Capacitor values)

    startTime = micros(); // start time, microseconds
    while (analogRead(A0) < 646) {} // monitor input until reaching 646(value we chose to find time for best results), to indicate that capacitor has charged to a certain voltage 

    tau = micros() - startTime; // calculate tau(time it took to charge capacitor)
    C = ((float)tau / R); // Calculate Capacitance
    units = " micro Farads"; // default units

    if (C < 1) { // if capacitor is less than 1 micro farad
      C *= 1000.0; // divide by 1000 to get nano farad
      units = " nano Farads"; // units in nano farads
    } // end if statement

    lcd.setCursor(0, 0); // first row, first column
    lcd.print("Capacitance: "); // print message
    lcd.setCursor(0, 1);  // second rwo
    lcd.print(C, 2); // print capacitor value, two decimal points
    lcd.print(units); // print units
    delay(3000); // for 3 seconds

    lcd.clear(); // clear screen
    lcd.setCursor(0, 0); // first row, first column
    lcd.print("Discharging..."); // print message
    digitalWrite(5, LOW); // set charge resistor to gorund so capacitor stops charging

    while (analogRead(A0) > 0) {} // wait until capacitor is fully discharged
    delay(2000); // wait for 2 seconds
  } // ends while loop
} // end capacitor code


void inductanceMeter(){//begin inductance code
 
  
  pinMode(4, INPUT); // set pin 4 to input, waiting to receive signal from comparator (Will be used to measure pulse)
  pinMode(13, OUTPUT);// will control charge resistor, 110 ohms 

  lcd.clear();// clear screen
  lcd.setCursor(0,0);// first row, first column
  lcd.print("Inductance:");//print inductance
  delay(2);//small delay


  digitalWrite(13, HIGH);//set charge resistor to high(gives us the top of our sine wave)
  delay(2);//give some time to charge inductor.
  digitalWrite(13,LOW);// stop charge(gives us the bottom of our sine wave)
  pulse = pulseIn(4,HIGH,10000);//comparator turns sine wave into square wave so arduino board can read results better

  
  Serial.print("Pulse: ");// print pulse in serial monitor
  Serial.println(pulse);// print pulse value
  
  if (pulse > 1){//if a valide signal was taken:
    
    period = double(2.0*pulse);//Calculate the period of the signal (the full cycle time), by doubling the pulse duration (since pulseIn measures only half the period)
    inductance = (period*period/(capacitance*fp2))*1.65;//Calculate the inductance (L) using the formula L = T^2 / (C * 4 * pi^2), T is the period, C is the capacitance, and fp2 is 4 * pi^2 
    // the 1.65 is to make up for the interference loss we get from the IR remote  
    Serial.print("Inductance: ");// print inductance in serial monitor
  	Serial.println(inductance);// print inductaance value in serial monitor
    
    lcd.setCursor(0,1);//second row
    lcd.print(inductance);// print inductance value
    lcd.setCursor(14,1);// 14th column second row
  	lcd.print("uH");          // units in uH
  	delay(10);//delay
  }//end if statement
  delay(2000);// delay before charging inductor again
}//end inductor code

