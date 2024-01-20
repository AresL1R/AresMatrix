#define EEPROM_OK 0xA5                      
#define GAME_EEPROM 160                    
#define EFFECT_EEPROM 174                  

void loadSettings() {

  // Adresy v EEPROM:
  // 0 - pokud je inicializováno EEPROM_OK - EEPROM, pokud není jiná hodnota
  // 1 - maximální jas pásky 1-255
  // 2 - výchozí rychlost posouvání textu
  // 3 - výchozí rychlost efektů
  // 4 - výchozí rychlost hry
  // 5 - povoleno překryvné hodiny pro efekty
  // 6 - režim automatické změny v ukázce: zapnuto / vypnuto
  // 7 - čas automatického režimu změny
  // 8 - doba nečinnosti před přepnutím do automatického režimu
  // 9 - použití synchronizace času pomocí NTP
  // 10,11 - NTP synchronizační období (int16_t - 2 bajty)
  // 12 - časové pásmo
  // 13 - orientace hodin: 0 - vodorovně; 1 - svisle
  // 14 - barevné schéma hodin 0 - monochromatické; 1 - každá číslice hladce zabarví; 2 - hodiny, body, minuty - barva je hladká; 3 - hodiny, body, minuty - barva uživatele
  // 15 - použití běžících textových efektů v demo režimu
  // 16 - Zobrazení aktuálního data pomocí hodin
  // 17 - Počet sekund k zobrazení data
  // 18 - Počet sekund zobrazení hodin
  // 19 - globalColor R
  // 20 - globalColor G
  // 21 - globalColor B
  // 22 - Budík, dny v týdnu
  // 23 - Alarm, doba trvání „úsvitu“
  // 24 - Budík, efekt „úsvitu“
  // 25 - Alarm, použijte zvuk
  // 26 - Budík, přehrajte zvuk N minut po operaci
  // 27 - Budík, číslo vyzváněcího tónu budíku (ze složky 01 na SD kartě)
  // 28 - Budík, Dawn melodické číslo (ze složky 02 na SD kartě)
  // 29 - Alarm, Maximální hlasitost alarmu
  // 30 - Použijte režim přístupového bodu
  // 31 - Použijte automatické ovládání jasu
  // 32 - Minimální hodnota jasu s automatickým nastavením
  // 33 - Časový režim 1 - hodiny
  // 34 - Časový režim 1 - minuty
  // 35 - Časový režim 1 - ID efektu nebo -1 - vypnuto; 0 - náhodně;
  // 36 - Režim 2 - hodiny
  // 37 - Režim 2 - minuty
  // 38 - Časový režim 2 - ID efektu nebo -1 - vypnuto; 0 - náhodně;
  // 39 - Aktuální speciální režim, takže když je lampa zapnutá, pokud je zapnutý speciální režim, přepněte na ni nebo -1 není speciální. režim
  // 40 - Budík, čas: pondělí: hodiny
  // 41 - Alarm, čas: pondělí: minuty
  // 42 - Budík, čas: úterý: hodiny
  // 43 - Budík, čas: úterý: minuty
  // 44 - Budík, čas: středa: hodiny
  // 45 - Budík, čas: středa: minuty
  // 46 - Budík, čas: čtvrtek: hodiny
  // 47 - Budík, čas: čtvrtek: minuty
  // 48 - Budík, čas: pátek: hodiny
  // 49 - Budík, čas: pátek: minuty
  // 50 - Budík, čas: sobota: hodiny
  // 51 - Budík, čas: sobota: minuty
  // 52 - Budík, čas: neděle: hodiny
  // 53 - Budík, čas: neděle: minuty
  // 54-63 - název přístupového bodu - 10 bajtů
  // 64-79 - heslo přístupového bodu - 16 bajtů
  // 80-103 - Název sítě WiFi - 24 bajtů
  // 104-119 - Heslo sítě WiFi - 16 bajtů
  // 120-149 - Název serveru NTP - 30 bajtů
  // 150-153 - Statická IP adresa lampy
  // 154 - Náhodná posloupnost zapnutí efektů
  // 155 - Poslední ručně aktivovaný efekt
  // 156,157 - aktuální limit
  // 158 - Režim Creeping Line color: 0 - globalColor; 1 - duha; 2 - barevná písmena
  // 159 - vyhrazeno
  // ....
  // 160 - 160+ (Nigr * 2) - rychlost hry
  // 161 - 160+ (Nigr * 2) +1 - použití hry v demo režimu
  // ....
  // 174 - 174+ (Neff * 3) - efektivní rychlost
  // 175 - 174+ (Neff * 3) +1 - 1 - je povoleno překrytí hodinek; 0 - žádné překrytí hodinek
  // 176 - 174+ (Neff * 3) +2 - efekt v automatickém režimu: 1 - použití; 0 - nepoužívejte
  // ....
  // 300 -

 
  strcpy(apName, DEFAULT_AP_NAME);
  strcpy(apPass, DEFAULT_AP_PASS);
  strcpy(ssid, NETWORK_SSID);
  strcpy(pass, NETWORK_PASS);
  strcpy(ntpServerName, DEFAULT_NTP_SERVER);    

  bool isInitialized = EEPROMread(0) == EEPROM_OK;  
    
  if (isInitialized) {    
    globalBrightness = getMaxBrightness();
    scrollSpeed = getScrollSpeed();
    effectSpeed = map(EEPROMread(3),0,255,D_EFFECT_SPEED_MIN,D_EFFECT_SPEED_MAX);
    gameSpeed = map(EEPROMread(4),0,255,D_GAME_SPEED_MIN,D_GAME_SPEED_MAX); 
    if (scrollSpeed == 0) scrollSpeed = 1;
    if (effectSpeed == 0) effectSpeed = 1;
    if (gameSpeed == 0) gameSpeed = 1;
    AUTOPLAY = getAutoplay();
    autoplayTime = getAutoplayTime();
    idleTime = getIdleTime();        
    overlayEnabled = getClockOverlayEnabled();
    useNtp = getUseNtp();
    SYNC_TIME_PERIOD = getNtpSyncTime();
    timeZoneOffset = getTimeZone();
    CLOCK_ORIENT = getClockOrientation();
    COLOR_MODE = getClockColorMode();
    CURRENT_LIMIT = getPowerLimit();
    showDateInClock = getShowDateInClock();  
    showDateDuration = getShowDateDuration();
    showDateInterval = getShowDateInterval();
    useRandomSequence = getRandomMode();
    
    alarmWeekDay = getAlarmWeekDay();
    alarmEffect = getAlarmEffect();
    alarmDuration = getAlarmDuration();

    for (byte i=0; i<7; i++) {
      alarmHour[i] = getAlarmHour(i+1);
      alarmMinute[i] = getAlarmMinute(i+1);
    }

    dawnDuration = getDawnDuration();
    
    #if (USE_MP3 == 1)
    useAlarmSound = getUseAlarmSound();    
    alarmSound = getAlarmSound();
    dawnSound = getDawnSound();
    maxAlarmVolume = getMaxAlarmVolume();
    #endif

    globalColor = getGlobalColor();
    textColorMode = getTextColorMode();

    useSoftAP = getUseSoftAP();
    getSoftAPName().toCharArray(apName, 10);       
    getSoftAPPass().toCharArray(apPass, 17);       
    getSsid().toCharArray(ssid, 25);                
    getPass().toCharArray(pass, 17);                
    getNtpServer().toCharArray(ntpServerName, 31);  
    if (strlen(apName) == 0) strcpy(apName, DEFAULT_AP_NAME);
    if (strlen(apPass) == 0) strcpy(apPass, DEFAULT_AP_PASS);
    if (strlen(ntpServerName) == 0) strcpy(ntpServerName, DEFAULT_NTP_SERVER);

    useAutoBrightness = getUseAutoBrightness();
    autoBrightnessMin = getAutoBrightnessMin();

    AM1_hour = getAM1hour();
    AM1_minute = getAM1minute();
    AM1_effect_id = getAM1effect();
    AM2_hour = getAM2hour();
    AM2_minute = getAM2minute();
    AM2_effect_id = getAM2effect();

    loadStaticIP();
    
  } else {
    globalBrightness = BRIGHTNESS;
    scrollSpeed = D_TEXT_SPEED;
    effectSpeed = D_EFFECT_SPEED;
    gameSpeed = D_GAME_SPEED;
    AUTOPLAY = true;
    autoplayTime = ((long)AUTOPLAY_PERIOD * 1000L);     
    idleTime = ((long)IDLE_TIME * 60L * 1000L);         
    overlayEnabled = true;
    useNtp = true;
    SYNC_TIME_PERIOD = 60;
    timeZoneOffset = 7;
    CLOCK_ORIENT = 0;
    COLOR_MODE = 0;
    showDateInClock = true;  
    showDateDuration = 3;
    showDateInterval = 240;
    alarmWeekDay = 0;
    dawnDuration = 20;
    alarmEffect = EFFECT_DAWN_ALARM;
    useSoftAP = false;
    alarmDuration = 1;

    #if (USE_MP3 == 1)
    useAlarmSound = false;
    alarmSound = 1;
    dawnSound = 1;
    maxAlarmVolume = 30;
    #endif
        
    useAutoBrightness = false;
    autoBrightnessMin = 1;
    globalColor = 0xFFFFFF;
    useRandomSequence = true;

    textColorMode = 0;

    AM1_hour = 0;
    AM1_minute = 0;
    AM1_effect_id = -5;
    AM2_hour = 0;
    AM2_minute = 0;
    AM2_effect_id = -5;    
  }

  scrollTimer.setInterval(scrollSpeed);
  effectTimer.setInterval(effectSpeed);
  gameTimer.setInterval(gameSpeed);
  idleTimer.setInterval(idleTime == 0 ? 4294967295 : idleTime);
  ntpSyncTimer.setInterval(1000 * 60 * SYNC_TIME_PERIOD);
  
  if (!isInitialized) {
    saveDefaults();
    saveSettings();
  }
}

void saveDefaults() {

  EEPROMwrite(1, globalBrightness);

  EEPROMwrite(2, D_TEXT_SPEED);
  EEPROMwrite(3, D_EFFECT_SPEED);
  EEPROMwrite(4, D_GAME_SPEED);

  EEPROMwrite(6, AUTOPLAY ? 1 : 0);
  EEPROMwrite(7, autoplayTime / 1000L);
  EEPROMwrite(8, constrain(idleTime / 60L / 1000L, 0, 255));

  EEPROMwrite(5, overlayEnabled);
  EEPROMwrite(9, useNtp ? 1 : 0);
  EEPROM_int_write(10, SYNC_TIME_PERIOD);
  EEPROMwrite(12, (byte)timeZoneOffset);
  EEPROMwrite(13, CLOCK_ORIENT == 1 ? 1 : 0);
  EEPROMwrite(14, COLOR_MODE);
  EEPROMwrite(16, showDateInClock ? 1 : 0);
  EEPROMwrite(17, showDateDuration);
  EEPROMwrite(18, showDateInterval);

  // globalColor = 0xFFFFFF
  EEPROMwrite(19, 0xFF);
  EEPROMwrite(20, 0xFF);
  EEPROMwrite(21, 0xFF);
  
  saveAlarmParams(alarmWeekDay,dawnDuration,alarmEffect);
  for (byte i=0; i<7; i++) {
      setAlarmTime(i+1, alarmHour[i], alarmMinute[i]);
  }

  EEPROMwrite(15, 1);    
  EEPROMwrite(30, 0);    

  EEPROMwrite(31, 0);    
  EEPROMwrite(32, 1);    

  EEPROMwrite(33, AM1_hour);            
  EEPROMwrite(34, AM1_minute);          
  EEPROMwrite(35, (byte)AM1_effect_id);
  EEPROMwrite(36, AM2_hour);            
  EEPROMwrite(37, AM2_minute);          
  EEPROMwrite(38, (byte)AM2_effect_id); 
  EEPROMwrite(39, (byte)-1);            
  
  for (int i = 0; i < MAX_EFFECT; i++) {
    saveEffectParams(i, effectSpeed, true, true);
  }

  for (int i = 0; i < MAX_GAME; i++) {
    saveGameParams(i, gameSpeed, true);
  }
    
  strcpy(apName, DEFAULT_AP_NAME);
  strcpy(apPass, DEFAULT_AP_PASS);
  strcpy(ssid, NETWORK_SSID);
  strcpy(pass, NETWORK_PASS);
  setSoftAPName(String(apName));
  setSoftAPPass(String(apPass));
  setSsid(String(ssid));
  setPass(String(pass));
  
  strcpy(ntpServerName, DEFAULT_NTP_SERVER);
  setNtpServer(String(ntpServerName));

  EEPROMwrite(150, IP_STA[0]);
  EEPROMwrite(151, IP_STA[1]);
  EEPROMwrite(152, IP_STA[2]);
  EEPROMwrite(153, IP_STA[3]);

  EEPROMwrite(154, useRandomSequence ? 1 : 0);

  setPowerLimit(CURRENT_LIMIT);
  setTextColorMode(textColorMode);
  
  eepromModified = true;
}

void saveSettings() {

  saveSettingsTimer.reset();

  if (!eepromModified) return;
  
  EEPROMwrite(0, EEPROM_OK);
  
  EEPROM.commit();
  Serial.println(F("nastavení uložene v EEPROM"));

  eepromModified = false;
}

void saveEffectParams(byte effect, int speed, boolean overlay, boolean use) {
  const int addr = EFFECT_EEPROM;  
  EEPROMwrite(addr + effect*3, constrain(map(speed, D_EFFECT_SPEED_MIN, D_EFFECT_SPEED_MAX, 0, 255), 0, 255));        
  EEPROMwrite(addr + effect*3 + 1, overlay ? 1 : 0);              
  EEPROMwrite(addr + effect*3 + 2, use ? 1 : 0);                   
  eepromModified = true;
}
 
void saveEffectSpeed(byte effect, int speed) {
  if (speed != getEffectSpeed(effect)) {
    const int addr = EFFECT_EEPROM;  
    EEPROMwrite(addr + effect*3, constrain(map(speed, D_EFFECT_SPEED_MIN, D_EFFECT_SPEED_MAX, 0, 255), 0, 255));        
    eepromModified = true;
  }
}

int getEffectSpeed(byte effect) {
  const int addr = EFFECT_EEPROM;
  return map(EEPROMread(addr + effect*3),0,255,D_EFFECT_SPEED_MIN,D_EFFECT_SPEED_MAX);
}

void saveEffectUsage(byte effect, boolean use) {
  if (use != getEffectUsage(effect)) {
    const int addr = EFFECT_EEPROM;  
    EEPROMwrite(addr + effect*3 + 2, use ? 1 : 0);               
    eepromModified = true;
  }
}

boolean getEffectUsage(byte effect) {
  const int addr = EFFECT_EEPROM;
  return EEPROMread(addr + effect*3 + 2) == 1;
}

void saveGameParams(byte game, int speed, boolean use) {
  const int addr = GAME_EEPROM;  
  EEPROMwrite(addr + game*2, constrain(map(speed, D_GAME_SPEED_MIN, D_GAME_SPEED_MAX, 0, 255), 0, 255));         
  EEPROMwrite(addr + game*2 + 1, use ? 1 : 0);                                                                   
  eepromModified = true;
}

void saveGameSpeed(byte game, int speed) {
  if (speed != getGameSpeed(game)) {
    const int addr = GAME_EEPROM;  
    EEPROMwrite(addr + game*2, constrain(map(speed, D_GAME_SPEED_MIN, D_GAME_SPEED_MAX, 0, 255), 0, 255));         
    eepromModified = true;
  }
}

int getGameSpeed(byte game) {
  const int addr = GAME_EEPROM;  
  return map(EEPROMread(addr + game*2),0,255,D_GAME_SPEED_MIN,D_GAME_SPEED_MAX);
}

void saveGameUsage(byte game, boolean use) {
  if (use != getGameUsage(game)) {
    const int addr = GAME_EEPROM;  
    EEPROMwrite(addr + game*2 + 1, use ? 1 : 0);               
    eepromModified = true;
  }
}

boolean getGameUsage(byte game) {
  const int addr = GAME_EEPROM;
  return EEPROMread(addr + game*2 + 1) == 1;
}

void saveScrollSpeed(int speed) {
  if (speed != getScrollSpeed()) {
    EEPROMwrite(2, constrain(map(speed, D_TEXT_SPEED_MIN, D_TEXT_SPEED_MAX, 0, 255), 0, 255));
    eepromModified = true;
  }
}

int getScrollSpeed() {
  return map(EEPROMread(2),0,255,D_TEXT_SPEED_MIN,D_TEXT_SPEED_MAX);
}

byte getMaxBrightness() {
  return EEPROMread(1);
}

void saveMaxBrightness(byte brightness) {
  if (brightness != getMaxBrightness()) {
    EEPROMwrite(1, brightness);
    eepromModified = true;
  }
}

void saveAutoplay(boolean value) {
  if (value != getAutoplay()) {
    EEPROMwrite(6, value ? 1 : 0);
    eepromModified = true;
  }
}

bool getAutoplay() {
  return EEPROMread(6) == 1;
}

void saveAutoplayTime(long value) {
  if (value != getAutoplayTime()) {
    EEPROMwrite(7, constrain(value / 1000L, 0, 255));
    eepromModified = true;
  }
}

long getAutoplayTime() {
  long time = EEPROMread(7) * 1000L;  
  if (time == 0) time = ((long)AUTOPLAY_PERIOD * 1000);
  return time;
}

void saveIdleTime(long value) {
  if (value != getIdleTime()) {
    EEPROMwrite(8, constrain(value / 60L / 1000L, 0, 255));
    eepromModified = true;
  }
}

long getIdleTime() {
  long iTime = EEPROMread(8) * 60L * 1000L;  
  return iTime;
}

void saveEffectClock(byte effect, boolean overlay) {
  if (overlay != getEffectClock(effect)) {
    const int addr = EFFECT_EEPROM;  
    EEPROMwrite(addr + effect*3 + 1, overlay ? 1 : 0);               
    eepromModified = true;
  }
}

boolean getEffectClock(byte effect) {
  const int addr = EFFECT_EEPROM;
  return EEPROMread(addr + effect*3 + 1) == 1;
}

boolean getClockOverlayEnabled() {
  return (EEPROMread(5) == 1) && (allowVertical || allowHorizontal);
}

void saveClockOverlayEnabled(boolean enabled) {
  if (enabled != getClockOverlayEnabled()) {
    EEPROMwrite(5, enabled ? 1 : 0);
    eepromModified = true;
  }
}

void saveUseNtp(boolean value) {
  if (value != getUseNtp()) {
    EEPROMwrite(9, value);
    eepromModified = true;
  }
}

bool getUseNtp() {
  return EEPROMread(9) == 1;
}

void saveNtpSyncTime(uint16_t value) {
  if (value != getNtpSyncTime()) {
    EEPROM_int_write(10, SYNC_TIME_PERIOD);
    eepromModified = true;
  }
}

uint16_t getNtpSyncTime() {
  uint16_t time = EEPROM_int_read(10);  
  if (time == 0) time = 60;
  return time;
}

void saveTimeZone(int8_t value) {
  if (value != getTimeZone()) {
    EEPROMwrite(12, (byte)value);
    eepromModified = true;
  }
}

int8_t getTimeZone() {
  return (int8_t)EEPROMread(12);
}

byte getClockOrientation() {
  byte val = EEPROMread(13) == 1 ? 1 : 0;

  if (val == 0 && !allowHorizontal) val = 1;
  if (val == 1 && !allowVertical) val = 0;
  
  return val;
}

void saveClockOrientation(byte orientation) {
  if (orientation != getClockOrientation()) {
    EEPROMwrite(13, orientation == 1 ? 1 : 0);
    eepromModified = true;
  }
}

byte getClockColorMode() {
  return EEPROMread(14);
}

void saveClockColorMode(byte ColorMode) {
  if (ColorMode != getClockColorMode()) {
    EEPROMwrite(14, COLOR_MODE);
    eepromModified = true;
  }
}

bool getShowDateInClock() {
  bool val = EEPROMread(16) == 1;
  if (val && HEIGHT < 11) val = 0;
  return val;
}

void setShowDateInClock(boolean use) {  
  if (use != getShowDateInClock()) {
    EEPROMwrite(16, use ? 1 : 0);
    eepromModified = true;
  }
}

byte getShowDateDuration() {
  return EEPROMread(17);
}

void setShowDateDuration(byte Duration) {
  if (Duration != getShowDateDuration()) {
    EEPROMwrite(17, Duration);
    eepromModified = true;
  }
}

byte getShowDateInterval() {
  return EEPROMread(18);
}

void setShowDateInterval(byte Interval) {
  if (Interval != getShowDateInterval()) {
    EEPROMwrite(18, Interval);
    eepromModified = true;
  }
}

void saveAlarmParams(byte alarmWeekDay, byte dawnDuration, byte alarmEffect) {

  
  if (alarmWeekDay != getAlarmWeekDay()) {
    EEPROMwrite(22, alarmWeekDay);
    eepromModified = true;
  }
  if (dawnDuration != getDawnDuration()) {
    EEPROMwrite(23, dawnDuration);
    eepromModified = true;
  }
  if (alarmEffect != getAlarmEffect()) {
    EEPROMwrite(24, alarmEffect);
    eepromModified = true;
  }
}

byte getAlarmHour(byte day) { 
  return constrain(EEPROMread(40 + 2 * (day - 1)), 0, 23);
}

byte getAlarmMinute(byte day) { 
  return constrain(EEPROMread(40 + 2 * (day - 1) + 1), 0, 59);
}

void setAlarmTime(byte day, byte hour, byte minute) { 
  if (hour != getAlarmHour(day)) {
    EEPROMwrite(40 + 2 * (day - 1), constrain(hour, 0, 23));
    eepromModified = true;
  }
  if (minute != getAlarmMinute(day)) {
    EEPROMwrite(40 + 2 * (day - 1) + 1, constrain(minute, 0, 59));
    eepromModified = true;
  }
}

byte getAlarmWeekDay() { 
  return EEPROMread(22);
}

byte getDawnDuration() { 
  return constrain(EEPROMread(23),1,59);
}

byte getAlarmEffect() { 
  return EEPROMread(24);
}

void saveAlarmSounds(boolean useSound, byte duration, byte maxVolume, int8_t alarmSound, int8_t dawnSound) {

  if (useSound != getUseAlarmSound()) {
    EEPROMwrite(25, useSound ? 1 : 0);
    eepromModified = true;
  }
  if (duration != getAlarmDuration()) {
    EEPROMwrite(26, duration);
    eepromModified = true;
  }
  if (maxVolume != getMaxAlarmVolume()) {
    EEPROMwrite(29, maxVolume);
    eepromModified = true;
  }
  if (alarmSound != getAlarmSound()) {
    EEPROMwrite(27, (byte)alarmSound);
    eepromModified = true;
  }
  if (dawnSound != getDawnSound()) {
    EEPROMwrite(28, (byte)dawnSound);
  }
  if (alarmEffect != getAlarmEffect()) {
    EEPROMwrite(24, alarmEffect);
    eepromModified = true;
  }
}

bool getUseAlarmSound() { 
  return EEPROMread(25) == 1;
}

byte getAlarmDuration() { 
  return constrain(EEPROMread(26),1,10);
}

byte getMaxAlarmVolume() { 
  return constrain(EEPROMread(29),0,30);
}

int8_t getAlarmSound() { 
  return constrain((int8_t)EEPROMread(27),-1,127);
}

int8_t getDawnSound() { 
  return constrain((int8_t)EEPROMread(28),-1,127);
}

bool getUseTextInDemo() {
  return EEPROMread(15) == 1;
}

void setUseTextInDemo(boolean use) {  
  if (use != getUseTextInDemo()) {
    EEPROMwrite(15, use ? 1 : 0);
    eepromModified = true;
  }
}

bool getUseSoftAP() {
  return EEPROMread(30) == 1;
}

void setUseSoftAP(boolean use) {  
  if (use != getUseSoftAP()) {
    EEPROMwrite(30, use ? 1 : 0);
    eepromModified = true;
  }
}

String getSoftAPName() {
  return EEPROM_string_read(54, 10);
}

void setSoftAPName(String SoftAPName) {
  if (SoftAPName != getSoftAPName()) {
    EEPROM_string_write(54, SoftAPName, 10);
    eepromModified = true;
  }
}

String getSoftAPPass() {
  return EEPROM_string_read(64, 16);
}

void setSoftAPPass(String SoftAPPass) {
  if (SoftAPPass != getSoftAPPass()) {
    EEPROM_string_write(64, SoftAPPass, 16);
    eepromModified = true;
  }
}

String getSsid() {
  return EEPROM_string_read(80, 24);
}

void setSsid(String Ssid) {
  if (Ssid != getSsid()) {
    EEPROM_string_write(80, Ssid, 24);
    eepromModified = true;
  }
}

String getPass() {
  return EEPROM_string_read(104, 16);
}

void setPass(String Pass) {
  if (Pass != getPass()) {
    EEPROM_string_write(104, Pass, 16);
    eepromModified = true;
  }
}

String getNtpServer() {
  return EEPROM_string_read(120, 30);
}

void setNtpServer(String server) {
  if (server != getNtpServer()) {
    EEPROM_string_write(120, server, 30);
    eepromModified = true;
  }
}

bool getUseAutoBrightness() {
  return EEPROMread(31) == 1;
}

void setUseAutoBrightness(boolean use) {  
  if (use != getUseAutoBrightness()) {
    EEPROMwrite(31, use ? 1 : 0);
    eepromModified = true;
  }
}

byte getAutoBrightnessMin() {
  return EEPROMread(32);
}

void setAutoBrightnessMin(byte brightness) {
  if (brightness != getAutoBrightnessMin()) {
    EEPROMwrite(32, brightness);
    eepromModified = true;
  }
}

void setAM1params(byte hour, byte minute, int8_t effect) { 
  setAM1hour(hour);
  setAM1minute(minute);
  setAM1effect(effect);
}

byte getAM1hour() { 
  byte hour = EEPROMread(33);
  if (hour>23) hour = 0;
  return hour;
}

void setAM1hour(byte hour) {
  if (hour != getAM1hour()) {
    EEPROMwrite(33, hour);
    eepromModified = true;
  }
}

byte getAM1minute() {
  byte minute = EEPROMread(34);
  if (minute > 59) minute = 0;
  return minute;
}

void setAM1minute(byte minute) {
  if (minute != getAM1minute()) {
    EEPROMwrite(34, minute);
    eepromModified = true;
  }
}

int8_t getAM1effect() {
  return (int8_t)EEPROMread(35);
}

void setAM1effect(int8_t effect) {
  if (effect != getAM1effect()) {
    EEPROMwrite(35, (byte)effect);
    eepromModified = true;
  }
}

void setAM2params(byte hour, byte minute, int8_t effect) { 
  setAM2hour(hour);
  setAM2minute(minute);
  setAM2effect(effect);
}

byte getAM2hour() { 
  byte hour = EEPROMread(36);
  if (hour>23) hour = 0;
  return hour;
}

void setAM2hour(byte hour) {
  if (hour != getAM2hour()) {
    EEPROMwrite(36, hour);
    eepromModified = true;
  }
}

byte getAM2minute() {
  byte minute = EEPROMread(37);
  if (minute > 59) minute = 0;
  return minute;
}

void setAM2minute(byte minute) {
  if (minute != getAM2minute()) {
    EEPROMwrite(37, minute);
    eepromModified = true;
  }
}

int8_t getAM2effect() {
  return (int8_t)EEPROMread(38);
}

void setAM2effect(int8_t effect) {
  if (effect != getAM2effect()) {
    EEPROMwrite(38, (byte)effect);
    eepromModified = true;
  }
}

int8_t getCurrentSpecMode() {
  return (int8_t)EEPROMread(39);
}

void setCurrentSpecMode(int8_t mode) {
  if (mode != getCurrentSpecMode()) {
    EEPROMwrite(39, (byte)mode);
    eepromModified = true;
  }
}

int8_t getCurrentManualMode() {
  return (int8_t)EEPROMread(155);
}

void setCurrentManualMode(int8_t mode) {
  if (mode != getCurrentManualMode()) {
    EEPROMwrite(155, (byte)mode);
    eepromModified = true;
  }
}

void loadStaticIP() {
  IP_STA[0] = EEPROMread(150);
  IP_STA[1] = EEPROMread(151);
  IP_STA[2] = EEPROMread(152);
  IP_STA[3] = EEPROMread(153);
}

void saveStaticIP(byte p1, byte p2, byte p3, byte p4) {
  if (IP_STA[0] != p1 || IP_STA[1] != p2 || IP_STA[2] != p3 || IP_STA[3] != p4) {
    IP_STA[0] = p1;
    IP_STA[1] = p2;
    IP_STA[2] = p3;
    IP_STA[3] = p4;
    EEPROMwrite(150, p1);
    EEPROMwrite(151, p2);
    EEPROMwrite(152, p3);
    EEPROMwrite(153, p4);
    eepromModified = true;  
  }
}

uint32_t getGlobalColor() {
  byte r,g,b;
  r = EEPROMread(19);
  g = EEPROMread(20);
  b = EEPROMread(21);
  return (uint32_t)r<<16 | (uint32_t)g<<8 | (uint32_t)b;
}

void setGlobalColor(uint32_t color) {
  globalColor = color;
  if (color != getGlobalColor()) {
    CRGB cl = CRGB(color);
    EEPROMwrite(19, cl.r); // R
    EEPROMwrite(20, cl.g); // G
    EEPROMwrite(21, cl.b); // B
    eepromModified = true;
  }
}

void saveRandomMode(bool randomMode) {
  if (randomMode != getRandomMode()) {
    EEPROMwrite(154, randomMode ? 1 : 0);
    eepromModified = true;
  }  
}

bool getRandomMode() {
 return EEPROMread(154) != 0;
}

void setPowerLimit(uint16_t limit) {
  if (limit != getPowerLimit()) {
    EEPROM_int_write(156, limit);
    eepromModified = true;
  }
}

uint16_t getPowerLimit() {
  uint16_t val = (uint16_t)EEPROM_int_read(156);
  if (val !=0 && val < 1000) val = 1000;
  return val;
}

void setTextColorMode(byte cMode) {
  if (cMode != getTextColorMode()) {
    EEPROMwrite(158, cMode);
    eepromModified = true;
  }
}

byte getTextColorMode() {
  byte val = EEPROMread(158);
  if (val > 2) val = 0;
  return val;
}

// ----------------------------------------------------------

byte EEPROMread(uint16_t addr) {    
  return EEPROM.read(addr);
}

void EEPROMwrite(uint16_t addr, byte value) {    
  EEPROM.write(addr, value);
  saveSettingsTimer.reset();
}

// чтение uint16_t
uint16_t EEPROM_int_read(uint16_t addr) {    
  byte raw[2];
  for (byte i = 0; i < 2; i++) raw[i] = EEPROMread(addr+i);
  uint16_t &num = (uint16_t&)raw;
  return num;
}

// запись uint16_t
void EEPROM_int_write(uint16_t addr, uint16_t num) {
  byte raw[2];
  (uint16_t&)raw = num;
  for (byte i = 0; i < 2; i++) EEPROMwrite(addr+i, raw[i]);
  saveSettingsTimer.reset();
}

String EEPROM_string_read(uint16_t addr, int16_t len) {
   char buffer[len+1];
   memset(buffer,'\0',len+1);
   int16_t i = 0;
   while (i < len) {
     byte c = EEPROMread(addr+i);
     if (c == 0) break;
     buffer[i++] = c;
     // if (c != 0 && (isAlphaNumeric(c) || isPunct(c) || isSpace(c)))
   }
   return String(buffer);
}

void EEPROM_string_write(uint16_t addr, String buffer, int16_t max_len) {
   uint16_t len = buffer.length();
   int16_t i = 0;
   if (len > max_len) len = max_len;
   while (i < len) {
     EEPROMwrite(addr+i, buffer[i++]);
   }
   if (i < max_len) EEPROMwrite(addr+i,0);
   saveSettingsTimer.reset();
}

// ----------------------------------------------------------
