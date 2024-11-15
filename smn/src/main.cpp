#include "Arduino.h"
#include "pitches.h"
#include "DFRobotDFPlayerMini.h"
#include <HardwareSerial.h>
#include "apwifieeprommode.h"
#include <EEPROM.h>

/* Define pin numbers for LEDs, buttons and speaker: */
int ledPins[] = {13, 12, 26, 27};
int buttonPins[] = {5, 18, 33, 34};
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
  Serial.print(mensaje);
  myDFPlayer.playFolder(2,3);
  delay(7500);
  myDFPlayer.stop();
}

int selMode(){ //seleccionar el modo de juego
  Serial.print("\nSeleccione un modo de juego:\n1. Niveles\n2. Reto\n3. Adivinanza\n");
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
  Serial.print("\nSeleccione una dificultad:\n1. Fácil\n2. Regular\n3. Difícil\n");
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
  Serial.print("¿Desea volver a intentarlo?:\n1. Seguir\n2. Salir\n");
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
  int a = 100; int b = 100; int c =100; int d = 100;
  delay(100);

  if (opcion==0){
    Serial.print("1. ");
    Serial.println(opcCorrectas[cancion]);
  } else {
    a = random(0,8);
    Serial.print("1. ");
    Serial.println(opcMalas[a]);
  } 
  
  if (opcion==1){
    Serial.print("2. ");
    Serial.println(opcCorrectas[cancion]);
  } else{
    while (a==b || b==100){
      b = random(0,8);
    }
    Serial.print("2. ");
    Serial.println(opcMalas[b]);
  } 
  
  if (opcion==2){
    Serial.print("3. ");
    Serial.println(opcCorrectas[cancion]);
  } else{
    while (c==b || c==100){
      c = random(0,8);
    }
    Serial.print("3. ");
    Serial.println(opcMalas[c]);
  } 
  
  if (opcion==3){
    Serial.print("4. ");
    Serial.println(opcCorrectas[cancion]);
  } else{
    while (c==d || d==100){
      d = random(0,8);
    }
    Serial.print("4. ");
    Serial.println(opcMalas[d]);
  } 

  delay(100);

  while(1){
    if(digitalRead(buttonPins[0])==LOW){
      return 0;
    } else if(digitalRead(buttonPins[1])==LOW){
      return 1;
    } else if(digitalRead(buttonPins[2])==LOW){
      return 2;
    } else if(digitalRead(buttonPins[3])==LOW){
      return 3;
    }
    delay(1);
  }

}

bool elegirCancion(int cancion){
  myDFPlayer.playFolder(3, (cancion + 1));
  delay(30000);
  myDFPlayer.stop();
  Serial.print("\n¿Cual cancion es?:\n");
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
      Serial.print("\nNiveles de Secuencia\n");
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
    Serial.print("\nModo Reto\n");
    delay(1000);
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
    Serial.print("\nModo de Adivinanzas\n");
    delay(300);

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