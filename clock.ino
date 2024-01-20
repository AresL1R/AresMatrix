// režim hodin

// ****************** Nastavení hodin *****************
#define MIN_COLOR CRGB::White          // barva min
#define HOUR_COLOR CRGB::White         // barva hod
#define DOT_COLOR CRGB::White          // barva pixelu

#define NORMAL_CLOCK_COLOR CRGB::White // Normální barva monochromatických barev

#define CONTRAST_COLOR_1 CRGB::Orange  // kontrastní barva hodin
#define CONTRAST_COLOR_2 CRGB::Green  // kontrastní barva hodin
#define CONTRAST_COLOR_3 CRGB::Black   // kontrastní barva hodin

#define HUE_STEP 5          // krok změny barvy při duze
#define HUE_GAP 30          // krok barvy při změně

// ****************** Pro budoucí vývoj bez poznatku sem nešahat ****************
bool saveSpecialMode;
bool saveRunningFlag;
int8_t saveSpecialModeId;
byte saveThisMode;

byte clockHue;

#if (OVERLAY_CLOCK == 1)
CRGB overlayLEDs[165];                // Maximální kalendář - písmo 3x5 - 4 číslice ve dvou řádcích, jedna mezera mezi čísly a řádky - 15x11
                                       // Při maximálním počtu hodin je písmo 3x5 horizont - 4 číslice, jedna mezera mezi číslicemi - 15x5,
                                       // vert - dva řádky se dvěma číslicemi, jedna mezera mezi čísly a řádky - 7x11,
byte listSize = sizeof(overlayList);
#endif

CRGB clockLED[5] = {HOUR_COLOR, HOUR_COLOR, DOT_COLOR, MIN_COLOR, MIN_COLOR};

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
  Serial.print(F("Отправка NTP пакета на сервер "));
  Serial.println(ntpServerName);
  // set all bytes in the buffer to 0
  // memset(packetBuffer, 0, NTP_PACKET_SIZE);
  for (byte i=0; i<NTP_PACKET_SIZE; i++) packetBuffer[i] = 0;
  
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); // NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void parseNTP() {
    Serial.println(F("Rozebiraní packetu NTP"));
    ntp_t = 0; ntp_cnt = 0; init_time = true; refresh_time = false;
    unsigned long highWord = word(incomeBuffer[40], incomeBuffer[41]);
    unsigned long lowWord = word(incomeBuffer[42], incomeBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    unsigned long seventyYears = 2208988800UL ;
    time_t t = secsSince1900 - seventyYears + (timeZoneOffset) * 3600;
    Serial.print(F("Секунд с 1970: "));
    Serial.println(t);
    setTime(t);
    calculateDawnTime();
}

void getNTP() {
  if (!wifi_connected) return;
  WiFi.hostByName(ntpServerName, timeServerIP);
  IPAddress ip;
  ip.fromString(F("0.0.0.0"));
#if defined(ESP8266)
  if (!timeServerIP.isSet()) timeServerIP.fromString(F("192.36.143.130"));  // jeden z ru.pool.ntp.org // 195.3.254.2
#endif
#if defined(ESP32)
  if (timeServerIP==ip) timeServerIP.fromString(F("192.36.143.130"));  // jeden z ru.pool.ntp.org // 195.3.254.2
#endif
  printNtpServerName();
  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  ntp_t = millis();  
}

boolean overlayAllowed() {
#if (OVERLAY_CLOCK == 1)  

  // Překrytí není povoleno, pokud hodiny ještě nebyly inicializovány
  if (!init_time) return false;

  // V překryvných hrách nejsou hodiny;
  if (gamemodeFlag) return false;

  // Interferují hodiny s maticí?
  if (!(allowHorizontal || allowVertical)) return false;
  
  // Překrytí je povoleno aktuálními parametry zvláštního režimu?
  if (specialMode) return specialClock;

  // Překrytí povoleno obecným nastavením hodin?
  bool allowed = getClockOverlayEnabled();
  
  // Překrytí povoleno nastavením seznamu efektů povolených pro překrytí?
  if (allowed) {
    for (byte i = 0; i < listSize; i++) {
      allowed = (modeCode == overlayList[i]);
      if (allowed) break;
    }
  }

 // Překrytí povoleno nastavením parametrů efektu? (u efektu „Hodiny“ není povoleno překrývání hodin)
  if (allowed && BTcontrol && effectsFlag && effect != MC_CLOCK) {
    allowed = getEffectClock(effect);   
  }
    
// Pokud je v automatickém režimu - najděte shodu zobrazeného čísla režimu s číslem efektu a zkontrolujte, zda je povoleno překrytí hodin
  if (allowed && !BTcontrol) {
    byte tmp_effect = mapModeToEffect(thisMode);
    if (tmp_effect <= MAX_EFFECT) {
      allowed = getEffectClock(tmp_effect);   
    } else {
      allowed = false;  
    }
  }
  
  return allowed;
#else
  return false;
#endif
}

String clockCurrentText() {
  
  hrs = hour();
  mins = minute();

  String sHrs = "0" + String(hrs);  
  String sMin = "0" + String(mins);
  if (sHrs.length() > 2) sHrs = sHrs.substring(1);
  if (sMin.length() > 2) sMin = sMin.substring(1);
  return sHrs + ":" + sMin;
}

String dateCurrentTextShort() {
  
  aday = day();
  amnth = month();
  ayear = year();

  String sDay = "0" + String(aday);  
  String sMnth = "0" + String(amnth);
  String sYear = String(ayear);
  if (sDay.length() > 2) sDay = sDay.substring(1);
  if (sMnth.length() > 2) sMnth = sMnth.substring(1);
  return sDay + "." + sMnth + "." + sYear;
}

String dateCurrentTextLong() {
  
  aday = day();
  amnth = month();
  ayear = year();

  String sDay = "0" + String(aday);  
  String sMnth = "";
  String sYear = String(ayear);
  switch (amnth) {
    case  1: sMnth = F(" leden ");   break;
    case  2: sMnth = F(" unor ");  break;
    case  3: sMnth = F(" květen ");    break;
    case  4: sMnth = F(" duben ");   break;
    case  5: sMnth = F(" červen ");      break;
    case  6: sMnth = F(" červenec ");     break;
    case  7: sMnth = F(" srpen ");     break;
    case  8: sMnth = F(" září ");  break;
    case  9: sMnth = F(" listopad "); break;
    case 10: sMnth = F(" prosinec ");  break;
    case 11: sMnth = F("  ");   break;
    case 12: sMnth = F("  ");  break;
  }  
  if (sDay.length() > 2) sDay = sDay.substring(1);
  return sDay + sMnth + sYear + " roky";
}

void clockColor() {
  if (COLOR_MODE == 0) {     
    // Монохромные часы  
    for (byte i = 0; i < 5; i++) clockLED[i] = NORMAL_CLOCK_COLOR;
  } else if (COLOR_MODE == 1) {
    // Každá číslice má svou vlastní barvu, hladké barvy
    for (byte i = 0; i < 5; i++) clockLED[i] = CHSV(clockHue + HUE_GAP * i, 255, 255);
    clockLED[2] = CHSV(clockHue + 128 + HUE_GAP * 1, 255, 255); // body dělame jinou barvou
  } else if (COLOR_MODE == 2) {
    // Hodiny, tečky, minuty s jeho barvou, hladká barva smkna
    clockLED[0] = CHSV(clockHue + HUE_GAP * 0, 255, 255);
    clockLED[1] = CHSV(clockHue + HUE_GAP * 0, 255, 255);
    clockLED[2] = CHSV(clockHue + 128 + HUE_GAP * 1, 255, 255); // body vytvářejí jinou barvu
    clockLED[3] = CHSV(clockHue + HUE_GAP * 2, 255, 255);
    clockLED[4] = CHSV(clockHue + HUE_GAP * 2, 255, 255);
  } else if (COLOR_MODE == 3) {// pro kalendář -
     // Hodiny, body, minuty s určenou barvou
    clockLED [0] = HOUR_COLOR; // číslo
     clockLED [1] = HOUR_COLOR; // Měsíc
     clockLED [2] = DOT_COLOR; // oddělovač data / měsíce
     clockLED [3] = MIN_COLOR; // rok první dvě číslice
     clockLED [4] = MIN_COLOR; // rok 2. dvě číslice
  }
}

// nakreslit hodiny
void drawClock(byte hrs, byte mins, boolean dots, byte X, byte Y) {
  byte h10 = hrs / 10;
  byte h01 = hrs % 10;
  byte m10 = mins / 10;
  byte m01 = mins % 10;
  
  if (CLOCK_ORIENT == 0) {
    if (h10 == 1 && m01 != 1 && X > 0) X--;
    // 0 v hodinách Nezobrazujeme pro centrování zbývající číslice doleva na nulu
    if (h10 > 0) 
      drawDigit3x5(h10, X + (h10 == 1 ? 1 : 0), Y, clockLED[0]); // font 3x5, ve kterém 1 - ve středu známosti - posune doprava o 1 sloupec
    else 
      X -= 2;
    drawDigit3x5(h01, X + 4, Y, clockLED[1]);
    if (dots) {
      drawPixelXY(X + 7, Y + 1, clockLED[2]);
      drawPixelXY(X + 7, Y + 3, clockLED[2]);
    } else {
      if (modeCode == MC_CLOCK) {
        drawPixelXY(X + 7, Y + 1, 0);
        drawPixelXY(X + 7, Y + 3, 0);
      }
    }
    drawDigit3x5(m10, X + 8, Y, clockLED[3]);
    drawDigit3x5(m01, X + 12 + (m01 == 1 ? -1 : 0) + (m10 == 1 && m01 != 1 ? -1 : 0) , Y, clockLED[4]);// font 3x5, ve kterém 1 - ve středu známosti - posun doleva o 1 sloupec
  } else { // Vertikální hodiny
   // if (hrs> 9) // Takže opravdu krásné
    drawDigit3x5(h10, X, Y + 6, clockLED[0]);
    drawDigit3x5(h01, X + 4, Y + 6, clockLED[1]);
    if (dots) { // Blikající tečky jsou snadno spojeny s hodinami
      drawPixelXY(X + 3, Y + 5, clockLED[2]);
    } else {
      if (modeCode == MC_CLOCK) {
        drawPixelXY(X + 3, Y + 5, 0);
      }
    }
    drawDigit3x5(m10, X, Y, clockLED[3]);
    drawDigit3x5(m01, X + 4, Y, clockLED[4]);
  }
}

// нарисовать дату календаря
void drawCalendar(byte aday, byte amnth, int16_t ayear, boolean dots, byte X, byte Y) {

  // Число месяца
  drawDigit3x5(aday / 10, X, Y + 6, clockLED[0]); // font 3x5, ve kterém 1 - ve středu zkomísta - posune doprava o 1 sloupec
  drawDigit3x5(aday % 10, X + 4, Y + 6, clockLED[0]);

  // oddělovač data / měsíce
  if (dots) {
    drawPixelXY(X + 7, Y + 5, clockLED[2]);
  } else {
    if (modeCode == MC_CLOCK) {
      drawPixelXY(X + 7, Y + 5, 0);
    }
  }
  
  // Месяц
  drawDigit3x5(amnth / 10, X + 8, Y + 6, clockLED[1]); // font 3x5, ve kterém 1 - ve středu znákomista - posune doprava o 1 sloupec
  drawDigit3x5(amnth % 10, X + 12, Y + 6, clockLED[1]);

  // Год  
  drawDigit3x5(ayear / 1000, X, Y, clockLED[3]);
  drawDigit3x5((ayear / 100) % 10, X + 4, Y, clockLED[3]);
  drawDigit3x5((ayear / 10) % 10, X + 8, Y, clockLED[4]);
  drawDigit3x5(ayear % 10, X + 12, Y, clockLED[4]);
}

void clockRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    modeCode = MC_CLOCK;
  }

  FastLED.clear();
  clockTicker();

  if (!showDateInClock) {
    // ukazovat pouze hodiny
    drawClock(hrs, mins, dotFlag, CLOCK_X, CLOCK_Y);
  } else {
    // ukazovat hodiny a kalendar
    if (showDateState)
      drawCalendar(aday, amnth, ayear, dotFlag, CALENDAR_X, CALENDAR_Y);
    else  
      drawClock(hrs, mins, dotFlag, CLOCK_X, CLOCK_Y);
    
    if (millis() - showDateStateLastChange > (showDateState ? showDateDuration : showDateInterval) * 1000L) {
      showDateStateLastChange = millis();
      showDateState = !showDateState;
    }
  }
}

void calendarRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    modeCode = MC_CLOCK;
  }

  FastLED.clear();
  clockTicker();
  drawCalendar(aday, amnth, ayear, dotFlag, CALENDAR_X, CALENDAR_Y);
}

void clockTicker() {
  hrs = hour();
  mins = minute();
  aday = day();
  amnth = month();
  ayear = year();
  
  if (halfsecTimer.isReady()) {
    
    clockHue += HUE_STEP;

    #if (OVERLAY_CLOCK == 1)
      setOverlayColors();
    #endif

    dotFlag = !dotFlag;
  }
}

#if (OVERLAY_CLOCK == 1)
void clockOverlayWrapH(byte posX, byte posY) {
  byte thisLED = 0;
  for (byte i = posX; i < posX + 15; i++) {
    for (byte j = posY; j < posY + 5; j++) {
      overlayLEDs[thisLED] = leds[getPixelNumber(i, j)];
      thisLED++;
    }
  }
  clockTicker();
  if (init_time)
    drawClock(hrs, mins, dotFlag, posX, posY);
}

void clockOverlayUnwrapH(byte posX, byte posY) {
  byte thisLED = 0;
  for (byte i = posX; i < posX + 15; i++) {
    for (byte j = posY; j < posY + 5; j++) {
      leds[getPixelNumber(i, j)] = overlayLEDs[thisLED];
      thisLED++;
    }
  }
}

void clockOverlayWrapV(byte posX, byte posY) {
  byte thisLED = 0;
  for (byte i = posX; i < posX + 7; i++) {
    for (byte j = posY; j < posY + 11; j++) {
      overlayLEDs[thisLED] = leds[getPixelNumber(i, j)];
      thisLED++;
    }
  }
  clockTicker();
  if (init_time)
    drawClock(hrs, mins, dotFlag, posX, posY);
}

void clockOverlayUnwrapV(byte posX, byte posY) {
  byte thisLED = 0;
  for (byte i = posX; i < posX + 7; i++) {
    for (byte j = posY; j < posY + 11; j++) {
      leds[getPixelNumber(i, j)] = overlayLEDs[thisLED];
      thisLED++;
    }
  }
}

void calendarOverlayWrap(byte posX, byte posY) {
  byte thisLED = 0;
  for (byte i = posX; i < posX + 15; i++) {
    for (byte j = posY; j < posY + 11; j++) {
      overlayLEDs[thisLED] = leds[getPixelNumber(i, j)];
      thisLED++;
    }
  }
  clockTicker();
  if (init_time)
    drawCalendar(aday, amnth, ayear, dotFlag, CALENDAR_X, CALENDAR_Y);
}

void calendarOverlayUnwrap(byte posX, byte posY) {
  byte thisLED = 0;
  for (byte i = posX; i < posX + 15; i++) {
    for (byte j = posY; j < posY + 11; j++) {
      leds[getPixelNumber(i, j)] = overlayLEDs[thisLED];
      thisLED++;
    }
  }
}

boolean needUnwrap() {
  if (modeCode == MC_SNOW ||
      modeCode == MC_SPARKLES ||
      modeCode == MC_MATRIX ||
      modeCode == MC_STARFALL ||
      modeCode == MC_BALLS ||
      modeCode == MC_FIRE ||
      modeCode == MC_PAINTBALL ||
      modeCode == MC_SWIRL) return true;
  else return false;
}
#endif

void contrastClock() {  
  for (byte i = 0; i < 5; i++) clockLED[i] = NORMAL_CLOCK_COLOR;
}

void contrastClockA() {  
  for (byte i = 0; i < 5; i++) clockLED[i] = CONTRAST_COLOR_1;
}

void contrastClockB() {
  for (byte i = 0; i < 5; i++) clockLED[i] = CONTRAST_COLOR_2;
}

void contrastClockC(){
  if (specialMode && specialClock){
    CRGB color = -CRGB(globalColor);
    for (byte i = 0; i < 5; i++) clockLED[i] = color;  
  } else {  
    CRGB color = CONTRAST_COLOR_3;
    CRGB gc = CRGB(globalColor);
    if (color == gc) color = -color;
    for (byte i = 0; i < 5; i++) clockLED[i] = color;  
  }
}

void setOverlayColors() {
  if (COLOR_MODE == 0) {
    switch (modeCode) {
      case MC_CLOCK: 
      case MC_GAME: 
      case MC_SPARKLES:
      case MC_MATRIX:
      case MC_STARFALL:
      case MC_FIRE: 
      case MC_BALL:
      case MC_BALLS: 
      case MC_NOISE_RAINBOW:
      case MC_NOISE_RAINBOW_STRIP: 
      case MC_RAINBOW:
      case MC_RAINBOW_DIAG: 
      case MC_NOISE_PLASMA:
      case MC_LIGHTERS:
      case MC_PAINTBALL:
      case MC_SWIRL:
        contrastClock();
        break;
      case MC_SNOW:
      case MC_NOISE_ZEBRA: 
      case MC_NOISE_MADNESS:
      case MC_NOISE_CLOUD:
      case MC_NOISE_FOREST:
      case MC_NOISE_OCEAN: 
        contrastClockA();
        break;
      case MC_NOISE_LAVA:
        contrastClockB();
        break;
      case MC_DAWN_ALARM:
      case MC_FILL_COLOR:
        contrastClockC();
        break;
    }
  }
  else
    clockColor();
}

// расчет времени начала рассвета
void calculateDawnTime() {

  byte alrmHour;
  byte alrmMinute;
  
  dawnWeekDay = 0;
  if (!init_time || alarmWeekDay == 0) return;       // Inicializovaný čas? Je po dobu alespoň jednoho dne zapnutý alarm?

  int8_t alrmWeekDay = weekday()-1;                  // day of the week, Sunday is day 0   
  if (alrmWeekDay == 0) alrmWeekDay = 7;             // Sunday is day 7, Monday is day 1;

  byte h = hour();
  byte m = minute();
  byte w = weekday()-1;
  if (w == 0) w = 7;

  byte cnt = 0;
  while (cnt < 2) {
    cnt++;
    while ((alarmWeekDay & (1 << (alrmWeekDay - 1))) == 0) {
      alrmWeekDay++;
      if (alrmWeekDay == 8) alrmWeekDay = 1;
    }
      
    alrmHour = alarmHour[alrmWeekDay-1];
    alrmMinute = alarmMinute[alrmWeekDay-1];
  
    // "Сегодня" время будильника уже прошло? 
    if (alrmWeekDay == w && (h * 60L + m > alrmHour * 60L + alrmMinute)) {
      alrmWeekDay++;
      if (alrmWeekDay == 8) alrmWeekDay = cnt == 1 ? 1 : 7;
    }
  }

  // Serial.printf("Alarm: h:%d m:%d wd:%d\n", alrmHour, alrmMinute, alrmWeekDay);
  
  // počet času dawn
if (alrmMinute> dawnDuration) {// pokud jsou minuty v době alarmu delší než úsvit
     dawnHour = alrmHour; // hodina svítání se rovná hodině poplachu
     dawnMinute = alrmMinute - dawnDuration; // minuty svítání = minuty poplachu - pokračování. svítání
     dawnWeekDay = alrmWeekDay;
   } else {// pokud jsou minuty v budíku kratší než doba svítání
     dawnWeekDay = alrmWeekDay;
     dawnHour = alrmHour - 1; // znamená, že svítání bude o hodinu dříve
    if (dawnHour < 0) {
      dawnHour = 23;     
      dawnWeekDay--;
      if (dawnWeekDay == 0) dawnWeekDay = 7;                           
    }
    dawnMinute = 60 - (dawnDuration - alrmMinute);  // nachazime minutu dawn
    if (dawnMinute == 60) {
      dawnMinute=0; dawnHour++;
      if (dawnHour == 24) {
        dawnHour=0; dawnWeekDay++;
        if (dawnWeekDay == 8) dawnWeekDay = 1;
      }
    }
  }

  // Serial.printf("Dawn: h:%d m:%d wd:%d\n", dawnHour, dawnMinute, dawnWeekDay);

  Serial.print(String(F("Следующий рассвет в "))+String(dawnHour)+ F(":") + String(dawnMinute));
  switch(dawnWeekDay) {
    case 1: Serial.println(F(", понедельник")); break;
    case 2: Serial.println(F(", вторник")); break;
    case 3: Serial.println(F(", среда")); break;
    case 4: Serial.println(F(", четверг")); break;
    case 5: Serial.println(F(", пятница")); break;
    case 6: Serial.println(F(", суббота")); break;
    case 7: Serial.println(F(", воскресенье")); break;
    default: Serial.println(); break;
  }  
}

// Проверка времени срабатывания будильника
void checkAlarmTime() {
  
  // Будильник включен?
  if (init_time && dawnWeekDay > 0) {

    byte h = hour();
    byte m = minute();
    byte w = weekday()-1;
    if (w == 0) w = 7;

    // čas spouštění budiku dawn
    byte alrmWeekDay = dawnWeekDay;
    byte alrmHour = dawnHour;
    byte alrmMinute = dawnMinute + dawnDuration;
    if (alrmMinute >= 60) {
      alrmMinute = 60 - alrmMinute;
      alrmHour++;
    }
    if (alrmHour > 23) {
      alrmHour = 24 - alrmHour;
      alrmWeekDay++;
    }
    if (alrmWeekDay > 7) alrmWeekDay = 1;

   // Zhoduje se aktuální den v týdnu s vypočítaným dnem v týdnu svítání?
    if (w == dawnWeekDay) {
       // Začínají hodiny / minuty úsvitu? Dawn ještě neběží? Ještě nejste zastaveni uživatelem?
       if (!isAlarming && !isAlarmStopped && ((w * 1000L + h * 60L + m) >= (dawnWeekDay * 1000L + dawnHour * 60L + dawnMinute)) && ((w * 1000L + h * 60L + m) < (alrmWeekDay * 1000L + alrmHour * 60L + alrmMinute))) {
       // Uložte parametry aktuálního režimu pro zotavení po dokončení alarmu
         saveSpecialMode = specialMode;
         saveSpecialModeId = specialModeId;
         saveThisMode = thisMode;
         saveRunningFlag = runningFlag;
         // spustit budik
         isAlarming = true; 
         isAlarmStopped = false;
         setEffect(EFFECT_DAWN_ALARM);
         // realní delka budiku
         realDawnDuration = (alrmHour * 60L + alrmMinute) - (dawnHour * 60L + dawnMinute);
         if (realDawnDuration > dawnDuration) realDawnDuration = dawnDuration;
         // vypnout přechod do demo režimu
         idleTimer.setInterval(4294967295);
         #if (USE_MP3 ==1)
         if (useAlarmSound) PlayDawnSound();
         #endif
         sendPageParams(95);  // Parametry, stav IsAlarming (AL: 1) pro změnu zobrazení aktivity poplachu ve smartphonu
         Serial.println(String(F("Рассвет ВКЛ в "))+String(h)+ ":" + String(m));
       }
    }
    
    delay(0); // Chcete-li zabránit časovači Watchdog ESP8266
    
    // Když nastane čas budíku, pokud již není uživatelem vypnut - spusťte režim hodin a zvuk budíku
    if (alrmWeekDay == w && alrmHour == h && alrmMinute == m && isAlarming) {
      Serial.println(String(F("Dawn auto vyp "))+String(h)+ ":" + String(m));
      isAlarming = false;
      #if (USE_MP3 == 1)
      isAlarmStopped = false;
      if (isDfPlayerOk && useAlarmSound) {
        setSpecialMode(1);
        PlayAlarmSound();
      } else {
        isAlarmStopped = true;
        setModeByModeId(saveThisMode);
      }
      #else
      isAlarmStopped = true;
      #endif
      sendPageParams(95);  // Parametry, stav IsAlarming (AL: 1) pro změnu zobrazení aktivity poplachu ve smartphonu
    }

    delay(0); // Chcete-li zabránit časovači Watchdog ESP8266

   // Pokud uživatel zahájil a zastavil úsvit a čas začátku úsvitu již uplynul - resetujte příznaky a připravte je na další cyklus
    if (isAlarmStopped && ((w * 1000L + h * 60L + m) > (alrmWeekDay * 1000L + alrmHour * 60L + alrmMinute + alarmDuration))) {
      isAlarming = false;
      isAlarmStopped = false;
      StopSound(0);
    }
  }
  
  // Je čas vypnout budík - vypnout, zapnout demo režim
  if (alarmSoundTimer.isReady()) {
    alarmSoundTimer.setInterval(4294967295);
    StopSound(1500);   

    resetModes();

    BTcontrol = false;
    AUTOPLAY = true;

    if (saveSpecialMode){
       setSpecialMode(saveSpecialModeId);
    } else {       
       setModeByModeId(saveThisMode);
    }

    // setRandomMode();
       
    sendPageParams(95);  // Parametry, stav IsAlarming (AL: 1) pro změnu zobrazení aktivity poplachu ve smartphonu
  }


  delay(0); 
  // plynula změna hlasitostí
  #if (USE_MP3 == 1)
  if (fadeSoundTimer.isReady() && isDfPlayerOk) {
    if (fadeSoundDirection > 0) {
      // zvětšení hlasitostí
      dfPlayer.volumeUp();
      fadeSoundStepCounter--;
      if (fadeSoundStepCounter <= 0) {
        fadeSoundDirection = 0;
        fadeSoundTimer.setInterval(4294967295);
      }
    } else if (fadeSoundDirection < 0) {
      // zmenšení hlasitosti
      dfPlayer.volumeDown();
      fadeSoundStepCounter--;
      if (fadeSoundStepCounter <= 0) {
        isPlayAlarmSound = false;
        fadeSoundDirection = 0;
        fadeSoundTimer.setInterval(4294967295);
        StopSound(0);
      }
    }
    delay(0);  
  }
  #endif  
}

void stopAlarm() {
  #if (USE_MP3 == 1)  
  if ((isAlarming || isPlayAlarmSound) && !isAlarmStopped) {
    Serial.println(String(F("dawn vyp v ")) + String(hour())+ ":" + String(minute()));
    isAlarming = false;
    isAlarmStopped = true;
    isPlayAlarmSound = false;
    cmd95 = "";
    alarmSoundTimer.setInterval(4294967295);
    StopSound(1500);

    resetModes();  

    BTcontrol = false;
    AUTOPLAY = false;

    if (saveSpecialMode){
       setSpecialMode(saveSpecialModeId);
    } else {
       setModeByModeId(saveThisMode);
    }
    
    // setRandomMode();

    delay(0);    
    sendPageParams(95);  // Parametry, stav IsAlarming (AL: 1) pro změnu zobrazení aktivity budiku ve smartphonu
  }
  #endif
}

void setModeByModeId(byte aMode) {
  if (saveRunningFlag) {
    thisMode = aMode;
    startRunningText();
  } else {
    byte tmp = mapModeToEffect(aMode);
    if (tmp <= MAX_EFFECT) {
      setEffect(tmp);
    } else {  
      tmp = mapModeToGame(aMode);
      if (tmp <= MAX_GAME) {
        startGame(tmp, true, false);
      } else if (aMode == DEMO_TEXT_0 || aMode == DEMO_TEXT_1 || aMode == DEMO_TEXT_2) {
        thisMode = aMode;
        startRunningText();
        loadingFlag = true;
      }
      else {
        setEffect(EFFECT_FIRE);  
      }
    }
  }
  FastLED.setBrightness(globalBrightness);
}

void setRandomMode() {
  String s_tmp = String(ALARM_LIST);    
  uint32_t cnt = CountTokens(s_tmp, ','); 
  byte ef = random(0, cnt - 1); 
          
  // Povolí uvedený režim ze seznamu dostupných efektů bez další změny
   // Hodnota ef může být 0..N-1 - určený režim ze seznamu ALARM_LIST (obsazení do indexu od 0)     
  byte tmp = mapAlarmToEffect(ef);   
  // Pokud jste tento efekt nerozpoznali, zapněte režim „Krb“
  if (tmp != 255) setEffect(tmp);
  else            setEffect(EFFECT_FIRE);   
}

// Zkontrolujte potřebu aktivovat režim 1 v nastavený čas
void checkAutoMode1Time() {
  if (AM1_effect_id <= -5 || AM1_effect_id >= MAX_EFFECT || !init_time) return;
  
  hrs = hour();
  mins = minute();

  // Časový režim je povolen (povolit) a je čas aktivovat režim - aktivovat
  if (!AM1_running && AM1_hour == hrs && AM1_minute == mins) {
    AM1_running = true;
    SetAutoMode(1);
  }

// Režim je aktivován a doba odezvy režimu uplynula - vymažte příznak pro přípravu na další cyklus
  if (AM1_running && (AM1_hour != hrs || AM1_minute != mins)) {
    AM1_running = false;
  }
}
// Zkontrolujte, zda je třeba v nastaveném čase povolit režim 2
void checkAutoMode2Time() {

  // Akce se liší od „Žádná akce“ a je nastaven čas?
  if (AM2_effect_id <= -5 || AM2_effect_id >= MAX_EFFECT || !init_time) return;

 // Pokud alarm zhasl - svítání - nepřepínejte režim - zůstáváme v režimu zpracování alarmu
  if ((isAlarming || isPlayAlarmSound) && !isAlarmStopped) return;

  hrs = hour();
  mins = minute();

  // Časový režim je povolen (povolit) a je čas aktivovat režim - aktivovat
  if (!AM2_running && AM2_hour == hrs && AM2_minute == mins) {
    AM2_running = true;
    SetAutoMode(2);
  }

 // Režim je aktivován a doba odezvy režimu uplynula - vymažte příznak pro přípravu na další cyklus
  if (AM2_running && (AM2_hour != hrs || AM2_minute != mins)) {
    AM2_running = false;
  }
}

// Proveďte zařazení režimu 1 nebo 2 (amode) v nastavený čas
void SetAutoMode(byte amode) {
  
  Serial.print(F("Autorežim "));
  Serial.print(amode);
  Serial.print(F(" ["));
  Serial.print(amode == 1 ? AM1_hour : AM2_hour);
  Serial.print(":");
  Serial.print(amode == 1 ? AM1_minute : AM2_minute);
  Serial.print(F("] - "));

  int8_t ef = (amode == 1 ? AM1_effect_id : AM2_effect_id);

 // ef: -5 - žádná akce;
   // -4 - vypnout matici (černá obrazovka);
   // -3 - noční hodiny,
   // -2 - krb s hodinami,
   // -1 - plíživá linka,
   // 0 - náhodně,
   // 1 a další - účinek ALARM_LIST podle seznamu


  if (ef <= -5 || ef >= MAX_EFFECT) {
    Serial.print(F("není děj"));
  } else if (ef == -4) {

    // Выключить матрицу (черный экран)
    Serial.print(F("vypinaní displeje"));
    setSpecialMode(0);
    
  } else if (ef == -3) {

    // Ночные часы
    Serial.print(F("zapinani režimu "));    
    Serial.print(F("noční hodiny"));
    setSpecialMode(4);
    
  } else if (ef == -2) {

    // Камин с часами
    Serial.print(F("zapnout režim "));    
    Serial.print(F("krb s hodinami"));
    setSpecialMode(5);
    
  } else if (ef == -1) {

    // Бегущая строка
    Serial.print(F(" zapnou režim "));    
    Serial.print(F("bežicí text"));

    resetModes();  
    startRunningText();
    
  } else {

    Serial.print(F("zapnout režim "));    
    
    idleTimer.setInterval(ef == 0 ? idleTime : 4294967295); 
    idleTimer.reset();

    resetModes();  

    AUTOPLAY = ef == 0;
    if (AUTOPLAY) {
      autoplayTimer = millis();
      idleState = true;
      BTcontrol = false;
    }
    
    String s_tmp = String(ALARM_LIST);
    
    if (ef == 0) {
      // "Случайный" режим и далее автосмена
      Serial.print(F("демонcтрации эффектов:"));
      uint32_t cnt = CountTokens(s_tmp, ','); 
      ef = random(0, cnt - 1); 
    } else {
      ef -= 1; 
    }

    s_tmp = GetToken(s_tmp, ef+1, ',');
    Serial.print(F(" efekt "));
    Serial.print("'" + s_tmp + "'");
    
    // Povolí uvedený režim ze seznamu dostupných efektů bez další změny
     // Hodnota ef může být 0..N-1 - určený režim ze seznamu ALARM_LIST (obsazení do indexu od 0) 
    byte tmp = mapAlarmToEffect(ef);   
    
    if (tmp != 255) setEffect(tmp);
    else            setEffect(EFFECT_FIRE); 
  }
  
  Serial.println();
}

void checkClockOrigin() {
  if (allowVertical || allowHorizontal) {
  
    if (CLOCK_ORIENT == 1 && !allowVertical) {
      CLOCK_ORIENT = 0;
      saveClockOrientation(CLOCK_ORIENT);
    }
   
    if (CLOCK_ORIENT == 0 && !allowHorizontal) {
      CLOCK_ORIENT = 1;
      saveClockOrientation(CLOCK_ORIENT);
    }
  } else {
    overlayEnabled = false;
    saveClockOverlayEnabled(overlayEnabled);
    return;
  }

  CLOCK_X = CLOCK_ORIENT == 0 ? CLOCK_X_H : CLOCK_X_V;
  CLOCK_Y = CLOCK_ORIENT == 0 ? CLOCK_Y_H : CLOCK_Y_V;

  // šiřka a výška hodin
  byte cw = CLOCK_ORIENT == 0 ? 4*3 + 3*1 : 2*3 + 1; 
                                                     
  byte ch = CLOCK_ORIENT == 0 ? 1*5 : 2*5 + 1;       в
  while (CLOCK_X > 0 && CLOCK_X + cw > WIDTH)  CLOCK_X--;
  while (CLOCK_Y > 0 && CLOCK_Y + ch > HEIGHT) CLOCK_Y--;

  cw = 4*3 + 1;                                     
  ch = 2*5 + 1;                                     
  
  while (CALENDAR_X > 0 && CALENDAR_X + cw > WIDTH)  CALENDAR_X--; 
  while (CALENDAR_Y > 0 && CALENDAR_Y + ch > HEIGHT) CALENDAR_Y--;
}
