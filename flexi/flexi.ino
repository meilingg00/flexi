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
#define PIN_PIR       2   

// state var
enum RobotState { STATE_HAPPY, STATE_SAD, STATE_GAME, STATE_SLEEP };
RobotState currentState = STATE_HAPPY;
RobotState previousState = STATE_GAME; // force update on start

// timing stuff
unsigned long lastInteractionTime = 0; 
const long sadTimeout = 10000;   // 10 seconds to get Sad
const long sleepTimeout = 20000; // 20 seconds to Sleep
unsigned long lastBlinkTime = 0;
int blinkInterval = 3000; 

// game var
int streakCounter = 0;
int targetStreak = 5;        
int currentQuestionIndex = 0;
bool waitingForAnswer = false;
int lives = 3;               
unsigned long questionStartTime = 0;
const long questionTimeout = 5000; 
int lastDisplayedTime = -1; 

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A0)); 

  tft.begin(0x9481); 
  tft.setRotation(3); 
  tft.fillScreen(BLACK); 

  pinMode(PIN_BTN1, INPUT); 
  pinMode(PIN_BTN2, INPUT);
  pinMode(PIN_PIR, INPUT); 
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
  
  // if either button is pressed, it resets the sleep timer
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
        drawEyes(0); 
        setLED(0);
        previousState = STATE_HAPPY;
      }
      
      // blink logic
      if (currentMillis - lastBlinkTime > blinkInterval) {
        blinkEyes(0);
        lastBlinkTime = currentMillis;
        blinkInterval = random(2000, 4000);
      }

      // INTERACTION: JOY (Petting)
      // This triggers the Joy animation but stays in HAPPY state
      if (btn1 || btn2) {
          drawEyes(2); 
          
          digitalWrite(PIN_VIBRO, HIGH);
          delay(200); 
          digitalWrite(PIN_VIBRO, LOW);
          delay(1500); 
          
          lastInteractionTime = millis();
          currentMillis = lastInteractionTime;
          
          // Force the loop to redraw the Happy face next time
          previousState = STATE_GAME; 
          // Note: We do NOT change currentState to STATE_GAME here anymore
      }

      // CHECK TIMERS
      // If idle for > 10 sec, go to SAD
      if (currentMillis - lastInteractionTime > sadTimeout) {
        currentState = STATE_SAD;
      }
      break;

    // SAD
    case STATE_SAD:
      // draw logic
      if (previousState != STATE_SAD) {
        tft.fillScreen(BLACK);
        drawEyes(1); // Sad Eyes
        setLED(1);   // Sad LED
        previousState = STATE_SAD;
      }

      // CHECK TIMERS
      // If idle for > 20 sec, go to SLEEP
      if (currentMillis - lastInteractionTime > sleepTimeout) {
        currentState = STATE_SLEEP;
      }

      // Interaction Logic: Wake up from SAD to GAME
      if (btn1 || btn2) {
        currentState = STATE_GAME;
        streakCounter = 0;
        waitingForAnswer = false; 
      }
      break;
      
    // SLEEP STATE
    case STATE_SLEEP:
      if (previousState != STATE_SLEEP) {
        // --- UPDATED HERE ---
        // We use drawEyes(3) to call the new logic in animations.h
        tft.fillScreen(BLACK); // Ensure background is clean
        drawEyes(3); 
        
        // Turn off LEDs
        analogWrite(PIN_RGB_R, 255); 
        analogWrite(PIN_RGB_G, 255); 
        analogWrite(PIN_RGB_B, 255);
        previousState = STATE_SLEEP;
      }
      
      // WAKE UP CONDITION: PIR Sensor
      if (digitalRead(PIN_PIR) == HIGH) {
          Serial.println("Motion Detected - Waking Up!"); 
          currentState = STATE_HAPPY;
          lastInteractionTime = millis(); 
      }
      
      // WAKE UP CONDITION: Buttons
      if (btn1 || btn2) {
          currentState = STATE_HAPPY;
          lastInteractionTime = millis();
      }
      break;

    // MATH GAME
    case STATE_GAME:
      if (previousState != STATE_GAME) {
        tft.fillScreen(BLACK);
        setLED(2);
        previousState = STATE_GAME;
        waitingForAnswer = false;
        lives = 3;
        streakCounter = 0;
        lastDisplayedTime = -1; 
      }

      if (!waitingForAnswer) {
        currentQuestionIndex = random(0, TOTAL_QUESTIONS); 
        drawGameScreen(); 
        questionStartTime = millis();
        waitingForAnswer = true;
        lastDisplayedTime = -1; 
      } else {
        // --- REAL TIME TIMER UPDATE ---
        unsigned long elapsed = millis() - questionStartTime;
        int remainingTime = max(0, (int)((questionTimeout - elapsed) / 1000));
        
        if (remainingTime != lastDisplayedTime) {
            tft.fillRect(200, 60, 200, 50, BLACK); 
            tft.setCursor(200, 75);
            tft.setTextColor(WHITE);
            tft.setTextSize(3);
            tft.print("Time: ");
            tft.print(remainingTime);
            tft.print("s");
            lastDisplayedTime = remainingTime;
        }

        // Timeout Logic
        if (elapsed > questionTimeout) {
          lives--;
          
          if (lives <= 0) {
            // Game Over
            tft.fillScreen(BLACK);
            tft.setCursor(80, 140);
            tft.setTextColor(RED);
            tft.setTextSize(4);
            tft.print("Game Over!");
            
            delay(2000);
            currentState = STATE_HAPPY;
            lastInteractionTime = millis(); 
            return;
          }
          
          waitingForAnswer = false;
          streakCounter = 0;
        }
        
        if (btn1) handleAnswer(1);
        if (btn2) handleAnswer(2);
      }
      break;
  }
}

void handleAnswer(int userChoice) {
  if (userChoice == questionBank[currentQuestionIndex].correctBtn) {
    // CORRECT
    streakCounter++;
    
    digitalWrite(PIN_VIBRO, HIGH);
    tft.fillRect(0, 0, 480, 50, GREEN); 
    delay(300);
    digitalWrite(PIN_VIBRO, LOW);
    
    if (streakCounter >= targetStreak) {
      tft.fillScreen(GREEN);
      tft.setCursor(100, 140);
      tft.setTextColor(BLACK);
      tft.setTextSize(4);
      tft.print("Good Job!");

      for(int i=0; i<3; i++) {
          digitalWrite(PIN_BUZZER, LOW); delay(100);
          digitalWrite(PIN_BUZZER, HIGH); delay(100);
      }

      delay(2000);
      currentState = STATE_HAPPY; 
      lastInteractionTime = millis(); 
      return;
    }

  } else {
    // WRONG
    streakCounter = 0;
    lives--; // Decrease life on wrong answer

    // Visual Feedback (Red Flash)
    analogWrite(PIN_RGB_R, 0); 
    analogWrite(PIN_RGB_G, 255); 
    analogWrite(PIN_RGB_B, 255);
    digitalWrite(PIN_BUZZER, LOW);
    tft.fillRect(0, 0, 480, 50, RED); 
    delay(500);
    digitalWrite(PIN_BUZZER, HIGH); 
    analogWrite(PIN_RGB_R, 0); 
    analogWrite(PIN_RGB_G, 255); 
    analogWrite(PIN_RGB_B, 0);

    // Check for Game Over immediately after wrong answer
    if (lives <= 0) {
        tft.fillScreen(BLACK);
        tft.setCursor(80, 140);
        tft.setTextColor(RED);
        tft.setTextSize(4);
        tft.print("Game Over!");
        
        delay(2000);
        currentState = STATE_HAPPY;
        lastInteractionTime = millis();
        return;
    }
  }
  waitingForAnswer = false; 
}

void drawGameScreen() {
  tft.fillScreen(BLACK);
  
  tft.fillRect(0, 0, 480, 60, BLUE);
  tft.setCursor(20, 20);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.print("Streak: ");
  tft.print(streakCounter);
  tft.print("/5");
  
  drawHearts(380, 30, lives);
  
  tft.setCursor(120, 130);
  tft.setTextColor(WHITE);
  tft.setTextSize(4);
  tft.print(questionBank[currentQuestionIndex].question);
  
  tft.fillRect(20, 200, 200, 80, 0x3333);
  tft.setCursor(90, 225);
  tft.print(questionBank[currentQuestionIndex].optionA);
  tft.setCursor(30, 290);
  tft.setTextSize(2);
  tft.print("(Btn 1)");

  tft.fillRect(260, 200, 200, 80, 0x3333);
  tft.setCursor(340, 225);
  tft.setTextSize(4);
  tft.print(questionBank[currentQuestionIndex].optionB);
  tft.setCursor(270, 290);
  tft.setTextSize(2);
  tft.print("(Btn 2)");
}

void setLED(int state) {
  if (state == 0) {
    analogWrite(PIN_RGB_R, 255); 
    analogWrite(PIN_RGB_G, 0); 
    analogWrite(PIN_RGB_B, 255);
  } else if (state == 1) {
    analogWrite(PIN_RGB_R, 255); 
    analogWrite(PIN_RGB_G, 255); 
    analogWrite(PIN_RGB_B, 0);
  } else {
    analogWrite(PIN_RGB_R, 0); 
    analogWrite(PIN_RGB_G, 255); 
    analogWrite(PIN_RGB_B, 0);
  }
}

void drawHeartShape(int x, int y, uint16_t color) {
  tft.fillCircle(x - 5, y - 5, 6, color);
  tft.fillCircle(x + 5, y - 5, 6, color);
  tft.fillTriangle(x - 11, y - 3, x + 11, y - 3, x, y + 12, color);
}

void drawHearts(int x, int y, int numHearts) {
  for (int i = 0; i < 3; i++) {
    int xPos = x + (i * 30); 
    if (i < numHearts) {
      drawHeartShape(xPos, y, RED);
    } else {
      drawHeartShape(xPos, y, 0x528A); 
    }
  }
}