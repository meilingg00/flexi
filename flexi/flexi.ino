#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include "questions.h"
#include "colors.h"
#include "animations.h"

MCUFRIEND_kbv tft;

#define PIN_BTN1      11
#define PIN_BTN2      12
#define PIN_VIBRO     3
#define PIN_BUZZER    6 
#define PIN_RGB_R     8
#define PIN_RGB_G     9
#define PIN_RGB_B     10

// state var
enum RobotState { STATE_HAPPY, STATE_SAD, STATE_GAME };
RobotState currentState = STATE_HAPPY;
RobotState previousState = STATE_GAME; // force update on start

// timing stuff
unsigned long lastInteractionTime = 0; 
const long sadTimeout = 10000;  // ts is the time settings before the robot turns sad (in ms)
unsigned long lastBlinkTime = 0;
int blinkInterval = 3000; 

// game var
int streakCounter = 0;
int targetStreak = 5;        // if u wanna change the correct streak do it here
int currentQuestionIndex = 0;
bool waitingForAnswer = false;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A0)); // questions are randomized

  tft.begin(0x9481); 
  tft.setRotation(3); 
  tft.fillScreen(BLACK); 

  pinMode(PIN_BTN1, INPUT); 
  pinMode(PIN_BTN2, INPUT);
  pinMode(PIN_VIBRO, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, HIGH);

  pinMode(PIN_RGB_R, OUTPUT);
  pinMode(PIN_RGB_G, OUTPUT);
  pinMode(PIN_RGB_B, OUTPUT);

  lastInteractionTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  bool btn1 = (digitalRead(PIN_BTN1) == HIGH);
  bool btn2 = (digitalRead(PIN_BTN2) == HIGH);
  
  // if either button is pressed, it resets the sad state timer
  if (btn1 || btn2) {
    lastInteractionTime = currentMillis;
    delay(200);
  }

  // state machine
  switch (currentState) {
    // HAPPY
    case STATE_HAPPY:
      // draw logic
      if (previousState != STATE_HAPPY) {
        tft.fillScreen(BLACK);
        drawEyes(0); // 0 = happy | 1 = sad
        setLED(0);
        previousState = STATE_HAPPY;
      }
      
      // blink logic
      if (currentMillis - lastBlinkTime > blinkInterval) {
        blinkEyes(0);
        lastBlinkTime = currentMillis;
        blinkInterval = random(2000, 4000);
      }

      // trigger joy eyes logic
      if (btn1 || btn2) {
         drawEyes(2); 
         
         // also add vibration bcs why not
         digitalWrite(PIN_VIBRO, HIGH);
         delay(200); 
         digitalWrite(PIN_VIBRO, LOW);
         delay(1500); 
         
         // reset the timer
         lastInteractionTime = millis();
         currentMillis = lastInteractionTime;
         previousState = STATE_GAME; 
      }

      // check for timeout
      // if timeout go to sad state
      if (currentMillis - lastInteractionTime > sadTimeout) {
        currentState = STATE_SAD;
      }
      break;

    // SAD
    case STATE_SAD:
      // draw logic
      if (previousState != STATE_SAD) {
        tft.fillScreen(BLACK);
        drawEyes(1); // 0 = happy | 1 = sad
        setLED(1);
        previousState = STATE_SAD;
      }

      // start the game if the button is pressed
      if (btn1 || btn2) {
        currentState = STATE_GAME;
        streakCounter = 0;
        waitingForAnswer = false; 
      }
      break;

    // MATH
    case STATE_GAME:
      if (previousState != STATE_GAME) {
        tft.fillScreen(BLACK);
        setLED(2);
        previousState = STATE_GAME;
        waitingForAnswer = false;
      }

      if (!waitingForAnswer) {
        currentQuestionIndex = random(0, TOTAL_QUESTIONS); 
        drawGameScreen();
        waitingForAnswer = true;
      } else {
        if (btn1) handleAnswer(1);
        if (btn2) handleAnswer(2);
      }
      break;
  }
}

void handleAnswer(int userChoice) {
  // check ans correctness
  if (userChoice == questionBank[currentQuestionIndex].correctBtn) {
    // CORRECT
    streakCounter++;
    
    // vibrate motor
    digitalWrite(PIN_VIBRO, HIGH);
    tft.fillRect(0, 0, 480, 50, GREEN); // flash header green
    delay(300);
    digitalWrite(PIN_VIBRO, LOW);
    
    // check win cond
    if (streakCounter >= targetStreak) {
      tft.fillScreen(GREEN);

      tft.setCursor(100, 140);
      tft.setTextColor(BLACK);
      tft.setTextSize(4);
      tft.print("Good Job!");

      // win music
      for(int i=0; i<3; i++) {
          digitalWrite(PIN_BUZZER, LOW); delay(100);
          digitalWrite(PIN_BUZZER, HIGH); delay(100);
      }

      delay(2000);
      currentState = STATE_HAPPY; // return state to happy
      return;
    }

  } else {
    // WRONG
    streakCounter = 0;
    
    // flash LED red and sound buzzer
    analogWrite(PIN_RGB_R, 0); 
    analogWrite(PIN_RGB_G, 255); 
    analogWrite(PIN_RGB_B, 255);
    digitalWrite(PIN_BUZZER, LOW);
    tft.fillRect(0, 0, 480, 50, RED); // flash header red
    delay(500);
    digitalWrite(PIN_BUZZER, HIGH); 
    analogWrite(PIN_RGB_R, 0); 
    analogWrite(PIN_RGB_G, 255); 
    analogWrite(PIN_RGB_B, 0);
  }
  waitingForAnswer = false; 
}

void drawGameScreen() {
  tft.fillScreen(BLACK);
  
  // counter header during math game
  tft.fillRect(0, 0, 480, 60, BLUE);
  tft.setCursor(20, 20);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.print("Streak: ");
  tft.print(streakCounter);
  tft.print("/5");

  // question
  tft.setCursor(120, 120);
  tft.setTextColor(WHITE);
  tft.setTextSize(4);
  tft.print(questionBank[currentQuestionIndex].question);
  
  // left option (button 1)
  tft.fillRect(20, 200, 200, 80, 0x3333);
  tft.setCursor(90, 225);
  tft.print(questionBank[currentQuestionIndex].optionA);
  tft.setCursor(30, 290);
  tft.setTextSize(2);
  tft.print("(Btn 1)");

  // right option (button 2)
  tft.fillRect(260, 200, 200, 80, 0x3333);
  tft.setCursor(340, 225);
  tft.setTextSize(4);
  tft.print(questionBank[currentQuestionIndex].optionB);
  tft.setCursor(270, 290);
  tft.setTextSize(2);
  tft.print("(Btn 2)");
}

// LED is common anode so 0-255 is flipped
void setLED(int state) {
  if (state == 0) {
    // 0 happy - green
    analogWrite(PIN_RGB_R, 255); 
    analogWrite(PIN_RGB_G, 0); 
    analogWrite(PIN_RGB_B, 255);
  } else if (state == 1) {
    // 1 sad - blue
    analogWrite(PIN_RGB_R, 255); 
    analogWrite(PIN_RGB_G, 255); 
    analogWrite(PIN_RGB_B, 0);
  } else {
    // game - purple
    analogWrite(PIN_RGB_R, 0); 
    analogWrite(PIN_RGB_G, 255); 
    analogWrite(PIN_RGB_B, 0);
  }
}