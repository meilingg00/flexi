#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include "colors.h"

extern MCUFRIEND_kbv tft;

// eye config
int eyeRadius = 50;
int leftEyeX = 140;
int rightEyeX = 340;
int eyeY = 160;

// helper to draw a thick line for visibility
void drawThickLine(int x1, int y1, int x2, int y2, int color) {
  int thickness = 5; // change this for thickness
  
  for (int i = 0; i < thickness; i++) {
    for (int j = 0; j < thickness; j++) {
      tft.drawLine(x1 + i, y1 + j, x2 + i, y2 + j, color);
    }
  }
}

// 0 = happy, 1 = sad
void drawEyes(int mood) {

  // happy eyes (default)
  if (mood == 0) { 
    tft.fillCircle(leftEyeX, eyeY, eyeRadius, WHITE);
    tft.fillCircle(rightEyeX, eyeY, eyeRadius, WHITE);
    tft.fillCircle(leftEyeX, eyeY, 20, BLUE);
    tft.fillCircle(rightEyeX, eyeY, 20, BLUE);
  }

  // sad eyes
  else if (mood == 1) { 
    tft.fillCircle(leftEyeX, eyeY, eyeRadius, WHITE);
    tft.fillCircle(rightEyeX, eyeY, eyeRadius, WHITE);
    tft.fillCircle(leftEyeX, eyeY, 20, BLUE);
    tft.fillCircle(rightEyeX, eyeY, 20, BLUE);
    
    tft.fillRect(leftEyeX - eyeRadius, eyeY - eyeRadius, eyeRadius*2, 40, BLACK);
    tft.fillRect(rightEyeX - eyeRadius, eyeY - eyeRadius, eyeRadius*2, 40, BLACK);
  }

  // joy eyes ^^
  else if (mood == 2) { 
    tft.fillCircle(leftEyeX, eyeY, eyeRadius, BLACK);
    tft.fillCircle(rightEyeX, eyeY, eyeRadius, BLACK);

    // apex at (140, 130), legs down to 160
    drawThickLine(leftEyeX, eyeY-30, leftEyeX-25, eyeY+10, WHITE);
    drawThickLine(leftEyeX, eyeY-30, leftEyeX+25, eyeY+10, WHITE);
    drawThickLine(rightEyeX, eyeY-30, rightEyeX-25, eyeY+10, WHITE);
    drawThickLine(rightEyeX, eyeY-30, rightEyeX+25, eyeY+10, WHITE);
  }
}

void blinkEyes(int mood) {
  tft.fillCircle(leftEyeX, eyeY, eyeRadius, BLACK);
  tft.fillCircle(rightEyeX, eyeY, eyeRadius, BLACK);
  delay(100);
  drawEyes(mood);
}

#endif