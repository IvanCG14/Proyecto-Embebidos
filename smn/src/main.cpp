#include "Arduino.h"
#include "pitches.h"
#include "DFRobotDFPlayerMini.h"
#include <HardwareSerial.h>

/* Define pin numbers for LEDs, buttons and speaker: */
int ledPins[] = {13, 12, 26, 27};
int buttonPins[] = {17, 18, 33, 34};
#define MAX_GAME_LENGTH 100

const int SPEAKER_PIN = 5;
const int gameTones[] = { NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G5};
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

  for (byte i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);
  pinMode(16,OUTPUT);
  digitalWrite(16,HIGH);

  // Inicia la comunicación UART con el DFPlayer Mini
  mySerial.begin(9600, SERIAL_8N1, 16, 17); // UART1: RX en GPIO16, TX en GPIO17
  
  // Espera a que el DFPlayer se inicie
  Serial.println("Iniciando DFPlayer Mini...");
  
  if (!myDFPlayer.begin(mySerial)) {
    Serial.println("¡No se pudo encontrar el DFPlayer Mini!");
    while (true);  // Detener el programa si no se encuentra el DFPlayer
  }
  
  Serial.println("DFPlayer Mini listo!");

  // Ajusta el volumen del DFPlayer (0 a 30, 30 es el volumen máximo)
  myDFPlayer.volume(20);  // Ajuste de volumen

  pinMode(SPEAKER_PIN, OUTPUT);
  randomSeed(analogRead(A3));
}

void lightLedAndPlayTone(byte ledIndex, int t) {
  digitalWrite(ledPins[ledIndex], HIGH);
  myDFPlayer.playFolder(1,(ledIndex+1));
  //tone(SPEAKER_PIN, gameTones[ledIndex]);
  delay(t);
  digitalWrite(ledPins[ledIndex], LOW);
  myDFPlayer.stop();
  //noTone(SPEAKER_PIN);
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

/**
  Play the game over sequence, and report the game score
*/
void gameOver(char* mensaje, bool cond) {
  Serial.print(mensaje);
  if(cond){
    Serial.println(gameIndex - 1);}
  gameIndex = 0;
  delay(200);

  myDFPlayer.playFolder(2,1);
  delay(1100);
  myDFPlayer.stop();
  /* Play a Wah-Wah-Wah-Wah sound
  tone(SPEAKER_PIN, NOTE_DS5);
  delay(300);
  tone(SPEAKER_PIN, NOTE_D5);
  delay(300);
  tone(SPEAKER_PIN, NOTE_CS5);
  delay(300);
  for (byte i = 0; i < 10; i++) {
    for (int pitch = -10; pitch <= 10; pitch++) {
      tone(SPEAKER_PIN, NOTE_C5 + pitch);
      delay(5);
    }
  }
  noTone(SPEAKER_PIN);*/
  delay(500);
}

/**
   Get the user's input and compare it with the expected sequence.
*/
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

/**
   Plays a hooray sound whenever the user finishes a level
*/
void playLevelUpSound() {
  /*tone(SPEAKER_PIN, NOTE_E4);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G4);
  delay(150);
  tone(SPEAKER_PIN, NOTE_E5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_C5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_D5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G5);
  delay(150);
  noTone(SPEAKER_PIN);*/
  myDFPlayer.playFolder(2,2);
  delay(1500);
  myDFPlayer.stop();
}

void victoria(char* mensaje) {
  Serial.print(mensaje);
  /*tone(SPEAKER_PIN, NOTE_C4);
  delay(250);
  tone(SPEAKER_PIN, NOTE_G3);
  delay(125);
  tone(SPEAKER_PIN, NOTE_G3);
  delay(125);
  tone(SPEAKER_PIN, NOTE_A3);
  delay(250);
  tone(SPEAKER_PIN, NOTE_G3);
  delay(250);
  noTone(SPEAKER_PIN);
  delay(250);
  tone(SPEAKER_PIN, NOTE_B3);
  delay(250);
  tone(SPEAKER_PIN, NOTE_C4);
  delay(250);
  noTone(SPEAKER_PIN);*/
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
  Serial.print("\nSeleccione una dificultad:\n1. Fácil\n2. Regular\n3. Difícil\n4. Imposible\n");
  while(1){
    if(digitalRead(buttonPins[0])==LOW){
      return 1500;
    } else if(digitalRead(buttonPins[1])==LOW){
      return 500;
    } else if(digitalRead(buttonPins[2])==LOW){
      return 100;
    } else if(digitalRead(buttonPins[3])==LOW){
      return 50;
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
    delay(3000);

    int cancion = random(0, 3);
    if (elegirCancion(cancion)){
      victoria("¡FELICIDADES HAS ACERTADO!");
    } else {
      gameOver("Opción equivocada :(", 0);
    }
    salir();
  }

  delay(1000);

}