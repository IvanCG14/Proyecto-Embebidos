#include "Arduino.h"
#include "pitches.h"
//#include "DFRobotDFPlayerMini.h"
//#include "SoftwareSerial.h"

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
//SoftwareSerial serial(25,32);
//DFRobotDFPlayerMini myDFPlayer;

void setup(){
  /*
  Serial.begin(9600);
  Serial.begin(9600);
  myDFPlayer.begin(Serial1);
  delay(2000);
  myDFPlayer.volume(20);
  myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);*/
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

  pinMode(SPEAKER_PIN, OUTPUT);
  randomSeed(analogRead(A3));


}

void lightLedAndPlayTone(byte ledIndex) {
  digitalWrite(ledPins[ledIndex], HIGH);
  //myDFPlayer.playFolder(1,ledIndex);
  tone(SPEAKER_PIN, gameTones[ledIndex]);
  delay(300);
  digitalWrite(ledPins[ledIndex], LOW);
 // myDFPlayer.stop();
  noTone(SPEAKER_PIN);
}

/**
   Plays the current sequence of notes that the user has to repeat
*/
void playSequence(int t) {
  for (int i = 0; i < gameIndex; i++) {
    byte currentLed = gameSequence[i];
    lightLedAndPlayTone(currentLed);
    delay(t);
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
void gameOver(char* mensaje) {
  Serial.print(mensaje);
  Serial.println(gameIndex - 1);
  gameIndex = 0;
  delay(200);

  // Play a Wah-Wah-Wah-Wah sound
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
  noTone(SPEAKER_PIN);
  delay(500);
}

/**
   Get the user's input and compare it with the expected sequence.
*/
bool checkUserSequence() {
  for (int i = 0; i < gameIndex; i++) {
    byte expectedButton = gameSequence[i];
    byte actualButton = readButtons();
    lightLedAndPlayTone(actualButton);
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
  tone(SPEAKER_PIN, NOTE_E4);
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
  noTone(SPEAKER_PIN);
}

void victoria(char* mensaje) {
  Serial.print(mensaje);
  tone(SPEAKER_PIN, NOTE_C4);
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
  noTone(SPEAKER_PIN);
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
      return 10;
    }
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
  }
}

int mostrarOpcion(int cancion, int opcion){
  int a, b, c, d = 100;

  if (opcion==0){
    Serial.print("1. ");
    Serial.print(opcCorrectas[cancion]);
  } else {
    a = random(0,8);
    Serial.print("1. ");
    Serial.print(opcMalas[a]);
  } 
  
  if (opcion==1){
    Serial.print("2. ");
    Serial.print(opcCorrectas[cancion]);
  } else{
    while (a==b || b==100){
      b = random(0,8);
    }
    Serial.println("2. ");
    Serial.print(opcMalas[b]);
  } 
  
  if (opcion==2){
    Serial.println("3. ");
    Serial.print(opcCorrectas[cancion]);
  } else{
    while (c==b || c==100){
      c = random(0,8);
    }
    Serial.println("3. ");
    Serial.print(opcMalas[c]);
  } 
  
  if (opcion==3){
    Serial.println("4. ");
    Serial.print(opcCorrectas[cancion]);
  } else{
    while (c==d || d==100){
      d = random(0,8);
    }
    Serial.println("4. ");
    Serial.print(opcMalas[d]);
  } 

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
  }

}

bool elegirCancion(int cancion){
  //playFolder(3, (cancion + 1));
  //delay(10000);
  Serial.print("\n¿Cual cancion es?:\n");
  int opcion = random(0, 4);
  int respuesta = mostrarOpcion(cancion, opcion);

  return (opcion == respuesta);
}

void loop(){
  if (modo == 0){
    modo = selMode();
    delay(300);
  }

  // Add a random color to the end of the sequence
  if(modo==1){
    Serial.print("\nNiveles de Secuencia\n");
    delay(3000);
    gameSequence[gameIndex] = random(0, 4);
    gameIndex++;
    if (gameIndex >= MAX_GAME_LENGTH) {
      gameIndex = MAX_GAME_LENGTH - 1;
    }

    int dif = difficulty();
    playSequence(dif);
    if (!checkUserSequence()) {
      gameOver("Game over! your score: ");
      salir();
    }

    delay(300);

    if (gameIndex > 0) {
      playLevelUpSound();
      delay(300);
    }

  } else if (modo == 2){ //Segundo modo
    Serial.print("\nModo Reto\n");
    delay(3000);
    gameIndex = 7;
    for (int i = 0; i < gameIndex; i++) {
      gameSequence[i] = random(0, 4);
    }

    int dif = difficulty();
    playSequence(dif);
    if (!checkUserSequence()) {
      gameOver("Game over, good luck in your next attempt!");
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
      gameOver("Opción equivocada :(");
    }
    salir();
  }

}