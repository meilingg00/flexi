#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include "colors.h"

// --- COLOR DEFINITIONS ---
#ifndef PINK
#define PINK 0xF81F
#endif
#ifndef YELLOW
#define YELLOW 0xFFE0
#endif
// Assuming BLACK, WHITE, BLUE exist in colors.h

extern MCUFRIEND_kbv tft;

// Eye configuration
int leftEyeX = 140;
int rightEyeX = 340;
int eyeY = 160;
int centerX = (leftEyeX + rightEyeX) / 2; 

// Face background dimensions
const int faceW = 320;
const int faceH = 220;
const int faceTopY = eyeY - 90;


// --- HELPER FUNCTIONS ---

// Helper to draw a thick straight line
void drawThickLine(int x1, int y1, int x2, int y2, int color, int thickness) {
  for (int i = 0; i < thickness; i++) {
    tft.drawLine(x1 + i, y1 + i, x2 + i, y2 + i, color);
    if (thickness > 2) {
       tft.drawLine(x1 + i - 1, y1 + i, x2 + i - 1, y2 + i, color);
    }
  }
}

// Helper to draw the standardized white face background
void drawFaceBg() {
  tft.fillRoundRect(centerX - faceW/2, faceTopY, faceW, faceH, 40, WHITE);
}

// A Universal Arc Helper (Used for Sleep eyes, Joy eyes, cheeks, mouths)
// Creates smooth thick curves by drawing a ring and cutting part of it off with the background color
// type: 0=Bottom(U), 1=Top(^), 2=Left(C), 3=Right(D)
void drawArc(int x, int y, int r, int thick, int color, int bgColor, int type) {
   int rInner = r - thick;
   // 1. Draw the full thick ring
   tft.fillCircle(x, y, r, color);       // Outer circle
   tft.fillCircle(x, y, rInner, bgColor); // Inner circle (hole)

   // 2. Cut off parts using rectangles of the bgColor
   int cutExtra = 5; // Overlap to ensure clean cut
   if (type == 0) { // Bottom Arc ( U ) -> Cut Top half
      tft.fillRect(x - r, y - r - cutExtra, r * 2, r + cutExtra, bgColor);
   }
   else if (type == 1) { // Top Arc ( ^ ) -> Cut Bottom half
      tft.fillRect(x - r, y, r * 2, r + cutExtra, bgColor);
   }
   else if (type == 2) { // Left Arc ( C ) -> Cut Right half
      tft.fillRect(x, y - r, r + cutExtra, r * 2, bgColor);
   }
   else if (type == 3) { // Right Arc ( D ) -> Cut Left half
      tft.fillRect(x - r - cutExtra, y - r, r + cutExtra, r * 2, bgColor);
   }
}

// New style Simple Eye: Small, solid black circle (Happy)
void drawSimpleEye(int x, int y) {
  tft.fillCircle(x, y, 28, BLACK);
}

// New style Flat Eye: Simple thick horizontal line (Sad)
void drawFlatEye(int x, int y) {
  int w = 60; // Width of the line
  int h = 8;  // Thickness of the line
  tft.fillRect(x - w/2, y - h/2, w, h, BLACK);
}

// --- MOUTH/FACE COMPONENT HELPERS ---

// Simple Smile (Arc) - For Happy face
void drawSmile(int x, int y) {
  // Using drawArc type 0 (Bottom arc)
  drawArc(x, y + 15, 25, 4, BLACK, WHITE, 0);
}

// Frown (Arc) - For Sad face
void drawFrown(int x, int y) {
  // Using drawArc type 1 (Top arc)
  drawArc(x, y + 100, 50, 5, BLACK, WHITE, 1);
}

// 'o' mouth with nose connection (Joy/Sleep style)
void drawJoyMouth(int x, int y) {
  tft.fillCircle(x, y + 35, 12, BLACK); // Nose dot
  tft.fillCircle(x, y + 75, 15, BLACK); // Outer mouth ring
  tft.fillCircle(x, y + 75, 8, WHITE);  // Inner mouth hole (white)
  drawThickLine(x, y + 45, x, y + 63, BLACK, 5); // Line connecting nose/mouth
}

// Cheeks with dots (used in Happy/Sad/Joy/Sleep)
void drawCheekDetails(int cy) {
  int cheekR = 35; int cheekThick = 4; int cheekY = cy + 50;
  int dotSize = 3;
  
  // Left Cheek
  drawArc(leftEyeX - 60, cheekY, cheekR, cheekThick, BLACK, WHITE, 2); // Left Arc
  int lx = leftEyeX - 50; 
  tft.fillCircle(lx, cheekY - 15, dotSize, BLACK);
  tft.fillCircle(lx - 8, cheekY + 5, dotSize, BLACK);
  tft.fillCircle(lx + 5, cheekY + 20, dotSize, BLACK);

  // Right Cheek
  drawArc(rightEyeX + 60, cheekY, cheekR, cheekThick, BLACK, WHITE, 3); // Right Arc
  int rx = rightEyeX + 50; 
  tft.fillCircle(rx, cheekY - 15, dotSize, BLACK);
  tft.fillCircle(rx + 8, cheekY + 5, dotSize, BLACK);
  tft.fillCircle(rx - 5, cheekY + 20, dotSize, BLACK);
}


// --- MAIN DRAW LOGIC ---
// 0=Happy, 1=Sad, 2=Joy, 3=Sleep
void drawEyes(int mood) {

  // --- HAPPY (0) ---
  if (mood == 0) { 
    drawFaceBg(); // Standard White Background
    drawSimpleEye(leftEyeX, eyeY);
    drawSimpleEye(rightEyeX, eyeY);
    tft.fillCircle(centerX, eyeY + 25, 10, BLACK); // Nose dot
    drawSmile(centerX, eyeY);
    drawCheekDetails(eyeY);
  }

  // --- SAD (1) - Updated to match new image ---
  else if (mood == 1) { 
    drawFaceBg(); // Standard White Background
    drawFlatEye(leftEyeX, eyeY);
    drawFlatEye(rightEyeX, eyeY);
    
    tft.fillCircle(centerX, eyeY + 35, 10, BLACK); // Nose dot
    drawFrown(centerX, eyeY);
    drawCheekDetails(eyeY);
  }

  // --- JOY (2) ---
  else if (mood == 2) { 
    drawFaceBg(); // Standard White Background
    
    // "Arc" style happy eyes (^^) using Top Arcs (type 1)
    int eyeR = 40; int eyeThick = 6;
    drawArc(leftEyeX, eyeY, eyeR, eyeThick, BLACK, WHITE, 1);
    drawArc(rightEyeX, eyeY, eyeR, eyeThick, BLACK, WHITE, 1);

    // Pink blush pads under eyes
    tft.fillRoundRect(leftEyeX - 30, eyeY + 45, 60, 25, 12, PINK);
    tft.fillRoundRect(rightEyeX - 30, eyeY + 45, 60, 25, 12, PINK);
    
    drawJoyMouth(centerX, eyeY);
  }

  // --- SLEEP (3) ---
  else if (mood == 3) { 
    drawFaceBg(); // Standard White Background
    
    // "Arc" style sleep eyes (UU) using Bottom Arcs (type 0)
    int eyeR = 40; int eyeThick = 6;
    drawArc(leftEyeX, eyeY - 10, eyeR, eyeThick, BLACK, WHITE, 0);
    drawArc(rightEyeX, eyeY - 10, eyeR, eyeThick, BLACK, WHITE, 0);

    drawJoyMouth(centerX, eyeY);
    drawCheekDetails(eyeY);
  }
}

void blinkEyes(int mood) {
  // Clear screen area to black (slightly larger than faceBg to ensure clean wipe)
  tft.fillRect(centerX - faceW/2 - 10, faceTopY - 10, faceW + 20, faceH + 20, BLACK);
  delay(150); 
  drawEyes(mood);
}

#endif