#include <EEPROM.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //Angabe der erforderlichen Pins

int analogInput = A8;

float vout = 0.0;
float vin = 0.0;
float vold = 0.0;
int value = 0;

float R1 = 100000.0; // resistance of R1 (100K)
float R2 = 10000.0; // resistance of R2 (10K)

byte selected = 0; // 0 off, 1 r1, 2 r2
byte update_blink = 0;

// erstellen einiger Variablen
int Taster = 0;
int Analogwert = 0;
#define Tasterrechts 0
#define Tasteroben 1
#define Tasterunten 2
#define Tasterlinks 3
#define Tasterselect 4
#define KeinTaster 5


int Tasterstatus()
{
  Analogwert = analogRead(A0); // Auslesen der Taster am Analogen Pin A0.
  if (Analogwert > 1000) return KeinTaster;
  if (Analogwert < 50) return Tasterrechts;
  if (Analogwert < 195) return Tasteroben;
  if (Analogwert < 380) return Tasterunten;
  if (Analogwert < 555) return Tasterlinks;
  if (Analogwert < 790) return Tasterselect;

  return KeinTaster; // Ausgabe wenn kein Taster gedrückt wurde.
}


void updateScreen(float messV) {

  update_blink++;
 
  lcd.setCursor(11, 0);
  lcd.print(messV); // output v
  lcd.setCursor(15, 0);
  lcd.print("V");
 
  lcd.setCursor(0, 1);
  if (selected == 1 && update_blink <= 3) {
    lcd.print("       ");
  } else {
    lcd.print("R1:");
    if (R1 > 1000) {
      lcd.print((int)(floor)(R1 / 1000));
      lcd.print("k");
    } else {
      lcd.print((int)R1);
    }
    // print a ohm sign at the end
    lcd.write(244);
  }

  lcd.setCursor(9,1);
  if (selected == 2 && update_blink <= 3) {
    lcd.print("         ");
  } else {
    lcd.print("R2:");
    if (R2 > 1000) {
      lcd.print((int)(floor)(R2 / 1000));
      lcd.print("k");
    } else {
      lcd.print((int)R2);
    }
    lcd.write(244);
  }

  // reset the blink counter
  if(update_blink == 6){
    update_blink = 0;
  }

}



void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  lcd.begin(16, 2); // Starting the lcd driver
  lcd.clear();
 
  // read custom R settings from eeprom
  if (EEPROM.read(2) != 7) {
    Serial.print("EEPROM setup");
    EEPROM.update(0, (int)(R1/1000));
    EEPROM.update(1, (int)(R2/1000));
    EEPROM.update(2, 7);
  } else {
    R1 = EEPROM.read(0)*1000.0;
    R2 = EEPROM.read(1)*1000.0;
    Serial.println("EEPROM read data");
  }
  Serial.println((int)(R1/1000));
  updateScreen(0.00);

  pinMode(analogInput, INPUT);
  pinMode(A12, OUTPUT);
  digitalWrite(A12, HIGH);
}



void loop()
{
  value = analogRead(analogInput);
  vout = (value * 5.0) / 1024.0; // 5.0 is the atmega power
  vin = vout / (R2 / (R1 + R2));
  if (vin < 0.09) {
    vin = 0.0; //statement to quash undesired reading !
  }
 
  updateScreen((vin+vold*2) / 3);
  vold = (vin+vold*2) / 3;
  Serial.println(vin);
  delay(200);

  Taster = Tasterstatus(); //Hier springt der Loop in den oben angegebenen Programmabschnitt "Tasterstatus" und liest dort den gedrückten Taster aus.

  switch (Taster) 
  {
    case Tasterrechts: // Wenn die rechte Taste gedrückt wurde...
      {
        R1 = 100000.0;
        R2 = 10000.0;
        break;
      }
    case Tasterlinks:  // Wenn die linke Taste gedrückt wurde...
      {
        lcd.print("Links       ");  //Erscheint diese Zeile... usw...
        break;
      }

    case Tasteroben: {
        if (selected) {

          if (selected == 1) {
            R1 = R1 + 1000;
          }
          if (selected == 2) {
            R2 = R2 + 1000;
          }
        }
        break;
      }

    case Tasterunten: {
        if (selected) {
          if (selected == 1) {
            R1 = R1 - 1000;
          }
          if (selected == 2) {
            R2 = R2 - 1000;
          }
        }
        break;
      }

    case Tasterselect:  {
        if (not selected) {
          selected = 1;
          break;
        }

        if (selected == 1) {
          selected = 2;
          break;
        }

        if (selected == 2) {
          // store changed data
          EEPROM.update(0, (int)(R1/1000));
          EEPROM.update(1, (int)(R2/1000));
          Serial.println("EEPROM updated by user");
          selected = 0;
          break;
        }
      }

    case KeinTaster: {
        //lcd.print("no key ");
        break;
      }
  } // switch

} // loop