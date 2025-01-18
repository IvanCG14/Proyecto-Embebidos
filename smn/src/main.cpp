#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <HardwareSerial.h>
#include "apwifieeprommode.h"
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <EEPROM.h>
#include <FirebaseESP32.h>
#include <ESP32Servo.h>

FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;
// Credenciales de Firebase
#define URL "prueba-27492-default-rtdb.firebaseio.com"  // URL de tu base de datos
#define CLAV "H7BQhMuA8WG5vfMVE9AIRxWcZTvrfvUOGN1PfUI8" // Clave secreta de Firebase

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 128 // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin (not used)

// Crear una instancia del objeto Servo
Servo myServo;

// Pin de control del servo en el ESP32
const int servoPin = 18;

// Variables para controlar velocidad y dirección
int velocidad = 1500; // Valor de señal en microsegundos (1000-2000)
// 1500 µs: Detener, <1500: Sentido antihorario, >1500: Sentido horario

// Declaración de funciones
void detenerServo();
void rotarSentidoHorario();

Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);

/* Define pin numbers for LEDs, buttons and speaker: */
int ledPins[] = {13, 14, 26, 33};
int buttonPins[] = {12, 27, 25, 32};
#define MAX_GAME_LENGTH 100

const char *opcMalas[] = {
    "Bob_Esponja", "Los_Simpsons", "Avatar:_La_Leyenda_de_Aang", "Rick_and_Morty",
    "Teen_Titans_Go!", "Hora_de_Aventura", "The_Owl_House",
    "Phineas_y_Ferb"};
const char *opcCorrectas[] = {"The_Office", "Gravity_Falls", "Los_Padrinos_Magicos"};

int gameSequence[MAX_GAME_LENGTH] = {0};
int gameIndex = 0;
int modo = 0;
int dif;
int pos = 0;

HardwareSerial mySerial(1); // Usa el puerto UART1 (puedes usar UART0, UART1 o UART2 en el ESP32)
// Crea un objeto DFPlayer
DFRobotDFPlayerMini myDFPlayer;

void setup()
{
  myServo.attach(servoPin, 1000, 2000); // Pin, min = 1000 µs, max = 2000 µs

  Serial.begin(9600);
  Wire.begin(21, 22); // Initialize I2C pins (SDA = 21, SCL = 22)

  detenerServo(); // Iniciar en estado detenido

  // Initialize the display
  if (!display.begin(0x3C, true))
  { // I2C address 0x3C
    Serial.println(F("OLED initialization failed!"));
    while (1)
      ; // Halt execution if display initialization fails
  }

  display.display(); // Show initial Adafruit splashscreen
  delay(1000);       // Pause for 2 seconds

  display.clearDisplay(); // Clear the buffer

  intentoconexion("SimonDice", "simondice");

  for (byte i = 0; i < 4; i++)
  {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Inicia la comunicación UART con el DFPlayer Mini
  mySerial.begin(9600, SERIAL_8N1, 16, 17); // UART1: RX en GPIO16, TX en GPIO17

  // Espera a que el DFPlayer se inicie
  Serial.println("Iniciando DFPlayer Mini...");

  while (!myDFPlayer.begin(mySerial))
  {
    Serial.println("¡No se pudo encontrar el DFPlayer Mini!");
    delay(1000);
    // while (true); // Detener el programa si no se encuentra el DFPlayer
  }

  Serial.println("DFPlayer Mini listo!");

  // Ajusta el volumen del DFPlayer (0 a 30, 30 es el volumen máximo)
  myDFPlayer.volume(30); // Ajuste de volumen

  randomSeed(analogRead(A3));
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Conectado a Wi-Fi con éxito");
    Serial.print("IP local: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Error al conectar a Wi-Fi. Verifique la configuración.");
    return;
  }

  // Configurar Firebase
  config.host = URL;
  config.signer.tokens.legacy_token = CLAV;

  // Tiempo de espera para la conexión al servidor
  config.timeout.serverResponse = 10 * 1000; // 10 segundos

  // Inicializar Firebase con configuración y autenticación
  Firebase.begin(&config, &auth);

  // Habilitar reconexión automática al Wi-Fi
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase inicializado con éxito");
}

// Función para detener el servomotor
void detenerServo()
{
  velocidad = 1500; // Señal de parada para un servo de rotación continua
  myServo.writeMicroseconds(velocidad);
}

// Función para rotar en sentido horario
void rotarSentidoHorario()
{
  velocidad = 1700; // Ajusta según la velocidad deseada
  myServo.writeMicroseconds(velocidad);
}
void rotarSentidoAntihorario()
{
  velocidad = 1300; // Ajusta según la velocidad deseada
  myServo.writeMicroseconds(velocidad);
}

void displayMessage(const char *message, int textSize, int cursorX, int cursorY, uint16_t textColor)
{
  display.clearDisplay();              // Clear the display buffer
  display.setTextSize(textSize);       // Set text size (1 = default size)
  display.setTextColor(textColor);     // Set text color
  display.setCursor(cursorX, cursorY); // Set cursor position
  display.println(message);            // Print the message
  display.display();                   // Show the updated buffer on the display
}

void displayOption(int optionNumber, const char *text, int yPosition)
{
  display.setCursor(0, yPosition);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.print(optionNumber);
  display.print(". ");
  display.println(text);
  if (optionNumber == 1)
  {
    if (Firebase.setString(firebaseData, "/DESAFIO/OPCION1", text))
    {
    }
    else
    {
      Serial.print("Error al actualizar el valor: ");
      Serial.println(firebaseData.errorReason());
    }
  }
  else if (optionNumber == 2)
  {
    if (Firebase.setString(firebaseData, "/DESAFIO/OPCION2", text))
    {
    }
    else
    {
      Serial.print("Error al actualizar el valor: ");
      Serial.println(firebaseData.errorReason());
    }
  }
  else if (optionNumber == 3)
  {
    if (Firebase.setString(firebaseData, "/DESAFIO/OPCION3", text))
    {
    }
    else
    {
      Serial.print("Error al actualizar el valor: ");
      Serial.println(firebaseData.errorReason());
    }
  }
  else if (optionNumber == 4)
  {
    if (Firebase.setString(firebaseData, "/DESAFIO/OPCION4", text))
    {
    }
    else
    {
      Serial.print("Error al actualizar el valor: ");
      Serial.println(firebaseData.errorReason());
    }
  }
}

void lightLedAndPlayTone(byte ledIndex, int t)
{
  myDFPlayer.playFolder(1, (ledIndex + 1));
  delay(500);
  digitalWrite(ledPins[ledIndex], HIGH);
  delay(t);
  digitalWrite(ledPins[ledIndex], LOW);
  myDFPlayer.stop();
}

/**
Plays the current sequence of notes that the user has to repeat
*/
void playSequence(int t)
{
  for (int i = 0; i < gameIndex; i++)
  {
    byte currentLed = gameSequence[i];
    lightLedAndPlayTone(currentLed, t);
    delay(300);
  }
}

/**
Waits until the user pressed one of the buttons,
and returns the index of that button
*/
byte readButtons()
{
  while (true)
  {
    for (byte i = 0; i < 4; i++)
    {
      byte buttonPin = buttonPins[i];
      if (digitalRead(buttonPin) == LOW)
      {
        return i;
      }
    }
    delay(1);
  }
}

void gameOver(char *mensaje, bool cond)
{
  myDFPlayer.playFolder(2, 1);
  delay(200);
  display.clearDisplay();                              // Clear the display buffer
  display.setTextSize(2);                              // Set text size (1 = default size)
  display.setTextColor(SH110X_WHITE);                  // Set text color
  int16_t textHeight = 8 * 2;                          // Height of the text, assuming text size 2 (8 pixels per line)
  display.setCursor(10, 50);             // Set cursor position
  display.print(mensaje);                // Print the message
  if (cond)
  {
    display.print(gameIndex - 1);
  }
  display.display();
  gameIndex = 0;
  delay(1000);
  myDFPlayer.stop();
  delay(1);
}

bool checkUserSequence()
{
  for (int i = 0; i < gameIndex; i++)
  {
    byte expectedButton = gameSequence[i];
    byte actualButton = readButtons();
    lightLedAndPlayTone(actualButton, 500);
    if (expectedButton != actualButton)
    {
      return false;
    }
  }

  return true;
}

void playLevelUpSound()
{
  myDFPlayer.playFolder(2, 2);
  delay(1500);
  myDFPlayer.stop();
}

void victoria(char *mensaje)
{
  displayMessage(mensaje, 1.8, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
  myDFPlayer.playFolder(2, 3);
  delay(1000);
  rotarSentidoAntihorario();
  delay(2000);
  detenerServo();
  delay(4500);
  myDFPlayer.stop();
}
int selMode()
{                                                                                                                                  // Seleccionar el modo de juego desde Firebase
  displayMessage("\nSeleccione un modo de juego:\n1. Niveles\n2. Reto\n3. Adivinanza\n4.Personlizado", 1.5, 10, 50, SH110X_WHITE); // Mensaje en pantalla

  while (true)
  {
    if (Firebase.getString(firebaseData, "simon_dice/GG"))
    { // Leer el valor de Firebase en la dirección "/GG"
      String modoStr = firebaseData.stringData();

      if (modoStr == "1" || digitalRead(buttonPins[0]) == LOW)
      {
        return 1;
      }
      else if (modoStr == "2" || digitalRead(buttonPins[1]) == LOW)
      {
        return 2;
      }
      else if (modoStr == "3" || digitalRead(buttonPins[2]) == LOW)
      {
        return 3;
      }
      else if (modoStr == "4" || digitalRead(buttonPins[3]) == LOW)
      {
        return 4;
      }
    }
    else
    {
      Serial.print("Error al leer Firebase: ");
      Serial.println(firebaseData.errorReason());
    }

    delay(10); // Evitar lecturas demasiado rápidas
  }
}

int difficulty()
{                                                                                                                // seleccionar dificultad
  displayMessage("\nSeleccione una dificultad:\n1. Facil\n2. Regular\n3. Dificil\n", 1.5, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
  while (1)
  {
    if (Firebase.getString(firebaseData, "simon_dice/dificultad"))
    { // Leer el valor de Firebase en la dirección "/dificultad"
      String ndif = firebaseData.stringData();
      if (digitalRead(buttonPins[0]) == LOW || ndif == "1")
      {
        return 1600;
      }
      else if (digitalRead(buttonPins[1]) == LOW || ndif == "2")
      {
        return 800;
      }
      else if (digitalRead(buttonPins[2]) == LOW || ndif == "3")
      {
        return 500;
      }
    }
    else
    {
      Serial.print("Error al leer Firebase: ");
      Serial.println(firebaseData.errorReason());
    }
    delay(10);
  }

  if (Firebase.setString(firebaseData, "simon_dice/dificultad", "0"))
  {
  }
  else
  {
    Serial.print("Error al leer Firebase: ");
    Serial.println(firebaseData.errorReason());
  }
}

void salir()
{
  displayMessage("¿Desea volver a intentarlo?:\n1. Seguir\n2. Salir\n", 1.5, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
  while (1)
  {
    if (Firebase.getString(firebaseData, "simon_dice/seguir"))
    { // Leer el valor de Firebase en la dirección "/seguir"
      String seguir = firebaseData.stringData();
      if (digitalRead(buttonPins[0]) == LOW || seguir == "true")
      {
        if (Firebase.setString(firebaseData, "/simon_dice/seguir", "false"))
        {
          Serial.println("Valor actualizado correctamente.");
          return;
        }
        else
        {
          Serial.print("Error al actualizar el valor: ");
          Serial.println(firebaseData.errorReason());
        }
      }
    }
    else
    {
      Serial.print("Error al leer Firebase: ");
      Serial.println(firebaseData.errorReason());
    }

    if (Firebase.getString(firebaseData, "simon_dice/salir"))
    { // Leer el valor de Firebase en la dirección "/seguir"
      String salir = firebaseData.stringData();
      if (digitalRead(buttonPins[1]) == LOW || salir == "true")
      {
        if (Firebase.setString(firebaseData, "/simon_dice/salir", "false"))
        {
          Serial.println("Valor actualizado correctamente.");
          modo = 0;
          return;
        }
        else
        {
          Serial.print("Error al actualizar el valor: ");
          Serial.println(firebaseData.errorReason());
        }
      }
    }
    else
    {
      Serial.print("Error al leer Firebase: ");
      Serial.println(firebaseData.errorReason());
    }
    delay(10);
  }
}

byte mostrarOpcion(int cancion, int opcion)
{
  int a = 100, b = 100, c = 100, d = 100;

  if (opcion == 0)
  {
    displayOption(1, opcCorrectas[cancion], 32);
  }
  else
  {
    a = random(0, 8);
    displayOption(1, opcMalas[a], 32);
  }

  if (opcion == 1)
  {
    displayOption(2, opcCorrectas[cancion], 48);
  }
  else
  {
    while (a == b || b == 100)
    {
      b = random(0, 8);
    }
    displayOption(2, opcMalas[b], 48);
  }

  if (opcion == 2)
  {
    displayOption(3, opcCorrectas[cancion], 64);
  }
  else
  {
    while (c == b || c == 100)
    {
      c = random(0, 8);
    }
    displayOption(3, opcMalas[c], 64);
  }

  if (opcion == 3)
  {
    displayOption(4, opcCorrectas[cancion], 80);
  }
  else
  {
    while (c == d || d == 100)
    {
      d = random(0, 8);
    }
    displayOption(4, opcMalas[d], 80);
  }
  display.display();

  byte x = readButtons();
  return x;
}

bool elegirCancion(int cancion)
{
  myDFPlayer.playFolder(3, (cancion + 1));
  delay(10000);
  myDFPlayer.stop();
  displayMessage("\n¿Cual cancion es?:\n", 1, 0, 16, SH110X_WHITE); // Larger text at position (10, 50)
  int opcion = random(0, 4);
  int respuesta = mostrarOpcion(cancion, opcion);
  delay(1);

  return (opcion == respuesta);
}

void loop()
{

  loopAP();

  for (int i = 0; i <= 9; i++)
  {
    if (Firebase.set(firebaseData, "test", i))
    {
      Serial.println("Dato enviado con exito: ");
    }
    else
    {
      Serial.println("Error al enviar dato: ");
    }
  }

  if (modo == 0)
  {
    if (Firebase.setString(firebaseData, "/simon_dice/GG", "0"))
    {
    }
    else
    {
      Serial.print("Error al actualizar el valor: ");
      Serial.println(firebaseData.errorReason());
    }
    modo = selMode();
    dif = 10;
    delay(1);
  }

  // Add a random color to the end of the sequence
  if (modo == 1)
  {

    if (dif == 10 || gameIndex == 0)
    {
      displayMessage("Modo\nClasico", 2, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
      delay(2000);
      dif = difficulty();
      delay(100);
    }

    gameSequence[gameIndex] = random(0, 4);
    gameIndex++;
    if (gameIndex >= MAX_GAME_LENGTH)
    {
      gameIndex = MAX_GAME_LENGTH - 1;
    }

    delay(500);
    playSequence(dif);
    if (!checkUserSequence())
    {
      gameOver("Game over\nYou score", 1);
      salir();
    }

    delay(300);

    if (gameIndex > 0)
    {
      playLevelUpSound();
      delay(10);
    }
  }
  else if (modo == 2)
  {                                                           // Segundo modo
    displayMessage("Modo\nDesafio", 2, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
    delay(2000);
    gameIndex = 7;
    for (int i = 0; i < gameIndex; i++)
    {
      gameSequence[i] = random(0, 4);
    }

    int dif = difficulty();
    delay(500);
    playSequence(dif);
    if (!checkUserSequence())
    {
      gameOver("Game over,\ngood luck\nin your\nnext attempt!", 0);
    }
    else
    {
      victoria("¡FELICIDADES\nHAS GANADO!");
    }
    salir();
  }
  else if (modo == 3)
  {
    displayMessage("Modo\nAdivinanza", 2, 10, 50, SH110X_WHITE); // Larger text at position (10, 50)
    delay(2000);

    int cancion = random(0, 3);
    if (elegirCancion(cancion))
    {
      victoria("¡FELICIDADES\nHAS ACERTADO!");
    }
    else
    {
      gameOver("Opcion\nequivocada :(", 0);
    }
    salir();
  }
  else if (modo == 4)
  {
    displayMessage("Modo\nPersonalizado\nIngresa secuencia", 2, 10, 50, SH110X_WHITE); // Mensaje en pantalla
    delay(2000);

    // Array para guardar la secuencia de botones
    int secuencia[7] = {0};

    // Pedimos al usuario que ingrese 7 botones
    for (int i = 0; i < 7; i++)
    {
      displayMessage("Presione\nun boton", 2, 10, 50, SH110X_WHITE);
      int botonPresionado = readButtons();        // Obtener la posición del botón presionado
      secuencia[i] = botonPresionado;             // Guardar en la secuencia
      lightLedAndPlayTone(botonPresionado, 1000); // Iluminar LED correspondiente y reproducir tono
      delay(300);                                 // Esperar un poco antes de continuar
    }

    // Ahora vamos a almacenar la secuencia en Firebase
    for (int i = 0; i < 7; i++)
    {
      String path = "/secuencia/p" + (i + 1); // Crear la ruta para la carpeta p1, p2, ..., p7
      String value = String(secuencia[i]);    // Convertir el valor a cadena
      if (Firebase.setString(firebaseData, path.c_str(), value.c_str()))
      {
        Serial.print("Valor de p");
        Serial.print(i + 1);
        Serial.print(" almacenado correctamente: ");
        Serial.println(value);
      }
      else
      {
        Serial.print("Error al almacenar valor en ");
        Serial.print(path);
        Serial.print(": ");
        Serial.println(firebaseData.errorReason());
      }
    }

    // Recuperamos la secuencia almacenada en Firebase y la reproducimos
    int secuenciaRecuperada[7];
    for (int i = 0; i < 7; i++)
    {
      String path = "/secuencia/p" + (i + 1); // Crear la ruta de la carpeta
      if (Firebase.getString(firebaseData, path.c_str()))
      {
        secuenciaRecuperada[i] = firebaseData.stringData().toInt(); // Convertir la respuesta en un entero
        Serial.print("p");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(secuenciaRecuperada[i]);
      }
      else
      {
        Serial.print("Error al recuperar valor de ");
        Serial.print(path);
        Serial.print(": ");
        Serial.println(firebaseData.errorReason());
      }
    }

    // Reproducimos la secuencia recuperada
    displayMessage("Reproduciendo\nSecuencia Personalizada", 1.7, 10, 50, SH110X_WHITE); // Mensaje en pantalla
    delay(1000);

    for (int i = 0; i < 7; i++)
    {
      lightLedAndPlayTone(secuenciaRecuperada[i], 500); // Reproducir la secuencia
      delay(500);                                       // Pausa entre los tonos
    }

    // Ahora pedimos al jugador que repita la secuencia
    displayMessage("Repita la\nsecuencia", 2, 10, 50, SH110X_WHITE); // Mensaje de instrucción
    delay(1000);

    bool bvictoria = true;

    for (int i = 0; i < 7; i++)
    {
      byte botonPresionado = readButtons();       // Obtener la posición del botón presionado
      lightLedAndPlayTone(botonPresionado, 1000); // Iluminar LED correspondiente y reproducir tono

      if (botonPresionado != secuenciaRecuperada[i])
      {
        bvictoria = false;
        break; // Si hay un error, se sale del ciclo
      }
      delay(300); // Esperar un poco antes de continuar
    }

    if (bvictoria)
    {
      victoria("¡Felicidades, has ganado!"); // Mensaje de victoria
      delay(100);                            // Pausa antes de continuar
    }
    else
    {
      gameOver("¡Juego\nterminado!", 0); // Mensaje de derrota
      delay(100);                       // Pausa antes de continuar
    }

    // Regresar a estado inicial
    gameIndex = 0;

    salir();
  }
  if (Firebase.setString(firebaseData, "/simon_dice/dificultad", "0"))
  {
  }
  else
  {
    Serial.print("Error al actualizar el valor: ");
    Serial.println(firebaseData.errorReason());
  }

  delay(1);
}
