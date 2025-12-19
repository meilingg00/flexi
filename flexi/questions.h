#ifndef QUESTIONS_H
#define QUESTIONS_H

struct MathProblem {
  String question; // the math problem text
  int optionA;     // value for button 1
  int optionB;     // value for button 2
  int correctBtn;  // correct button (has to be 1 or 2)
};

const int TOTAL_QUESTIONS = 15;

MathProblem questionBank[TOTAL_QUESTIONS] = {
  {"2 + 2 = ?",    4,   5,   1},
  {"5 + 3 = ?",    9,   8,   2},
  {"10 - 4 = ?",   6,   4,   1},
  {"3 x 3 = ?",    6,   9,   2},
  {"12 / 2 = ?",   6,   5,   1},
  {"6 + 7 = ?",    13,  12,  1},
  {"9 - 5 = ?",    3,   4,   2},
  {"8 + 8 = ?",    18,  16,  2},
  {"5 x 2 = ?",    10,  7,   1},
  {"15 - 7 = ?",   9,   8,   2},
  {"9 + 10 = ?",   21,  19,  2},
  {"20 + 5 = ?",   25,  27,  1},
  {"sqrt(4)",      1,   2,   2},
  {"30 / 10",      3,   5,   1},
  {"4 * 4",        16,  12,  1}
};

#endif