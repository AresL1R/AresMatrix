#if (USE_TETRIS == 1)
#define FAST_SPEED 20     
#define STEER_SPEED 40    

// --------------------- pro vývoj ----------------------
#define ADD_COLOR 0x010101

int8_t fig = 0, ang = 0, pos = WIDTH / 2, height = HEIGHT - 1;
int8_t prev_ang, prev_pos, prev_height;

uint32_t colors[6] {0x0000EE, 0xEE0000, 0x00EE00, 0x00EEEE, 0xEE00EE, 0xEEEE00};
uint32_t color = 0x000088;
byte color_index;
byte linesToClear;
boolean down_flag = true;
byte lineCleanCounter;


const int8_t figures[7][12][2] PROGMEM = {
  {
    { -1, 0}, {1, 0}, {2, 0},
    {0, 1}, {0, 2}, {0, 3},
    { -1, 0}, {1, 0}, {2, 0},
    {0, 1}, {0, 2}, {0, 3},
  },
  {
    {0, 1}, {1, 0}, {1, 1},
    {0, 1}, {1, 0}, {1, 1},
    {0, 1}, {1, 0}, {1, 1},
    {0, 1}, {1, 0}, {1, 1},
  },
  {
    { -1, 0}, { -1, 1}, {1, 0},
    {0, 1}, {0, 2}, {1, 2},
    { -2, 1}, { -1, 1}, {0, 1},
    { -1, 0}, {0, 1}, {0, 2},
  },
  {
    { -1, 0}, {1, 0}, {1, 1},
    {0, 1}, {0, 2}, {1, 0},
    {0, 1}, {1, 1}, {2, 1},
    {0, 1}, {0, 2}, { -1, 2},
  },
  {
    { -1, 0}, {0, 1}, {1, 1},
    {0, 1}, { -1, 1}, { -1, 2},
    { -1, 0}, {0, 1}, {1, 1},
    {0, 1}, { -1, 1}, { -1, 2},
  },
  {
    { -1, 1}, {0, 1}, {1, 0},
    {0, 1}, {1, 1}, {1, 2},
    { -1, 1}, {0, 1}, {1, 0},
    {0, 1}, {1, 1}, {1, 2},
  },
  {
    { -1, 0}, {0, 1}, {1, 0},
    {0, 1}, {0, 2}, {1, 1},
    { -1, 1}, {0, 1}, {1, 1},
    { -1, 1}, {0, 1}, {0, 2},
  }
};

void tetrisRoutine() {
  if (loadingFlag) {
    FastLED.clear();
    loadingFlag = false;
    newGameTetris();
    gamemodeFlag = true;
    modeCode = MC_GAME;
  }

  if (checkButtons()) {

    if (buttons == 3) {   
      buttons = 4;
      stepLeft();
    }

    if (buttons == 1) {
      buttons = 4;
      stepRight();
    }

    if (buttons == 0) {
      buttons = 4;
      if (checkArea(3)) {      
        prev_ang = ang;         
        ang = ++ang % 4;        
        redrawFigure(prev_ang, pos, height);    
      }
    }

    if (buttons == 2) {             
      buttons = 4;
      gameTimer.setInterval(FAST_SPEED); ´
    }
  }

 

  if (gameTimer.isReady()) {        
    prev_height = height;

    if (!checkArea(0)) {            
      if (height >= HEIGHT - 2) {   
        gameOverTetris();                 
        newGameTetris();                
      } else {                      
        fixFigure();                
        checkAndClear();            
        newGameTetris();                 
      }
    } else if (height == 0) {       
      fixFigure();                  
      checkAndClear();             
      newGameTetris();                   
    } else {                        
      height--;                          
      redrawFigure(ang, pos, prev_height); 
    }
  }
}

void checkAndClear() {
  linesToClear = 1;                 
  boolean full_flag = true;     
  while (linesToClear != 0) {       
    linesToClear = 0;
    byte lineNum = 255;      
    for (byte Y = 0; Y < HEIGHT; Y++) {  
      full_flag = true;                   
      for (byte X = 0; X < WIDTH; X++) {  
        if ((long)getPixColorXY(X, Y) == (long)0x000000) {  
          full_flag = false;                                 
        }
      }
      if (full_flag) {        
        linesToClear++;       
        if (lineNum == 255)   
          lineNum = Y;        
      } else {                
        if (lineNum != 255)   
          break;            
      }
    }
    if (linesToClear > 0) {            
      lineCleanCounter += linesToClear;   

      
      for (byte X = 0; X < WIDTH; X++) {
        for (byte i = 0; i < linesToClear; i++) {
          leds[getPixelNumber(X, lineNum + i)] = CHSV(0, 0, 255);         
        }
        FastLED.show();
        delay(5);    
      }
      delay(10);

     
      for (byte val = 0; val <= 30; val++) {
        for (byte X = 0; X < WIDTH; X++) {
          for (byte i = 0; i < linesToClear; i++) {
            leds[getPixelNumber(X, lineNum + i)] = CHSV(0, 0, 240 - 8 * val);  
          }
        }
        FastLED.show();
        delay(5);       
      }
      delay(10);

      for (byte i = 0; i < linesToClear; i++) {
        for (byte Y = lineNum; Y < HEIGHT - 1; Y++) {
          for (byte X = 0; X < WIDTH; X++) {
            drawPixelXY(X, Y, getPixColorXY(X, Y + 1));      
          }
          FastLED.show();
        }
        delay(100);       
      }
    }
  }
  gameTimer.reset();
}


void fixFigure() {
  color += ADD_COLOR;                   
  redrawFigure(ang, pos, prev_height);  
}

void gameOverTetris() {
  FastLED.clear();
  FastLED.show();

  
  if (!gameDemo) displayScore(lineCleanCounter);
  delay(1000);
  lineCleanCounter = 0;  
  FastLED.clear();
  FastLED.show();
  delay(20);
}


void newGameTetris() {
  delay(10);
  buttons = 4;
  height = HEIGHT;    
  pos = WIDTH / 2;    
  fig = random(7);    
  ang = random(4);   
  //color = colors[random(6)];     

  color_index = ++color_index % 6; 
  color = colors[color_index];

  
  if (gameDemo) pos = random(1, WIDTH - 1);


  gameTimer.setInterval(gameSpeed);
  down_flag = false;  
  delay(10);
}


void stepRight() {
  if (checkArea(1)) {
    prev_pos = pos;
    if (++pos > WIDTH) pos = WIDTH;
    redrawFigure(ang, prev_pos, height);
  }
}
void stepLeft() {
  if (checkArea(2)) {
    prev_pos = pos;
    if (--pos < 0) pos = 0;
    redrawFigure(ang, prev_pos, height);
  }
}

boolean checkArea(int8_t check_type) {
  

  boolean flag = true;
  int8_t X, Y;
  boolean offset = 1;
  int8_t this_ang = ang;

  
  if (check_type == 3) {
    this_ang = ++this_ang % 4;
    offset = 0;   
  }

  for (byte i = 0; i < 4; i++) {
    
    if (i == 0) {   
      X = pos;
      Y = height;
    } else {        
      X = pos + (int8_t)pgm_read_byte(&figures[fig][this_ang * 3 + i - 1][0]);
      Y = height + (int8_t)pgm_read_byte(&figures[fig][this_ang * 3 + i - 1][1]);

      
    }

    if (check_type == 1 || check_type == 3) {
      if (X + 1 > WIDTH - 1) flag = false;    
      uint32_t getColor = 0;
      if (Y < HEIGHT)
        getColor = getPixColorXY(X + offset, Y);
      if ((getColor != color) && (getColor != 0x000000)) {
        flag = false;        
      }
    }
    
    if (check_type == 2 || check_type == 3) {
      if (X - 1 < 0) flag = false;    
      uint32_t getColor = 0;
      if (Y < HEIGHT)
        getColor = getPixColorXY(X - offset, Y);
      if ((getColor != color) && (getColor != 0x000000)) {
        flag = false;         
      }
    }

    if (check_type == 0 || check_type == 3) {
      uint32_t getColor = 0;
      if (Y < HEIGHT) {
        getColor = getPixColorXY(X, Y - 1);
        if ((getColor != color) && (getColor != 0x000000)) {
          flag = false;         
        }
      }
    }
  }
  return flag;    
}


void redrawFigure(int8_t clr_ang, int8_t clr_pos, int8_t clr_height) {
  drawFigure(fig, clr_ang, clr_pos, clr_height, 0x000000);            
  drawFigure(fig, ang, pos, height, color);                           
  FastLED.show();
}

void drawFigure(byte figure, byte angle, byte x, byte y, uint32_t color) {
  drawPixelXY(x, y, color);         
  int8_t X, Y;                      
  for (byte i = 0; i < 3; i++) {    
    

    X = x + (int8_t)pgm_read_byte(&figures[figure][angle * 3 + i][0]);
    Y = y + (int8_t)pgm_read_byte(&figures[figure][angle * 3 + i][1]);
    if (Y > HEIGHT - 1) continue;   
    drawPixelXY(X, Y, color);
  }
}

#elif (USE_TETRIS == 0)
void tetrisRoutine() {
  return;
}
#endif
