#include "Arduino.h"
#include "pitches.h"
#include "DFRobotDFPlayerMini.h"
#include <HardwareSerial.h>
#include "apwifieeprommode.h"
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 128 // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin (not used)
Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);

/* Define pin numbers for LEDs, buttons and speaker: */
int ledPins[] = {13, 14, 26, 33};
int buttonPins[] = {12, 27, 25, 32};
#define MAX_GAME_LENGTH 100

const char* opcMalas[] = {
  "Bob Esponja", "Los Simpsons", "Avatar: La Leyenda de Aang", "Rick and Morty",
  "Teen Titans Go!", "Adventure Time (Hora de Aventura)", "The Owl House",
  "Phineas y Ferb"};
const char* opcCorrectas[] = {"The Office", "Gravity Falls","Los Padrinos Mágicos"};

int gameSequence[MAX_GAME_LENGTH] = {0};
int gameIndex = 0;
int modo = 0;
int dif;

HardwareSerial mySerial(1); // Usa el puerto UART1 (puedes usar UART0, UART1 o UART2 en el ESP32)
// Crea un objeto DFPlayer
DFRobotDFPlayerMini myDFPlayer;

void setup(){
  Serial.begin(9600);
  Wire.begin(21, 22);  // Initialize I2C pins (SDA = 21, SCL = 22)

  // Initialize the display
  if (!display.begin(0x3C, true)) { // I2C address 0x3C
    Serial.println(F("OLED initialization failed!"));
    while (1); // Halt execution if display initialization fails
  }

  display.display();  // Show initial Adafruit splashscreen
  delay(1000);        // Pause for 2 seconds

  display.clearDisplay();  // Clear the buffer

  intentoconexion("SimonDice", "simondice");

  for (byte i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(15,OUTPUT);
  digitalWrite(15,HIGH);
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);
  pinMode(19,OUTPUT);
  digitalWrite(19,HIGH);

  // Inicia la comunicación UART con el DFPlayer Mini
  mySerial.begin(9600, SERIAL_8N1, 16, 17); // UART1: RX en GPIO16, TX en GPIO17
  
  // Espera a que el DFPlayer se inicie
  Serial.println("Iniciando DFPlayer Mini...");
  
  while (!myDFPlayer.begin(mySerial)) {
    Serial.println("¡No se pudo encontrar el DFPlayer Mini!");
    delay(1000);
    //while (true);  // Detener el programa si no se encuentra el DFPlayer
  }
  
  Serial.println("DFPlayer Mini listo!");

  // Ajusta el volumen del DFPlayer (0 a 30, 30 es el volumen máximo)
  myDFPlayer.volume(30);  // Ajuste de volumen

  randomSeed(analogRead(A3));
}

void displayMessage(const char *message, int textSize, int cursorX, int cursorY, uint16_t textColor) {
  display.clearDisplay();         // Clear the display buffer
  display.setTextSize(textSize);  // Set text size (1 = default size)
  display.setTextColor(textColor); // Set text color
  display.setCursor(cursorX, cursorY); // Set cursor position
  display.println(message);       // Print the message
  display.display();              // Show the updated buffer on the display
}

void displayOption(int optionNumber, const char *text, int yPosition) {
  display.setCursor(0, yPosition);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.print(optionNumber);
  display.print(". ");
  display.println(text);
}

void lightLedAndPlayTone(byte ledIndex, int t) {
  myDFPlayer.playFolder(1,(ledIndex+1));
  delay(500);
  digitalWrite(ledPins[ledIndex], HIGH);
  delay(t);
  digitalWrite(ledPins[ledIndex], LOW);
  myDFPlayer.stop();
}

/**
   Plays the current sequence of notes that the user has to repeat
*/
void playSequence(int t) {
  for (int i = 0; i < gameIndex; i++) {
    byte currentLed = gameSequence[i];
    lightLedAndPlayTone(currentLed, t);
    delay(300);
  }
}

/**
    Waits until the user pressed one of the buttons,
    and returns the index of that button
*/
byte readButtons() {
  while (true) {
    for (byte i = 0; i < 4; i++) {
      byte buttonPin = buttonPins[i];
      if (digitalRead(buttonPin) == LOW) {
        return i;
      }
    }
    delay(1);
  }
}

void gameOver(char* mensaje, bool cond) {
  delay(1000);
  myDFPlayer.playFolder(2,1);
  delay(200);
  Serial.print(mensaje);
  if(cond){
    Serial.println(gameIndex - 1);}
  char* txt = mensaje + (gameIndex-1);
  displayMessage(txt, 1, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
  gameIndex = 0;
  delay(1000);
  myDFPlayer.stop();
  delay(500);
}

bool checkUserSequence() {
  for (int i = 0; i < gameIndex; i++) {
    byte expectedButton = gameSequence[i];
    byte actualButton = readButtons();
    lightLedAndPlayTone(actualButton, 500);
    if (expectedButton != actualButton) {
      return false;
    }
  }

  return true;
}

void playLevelUpSound() {
  myDFPlayer.playFolder(2,2);
  delay(1500);
  myDFPlayer.stop();
}

void victoria(char* mensaje) {
  displayMessage(mensaje, 2, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
  myDFPlayer.playFolder(2,3);
  delay(7500);
  myDFPlayer.stop();
}

int selMode(){ //seleccionar el modo de juego
  displayMessage("\nSeleccione un modo de juego:\n1. Niveles\n2. Reto\n3. Adivinanza\n", 2, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
  while(1){
    if(digitalRead(buttonPins[0])==LOW){
      return 1;
    } else if(digitalRead(buttonPins[1])==LOW){
      return 2;
    } else if(digitalRead(buttonPins[2])==LOW){
      return 3;
    }
    delay(1);
  }
}

int difficulty(){ //seleccionar dificultad
  displayMessage("\nSeleccione una dificultad:\n1. Fácil\n2. Regular\n3. Difícil\n", 2, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
  while(1){
    if(digitalRead(buttonPins[0])==LOW){
      return 1600;
    } else if(digitalRead(buttonPins[1])==LOW){
      return 800;
    } else if(digitalRead(buttonPins[2])==LOW){
      return 500;
    }
    delay(1);
  }
}

void salir(){
  displayMessage("¿Desea volver a intentarlo?:\n1. Seguir\n2. Salir\n", 2, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
  while(1){
    if(digitalRead(buttonPins[0])==LOW){
      return;
    } else if(digitalRead(buttonPins[1])==LOW){
      modo = 0;
      return;
    }
    delay(1);
  }
}

int mostrarOpcion(int cancion, int opcion){
  int a = 100, b = 100, c = 100, d = 100;
  delay(100);

  display.clearDisplay();

  if (opcion == 0) {
    displayOption(1, opcCorrectas[cancion], 0);
  } else {
    a = random(0, 8);
    displayOption(1, opcMalas[a], 0);
  }

  if (opcion == 1) {
    displayOption(2, opcCorrectas[cancion], 16);
  } else {
    while (a == b || b == 100) {
      b = random(0, 8);
    }
    displayOption(2, opcMalas[b], 16);
  }

  if (opcion == 2) {
    displayOption(3, opcCorrectas[cancion], 32);
  } else {
    while (c == b || c == 100) {
      c = random(0, 8);
    }
    displayOption(3, opcMalas[c], 32);
  }

  if (opcion == 3) {
    displayOption(4, opcCorrectas[cancion], 48);
  } else {
    while (c == d || d == 100) {
      d = random(0, 8);
    }
    displayOption(4, opcMalas[d], 48);
  }

  display.display();
  delay(100);

  while (1) {
    if (digitalRead(buttonPins[0]) == LOW) {
      return 0;
    } else if (digitalRead(buttonPins[1]) == LOW) {
      return 1;
    } else if (digitalRead(buttonPins[2]) == LOW) {
      return 2;
    } else if (digitalRead(buttonPins[3]) == LOW) {
      return 3;
    }
    delay(1);
  }
}

bool elegirCancion(int cancion){
  myDFPlayer.playFolder(3, (cancion + 1));
  delay(30000);
  myDFPlayer.stop();
  displayMessage("\n¿Cual cancion es?:\n", 2, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
  int opcion = random(0, 4);
  int respuesta = mostrarOpcion(cancion, opcion);
  delay(100);

  return (opcion == respuesta);
}

void loop(){

  loopAP();

  if (modo == 0){
    modo = selMode();
    dif = 10;
    delay(300);
  }

  // Add a random color to the end of the sequence
  if(modo==1){

    if (dif==10 || gameIndex == 0){
      displayMessage("Niveles de Secuencia", 2, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
      delay(2000);
      dif = difficulty();
      delay(1000);
    }

    gameSequence[gameIndex] = random(0, 4);
    gameIndex++;
    if (gameIndex >= MAX_GAME_LENGTH) {
      gameIndex = MAX_GAME_LENGTH - 1;
    }

    delay(500);
    playSequence(dif);
    if (!checkUserSequence()) {
      gameOver("Game over! your score: ", 1);
      salir();
    }

    delay(300);

    if (gameIndex > 0) {
      playLevelUpSound();
      delay(300);
    }

  } else if (modo == 2){ //Segundo modo
    displayMessage("Modo Reto", 2, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
    delay(2000);
    gameIndex = 7;
    for (int i = 0; i < gameIndex; i++) {
      gameSequence[i] = random(0, 4);
    }

    int dif = difficulty();
    delay(500);
    playSequence(dif);
    if (!checkUserSequence()) {
      gameOver("Game over, good luck in your next attempt!", 0);
    } else {
      victoria("¡FELICIDADES HAS GANADO!");
    }
    salir();
  } else if (modo == 3){
    displayMessage("Modo Adivinanza", 2, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
    delay(2000);

    int cancion = random(0, 3);
    if (elegirCancion(cancion)){
      victoria("\n¡FELICIDADES HAS ACERTADO!\n");
    } else {
      gameOver("\nOpción equivocada :(\n", 0);
    }
    salir();
  }

  delay(1000);

}