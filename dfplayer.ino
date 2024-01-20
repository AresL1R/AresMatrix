void InitializeDfPlayer1() {
#if (USE_MP3 == 1)
#if defined(ESP8266)
//mp3Serial.begin(9600);                           //  ESP8266 verze 2.5.2
//mp3Serial.begin(9600, SRX, STX);                 //  ESP8266 verze 2.6.0
  mp3Serial.begin(9600, SWSERIAL_8N1, SRX, STX);   // ESP8266 verze 2.6.1
#endif
#if defined(ESP32)
//mp3Serial.begin(9600, SRX, STX);                 //  EspSoftwareSerial v5.4
  mp3Serial.begin(9600, SWSERIAL_8N1, SRX, STX);   //  EspSoftwareSerial v6.0
#endif  
  dfPlayer.begin(mp3Serial, false, false);
  dfPlayer.setTimeOut(1000);
  dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
  dfPlayer.volume(1);
#endif  
}

void InitializeDfPlayer2() {    
#if (USE_MP3 == 1)
  Serial.print(F("spouštění přehravače."));
  refreshDfPlayerFiles();    
  Serial.println(String(F("zvuky nalezeno: ")) + String(alarmSoundsCount));
  Serial.println(String(F("zvuky dawn nalezeno: ")) + String(dawnSoundsCount));
  isDfPlayerOk = alarmSoundsCount + dawnSoundsCount > 0;
#else  
  isDfPlayerOk = false;
#endif  
}

#if (USE_MP3 == 1)
void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      //Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      //Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println(F("USB Inserted!"));
      break;
    case DFPlayerUSBRemoved:
      Serial.println(F("USB Removed!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number: "));
      Serial.print(value);
      Serial.println(F(". Play Finished!"));
      if (!(isAlarming || isPlayAlarmSound) && soundFolder == 0 && soundFile == 0) {
        dfPlayer.stop();
      }
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void refreshDfPlayerFiles() {
 // Čtení z nějakého důvodu nemusí vždy fungovat, někdy vrátí 0 nebo číslo z nějakého předchozího požadavku
   // Aby se jistě přečetla hodnota - první čtení se ignoruje, pak se před opakováním několikrát přečte.

   // Složka se soubory pro budik
  int cnt = 0, val = 0, new_val = 0; 
  do {
    val = dfPlayer.readFileCountsInFolder(1);     delay(10);
    new_val = dfPlayer.readFileCountsInFolder(1); delay(10);    
    if (val == new_val && val != 0) break;
    cnt++;
    delay(100);
    Serial.print(F("."));
  } while ((val == 0 || new_val == 0 || val != new_val) && cnt < 5);
  alarmSoundsCount = val < 0 ? 0 : val;
  
  
  cnt = 0, val = 0, new_val = 0; 
  do {
    val = dfPlayer.readFileCountsInFolder(2);     delay(10);
    new_val = dfPlayer.readFileCountsInFolder(2); delay(10);     
    if (val == new_val && val != 0) break;
    cnt++;
    delay(100);
    Serial.print(F("."));
  } while ((val == 0 || new_val == 0 || val != new_val) && cnt < 5);    
  dawnSoundsCount = val < 0 ? 0 : val;
  Serial.println();  
}
#endif

void PlayAlarmSound() {
  
  if (!isDfPlayerOk) return;

  #if (USE_MP3 == 1)
  int8_t sound = alarmSound;
  
  if (sound == 0) {
    sound = random(1, alarmSoundsCount);    
  }
 
  if (sound > 0) {
    dfPlayer.stop();
    delay(100);                              
    dfPlayer.volume(constrain(maxAlarmVolume,1,30));
    delay(100);
    dfPlayer.playFolder(1, sound);
    delay(100);
    dfPlayer.enableLoop();
    delay(100);    
    alarmSoundTimer.setInterval(alarmDuration * 60L * 1000L);
    alarmSoundTimer.reset();
    isPlayAlarmSound = true;
  } else {
   
    StopSound(1500);
  }
  #endif
}

void PlayDawnSound() {
  
  if (!isDfPlayerOk) return;

  #if (USE_MP3 == 1)
  
  int8_t sound = dawnSound;
  
  if (sound == 0) {
    sound = random(1, dawnSoundsCount);     
  }
  
  if (sound > 0) {
    dfPlayer.stop();
    delay(100);                             
    dfPlayer.volume(1);
    delay(100);
    dfPlayer.playFolder(2, sound);
    delay(100);
    dfPlayer.enableLoop();
    delay(100);
    fadeSoundDirection = 1;   
    fadeSoundStepCounter = maxAlarmVolume;
    if (fadeSoundStepCounter <= 0) fadeSoundStepCounter = 1;
    if (realDawnDuration <= 0) realDawnDuration = 1;
    fadeSoundTimer.setInterval(realDawnDuration * 60L * 1000L / fadeSoundStepCounter);
    alarmSoundTimer.setInterval(4294967295);
  } else {
    StopSound(1500);
  }
  #endif
}

void StopSound(int duration) {

  if (!isDfPlayerOk) return;

  #if (USE_MP3 == 1)
  
  isPlayAlarmSound = false;

  if (duration <= 0) {
    dfPlayer.stop();
    delay(100);
    dfPlayer.volume(0);
    return;
  }
  
  fadeSoundStepCounter = dfPlayer.readVolume();
  if (fadeSoundStepCounter <= 0) fadeSoundStepCounter = maxAlarmVolume;
  if (fadeSoundStepCounter <= 0) fadeSoundStepCounter = 1;
    
  fadeSoundDirection = -1;   
  if (duration < fadeSoundStepCounter) duration = fadeSoundStepCounter;
  fadeSoundTimer.setInterval(duration / fadeSoundStepCounter);
  #endif
}
