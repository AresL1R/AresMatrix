
#define MIRR_V 0         
#define MIRR_H 0          

#define TEXT_HEIGHT 0     
#define LET_WIDTH 5      
#define LET_HEIGHT 8     
#define SPACE 1           ´


int offset = WIDTH;

void fillString(String text, uint32_t color) {
  if (loadingFlag) {
    offset = WIDTH;   
    loadingFlag = false;    
#if (SMOOTH_CHANGE == 1)
    loadingFlag = modeCode == MC_TEXT && fadeMode < 2 ;
#else
    loadingFlag = false;        
#endif
    modeCode = MC_TEXT;
    fullTextFlag = false;
  }
  
  if (scrollTimer.isReady()) {
    FastLED.clear();
    byte i = 0, j = 0;
    while (text[i] != '\0') {
      if ((byte)text[i] > 191) {    
        i++;
      } else {
        drawLetter(j, text[i], offset + j * (LET_WIDTH + SPACE), color);
        i++;
        j++;
      }
    }
    fullTextFlag = false;

    offset--;
    if (offset < -j * (LET_WIDTH + SPACE)) {   
      offset = WIDTH + 3;
      fullTextFlag = true;
    }
            
    FastLED.show();
  }
}

void drawLetter(uint8_t index, uint8_t letter, int16_t offset, uint32_t color) {
  int8_t start_pos = 0, finish_pos = LET_WIDTH;
  int8_t LH = LET_HEIGHT;
  if (LH > HEIGHT) LH = HEIGHT;
  int8_t offset_y = (HEIGHT - LH) / 2;    
  
  CRGB letterColor;
  if (color == 1) letterColor = CHSV(byte(offset * 10), 255, 255);
  else if (color == 2) letterColor = CHSV(byte(index * 30), 255, 255);
  else letterColor = color;

  if (offset < -LET_WIDTH || offset > WIDTH) return;
  if (offset < 0) start_pos = -offset;
  if (offset > (WIDTH - LET_WIDTH)) finish_pos = WIDTH - offset;

  for (byte i = start_pos; i < finish_pos; i++) {
    int thisByte;
    if (MIRR_V) thisByte = getFont((byte)letter, LET_WIDTH - 1 - i);
    else thisByte = getFont((byte)letter, i);

    for (byte j = 0; j < LH; j++) {
      boolean thisBit;

      if (MIRR_H) thisBit = thisByte & (1 << j);
      else thisBit = thisByte & (1 << (LH - 1 - j));

      if (thisBit) leds[getPixelNumber(offset + i, offset_y + TEXT_HEIGHT + j)] = letterColor;
    }
  }
}


uint8_t getFont(uint8_t font, uint8_t row) {
  font = font - '0' + 16;  
  if (font <= 90) return pgm_read_byte(&(fontHEX[font][row]));     
  else if (font >= 112 && font <= 159) {    
    return pgm_read_byte(&(fontHEX[font - 17][row]));
  } else if (font >= 96 && font <= 111) {
    return pgm_read_byte(&(fontHEX[font + 47][row]));
  }
  return 0;
}
