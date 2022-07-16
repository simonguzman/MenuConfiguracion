#include <LiquidCrystal.h>
#include <Keypad.h>
#include <EasyBuzzer.h>
#include "AsyncTaskLib.h"

#define analogPin A0 //the thermistor attach to
#define beta 4090 //the beta of the thermistor
#define resistance 10 //the value of the pull-down resistorvoid

//LED
#define BLUE 7
#define GREEN 8
#define RED 9

const int bzz = A7;
const int btn = 48;
const int micro = A1;
const int fotoPin = A2;
const int soundPin = A3;
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {31,33,35,37}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {39,41,43}; //connect to the column pinouts of the keypad

int pos = 0;
float tempAlta = 25.0f;
float tempBaja = 18.0f;
float intensidad = 300.0f;
boolean enter = false;

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
void leerTemp();
//Asynctask es la opcion para una tarea asincrona de la temperatura
AsyncTask asyncTemperatura(2000, true, leerTemp );

void luz();
//Asynctask es la opcion para una tarea asincrona de la luz
AsyncTask asyncLuz(2000, true, luz );
int outputValue = 0;

void setup(){
  Serial.begin(115200);
  // set up the LCD’s number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0,0);
  //Activa los leds y el boton
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(btn, INPUT);
  //pinMode(bzz, OUTPUT);
  //Activar el buzzer, o alarma
  EasyBuzzer.setPin(bzz);

  //Inicia las tareas para la temperatura
  asyncTemperatura.Start();
  asyncLuz.Start();
}

const char frame[5][16] = {
  {"UtempHigh      "},
  {"UtempLow       "},
  {"Uluz           "},
  {"Usonido        "},
  {"reset          "}
};
int page = 0; // Pantallas que se presentan, en este caso 5
int selectedIndex = 0; //Indice o pantalla que se seleccion
int lastFrame = 4; //Ultima pantalla del lcd
//---------------------------------Opciones de Menu---------------------------------
void imprimirMenu(){
  lcd.setCursor(0,0);
  lcd.print( frame[page]);
  lcd.setCursor(0,1);
  lcd.print( frame[page+1]);
  indeceCursor();
}

void MenuConfig(){
  switch (page)
  {
     case 0:
        if(selectedIndex == 0){
          configTempAlta();
        }else{
          configTempBaja();
        }
        break;
     case 1:
        if(selectedIndex == 0){
          configTempBaja();
        }else{
          Luz();
        }
        break;
     case 2:
        if(selectedIndex == 0){
          Luz();
        }else{
          Sonido();
        }
        break;                    
     case 3:
        if(selectedIndex == 0){
          Sonido();
        }else{
          Sonido();
        }
        break;
     default: break;
  }
}

void controlMenu(){
  char key = keypad.getKey();
    if (key == '0'){
      lcd.clear();
      EasyBuzzer.singleBeep(200, 80);
      selectedIndex++;
    }else if(key == '*'){
      lcd.clear();
      EasyBuzzer.singleBeep(200, 80);
      selectedIndex--;
    }else if(digitalRead(btn) == 0){
      enter = true;
    }
    controlIndice();
    controlPagina();  
    while(enter){
      MenuConfig();
    }
    imprimirMenu(); 
}
//---------------------------------Manejo de lcd---------------------------------
void indeceCursor(){
  //selectedIndex == 0 ? lcd.setCursor(0,0) : lcd.setCursor(0,1);
  if(selectedIndex == 0){
    lcd.setCursor(15,0);
  }else if(selectedIndex == 1){
    lcd.setCursor(15,1);
  }
  lcd.print("<-");
}           

void controlIndice(){ // controlar el indice
  if(selectedIndex == 2){
    selectedIndex = 0;
    page++;              
  }              
  if(selectedIndex < 0){
    page--;  
    selectedIndex = 1;
  }
}

//Controla la pagina
void controlPagina(){
  if(page == lastFrame ){
    lcd.clear();
    page = 0;
  }else if(page < 0){
    lcd.clear();
    page = lastFrame - 1;
  }
}
//---------------------------------Calculos de temperatura, luz, sonido---------------------------------
//Funcion para leer temperatura
void leerTemp(){
  long a = 1023 - analogRead(analogPin);
  float tempC = beta /(log((1025.0 * 10 / a - 10) / 10) + beta / 298.0) - 273.0;
 
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(tempC);
  lcd.print(" C");

  colorTemp(tempC);
}

//Funcion para verificar la temperatura
void colorTemp(float temp){
  int ligth = 255;

  if(temp > getTempAlta()){
    //rojo
    analogWrite(RED, ligth);
  }else if(temp < getTempBaja()){
    //azul
    analogWrite(BLUE, ligth);
  }else{
    //verde
    analogWrite(GREEN, ligth);
  }

  delay(1000);
  analogWrite(RED, 0);
  analogWrite(BLUE, 0);
  analogWrite(GREEN, 0);
}

//Funcion para verificar la luz
void luz(){
  outputValue = analogRead(fotoPin);
  lcd.setCursor(0, 1);
  lcd.print("Photocell: ");
  lcd.setCursor(11, 1);
  lcd.print(outputValue);//print the temperature on lcd1602
  Serial.println(outputValue);
  delay(1000);
  lcd.setCursor(11, 1);
  lcd.print(" ");  
}

//Funcion para verificar el sonido
void microfono()
{
  int value = analogRead(soundPin);
  Serial.println(value);
  if(value > 44)
  {
    Serial.println("AHH");
    //digitalWrite(ledPin,HIGH);
    //delay(100);
  }
  else
  {
  //digitalWrite(ledPin,LOW);
  }
  delay(100);
}
//---------------------------------Configuraciones de temperatura, luz y sonido---------------------------------
void configTempAlta(){
lcd.setCursor(0,0);
lcd.print("Temp Alta");
char key = keypad.getKey();
if (key == '0' && tempAlta < 40.f ){
  tempAlta = tempAlta + 1.0f;
  EasyBuzzer.stopBeep();
  analogWrite(RED, 0);
}else if(key == '*' && tempAlta > 24.0f){
  tempAlta = tempAlta - 1.0f;
  EasyBuzzer.stopBeep();
  analogWrite(RED, 0);
}else if(digitalRead(btn) == 0 && enter == true){
  enter = false;
  analogWrite(RED, 0);
  EasyBuzzer.stopBeep();
}    

if(key == '0' && tempAlta == 40.f || key == '*' && tempAlta == 24.0f){
  analogWrite(RED, 255);
  EasyBuzzer.beep(180, 100, 60, 5, 1000, 2);
}


lcd.setCursor(0,1); //Imprima en la segunda fila
lcd.print("Valor: ");
lcd.print(tempAlta,2);
}

//Configuracion de la temperatura con el buzzer
void configTempBaja(){
  lcd.setCursor(0,0);
  lcd.print("Temp baja");
  char key = keypad.getKey();
  if (key == '0' && tempBaja < 19.0f){
    tempBaja = tempBaja + 1.0f;
    analogWrite(BLUE, 0);
    EasyBuzzer.stopBeep();
  }else if(key == '*' && tempBaja > 0.0f){
    tempBaja = tempBaja - 1.0f;
    analogWrite(BLUE, 0);
    EasyBuzzer.stopBeep();
  }else if(digitalRead(btn) == 0 && enter == true){
    enter = false;
    analogWrite(BLUE, 0);
    EasyBuzzer.stopBeep();
  }    
 if(key == '0' && tempBaja > 18.0f || key == '*' && tempBaja == 0.0f){
    analogWrite(BLUE, 255);
    EasyBuzzer.beep(180, 100, 60, 5, 1000, 2);
 }
  lcd.setCursor(0,1);
  lcd.print("Valor: ");
  lcd.print(tempBaja,2);              
}

//Configuracion de la luz con el buzzer
void Luz(){
  lcd.setCursor(0,0);
  lcd.print("Luz");
  char key = keypad.getKey();
  if (key == '0' && intensidad < 18.0f){
    tempBaja = tempBaja + 1.0f;
    analogWrite(RED, 0);
    EasyBuzzer.stopBeep();
  }else if(key == '*' && tempBaja > 0.0f){
    tempBaja = tempBaja - 1.0f;
    analogWrite(RED, 0);
    EasyBuzzer.stopBeep();
  }else if(digitalRead(btn) == 0 && enter == true){
    enter = false;
    analogWrite(RED, 0);
    EasyBuzzer.stopBeep();
  }    
 if(key == '0' && tempBaja == 18.0f || key == '*' && tempBaja == 0.0f){
    analogWrite(RED, 255);
    EasyBuzzer.beep(180, 100, 60, 5, 1000, 2);
 }
  lcd.setCursor(0,1);
  lcd.print("Valor: ");
  lcd.print(tempBaja,2);
}

void Sonido(){

}
//---------------------------------Retorno de variables---------------------------------
float getTempAlta(){
  return tempAlta;
}
float getTempBaja(){
  return tempBaja;
}
