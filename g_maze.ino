//hra labirynt!
#if (USE_MAZE == 1)
// ***************** nastavení generace *****************
#define GAMEMODE 0       
#define FOV 3             

#define MAZE_WIDTH 17     
#define MAZE_HEIGHT 17    
#define SHIFT 1           

#define MAX_STRAIGHT 7  
#define HOLES 2           
#define MIN_PATH 20       

// ---------------------logicka čast ----------------------
const uint16_t maxSolves = MAZE_WIDTH * MAZE_WIDTH * 5;
char *maze = (char*)malloc(MAZE_WIDTH * MAZE_HEIGHT * sizeof(char));
int8_t playerPos[2];
uint32_t labTimer;
boolean mazeMode = false;

void newGameMaze() {
  playerPos[0] = !SHIFT;
  playerPos[1] = !SHIFT;

  buttons = 4;
  FastLED.clear();

  GenerateMaze(maze, MAZE_WIDTH, MAZE_HEIGHT);    
  SolveMaze(maze, MAZE_WIDTH, MAZE_HEIGHT);       

  if (!(GAMEMODE || mazeMode)) {
    for (byte y = 0; y < HEIGHT; y++) {
      for (byte x = 0; x < WIDTH; x++) {
        switch (maze[(y + SHIFT) * MAZE_WIDTH + (x + SHIFT)]) {
          case 1:  drawPixelXY(x, y, GLOBAL_COLOR_1); break;
          case 2:
            // drawPixelXY(x, y, GLOBAL_COLOR_1);    
            break;
          default: drawPixelXY(x, y, 0x000000); break;
        }
      }
      FastLED.show();
      delay(25);
    }
  } else {
    for (byte y = 0; y < FOV; y++) {
      for (byte x = 0; x < FOV; x++) {
        switch (maze[(y + SHIFT) * MAZE_WIDTH + (x + SHIFT)]) {
          case 1:  drawPixelXY(x, y, GLOBAL_COLOR_1);  break;
          case 2:
            // drawPixelXY(x, y, GLOBAL_COLOR_1);    
            break;
          default: drawPixelXY(x, y, 0x000000);  break;
        }
      }
    }
  }
  drawPixelXY(playerPos[0], playerPos[1], GLOBAL_COLOR_2);

  FastLED.show();
  delay(25);

  labTimer = millis();
}

void mazeRoutine() {
  if (loadingFlag) {
    FastLED.clear();
    loadingFlag = false;
    newGameMaze();
    gamemodeFlag = true;
    modeCode = MC_GAME;
  }

  if (gameDemo && !gamePaused) demoMaze();
  buttonsTickMaze();
}

void buttonsTickMaze() {
  if (checkButtons()) {
    if (buttons == 3) {   // кнопка нажата
      int8_t newPos = playerPos[0] - 1;
      if (newPos >= 0 && newPos <= WIDTH - 1)
        if (getPixColorXY(newPos, playerPos[1]) == 0) {
          movePlayer(newPos, playerPos[1], playerPos[0], playerPos[1]);
          playerPos[0] = newPos;
        }
      buttons = 4;
    }
    if (buttons == 1) {   // кнопка нажата
      int8_t newPos = playerPos[0] + 1;
      if (newPos >= 0 && newPos <= WIDTH - 1)
        if (getPixColorXY(newPos, playerPos[1]) == 0) {
          movePlayer(newPos, playerPos[1], playerPos[0], playerPos[1]);
          playerPos[0] = newPos;
        }
      buttons = 4;
    }
    if (buttons == 0) {   // кнопка нажата
      int8_t newPos = playerPos[1] + 1;
      if (newPos >= 0 && newPos <= HEIGHT - 1)
        if (getPixColorXY(playerPos[0], newPos) == 0) {
          movePlayer(playerPos[0], newPos, playerPos[0], playerPos[1]);
          playerPos[1] = newPos;
        }
      buttons = 4;
    }
    if (buttons == 2) {   // кнопка нажата
      int8_t newPos = playerPos[1] - 1;
      if (newPos >= 0 && newPos <= HEIGHT - 1)
        if (getPixColorXY(playerPos[0], newPos) == 0) {
          movePlayer(playerPos[0], newPos, playerPos[0], playerPos[1]);
          playerPos[1] = newPos;
        }
      buttons = 4;
    }
  }
}

void movePlayer(int8_t nowX, int8_t nowY, int8_t prevX, int8_t prevY) {
  drawPixelXY(prevX, prevY, 0x000000);
  drawPixelXY(nowX, nowY, GLOBAL_COLOR_2);
  FastLED.show();
  if ((nowX == (MAZE_WIDTH - 2) - SHIFT) && (nowY == (MAZE_HEIGHT - 1) - SHIFT)) {
    loadingFlag = true;
    FastLED.clear();
    FastLED.show();
    if (!gameDemo) {
      displayScore((millis() - labTimer) / 1000);
      delay(1000);
    }
    return;
  }
  if (GAMEMODE || mazeMode) {
    for (int8_t y = nowY - FOV; y < nowY + FOV; y++) {
      for (int8_t x = nowX - FOV; x < nowX + FOV; x++) {
        if (x < 0 || x > WIDTH - 1 || y < 0 || y > WIDTH - 1) continue;
        if (maze[(y + SHIFT) * MAZE_WIDTH + (x + SHIFT)] == 1) {
          drawPixelXY(x, y, GLOBAL_COLOR_1);
        }
      }
      FastLED.show();
    }
  }
}

void demoMaze() {
  if (gameTimer.isReady()) {
    if (checkPath(0, 1)) buttons = 0;
    if (checkPath(1, 0)) buttons = 1;
    if (checkPath(0, -1)) buttons = 2;
    if (checkPath(-1, 0)) buttons = 3;
  }
}

boolean checkPath(int8_t x, int8_t y) {
  if ( (maze[(playerPos[1] + y + SHIFT) * MAZE_WIDTH + (playerPos[0] + x + SHIFT)]) == 2) {
    maze[(playerPos[1] + SHIFT) * MAZE_WIDTH + (playerPos[0] + SHIFT)] = 4;  
    return true;
  }
  else return false;
}


void smartMaze() {
  byte sum = 0, line;
  int attempt;
  while (sum < MIN_PATH) {                 
    attempt++;
    //randomSeed(millis());                 
    GenerateMaze(maze, MAZE_WIDTH, MAZE_HEIGHT);    
    SolveMaze(maze, MAZE_WIDTH, MAZE_HEIGHT);      

    sum = 0;
    for (byte y = 0; y < MAZE_HEIGHT; y++) {      
      for (byte x = 0; x < MAZE_WIDTH; x++) {
        if (maze[y * MAZE_WIDTH + x] == 2) sum++;   
      }
    }
    if (MAX_STRAIGHT > 0) {
      line = 0;
      for (byte y = 0; y < MAZE_HEIGHT; y++)
        if (maze[y * MAZE_WIDTH + MAZE_WIDTH - 2] == 2) line++;
      if (line > MAX_STRAIGHT) sum = 0;

      line = 0;
      for (byte x = 0; x < MAZE_WIDTH; x++)
        if (maze[MAZE_WIDTH + x] == 2) line++;
      if (line > MAX_STRAIGHT) sum = 0;
    }
  }
}

void makeHoles() {
  byte holes = 0;
  byte attempt = 0;
  while (holes < HOLES) {                           
    attempt++;                                     
    if (attempt > 200) break;                       

    byte x = random(1, MAZE_WIDTH - 1);
    byte y = random(1, MAZE_HEIGHT - 1);

    if (maze[y * MAZE_WIDTH + x] == 1                   
        && maze[y * MAZE_WIDTH + (x - 1)] == 1          
        && maze[y * MAZE_WIDTH + (x + 1)] == 1         
        && maze[(y - 1) * MAZE_WIDTH + x] != 1          
        && maze[(y + 1) * MAZE_WIDTH + x] != 1) {      
      maze[y * MAZE_WIDTH + x] = 0;                    
      holes++;                                      
    } else if (maze[y * MAZE_WIDTH + x] == 1           
               && maze[(y - 1) * MAZE_WIDTH + x] == 1  
               && maze[(y + 1) * MAZE_WIDTH + x] == 1  
               && maze[y * MAZE_WIDTH + (x - 1)] != 1  
               && maze[y * MAZE_WIDTH + (x + 1)] != 1) {
      maze[y * MAZE_WIDTH + x] = 0;                     
      holes++;                                    
    }
  }
}


void CarveMaze(char *maze, int width, int height, int x, int y) {
  int x1, y1;
  int x2, y2;
  int dx, dy;
  int dir, count;

  dir = random(10) % 4;
  count = 0;
  while (count < 4) {
    dx = 0; dy = 0;
    switch (dir) {
      case 0:  dx = 1;  break;
      case 1:  dy = 1;  break;
      case 2:  dx = -1; break;
      default: dy = -1; break;
    }
    x1 = x + dx;
    y1 = y + dy;
    x2 = x1 + dx;
    y2 = y1 + dy;
    if (   x2 > 0 && x2 < width && y2 > 0 && y2 < height
           && maze[y1 * width + x1] == 1 && maze[y2 * width + x2] == 1) {
      maze[y1 * width + x1] = 0;
      maze[y2 * width + x2] = 0;
      x = x2; y = y2;
      dir = random(10) % 4;
      count = 0;
    } else {
      dir = (dir + 1) % 4;
      count += 1;
    }
  }
}

// генерацтор лабиринта
void GenerateMaze(char *maze, int width, int height) {
  int x, y;
  for (x = 0; x < width * height; x++) {
    maze[x] = 1;
  }
  maze[1 * width + 1] = 0;
  for (y = 1; y < height; y += 2) {
    for (x = 1; x < width; x += 2) {
      CarveMaze(maze, width, height, x, y);
    }
  }
  // вход и выход
  maze[0 * width + 1] = 0;
  maze[(height - 1) * width + (width - 2)] = 0;
}

// решатель (ищет путь)
void SolveMaze(char *maze, int width, int height) {
  int dir, count;
  int x, y;
  int dx, dy;
  int forward;
  /* Remove the entry and exit. */
  maze[0 * width + 1] = 1;
  maze[(height - 1) * width + (width - 2)] = 1;

  forward = 1;
  dir = 0;
  count = 0;
  x = 1;
  y = 1;
  unsigned int attempts = 0;
  while (x != width - 2 || y != height - 2) {
    if (attempts++ > maxSolves) {   
      break;                        
    }
    dx = 0; dy = 0;
    switch (dir) {
      case 0:  dx = 1;  break;
      case 1:  dy = 1;  break;
      case 2:  dx = -1; break;
      default: dy = -1; break;
    }
    if (   (forward  && maze[(y + dy) * width + (x + dx)] == 0)
           || (!forward && maze[(y + dy) * width + (x + dx)] == 2)) {
      maze[y * width + x] = forward ? 2 : 3;
      x += dx;
      y += dy;
      forward = 1;
      count = 0;
      dir = 0;
    } else {
      dir = (dir + 1) % 4;
      count += 1;
      if (count > 3) {
        forward = 0;
        count = 0;
      }
    }
  }
  /* Replace the entry and exit. */
  maze[(height - 2) * width + (width - 2)] = 2;
  maze[(height - 1) * width + (width - 2)] = 2;
}

#elif (USE_MAZE == 0)
void mazeRoutine() {
  return;
}
#endif
