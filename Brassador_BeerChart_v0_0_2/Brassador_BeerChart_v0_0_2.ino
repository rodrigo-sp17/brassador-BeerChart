// Brews beer according to user inputed temperature ramps

// Include Files
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>
  
// Version
#define BVERSION "0.1"

// Setups
#define BUTTON_DELAY 200  // Sensitivity adjustment
#define LOWER_MARGIN 1
#define UPPER_MARGIN 1

// Setup relays
#define PIN_RELAY 11

// Loads Temperature Sensor
#define DS18B20 2
OneWire ourWire(DS18B20);
DallasTemperature sensors(&ourWire);

// Sets LCD Display
const int pin_RS = 8; 
const int pin_EN = 9; 
const int pin_d4 = 4; 
const int pin_d5 = 5; 
const int pin_d6 = 6; 
const int pin_d7 = 7; 
const int pin_BL = 10; 
LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);

// Defines ramps
typedef struct
{
  int degreesCelsius;
  int minutes;
}
ramps;


void setup() {
  // Title and Version
  lcd.begin(16,2);
  lcd.setCursor(0,0);
  lcd.print("Beerchart v") + lcd.print(BVERSION);
  lcd.setCursor(0,1);
  lcd.print("por Digaum");
  delay(5000);
  lcd.clear();

  // Ramp Setup
  // Prompts user for ramp number
  lcd.setCursor(0,0);
  lcd.print("Qtd. de Rampas:");
    
  // Sets ramp number
  int number_ramps = get_user_parameter("");

  // Creates struct RAMP
  ramps ramp[number_ramps];
  
  // Prints number of ramps to user 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(number_ramps + String(" rampas!"));
  delay(2000);
   
  // Prompts user for ramp information 
  for (int i = 0; i < number_ramps; i++)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Rampa ") + lcd.print(i + 1);

    // Gets time from user
    ramp[i].minutes = get_user_parameter("Tempo (min): ");

    // Clear LCD line
    lcd.setCursor(0,1);
    lcd.print("                ");
    
    // Gets temperature from user
    ramp[i].degreesCelsius = get_user_parameter("Graus (C): ");
  }
  
  // Prints end of ramp recording 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Rampas gravadas!");
  delay(2000);

  // Prompts user for SELECT to start brewing
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Press. SELECT");
  lcd.setCursor(0,1);
  lcd.print("para brassar...");
  wait_for_select();
    
  // OPTIONAL - Countdown to start brewing
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Brassando em");
  for (int i = 3; i > 0; i--)
  {                 
    lcd.setCursor(0,1);
    lcd.print(i + String(" ..."));
    delay(1000);
  }

  // Ramp Brewing Sequence
  sensors.begin();
  pinMode(PIN_RELAY, OUTPUT);
  for (int i = 0; i < number_ramps; i++)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(String("R") + (i + 1));
    lcd.print(String(" - ") + ramp[i].minutes + String("m/") + ramp[i].degreesCelsius + String("C"));
    
    pre_heat(ramp[i].degreesCelsius);
    lcd.setCursor(0,1);
    lcd.print("Temperatura OK! ");
    delay(2000);

    brew(ramp[i].degreesCelsius, ramp[i].minutes);
    lcd.setCursor(0,1);
    lcd.print(String("Rampa ") + (i + 1) + String(" OK!"));
    delay(2000);

  }

  // Prompt for MASHOUT
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Fim de Rampas!");
  lcd.setCursor(0,1);
  lcd.print("Mashout?");
  wait_for_select();
  
  // MASHOUT
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Mashout - 76C");
  pre_heat(76);  
  
  // PROMPT - End of Ramps
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Fim do Mash-Out!");
  delay(2000);
  
  // PROMPT - Boiling start
  lcd.setCursor(0,0);
  lcd.print("Iniciar fervura?");
  lcd.setCursor(0,1);
  lcd.print("Press. SELECT");
  wait_for_select();

  // Boiling loop - 100C, 60 minutes
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Fervendo..."); 
  
  pre_heat(100);
  brew(100, 60);
  
  // PROMPT - End of Brewing
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Brassagem");
  lcd.setCursor(0,1);
  lcd.print("encerrada!");
  delay(7000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Boa cerveja!");
  lcd.setCursor(0,1);
  lcd.print(":)");

} // END OF SETUP


void loop(){}

// Allows user to increase/decrease option showed on LCD, returns chosen value when SET is pressed
int get_user_parameter(String prefix)
{
  int button_value = 1000;
  
  int temp = 0;
  do
  {      
    lcd.setCursor(0,1);
    lcd.print(prefix) + lcd.print(temp);
    
    button_value = analogRead(0);
    if (button_value < 60)
    {
      temp++;
    }
    else if (button_value > 400 && button_value < 600)
    {      
      if (temp > 0)
      {
        temp--;
      }
      else 
      {
        temp = 0;
      }
    }
       
    // Button sensitivity
    delay(BUTTON_DELAY);
  }
  while (button_value < 600 || button_value > 800);

  return temp;  
}

// Returns when SELECT is pressed
void wait_for_select(void)
{
  int button_value = 1000;
  do
  {
    button_value = 1000;
    button_value = analogRead(0);
    delay(BUTTON_DELAY);
  }
  while (button_value < 600 || button_value > 800);

  return;
}

// Reaches ramp temperature
void pre_heat(const int degrees_C)
{
  lcd.setCursor(0,1);
  lcd.print(String("PH - R(C): "));
  while(1)
  {
    sensors.requestTemperatures();
    int actual_C = round(sensors.getTempCByIndex(0));
    if (actual_C < 0)
    {
      actual_C = 0;
    }
    lcd.setCursor(11,1);
    lcd.print(actual_C);
    
    if (actual_C < degrees_C)
    {
      digitalWrite(PIN_RELAY, HIGH);
    }
    else
    {
      digitalWrite(PIN_RELAY, LOW);
      return;
    }
    delay(2000);
  }
}

// Brews to ramp specifications
void brew(const int degrees_C, const int minutes)
{
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("B - T");
  lcd.setCursor(7,1);
  lcd.print(" / RC");
  
  unsigned long start_time = millis();
  while(1)
  {
    unsigned long time_remaining = minutes - ((millis() - start_time)/60000);
    lcd.setCursor(5,1);
    lcd.print(time_remaining);

    sensors.requestTemperatures();
    int actual_C = round(sensors.getTempCByIndex(0));
    if (actual_C < 0)
    {
      actual_C = 0;
    }
    lcd.setCursor(12,1);
    lcd.print(actual_C);
    
    if (time_remaining <= 0)
    {
      digitalWrite(PIN_RELAY, LOW);
      return;
    }
    else if (actual_C <= degrees_C - LOWER_MARGIN)
    {
      digitalWrite(PIN_RELAY, HIGH);
    }
    else if (actual_C >= degrees_C + UPPER_MARGIN)
    {
      digitalWrite(PIN_RELAY, LOW);
    }
    delay(2000);
  }
}


  
