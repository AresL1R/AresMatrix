#if (USE_SNAKE == 1)
#define START_LENGTH 4   
#define MAX_LENGTH 80     

// **************** pro vývojВ ****************
int8_t vectorX, vectorY;
int8_t headX, headY, buttX, buttY;
int8_t appleX, appleY;
boolean apple_flag, missDelete = false;

int8_t buttVector[MAX_LENGTH];
int snakeLength;
boolean butt_flag, pizdetc;

void snakeRoutine() {
  if (loadingFlag) {
    FastLED.clear();
    loadingFlag = false;
    newGameSnake();
    gamemodeFlag = true;
    modeCode = MC_GAME;
    putApple();
  }  
  
  buttonsTickSnake();

  if (gameTimer.isReady()) {

    if (!apple_flag) putApple();

    if (vectorX > 0) buttVector[snakeLength] = 0;
    else if (vectorX < 0) buttVector[snakeLength] = 1;
    if (vectorY > 0) buttVector[snakeLength] = 2;
    else if (vectorY < 0) buttVector[snakeLength] = 3;

    headX += vectorX;
    headY += vectorY;

    if (headX < 0 || headX > WIDTH - 1 || headY < 0 || headY > HEIGHT - 1) {
      pizdetc = true;
    }

    if (!pizdetc) {
      if ((long)(getPixColorXY(headX, headY) != 0 && (long)getPixColorXY(headX, headY) != GLOBAL_COLOR_2)) {  
        pizdetc = true;                           
      }

      if (!pizdetc && (long)getPixColorXY(headX, headY) == (long)GLOBAL_COLOR_2) { 
        apple_flag = false;                       
        snakeLength++;                            
        buttVector[snakeLength] = 4;              
      }

     
      switch (buttVector[0]) {
        case 0: buttX += 1;
          break;
        case 1: buttX -= 1;
          break;
        case 2: buttY += 1;
          break;
        case 3: buttY -= 1;
          break;
        case 4: missDelete = true;  
          break;
      }

      for (byte i = 0; i < snakeLength; i++) {
        buttVector[i] = buttVector[i + 1];
      }

      if (!missDelete) {
        drawPixelXY(buttX, buttY, 0x000000);
      }
      else missDelete = false;

      drawPixelXY(headX, headY, GLOBAL_COLOR_1);
      FastLED.show();
    }
    if (gameDemo) snakeDemo();
  }

  if (pizdetc) {
    pizdetc = false;

    for (byte bright = 0; bright < 15; bright++) {
      FastLED.setBrightness(bright);
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Red;
      }
      FastLED.show();
      delay(10);
    }
    delay(100);
    FastLED.clear();
    FastLED.show();
    FastLED.setBrightness(globalBrightness);
    displayScore(snakeLength - START_LENGTH);
    delay(1000);
    FastLED.clear();
    FastLED.show();
    newGameSnake();                        
  }
}

void putApple() {
  while (!apple_flag) {                       
    appleX = random(0, WIDTH);                 
    appleY = random(0, HEIGHT);

    if ((long)getPixColorXY(appleX, appleY) == 0) {
      apple_flag = true;                       
      drawPixelXY(appleX, appleY, GLOBAL_COLOR_2);        
      FastLED.show();
      delay(10);
    }
  }  
}

void snakeDemo() {
  // смещение головы змеи
  int8_t nextX = headX + vectorX;
  int8_t nextY = headY + vectorY;

  if (headX == appleX) {                
    if (headY < appleY) buttons = 0;
    if (headY > appleY) buttons = 2;
  }
  if (headY == appleY) {               
    if (headX < appleX) buttons = 1;
    if (headX > appleX) buttons = 3;
  }

  if (getPixColorXY(nextX, nextY) == GLOBAL_COLOR_1) {   
    // поворачиваем налево
    if (vectorX > 0) buttons = 0;
    if (vectorX < 0) buttons = 2;
    if (vectorY > 0) buttons = 3;
    if (vectorY < 0) buttons = 1;
    return;
  }

  if (nextX < 0 || nextX > WIDTH - 1 || nextY < 0       
      || nextY > HEIGHT - 1) {

    
    if (vectorX > 0) buttons = 2;
    if (vectorX > 0 && headY == 0) buttons = 0;

    if (vectorX < 0 && headY == HEIGHT - 1) buttons = 2;
    if (vectorX < 0) buttons = 0;

    if (vectorY > 0) buttons = 1;
    if (vectorY > 0 && headX == WIDTH - 1) buttons = 3;

    if (vectorY < 0 && headX == 0) buttons = 1;
    if (vectorY < 0) buttons = 3;
  }
}

void buttonsTickSnake() {
  if (checkButtons()) {
    if (buttons == 3) {  
      vectorX = -1;
      vectorY = 0;
      buttons = 4;
    }
    if (buttons == 1) {   
      vectorX = 1;
      vectorY = 0;
      buttons = 4;
    }
    if (buttons == 0) {   
      vectorY = 1;
      vectorX = 0;
      buttons = 4;
    }
    if (buttons == 2) {   
      vectorY = -1;
      vectorX = 0;
      buttons = 4;
    }
  }
}

void newGameSnake() {
  FastLED.clear();
  randomSeed(millis());

  snakeLength = START_LENGTH;
  headX = WIDTH / 2;
  headY = HEIGHT / 2;
  buttY = headY;

  vectorX = 1; 
  vectorY = 0;
  buttons = 4;

  for (byte i = 0; i < snakeLength; i++) {
    drawPixelXY(headX - i, headY, GLOBAL_COLOR_1);
    buttVector[i] = 0;
  }
  FastLED.show();
  buttX = headX - snakeLength;   
  missDelete = false;
  apple_flag = false;
}

#elif (USE_SNAKE == 0)
void snakeRoutine() {
  return;
}
#endif
