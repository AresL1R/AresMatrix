#define UDP_PACKET_MAX_SIZE 1024
#define PARSE_AMOUNT 8          // maximální počet hodnot v poli, které chceme získat
#define header '$'              // počáteční znak
#define divider ' '             // znak oddělovače
#define ending ';'              // koncový znak
 
int16_t intData [PARSE_AMOUNT]; // pole číselných hodnot po analýze - pro synchronizaci času WiFi WiFi negativně +
                                 // synchronizační periody mb více než 255 min - je třeba zadat int16_t
uint32_t prevColor;
boolean recievedFlag;
byte lastMode = 0;
boolean parseStarted;
String pictureLine;
char answerBuffer [8]; // odpověď klientovi - potvrzení přijetí příkazu: "ack; / r / n / 0"
charcomeBuffer [UDP_PACKET_MAX_SIZE]; // Vyrovnávací paměť pro příjem příkazového řádku ze soketu wifi udp. Vyrovnávací paměť se také používá k odesílání linek do smartphonu.
                                                // Řádek se seznamem efektů může být dlouhý, plus Cyrillic v UTF trvá 2 bajty - buffer by měl být velký.
byte ackCounter = 0;
String receiveText = "", s_tmp = "";
byte tmpSaveMode = 0;

void bluetoothRoutine() {  

  parsing();                                    // přimame informace

  if (tmpSaveMode != thisMode) {
    tmpSaveMode = thisMode;
    if (thisMode == DEMO_TEXT_0 || thisMode == DEMO_TEXT_1 || thisMode == DEMO_TEXT_2) {
      // Это бегущий текст  
      Serial.print(F("Включена бегущая строка "));
      Serial.println(thisMode + 1);
    } else {
      byte tmp_effect = mapModeToEffect(thisMode);
      if (tmp_effect != 255) {
        s_tmp = String(EFFECT_LIST).substring(0,UDP_PACKET_MAX_SIZE);
        s_tmp = GetToken(s_tmp, tmp_effect+1, ',');
        Serial.print(F("Включен эффект "));
        Serial.println("'" + s_tmp + "'");
      } else {
        byte tmp_game = mapModeToGame(thisMode);
        if (tmp_game != 255) {
          s_tmp = String(GAME_LIST).substring(0,UDP_PACKET_MAX_SIZE);
          s_tmp = GetToken(s_tmp, tmp_game+1, ',');
          Serial.print(F("Включена игра "));
          Serial.println("'" + s_tmp + "'");
        } else {
          Serial.print(F("Включен режим "));
          Serial.println(thisMode);
        }
      }
    }
  }

  // neaktualizujte matici v době přijetí dat!
  if (!parseStarted) {                          

    if (wifi_connected && useNtp) {
      if (ntp_t > 0 && millis() - ntp_t > 5000) {
        Serial.println(F("Časový limit požadavku NTP!"));
        ntp_t = 0;
        ntp_cnt++;
        if (init_time && ntp_cnt >= 10) {
          Serial.println(F("Nepodařilo se navázat připojení k serveru NTP."));  
          refresh_time = false;
        }
      }
      bool timeToSync = ntpSyncTimer.isReady();
      if (timeToSync) { ntp_cnt = 0; refresh_time = true; }
      if (timeToSync || (refresh_time && ntp_t == 0 && (ntp_cnt < 10 || !init_time))) {
        getNTP();
        if (ntp_cnt >= 10) {
          if (init_time) {
            udp.flush();
          } else {
            //ESP.restart();
            ntp_cnt = 0;
            connectToNetwork();
          }
        }        
      }
    }

    #if (USE_PHOTO == 1)
    if (useAutoBrightness && autoBrightnessTimer.isReady()) {
      // Během práce budiku, v noci, pokud je matice „vypnutá“ nebo v jednom z režimů „lampy“ - osvětlení
       // žádné automatické ovládání jasu.  
      if (!(isAlarming || isNightClock || isTurnedOff || specialModeId == 2 || specialModeId == 3 || specialModeId == 6 || specialModeId == 7 || thisMode == DEMO_DAWN_ALARM)) {
        // 300 - to je na max. osvětlení, takže maximum 255 nepřichází na hranici prahu citlivosti FR, ale o něco dříve  
        int16_t vin = analogRead(PHOTO_PIN);
        int16_t val = brightness_filter.filtered((int16_t)map(vin,0,1023,-20,255)); 
        if (val < 1) val = 1;
        if (val < autoBrightnessMin) val = autoBrightnessMin;
        if (specialMode) {
           specialBrightness = val;
        } else {
           globalBrightness = val;
        }  
        FastLED.setBrightness(val);
        // V režimu kreslení je třeba vynutit aktualizaci obrazovky,
         // protože statický obrázek není automaticky aktualizován 
        if (drawingFlag) FastLED.show();
      }
    }
    #endif
    
   // Při jasu = 1 zůstanou svítit pouze červené LED diody a všechny efekty ztratí svůj vzhled.
     // proto zobrazují efekt „nočních hodin“
    byte br = specialMode ? specialBrightness : globalBrightness;
    if (br == 1 && !(loadingFlag || isAlarming)) {
      doEffectWithOverlay(DEMO_CLOCK);    
    } 
    
    else if (runningFlag && !isAlarming) {                         // run line - Spuštění textu
      String txt = runningText;
      uint32_t txtColor = globalColor;
      if (wifi_print_ip && (wifi_current_ip.length() > 0)) {
        txt = wifi_current_ip;     
        txtColor = 0xffffff;
      } else {
        if (thisMode == DEMO_TEXT_0) {
          txtColor = globalColor; 
        } else if (thisMode == DEMO_TEXT_1) {
          txtColor = 1;
        } else if (thisMode == DEMO_TEXT_2) {
          txtColor = 2;
        } else {
          switch (textColorMode) {
            case 1:  txtColor = 1; break;            //Duha
            case 2:  txtColor = 2; break;            //Barevne pismena
            default: txtColor = globalColor; break;  //Jedna barva procelý text
          } 
        }
      }
      if (txt.length() == 0) {
        if (thisMode == DEMO_TEXT_0) {
          txt = TEXT_1;
        } else if (thisMode == DEMO_TEXT_1) {
          txt = TEXT_2;
        } else if (thisMode == DEMO_TEXT_2) {
          txt = TEXT_3;
        }
      }
      if (txt.length() == 0) {
          txt = init_time
            ? clockCurrentText() + " " + dateCurrentTextLong()  // + dateCurrentTextShort()
            : TEXT_1; 
      }      
      if (txt.length() == 0) {
          txt = String(F("Matrix na adresovych LED"));
      }            
      fillString(txt, txtColor); 
    }
    
    // Jeden z herních režimů. Efekty se na hry nepřekrývají
    else if (gamemodeFlag && !isAlarming) {
      // Pro hry se vypina běžicí text a efekty
      effectsFlag = false;
      runningFlag = false;
      customRoutine();
    }

    //V kreslení vypnout veškere efekty
    else if (drawingFlag && !isAlarming) {
      
    }
    else if (effectsFlag || (thisMode == DEMO_TEXT_0 || thisMode == DEMO_TEXT_1 || thisMode == DEMO_TEXT_2)) {
      // Vytvořit a zaslat demo režim na obraz      
      customRoutine();
    }
       
    checkAlarmTime();
    checkAutoMode1Time();
    checkAutoMode2Time();

    butt.tick();  // požadovana fonkce pro render musí se vždy optavat
    byte clicks = 0;

    // Jeden click
    if (butt.isSingle()) clicks = 1;    
    // Dva click
    if (butt.isDouble()) clicks = 2;
    // Tři click
    if (butt.isTriple()) clicks = 3;
    // Čtyry a vic clicku
    if (butt.hasClicks()) clicks = butt.getClicks();
    
    if (butt.isHolded()) {
      // zmačknout a držet
      isButtonHold = true;
      hold_start_time = millis();
    }
    
    if (butt.isPress()) {
      // Stav tlačitko zmačkle  
    }
    
    if (butt.isRelease()) {
      // Stav tlačitko pustili
      isButtonHold = false;
      hold_start_time = 0;
    }

    // Jakekoliv zmačknutí tlačitka zastavuje budik
    if ((isAlarming || isPlayAlarmSound) && (isButtonHold || clicks > 0)) {
      stopAlarm();
    }
            
    // Optavaní tlačitka
    else if (isButtonHold) {
      
      // Pokud budik funguje - jakýkoli počet klepnutí nebo podržení přeruší alarm a zapne hodiny na černém pozadí
      if ((isAlarming || isPlayAlarmSound)) {
        stopAlarm();
      }

      // Pokud od stisknutí a podržení uplynulo více než HOLD TIMEOUT milisekund ...
      if (hold_start_time != 0 && (millis() - hold_start_time) > HOLD_TIMEOUT) {
        isButtonHold = false;
        if (isTurnedOff)
          // Jestli vypnout zapnout hodiny        
          setSpecialMode(1);
        else 
          // Pokud je povoleno, vypněte jej (povolte speciální režim černé obrazovky)
          setSpecialMode(0);
      }      
      
    } else if (!isTurnedOff) {
    
      // Ostatní kliknutí fungují, pouze pokud nejsou vypnuta
      
      // Byl jeden click
      if (clicks == 1) {
        if (wifi_print_ip) {
          wifi_print_ip = false;
          wifi_current_ip = "";
          runningFlag = false;
          effectsFlag = true;
        }
        if (specialMode) {
          // Pokud je ve zvláštním režimu - a bílá obrazovka - zapíná / vypíná hodiny na bílé obrazovce
          if (specialModeId == 2 || specialModeId == 3) {
             if (specialModeId == 2) setSpecialMode(3);
             else setSpecialMode(2);
          } else {
            // přepínání v kruhu mezi pravidelnými (1) a nočními (4) hodinami a hodinami s ohněm (5)
            if (specialModeId == 4) setSpecialMode(1);
            else if (specialModeId == 1) setSpecialMode(5);
            else setSpecialMode(4);              
          }
        } else {
          // Pokud je v demo režimu - další režim
          nextMode();
        }        
      }

      // Bylo dvakrát kliknuto
      if (clicks == 2) { 
        if (wifi_print_ip) {
          wifi_print_ip = false;
          wifi_current_ip = "";
          runningFlag = false;
          effectsFlag = true;
        }
        if (specialModeId < 0) {
          // Z jakéhokoli režimu - zapnout hodiny
          setSpecialMode(1);
        } else {
          // Pokud svítí jasně bílá (lampa) - vratite se k hodinám
          if (specialModeId == 2 || specialModeId == 3) 
            // Povolí normalní hodiny
            setSpecialMode(1);
          else  
            // Zapnutí jasně bílé obrazovky (lampa)
            setSpecialMode(2);
        }
      }

      // Trojite klinutí      
      if (clicks == 3) {
        // Zapnout demomode
        idleTimer.setInterval(idleTime == 0 ? 4294967295 : idleTime);
        idleTimer.reset();
        
        if (wifi_print_ip) {
          wifi_print_ip = false;
          wifi_current_ip = "";
          runningFlag = false;
          effectsFlag = true;
        }

        resetModes();  

        BTcontrol = false;
        AUTOPLAY = true;

        String s_tmp = String(ALARM_LIST);    
        uint32_t cnt = CountTokens(s_tmp, ','); 
        byte ef = random(0, cnt - 1); 
         // Povolí uvedený režim ze seznamu dostupných efektů bez další změny
         // Hodnota ef může být 0..N-1 - určený režim ze seznamu ALARM_LIST (obsazení do indexu od 0)
         byte tmp = mapAlarmToEffect (ef);
         // Pokud jste tento efekt nerozpoznali, zapněte režim „Krb“
         pokud (tmp! = 255) setEffect (tmp);
        else            setEffect(EFFECT_FIRE); 
      }

      // Čtvrté stisknutí - zobrazí aktuální IP připojení WiFi
      if (clicks == 4) {
        showCurrentIP();
      }
      
      // ... a tak dale
    }

    #if (USE_MP3 == 1)

   // Došlo ke změně stavu přehrávače MP3?
    if (dfPlayer.available()) {

      // Vyvest do protokolu podrobnosti o změně stavu
      byte msg_type = dfPlayer.readType();      
      printDetail(msg_type, dfPlayer.read());

      // Akce, které je třeba provést při změně některých stavů:
      if (msg_type == DFPlayerCardRemoved) {
        // Karta „spadla“ - děláme nepřístupné vše, co souvisí s přehrávačem MP3
        isDfPlayerOk = false;
        alarmSoundsCount = 0;
        dawnSoundsCount = 0;
        Serial.println(F("MP3 přehravač je nedostupny"));
      } else if (msg_type == DFPlayerCardOnline || msg_type == DFPlayerCardInserted) {
        // Karta rozpoznaná přehravačem - inicializace 2. fáze
        InitializeDfPlayer2();
        if (!isDfPlayerOk) Serial.println(F("MP3 přehravač je ndedotupny."));
      }
    }
    #endif
    
   // Zkontrolujte - pokud dlouho nebylo použite ruční ovládání - přepněte se do automatického režimu
    if (!(isAlarming || isPlayAlarmSound)) checkIdleState();

   // Pokud jsou v EEPROM neuložená data, uložte je
    if (saveSettingsTimer.isReady()) {
      saveSettings();
    }
  }
}

enum eModes {NORMAL, COLOR, TEXT} parseMode;

byte parse_index;
String string_convert = "";

bool haveIncomeData = false;
char incomingByte;

int16_t  bufIdx = 0;         // Lze přijímat pakety> 255 bajtů - nbg int16_t
int16_t  packetSize = 0;

// parsování řádku obrázku - příkaz $ 5
char *pch;
int pntX, pntY, pntColor, pntIdx;
char buf[14];               // bod obrázku FFFFFF XXX YYY
String pntPart[WIDTH];      // pole analyzovaného vstupního řetězce do tečkovaných řetězců

// ********************* PŘIMAME DATA **********************
void parsing() {
// ****************** ÚPRAVA *****************
  String str, str1, str2;
  byte b_tmp = 0;
  int8_t tmp_eff, idx;

  byte alarmHourVal;
  byte alarmMinuteVal;

  /*
     Komunikační protokol, balíček začíná režimem. Režimy:
     0 - poslat barvu $ 0 colorHEX;
     1 - odeslání souřadnic bodu $ 1 X Y;
     2 - vyplňte - 2 $;
     3 - čištění - 3 $;
     4 - jas -
       Hodnota $ 4 0 nastavuje aktuální úroveň jasu
       $ 4 1 U V nastavte režim automatického jasu a min. jas při automatickém nastavení,
                    kde U: 0 - vypnuto 1 - zapnuto; M - hodnota jasu - 1..255
   5 - řádek po řádku $ 5 Y colorHEX X | colorHEX X | ... | colorHEX X;
     6 - text $ 6 N | nějaký text, kde N je účel textu;
         0 - plíživý text řádku
         1 - Název serveru NTP
         2 - SSID síťové připojení
         3 - heslo pro připojení k síti
         4 - název přístupového bodu
         5 - heslo pro přístupový bod
         6 - nastavení budíku
    7 - správa textu:
         7 $ 1 - start;
         7 0 $ - stop;
         7 $ 2 - použití v demo režimu;
         $ 7 3 - NEPOUŽÍVEJTE v demo režimu;
         7 $ 4 X - režim barevného zobrazení, kde X 0 - jedna barva; 1 - duha 2 - každé písmeno má svou vlastní barvu
    8 - efekt
       - 8 0 $ efektové číslo;
       - 8 $ 1 N X start / stop; N - číslo efektu, X = 0 - stop X = 1 - start
       - 8 $ N 2 zapnuto / vypnuto pro použití v demo režimu; N - číslo efektu, X = 0 - nepoužívejte X = 1 - použití
   9 - hra
       - 9 $ $ 0;
       - $ 9 1 N X start / stop; N - číslo hry, X = 0 - stop X = 1 - start
       - 9 $ N 2 zapnuto / vypnuto pro použití v demo režimu; N - číslo hry, X = 0 - nepoužívejte X = 1 - použití
  10 - nahoru tlačítko
     11 - tlačítko vpravo
     12 - tlačítko dolů
     13 - levé tlačítko
     14 - rychlá instalace ručních režimů s přednastavením
       - 14 0 $; Černá obrazovka (vypnuto);
        - 14 $ 1; Černá obrazovka s hodinami;
        - 14 $ 2; Bílá obrazovka (osvětlení);
        - 14 $ 3; Bílá obrazovka s hodinami;
        - 14 $ 4; Černá obrazovka s hodinami jasu min - noční režim;
        - 14 5 $; Černá obrazovka s účinkem ohně a hodin (krb);
        - 14 6 $; Barevná obrazovka;
   - 14 $ 7; Barevná obrazovka s hodinami;
     15 - rychlostní časovač $ 15; 0 - časovač efektů, 1 - časovač posouvání textu 2 - časovač hry
     16 - Režim změny efektu: hodnota $ 16; N: 0 - Automatické přehrávání zapnuto; 1 - Automatické přehrávání vypnuto; 2 - PrevMode; 3 - NextMode; 4 - Náhodný režim zapnut / щаа; 5 - Náhodný režim vypnutý;
     17 - Časově automatické změny a nečinnost: $ 17 sec;
     18 - Žádost o aktuální parametry programem: stránka $ 18; strana: 1 - nastavení; 2 - výkres; 3 - obrázek; 4 - text; 5 - efekty; 6 - hra; 7 hodin; 8 - o aplikaci
     19 - práce s nastavením hodin
     20 - nastavení a správa alarmů
       - 20 $ 0; - vypněte alarm (reset isAlarming status)
       - 20 $ 1 DD EF WD;
           DD - nastavení doby svítání (svítání začíná DD minut před nastaveným časem budíku)
           EF - nastavení efektu, který bude použit jako úsvit
           WD - nastavení dní Mon-Sun jako bitmasky
       - 20 $ 2 X DD VV MA MB;
            X - použijte alarmový zvuk X = 0 - ne, X = 1 - ano
           DD - přehraje zvuk alarmu DD minut po alarmu
           VV - maximální objem
           MA - číslo zvukového souboru alarmu
           MB - číslo zvukového souboru za úsvitu
       - 20 $ 3 NN VV X; - příklad zvukové signalizace
            NN - číslo souboru zvukového signálu ze složky SD: / 01
            VV - úroveň hlasitosti
            X - 1 play 0 - stop
        - 20 $ 4 NN VV X; - příklad zvuku úsvitu
            NN - číslo souboru zvuku úsvitu ze složky SD: / 02
            VV - úroveň hlasitosti
            X - 1 play 0 - stop
       - 20 V $ 5 VV; - nastavit úroveň hlasitosti pro příklady přehrávání (když již hrajete)
           VV - úroveň hlasitosti
       - 20 $ 6 D HH MM; - nastavit čas budíku pro určený den v týdnu
           D - den v týdnu
           HH - čas budíku
           MM - minuty času buzení
    21 - nastavení síťového připojení / přístupového bodu
    22 - nastavení umožňující maticové režimy v určený čas
       - 22 $ X HH MM NN
           X - režim číslo 1 nebo 2
           HH - doba odezvy
           MM - minuty provozu
           NN - účinek: -2 - vypnuto; -1 - vypněte matici; 0 - náhodný režim a dále v kruhu; 1 a další - seznam režimů ALARM_LIST
    23 - další nastavení
       - $ 23 0 VAL - aktuální limit spotřeby
  */  
  if (recievedFlag) {      // jestli jsou přijate data
    recievedFlag = false;

    // Režimy 16,17,18 neaktivují idleTimer
    if (intData[0] < 16 || intData[0] > 18) {
      idleTimer.reset();
      idleState = false;      
    }

    // Režimy jiné než 4 (jas), 14 (nový speciální režim) a 18 (požadavek na parametry stránky),
     // 19 (nastavení hodin), 20 (nastavení budíku), 21 (nastavení sítě) resetuje speciální režim
    if (intData[0] != 4  && intData[0] != 14 && 
        intData[0] != 18 && intData[0] != 19 &&
        intData[0] != 20 && intData[0] != 21 && intData[0] != 23) {
      if (specialMode) {
        idleTimer.setInterval(idleTime == 0 ? 4294967295 : idleTime);
        idleTimer.reset();
        specialMode = false;
        isNightClock = false;
        isTurnedOff = false;
        specialModeId = -1;
      }
    }

    // Režimy jiné než 18 zastaví alarm, pokud to funguje (dawn)
    if (intData[0] != 18 && intData[0] != 20) {
      wifi_print_ip = false;
      wifi_current_ip = "";
      stopAlarm();
    }
    
    switch (intData[0]) {
      case 0:
        if (!runningFlag) drawingFlag = true;
        sendAcknowledge();
        break;
      case 1:
        BTcontrol = true;
        drawingFlag = true;
        runningFlag = false;
        if (gamemodeFlag && game==1) 
          gamePaused = true;
        else {
          gamemodeFlag = false;
          gamePaused = false;
        }
        effectsFlag = false;
        drawPixelXY(intData[1], intData[2], gammaCorrection(globalColor));
        FastLED.show();
        sendAcknowledge();
        break;
      case 2:
        BTcontrol = true;
        runningFlag = false;
        drawingFlag = true;
        gamemodeFlag = false;
        gamePaused = false;
        effectsFlag = false;
        fillAll(gammaCorrection(globalColor));
        FastLED.show();
        sendAcknowledge();
        break;
      case 3:
        BTcontrol = true;
        runningFlag = false;
        gamemodeFlag = false;
        gamePaused = false;
        drawingFlag = true;
        effectsFlag = false;
        FastLED.clear();
        FastLED.show();
        sendAcknowledge();
        break;
      case 4:
        if (intData[1] == 0) {
          globalBrightness = intData[2];
          breathBrightness = globalBrightness;
          saveMaxBrightness(globalBrightness);
          if (!(isNightClock || useAutoBrightness)) {          
            if (specialMode) specialBrightness = globalBrightness;
            FastLED.setBrightness(globalBrightness);
            FastLED.show();
          }
        }
        if (intData[1] == 1) {
          useAutoBrightness = intData[2] == 1;
          autoBrightnessMin = intData[3];
          if (autoBrightnessMin < 1) autoBrightnessMin = 1;
          setUseAutoBrightness(useAutoBrightness);
          setAutoBrightnessMin(autoBrightnessMin);
        }
        sendAcknowledge();
        break;
      case 5:
        BTcontrol = true;

        if (!drawingFlag) {
          FastLED.clear(); 
        }
        
        effectsFlag = false;        
        runningFlag = false;
        gamemodeFlag = false;
        drawingFlag = true;
        loadingFlag = false;

       // Analyzovat řádek z přijaté vyrovnávací paměti formátu 'Y colorHEX X | colorHEX X | ... | colorHEX X'
         // Získat číslo řádku (Y), pro které jsme obdrželi řádek s daty (číslo řádku je od shora dolů, zatímco matice je index řádku od shora dolů)
        b_tmp = pictureLine.indexOf(" ");
        str = pictureLine.substring(0, b_tmp);
        pntY = str.toInt();
        pictureLine = pictureLine.substring(b_tmp+1);

        pntIdx = 0;
        idx = pictureLine.indexOf("|");
        while (idx>0)
        {
          str = pictureLine.substring(0, idx);
          pictureLine = pictureLine.substring(idx+1);
          
          pntPart[pntIdx++] = str;
          idx = pictureLine.indexOf("|");

          if (idx<0 && pictureLine.length()>0) {
            pntPart[pntIdx++] = pictureLine;  
          }          
          delay(0);
        }

        for (int i=0; i<pntIdx; i++) {
          str = pntPart[i];
          idx = str.indexOf(" ");
          str1 = str.substring(0, idx);
          str2 = str.substring(idx+1);

          pntColor=HEXtoInt(str1);
          pntX=str2.toInt();
          
          // начало картинки - очистить матрицу
          if (pntX == 0 && pntY == 0) {
            FastLED.clear(); 
            FastLED.show();
          }
          
          drawPixelXY(pntX, HEIGHT - pntY - 1, gammaCorrection(pntColor));
          delay(0);
        }

        // Выводить построчно для ускорения вывода на экран
        if (pntX == WIDTH - 1)
          FastLED.show();

        // Подтвердить прием строки изображения
        str = "$5 " + String(pntY)+ "-" + String(pntX) + " ack" + String(ackCounter++) + ";";
  
        str.toCharArray(incomeBuffer, str.length()+1);    
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        udp.write((const uint8_t*)incomeBuffer, str.length()+1);
        udp.endPacket();
        delay(0);
        break;
      case 6:
        loadingFlag = true;
       // řetězec je přijat do proměnné recipientText, formát řetězce je N | text, kde N:
         // 0 - text pro běžící linii
         // 1 - Název serveru NTP
         // 2 - název sítě (SSID)
         // 3 - síťové heslo
         // 4 - název přístupového bodu
         // 5 - heslo přístupového bodu
         // 6 - nastavení alarmů ve formátu $ 6 6 | DD EF WD HH1 MM1 HH2 MM2 HH3 MM3 HH4 MM4 HH5 MM5 HH6 MM6 HH7 MM7
        tmp_eff = receiveText.indexOf("|");
        if (tmp_eff > 0) {
          b_tmp = receiveText.substring(0, tmp_eff).toInt();
          str = receiveText.substring(tmp_eff+1, receiveText.length()+1);
           switch(b_tmp) {
            case 0:
              runningText = str;
              break;
            case 1:
              str.toCharArray(ntpServerName, 30);
              setNtpServer(str);
              if (wifi_connected) {
                refresh_time = true; ntp_t = 0; ntp_cnt = 0;
              }
              break;
            case 2:
              str.toCharArray(ssid, 24);
              setSsid(str);
              break;
            case 3:
              str.toCharArray(pass, 16);
              setPass(str);
              break;
            case 4:
              str.toCharArray(apName, 10);
              setSoftAPName(str);
              break;
            case 5:
              str.toCharArray(apPass, 16);
              setSoftAPPass(str);
              // Přeneseno v jednom paketu - použijte SoftAP, název tečky a heslo
               // Po obdržení hesla restartujte vytváření přístupového bodu
              if (useSoftAP) startSoftAP();
              break;
            case 6:
              // Nastavení alarmu ve formátu $ 6 6 | DD EF WD HH1 MM1 HH2 MM2 HH3 MM3 HH4 MM4 HH5 MM5 HH6 MM6 HH7 MM7
               // DD - nastavení doby svítání (svítání začíná DD minut před nastaveným časem budíku)
               // EF - nastavení efektu, který bude použit jako úsvit
               // WD - nastavení dní Mon-Sun jako bitmask
               // HHx - hodiny dne v týdnu x (1-mon. 7-slunce)
               // MMx - minuty dne v týdnu x (1-mon. 7-slunce)
               //
               // Zastaví budik, pokud to funguje
              #if (USE_MP3 == 1)
              if (isDfPlayerOk) { 
                dfPlayer.stop();
              }
              soundFolder = 0;
              soundFile = 0;
              #endif
              isAlarming = false;
              isAlarmStopped = false;
              
              // Nastavení obsahuje 17 prvků (viz výše uvedený formát)
              tmp_eff = CountTokens(str, ' ');
              if (tmp_eff == 17) {
              
                dawnDuration = constrain(GetToken(str, 1, ' ').toInt(),1,59);
                alarmEffect = mapAlarmToEffect(GetToken(str, 2, ' ').toInt());
                alarmWeekDay = GetToken(str, 3, ' ').toInt();
                saveAlarmParams(alarmWeekDay,dawnDuration,alarmEffect);
                
                for(byte i=0; i<7; i++) {
                  alarmHourVal = constrain(GetToken(str, i*2+4, ' ').toInt(), 0, 23);
                  alarmMinuteVal = constrain(GetToken(str, i*2+5, ' ').toInt(), 0, 59);
                  alarmHour[i] = alarmHourVal;
                  alarmMinute[i] = alarmMinuteVal;
                  setAlarmTime(i+1, alarmHourVal, alarmMinuteVal);
                }
                // Okamžitě uložit nastavení budíku
                saveSettings();
                // Vypočítejte počáteční čas úsvitu budiku
                calculateDawnTime();            
              }
              break;
           }
        }

        if (b_tmp == 6) 
          sendPageParams(8);
        else
          sendAcknowledge();
        break;
      case 7:
        BTcontrol = true;
        if (intData[1] == 0 || intData[1] == 1) {
          if (intData[1] == 1) runningFlag = true;
          if (intData[1] == 0) runningFlag = false;          
          if (runningFlag) {
            startRunningText();
          }
        }
        else if (intData[1] == 2 || intData[1] == 3) {
          // On / Off "Použivat v demo": 2 - on; 3 - vypnuto
          setUseTextInDemo(intData[1] == 2);
        }
        else if (intData[1] == 4) {
          // Režim barvy běžicího textu: 0 - monochromatický (globalColor); 1 - duha; 2 - barevná písmena
          textColorMode = intData[2];
          setTextColorMode(textColorMode);
        }
        sendAcknowledge();
        break;
      case 8:      
        effect = intData[2];        
       // intData [1]: akce -> 0 - volba účinku; 1 - start / stop; 2 - zapnuto / vypnuto "použití v demo režimu"
         // intData [2]: číslo efektu
         // intData [3]: action = 1: 0 - stop 1 - start; akce = 2: 0 - vypnuto; 1 - zapnuto;
        if (intData[1] == 0 || intData[1] == 1) {          
          // Pokud jsou v aplikaci vybrány hodiny, ale kvůli velikosti matice nejsou k dispozici, projeví se následující efekt
          if (effect == EFFECT_CLOCK){
             if (!(allowHorizontal || allowVertical)) effect++;
          }
          setEffect(effect);
          BTcontrol = true;
          loadingFlag = intData[1] == 0;
          effectsFlag = intData[1] == 0 || (intData[1] == 1 && intData[3] == 1); // vyběr efektu a jeho nahle zpouštění           
          if (effect == EFFECT_FILL_COLOR && globalColor == 0x000000) setGlobalColor(0xffffff);
        } else if (intData[1] == 2) {
          // On / Off používá efekt v demo režimu
          saveEffectUsage(effect, intData[3] == 1); 
        }
        // Pro "0" a "2" - parametry jsou odeslány, není nutné potvrzení. Pro ostatní - potřebujete
        if (intData[1] == 0 || intData[1] == 2) {
          sendPageParams(5);
        } else { 
          sendAcknowledge();
        }
        break;
      case 9:        
        game = intData[2];
       // intData [1]: akce -> 0 - výběr hry; 1 - start / stop; 2 - zapnuto / vypnuto "použití v demo režimu"
// intData [2]: číslo hry
// intData [3]: action = 1: 0 - stop 1 - start; akce = 2: 0 - vypnuto; 1 - zapnuto;
        if (intData[1] == 0 || intData[1] == 1) {
          BTcontrol = true; 

          // spuštění nové hry při přepínání ze všech režimů kromě kreslení
           // Pokud je hra pozastavena hadem (hra == 0) - pokračujte, jinak spusťte novou hru
          startGame(game, 
                    intData[1] == 0 && (!drawingFlag || (drawingFlag && game != GAME_SNAKE) || runningFlag), // nova hra ?
                    intData[1] == 0 || (intData[1] == 1 && intData[3] == 0)                                  // je zastavena ?
          );          
        } else if (intData[1] == 2) {
          // On / off pomocí hry v demo režimu
          saveGameUsage(game, intData[3] == 1); 
        }

        // Pro "0" a "2" - parametry jsou odeslány, není nutné potvrzení. Pro ostatní - potřebujete
        if (intData[1] == 0 || intData[1] == 2) {
          sendPageParams(6);
        } else { 
          sendAcknowledge();
        }
        break;        
      case 10:
        BTcontrol = true;        
        buttons = 0;
        controlFlag = true;
        gamePaused = false;
        gameDemo = false;
        sendAcknowledge();
        break;
      case 11:
        BTcontrol = true;
        buttons = 1;
        controlFlag = true;
        gamePaused = false;
        gameDemo = false;
        sendAcknowledge();
        break;
      case 12:
        BTcontrol = true;
        buttons = 2;
        controlFlag = true;
        gamePaused = false;
        gameDemo = false;
        sendAcknowledge();
        break;
      case 13:
        BTcontrol = true;
        buttons = 3;
        controlFlag = true;
        gamePaused = false;
        gameDemo = false;
        sendAcknowledge();
        break;
      case 14:
        setSpecialMode(intData[1]);
        sendPageParams(1);
        break;
      case 15: 
        if (intData[2] == 0) {
          if (intData[1] == 255) intData[1] = 254;
          effectSpeed = 255 - intData[1]; 
          saveEffectSpeed(effect, effectSpeed);          
        } else if (intData[2] == 1) {
          scrollSpeed = 255 - intData[1]; 
          saveScrollSpeed(scrollSpeed);
        } else if (intData[2] == 2) {
          gameSpeed = map(constrain(255 - intData[1],0,255),0,255,D_GAME_SPEED_MIN,D_GAME_SPEED_MAX);     // pro hry je rychlost potřebná ménší! Vx 0,255 převést na 25..375
          saveGameSpeed(game, gameSpeed);
        }
        setTimersForMode(thisMode);
        sendAcknowledge();
        break;
      case 16:
       // 16 - Režim změny účinku: hodnota $ 16; N: 0 - Automatické přehrávání zapnuto; 1 - Automatické přehrávání vypnuto; 2 - PrevMode; 3 - NextMode; 4 - Režim automatického přehrávání zapnutý / щаа; 5 - Náhodný režim zapnuto / vypnuto;
        BTcontrol = intData[1] == 1;                
        if (intData[1] == 0) AUTOPLAY = true;
        else if (intData[1] == 1) AUTOPLAY = false;
        else if (intData[1] == 2) prevMode();
        else if (intData[1] == 3) nextMode();
        else if (intData[1] == 4) AUTOPLAY = intData[2] == 1;
        else if (intData[1] == 5) useRandomSequence = intData[2] == 1;

        idleState = !BTcontrol && AUTOPLAY; 
        if (AUTOPLAY) {
          autoplayTimer = millis();// Když zapnete automatický režim, resetujte časovač režimu automatické změny
          controlFlag = false; 
          gameDemo = true;          
        }
        saveAutoplay(AUTOPLAY);
        saveRandomMode(useRandomSequence);
        setCurrentManualMode(AUTOPLAY ? -1 : (int8_t)thisMode);

        if (!BTcontrol) {
          if (runningFlag) loadingFlag = true;       
          // if false - při přepnutí z textového efektu na efektový text do běžícího textového demo režimu není text demo režimu nejprve, ale z pozice, kde se text efektu objevil
           // pokud je pravda - text se nejprve spustí, pak plynule zmizí, aby se změnil režim, a pak se znovu začne znovu.
           // A tak to není dobré. Jak to opravit?
          controlFlag = false;      // Po spuštění hry se tlačítka ještě nedotýkejte - hra je automatická
          drawingFlag = false;
        } else {
          // Pokud při přepnutí do ručního režimu došlo k demo režimu běžecké linky - povolte ruční provoz běžecké linky
          if (intData[1] == 0 || (intData[1] == 1 && (thisMode == DEMO_TEXT_0 || thisMode == DEMO_TEXT_1 || thisMode == DEMO_TEXT_2))) {
            loadingFlag = true;
            runningFlag = true;          
          }
        }

        if (!BTcontrol && AUTOPLAY) {
          sendPageParams(1);
        } else {        
          sendAcknowledge();
        }
        
        break;
      case 17: 
        autoplayTime = ((long)intData[1] * 1000);   // sekundy -> milisekundy 
        idleTime = ((long)intData[2] * 60 * 1000);  // sekundy -> milisekundy 
        saveAutoplayTime(autoplayTime);
        saveIdleTime(idleTime);
        if (AUTOPLAY) {
          autoplayTimer = millis();
          BTcontrol = false;
          controlFlag = false; 
          gameDemo = true;
        }
        idleState = !BTcontrol && AUTOPLAY; 
        idleTimer.setInterval(idleTime == 0 ? 4294967295 : idleTime);
        idleTimer.reset();
        sendAcknowledge();
        break;
      case 18: 
        if (intData[1] == 2 || // kreslení
            intData[1] == 3 || // obrazek
            intData[1] == 4)   // text
        {
          resetModes();
          drawingFlag = intData[1] == 2 || intData[1] == 3;
          runningFlag = intData[1] == 4;
          BTcontrol = true;
          FastLED.clear();
          FastLED.show();
        }        
        if (intData[1] == 0) { // ping
          sendAcknowledge();
        } else {              // vyžádat si parametry stránky aplikace
          sendPageParams(intData[1]);
        }
        break;
      case 19: 
         switch (intData[1]) {
           case 0:               // $ 19 0 N X; - uložit nastavení X „Hodiny v platnosti“ pro daný efektN
             saveEffectClock(intData[2], intData[3] == 1);
             break;
           case 1:               // $ 1 1 X; - uložit nastavení X "Clock in effects"
             overlayEnabled = ((CLOCK_ORIENT == 0 && allowHorizontal) || (CLOCK_ORIENT == 1 && allowVertical)) ? intData[2] == 1 : false;
             saveClockOverlayEnabled(overlayEnabled);
             break;
           case 2:               // $ 2 2 X; - Použijte synchronizaci hodin NTP X: 0 - ne, 1 - ano
             useNtp = intData[2] == 1;
             saveUseNtp(useNtp);
             if (wifi_connected) {
               refresh_time = true; ntp_t = 0; ntp_cnt = 0;
             }
             break;
           case 3:               // $ 19 3 N Z; - Čas a časová zóna synchronizace hodin NTP
             SYNC_TIME_PERIOD = intData[2];
             timeZoneOffset = (int8_t)intData[3];
             saveTimeZone(timeZoneOffset);
             saveNtpSyncTime(SYNC_TIME_PERIOD);
             saveTimeZone(timeZoneOffset);
             ntpSyncTimer.setInterval(1000 * 60 * SYNC_TIME_PERIOD);
             if (wifi_connected) {
               refresh_time = true; ntp_t = 0; ntp_cnt = 0;
             }
             break;
           case 4:              // $ 19 4 X; - Orientace hodin X: 0 - vodorovně, 1 - svisle
             CLOCK_ORIENT = intData[2] == 1 ? 1  : 0;             
             if (allowHorizontal || allowVertical) {
               if (CLOCK_ORIENT == 0 && !allowHorizontal) CLOCK_ORIENT = 1;
               if (CLOCK_ORIENT == 1 && !allowVertical) CLOCK_ORIENT = 0;              
             } else {
               overlayEnabled = false;
               saveClockOverlayEnabled(overlayEnabled);
             }             
             // Vycentrujte hodiny vodorovně / svisle podél šířky / výšky matice
             checkClockOrigin();
             saveClockOrientation(CLOCK_ORIENT);
             break;
             break;
           case 5:               // $ 19 5 X; - Barevný režim hodin X: 0 - vodorovně, 1 - svisle
             COLOR_MODE = intData[2];
             if (COLOR_MODE > 3) COLOR_MODE = 0;
             saveClockColorMode(COLOR_MODE);
             break;
           case 6:               // $ 19 6 X; - Zobrazit datum v režimu hodin X: 0 - ne, 1 - ano
             if (allowHorizontal || allowVertical) {
               showDateInClock = intData[2] == 1;
             } else {
               overlayEnabled = false;
               showDateInClock = false;
               saveClockOverlayEnabled(overlayEnabled);
             }
             setShowDateInClock(showDateInClock);
             break;
           case 7:               // $ 19 7 D I; - Doba zobrazení data / hodin (v sekundách)
             showDateDuration = intData[2];
             showDateInterval = intData[3];
             setShowDateDuration(showDateDuration);
             setShowDateInterval(showDateInterval);
             break;
           case 8:               // $ 19 8 RRRR MM DD HH MM; - Nastavte aktuální čas RRRR.MM.DD HH: MM
             setTime(intData[5],intData[6],0,intData[4],intData[3],intData[2]);
             init_time = true; refresh_time = false; ntp_cnt = 0;
             break;
        }
        sendAcknowledge();
        break;
      case 20:
        switch (intData[1]) { 
          case 0:  
            if (isAlarming || isPlayAlarmSound) {
              stopAlarm();            
            }
            break;
          case 2:
            #if (USE_MP3 == 1)
            if (isDfPlayerOk) {
              // $ 20 2 X DD VV MA MB;
               // X - použijte zvuk budíku X = 0 - ne, X = 1 - ano
               // DD - přehraje zvuk alarmu DD minut po alarmu
               // VV - maximální objem
               // MA - číslo zvukového souboru alarmu
               // MB - číslo zvukového souboru za úsvitu
              dfPlayer.stop();
              soundFolder = 0;
              soundFile = 0;
              isAlarming = false;
              isAlarmStopped = false;

              useAlarmSound = intData[2] == 1;
              alarmDuration = constrain(intData[3],1,10);
              maxAlarmVolume = constrain(intData[4],0,30);
               alarmSound = intData [5] - 2; // Index z aplikace: 0 - ne; 1 - náhodně; 2 - 1. soubor; 3 - ... -> -1 - ne; 0 - náhodně; 1 - 1. soubor atd.
               dawnSound = intData [6] - 2; // Index z aplikace: 0 - ne; 1 - náhodně; 2 - 1. soubor; 3 - ... -> -1 - ne; 0 - náhodně; 1 - 1. soubor atd.
              saveAlarmSounds(useAlarmSound, alarmDuration, maxAlarmVolume, alarmSound, dawnSound);
            }
            #endif
            break;
          case 3:
            #if (USE_MP3 == 1)
            if (isDfPlayerOk) {
              // 20 $ 3 X NN VV; - příklad zvukové signalizace
               // X - 1 play 0 - stop
               // NN - číslo zvukového souboru alarmu ze složky SD: / 01
               // VV - úroveň hlasitosti
              if (intData[2] == 0) {
                StopSound(0);
                soundFolder = 0;
                soundFile = 0;
              } else {
                b_tmp = intData[3] - 2; // Hodnoty: -1 - ne; 0 - náhodně; 1 a další - soubory; -> Indexy v seznamu: 1 - ne; 2 - náhodou; 3 dále - soubory
                if (b_tmp > 0 && b_tmp <= alarmSoundsCount) {
                  dfPlayer.stop();
                  soundFolder = 1;
                  soundFile = b_tmp;
                  dfPlayer.volume(constrain(intData[4],0,30));
                  dfPlayer.playFolder(soundFolder, soundFile);
                  dfPlayer.enableLoop();
                } else {
                  soundFolder = 0;
                  soundFile = 0;
                }
              }
            }
            #endif  
            break;
          case 4:
            #if (USE_MP3 == 1)
            if (isDfPlayerOk) {
            // 20 $ 4 X NN VV; - příklad zvuku úsvitu
              // X - 1 play 0 - stop
              // NN - číslo souboru zvuku úsvitu ze složky SD: / 02
              // VV - úroveň hlasitosti
              if (intData[2] == 0) {
                StopSound(0);
                soundFolder = 0;
                soundFile = 0;
              } else {
                dfPlayer.stop();
                b_tmp = intData[3] - 2; // Hodnoty: -1 - ne; 0 - náhodně; 1 a další - soubory; -> Indexy v seznamu: 1 - ne; 2 - náhodou; 3 dále - soubory
                if (b_tmp > 0 && b_tmp <= dawnSoundsCount) {
                  soundFolder = 2;
                  soundFile = b_tmp;
                  dfPlayer.volume(constrain(intData[4],0,30));
                  dfPlayer.playFolder(soundFolder, soundFile);
                  dfPlayer.enableLoop();
                } else {
                  soundFolder = 0;
                  soundFile = 0;
                }
              }
            }
            #endif
            break;
          case 5:
            #if (USE_MP3 == 1)
            if (isDfPlayerOk && soundFolder > 0) {
              // $ 5 5 VV; - nastavit úroveň hlasitosti pro příklady přehrávání (když již hrajete)
               // VV - úroveň hlasitosti
              maxAlarmVolume = constrain(intData[2],0,30);
              dfPlayer.volume(maxAlarmVolume);
            }
            #endif
            break;
        }
        if (intData[1] == 0) {
          sendPageParams(8);
        } else if (intData[1] == 1 || intData[1] == 2) { // Режимы установки параметров - сохранить
          // saveSettings();
          sendPageParams(8);
        } else {
          sendPageParams(96);
        }        
        break;
      case 21:
        // Nastavení síťového připojení
        switch (intData[1]) { 
          // $ 21 0 0 - nepoužívejte přístupový bod $ 21 0 1 - použijte přístupový bod
          case 0:  
            useSoftAP = intData[2] == 1;
            setUseSoftAP(useSoftAP);
            if (useSoftAP && !ap_connected) 
              startSoftAP();
            else if (!useSoftAP && ap_connected) {
              if (wifi_connected) { 
                ap_connected = false;              
                WiFi.softAPdisconnect(true);
                Serial.println(F("přistupový bod vyply"));
              }
            }      
            break;
          case 1:  
           // $ 21 1 IP1 IP2 IP3 IP4 - nastavení statické adresy IP pro připojení k místní síti WiFi, například: 21 1 192 168 0 106 $
             // Lokální síť - 10.x.x.x nebo 172.16.x.x - 172.31.x.x nebo 192.168.x.x
             // Je-li zadána jiná než lokální síťová adresa, resetujte ji na 0.0.0.0, což znamená příjem dynamické adresy 
            if (!(intData[2] == 10 || (intData[2] == 172 && intData[3] >= 16 && intData[3] <= 31) || (intData[2] == 192 && intData[3] == 168))) {
              intData[2] = 0;
              intData[3] = 0;
              intData[4] = 0;
              intData[5] = 0;
            }
            saveStaticIP(intData[2], intData[3], intData[4], intData[5]);
            break;
          case 2:  
           // $ 21 2; Znovu se připojit k síti Wi-Fi
            FastLED.clear();
            FastLED.show();
            startWiFi();
            showCurrentIP();
            break;
        }
        if (intData[1] == 0 || intData[1] == 1) {
          sendAcknowledge();
        } else {
          sendPageParams(9);
        }
        break;
      case 22:
      /* 22 - nastavení umožňující maticové režimy v určený čas
        - 22 $ HH1 MM1 NN1 HH2 MM2 NN2
            HHn - doba odezvy
            MMn - minuty odezvy
            NNn - účinek: -5 - vypnuto; -4 - vypněte matici; -3 - noční hodiny; -2 - krb s hodinami; -1 běžecká čára .. 0 - náhodný režim a dále v kruhu; 1 a další - seznam režimů ALARM_LIST 
      */    
        AM1_hour = intData[1];
        AM1_minute = intData[2];
        AM1_effect_id = intData[3];
        if (AM1_hour < 0) AM1_hour = 0;
        if (AM1_hour > 23) AM1_hour = 23;
        if (AM1_minute < 0) AM1_minute = 0;
        if (AM1_minute > 59) AM1_minute = 59;
        if (AM1_effect_id < -5) AM1_effect_id = -5;
        setAM1params(AM1_hour, AM1_minute, AM1_effect_id);
        AM2_hour = intData[4];
        AM2_minute = intData[5];
        AM2_effect_id = intData[6];
        if (AM2_hour < 0) AM2_hour = 0;
        if (AM2_hour > 23) AM2_hour = 23;
        if (AM2_minute < 0) AM2_minute = 0;
        if (AM2_minute > 59) AM2_minute = 59;
        if (AM2_effect_id < -5) AM2_effect_id = -5;
        setAM2params(AM2_hour, AM2_minute, AM2_effect_id);

        saveSettings();
        sendPageParams(10);
        break;
      case 23:
        // $ 23 0 VAL - aktuální limit spotřeby
        switch(intData[1]) {
          case 0:
            setPowerLimit(intData[2]);
            CURRENT_LIMIT = getPowerLimit();
            FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT == 0 ? 100000 : CURRENT_LIMIT);            
            break;
        }
        sendPageParams(1);
        break;
    }
    lastMode = intData[0];  // zapamatujte si předchozí režim
  }

  // ****************** Parsync *****************
  
 // Pokud předchozí vyrovnávací paměť ještě nebyla analyzována - nebudeme číst nová data ze soketu, pokračujte v analýze vyrovnávací paměti, která již byla přečtena.
  haveIncomeData = bufIdx > 0 && bufIdx < packetSize; 
  if (!haveIncomeData) {
    packetSize = udp.parsePacket();      
    haveIncomeData = packetSize > 0;      
  
    if (haveIncomeData) {                
      // read the packet into packetBuffer
      int len = udp.read(incomeBuffer, UDP_PACKET_MAX_SIZE);
      if (len > 0) {          
        incomeBuffer[len] = 0;
      }
      bufIdx = 0;
      
      delay(0);            // ESP8266, když volá zpoždění, zpracuje zásobník protokolu IP, nechme to fungovat        

      Serial.print(F("UDP пакeт размером "));
      Serial.print(packetSize);
      Serial.print(F(" от "));
      IPAddress remote = udp.remoteIP();
      for (int i = 0; i < 4; i++) {
        Serial.print(remote[i], DEC);
        if (i < 3) {
          Serial.print(F("."));
        }
      }
      Serial.print(F(", port "));
      Serial.println(udp.remotePort());
      if (udp.remotePort() == udp_port) {
        Serial.print(F("Data: "));
        Serial.println(incomeBuffer);
      }
    }

    // NTP packet from time server
    if (haveIncomeData && udp.remotePort() == 123) {
      parseNTP();
      haveIncomeData = false;
      bufIdx = 0;
    }
  }

  if (haveIncomeData) {         
    
   // Kvůli chybě komponenty UdpSender v Thunkable - polovina odeslaných je ztracena
     // znaků, pokud jejich kódování je dvoubajtový UTF8, protože vypočítá délku řetězce bez dvojbajtu
     // Aby nedošlo ke ztrátě znaků - při odesílání řetězce z programu Android získává mezery od konce
     // Zde musí být nejprve odstraněny tyto koncové mezery
    while (packetSize > 0 && incomeBuffer[packetSize-1] == ' ') packetSize--;
    incomeBuffer[packetSize] = 0;

    if (parseMode == TEXT) {                         // pokud potřebujete řetězec přijmout - přijměte celek

        // Převede zbývající buffer s řetězcem
        if (intData[0] == 5) {  // řetězec obrazku
          pictureLine = String(&incomeBuffer[bufIdx]);
        } else if (intData[0] == 6) {  // text
          receiveText = String(&incomeBuffer[bufIdx]);
          receiveText.trim();
        }
                  
        incomingByte = ending;                       // okunčuje parsync
        parseMode = NORMAL;
        bufIdx = 0; 
        packetSize = 0;                              // Veškere packety vstupujeticho souboru jsou opracovane
      } else { 
        incomingByte = incomeBuffer[bufIdx++];       // čteme vstupujici text bez vyjemek
      } 
   }       
  
  if (haveIncomeData) {

    if (parseStarted) {                                             // pokud je přijat, počáteční znak (analýza je povolena)
      if (incomingByte != divider && incomingByte != ending) {     // pokud to není mezera a ne konec
        string_convert += incomingByte;                             // přidat do řetězce
      } else {                                                      // pokud se jedná o mezeru nebo; konec balení
        if (parse_index == 0) {
          byte cmdMode = string_convert.toInt();
          intData[0] = cmdMode;
          if (cmdMode == 0) parseMode = COLOR;                      // přenos barev (do samostatné proměnné)
          else if (cmdMode == 6 || cmdMode == 5) {
            parseMode = TEXT;
          }
          else parseMode = NORMAL;
          // if (cmdMode != 7 || cmdMode != 0) runningFlag = false;
        }

        if (parse_index == 1) {      // pro druhý (od nuly) znak v balíčku
          if (parseMode == NORMAL) intData[parse_index] = string_convert.toInt();            // převeďte řetězec na int a vložte do pole}
          if (parseMode == COLOR) {                                                           // převede řetězec HEX na číslici
            setGlobalColor((uint32_t)HEXtoInt(string_convert));
            if (intData[0] == 0) {
              if (runningFlag && effectsFlag) effectsFlag = false;   
              incomingByte = ending;
              parseStarted = false;
              BTcontrol = true;
            } else {
              parseMode = NORMAL;
            }
          }
        } else {
          intData[parse_index] = string_convert.toInt(); // převeďte řetězec na int a vložte do pole
        }
        string_convert = "";                        // vyčistit řádek
        parse_index++;                              // přejít na analýzu dalšího prvku pole
      }
    }

    if (incomingByte == header) {                   // pokud je to $
      parseStarted = true; // zvedname flag, kterou lze analyzovat
       parse_index = 0; // resetování indexu
       string_convert = ""; // vyčistěte řádek
    }

   if (comingByte == ending) {// pokud ano; - konec analýzy
       parseMode = NORMAL;
       parseStarted = false; // reset
       recievedFlag = true; // příznak pro přijetí
       bufIdx = 0;
    }

    if (bufIdx >= packetSize) {                     // Všechna vyrovnávací paměť byla analyzována
      bufIdx = 0;
      packetSize = 0;
    }
  }
}

void sendPageParams(int page) {
  // W: šířka matice čísel
   // H: výška matice čísel
   // DM: X demo režim, kde X = 0 - vypnuto (ruční ovládání); 1 - zapnuto
   // AP: X režimy automatické změny, kde X = 0 - vypnuto; 1 - zapnuto
   // RM: X změna režimů v náhodném pořadí, kde X = 0 - vypnuto; 1 - zapnuto
   // PD: počet trvání režimu v sekundách
   // IT: počet nečinnosti v sekundách
   // BR: jas čísla
   // CL: HHHHHH aktuální barva výkresu, HEX
   // TX: [text] text, oddělovače [] jsou povinné
   // TS: X pohyblivy stav linky, kde X = 0 - vypnuto; 1 - zapnuto
   // ST: text rychlostí posunu čísla
 / EF: číslo aktuálního efektu
  // ES: X stav účinků, kde X = 0 - vypnuto; 1 - zapnuto
  // EC: X překrytí hodin pro efekt zapnutí / vypnutí, kde X = 0 - vypnuto; 1 - zapnuto
  // SE: počet efektů rychlosti
  // GM: aktuální číslo hry
  // GS: X stav hry, kde X = 0 - vypnuto; 1 - zapnuto
  // SG: číslo rychlosti hry
  // CE: X překrytí hodin zapnuto / vypnuto, kde X = 0 - vypnuto; 1 - zapnuto
  // CC: X barevný režim hodin: 0,1,2
  // CO: X orientace hodin: 0 - horizontálně, 1 - vertikálně
  // NP: X používá NTP, kde X = 0 - vypnuto; 1 - zapnuto
  // NT: počet period synchronizace NTP v minutách
  // NZ: počet časových pásem je -12 .. + 12
  // NS: [text] NTP server, oddělovače []
  // UT: X používá plíživou linii v demo režimu 0-ne, 1-ano
  // UE: X použije efekt v demo režimu 0-ne, 1-ano
  // UG: X používá hru v demo režimu 0-ne, 1-ano
  // DC: X zobrazuje datum s hodinami 0-ne, 1-ano
  // DD: kolikrát se datum zobrazí, když jsou zobrazeny hodiny (v sekundách)
  // DI: počet hodin k zobrazení data (v sekundách)
  // LG: [seznam] je vyžadován seznam her oddělených čárkami a oddělovači []
  // LE: [list] je vyžadován seznam efektů oddělených čárkami, oddělovače []
  // LA: [seznam] seznam efektů pro poplach oddělený čárkami, omezovače []
  // AL: X alarm byl vypnut 0-ne, 1-ano
  // AT: HH MM hodiny-minuty času budíku -> například „09 15“
  // AW: číselná bitová maska ​​dnů v týdnu poplachu b6..b0: b0 - Po .. b7 - Ne
  // AD: číslo trvání svítání, min
  // AE: počet efektů použitých pro poplach
  // AO: X aktivoval alarm 0-ne, 1-ano
  // NW: [text] SSID síťového připojení
  // NA: [text] heslo pro připojení k síti
  // AU: X vytvoří přístupový bod 0-ne, 1-ano
  // AN: [text] název přístupového bodu
  // AA: [text] heslo přístupového bodu
  // MX: X MP3 přehrávač k dispozici pro použití 0-ne, 1-ano
  // MU: X používá zvuk v alarmu 0-ne, 1-ano
  // MD: číslo, kolik minut zazní alarm, pokud nebyl vypnut
  // MV: číslo maximální hlasitosti alarmu
  // MA: číslo čísla zvukového souboru alarmu z SD: / 01
  // MB: číslo zvukového souboru úsvitu z SD: / 02
  // MP: složka Zadejte číslo složky a zvukového souboru, který se přehrává
  // BU: X používá automatický jas 0-ne, 1-ano
  // BY: počet minimálních hodnot chabosti během automatického nastavení
  // IP: xx.xx.xx.xx Aktuální IP adresa WiFi připojení v síti
  // AM1H: HH hodina zapnutí režimu 1 00..23
  // AM1M: MM minut startovacího režimu 1 00.,59
  // AM1E: NN číslo efektu režimu 1: -2 - nepoužito; -1 - vypněte matici; 0 - povolení náhodné s automatickou změnou; 1 - číslo režimu ze seznamu ALARM_LIST
  // AM2H: HH hodina pro startovací režim 2 00..23
  // AM2M: MM minut startovacího režimu 2 00..59
  // AM2E: NN číslo účinku režimu 1: -2 - nepoužito; -1 - vypněte matici; 0 - povolení náhodné s automatickou změnou; 1 - číslo režimu ze seznamu ALARM_LIST
  // PW: aktuální proudový limit v miliampérech
  // S1: [seznam] seznam zvuků poplachu oddělených čárkami, omezovače []
  // S2: [seznam] je vyžadován seznam zvuků úsvitu oddělených čárkami, omezovače []
  
  String str = "", color, text;
  boolean allowed;
  byte b_tmp;

  switch (page) { 
    case 1:  // Nastavení. Návrat: Šířka / Výška matice; Jas; Demorezhm a Auto-shift; Čas změny režimu
      str="$18 W:"+String(WIDTH)+"|H:"+String(HEIGHT)+"|DM:";
      if (BTcontrol)  str+="0|AP:"; else str+="1|AP:";
      if (AUTOPLAY)   str+="1|BR:"; else str+="0|BR:";
      str+=String(globalBrightness) + "|PD:" + String(autoplayTime / 1000) + "|IT:" + String(idleTime / 60 / 1000) +  "|AL:";
      if ((isAlarming || isPlayAlarmSound) && !isAlarmStopped) str+="1"; else str+="0";
      str+="|BU:" + String(useAutoBrightness ? "1" : "0");
      str+="|BY:" + String(autoBrightnessMin);
      str+="|RM:" + String(useRandomSequence);
      str+="|PW:" + String(CURRENT_LIMIT);
      str+=";";
      break;
    case 2:  // Malování. Návrat: Jas; Barva bodu;
      color = ("000000" + String(globalColor, HEX));
      color = color.substring(color.length() - 6); // FFFFFF             
      str="$18 BR:"+String(globalBrightness) + "|CL:" + color;
      str+="|BU:" + String(useAutoBrightness ? "1" : "0");    
      str+="|BY:" + String(autoBrightnessMin);       
      str+=";";
      break;
    case 3:  // Obrázek. Návrat: Jas;
      str="$18 BR:"+String(globalBrightness);
      str+="|BU:" + String(useAutoBrightness ? "1" : "0");    
      str+="|BY:" + String(autoBrightnessMin);       
      str+=";";
      break;
    case 4:  // Text. Návrat: Jas; Rychlost textu; Zapnuto vypnuto; Text; Použití v ukázce
      text = runningText;
      text.replace(";","~");
      str="$18 BR:"+String(globalBrightness) + "|ST:" + String(255 - constrain(map(scrollSpeed, D_TEXT_SPEED_MIN,D_TEXT_SPEED_MAX, 0, 255), 0,255)) + "|TS:";
      if (runningFlag)  str+="1|TX:["; else str+="0|TX:[";
      str += text + "]" + "|UT:";
      if (getUseTextInDemo())  str+="1"; else str+="0";
      str+="|BU:" + String(useAutoBrightness ? "1" : "0");    
      str+="|BY:" + String(autoBrightnessMin);       
      str+="|TM:" + String(textColorMode);       
      str+=";";
      break;
    case 5: // Efekty. Návrat: číslo efektu, zastavení nebo přehrávání; Jas; Efektová rychlost; Hodinky překrytí; Použití v ukázce
      allowed = false;
#if (OVERLAY_CLOCK == 1)      
      b_tmp = mapEffectToModeCode(effect); 
      if (b_tmp != 255) {
        for (byte i = 0; i < sizeof(overlayList); i++) {
          allowed = (b_tmp == overlayList[i]);
          if (allowed) break;
        }
      }
#endif      
      str="$18 EF:"+String(effect+1) + "|ES:";
      if (effectsFlag)  str+="1|BR:"; else str+="0|BR:";
      str+=String(globalBrightness);
      str+="|SE:" + (effect == EFFECT_CLOCK ? "X" : String(255 - constrain(map(effectSpeed, D_EFFECT_SPEED_MIN,D_EFFECT_SPEED_MAX, 0, 255), 0,255)));
      str+="|BU:" + String(useAutoBrightness ? "1" : "0");    
      str+="|BY:" + String(autoBrightnessMin);       
      if (!allowed || effect == EFFECT_CLOCK) 
          str+="|EC:X";  // X - parametr není použit (není použitelný)
      else    
          str+="|EC:" + String(getEffectClock(effect));
      if (getEffectUsage(effect))
          str+="|UE:1";
      else    
          str+="|UE:0";
      str+=";";
      break;
    case 6:  // Hry. Návrat: Číslo hry; Zapnuto vypnuto; Jas; Rychlost hry; Použití v ukázce
      str="$18 GM:"+String(game+1) + "|GS:";
      if (gamemodeFlag && !gamePaused)  str+="1|BR:"; else str+="0|BR:";
      str+=String(globalBrightness) + "|SG:" + String(255 - constrain(map(gameSpeed, D_GAME_SPEED_MIN,D_GAME_SPEED_MAX, 0, 255), 0,255)); 
      str+="|BU:" + String(useAutoBrightness ? "1" : "0");    
      str+="|BY:" + String(autoBrightnessMin);       
      str+="|UG:" + String(getGameUsage(game) ? "1" : "0");    
      str+=";";
      break;
    case 7:  // Nastavení hodin. Návrat: Overlay on / off
       // Lze zobrazit hodiny:
       // - vertikální s výškou matice> = 11 a šířkou> = 7;
       // - horizontální se šířkou matice> = 15 a výškou> = 5
       // Nastavení hodin lze zobrazit, pouze pokud jsou hodiny k dispozici ve velikosti: - vertikální nebo horizontální hodiny budou rušit matici
       // Nastavení orientace má smysl pouze tehdy, když lze na matici zobrazit jak horizontální, tak vertikální hodiny; Jinak to nedává smysl, protože volba je zřejmá (pouze jedna možnost)
      str="$18 CE:" + (allowVertical || allowHorizontal ? String(getClockOverlayEnabled()) : "X") + "|CC:" + String(COLOR_MODE) +
             "|CO:" + (allowVertical && allowHorizontal ? String(CLOCK_ORIENT) : "X") + "|NP:"; 
      if (useNtp)  str+="1|NT:"; else str+="0|NT:";
      str+=String(SYNC_TIME_PERIOD) + "|NZ:" + String(timeZoneOffset) + "|DC:"; 
      if (showDateInClock)  str+="1|DD:"; else str+="0|DD:";
      str+=String(showDateDuration) + "|DI:" + String(showDateInterval) + "|NS:["; 
      str+=String(ntpServerName)+"]";
      str+=";";
      break;
    case 8:  // Nastavení budiku ( v praci není použii, ale nevyčlenil jsem ho zduvodu že muže rozbit něco co funguje)
      str="$18 AL:"; 
      if ((isAlarming || isPlayAlarmSound) && !isAlarmStopped) str+="1|AD:"; else str+="0|AD:";
      str+=String(dawnDuration)+"|AW:";
      for (int i=0; i<7; i++) {
         if (((alarmWeekDay>>i) & 0x01) == 1) str+="1"; else str+="0";  
         if (i<6) str+='.';
      }
      for (int i=0; i<7; i++) {      
            str+="|AT:"+String(i+1)+" "+String(alarmHour[i])+" "+String(alarmMinute[i]);
      }
      str+="|AE:" + String(mapEffectToAlarm(alarmEffect) + 1); // Index v seznamu v aplikaci smartphonu začíná 1
       str + = "| MX:" + String (isDfPlayerOk? "1": "0"); // 1 - MP3 je k dispozici; 0 - MP3 není k dispozici
       #if (USE_MP3 == 1)
       str + = "| MU:" + String (useAlarmSound? "1": "0"); // 1 - použití zvuku; 0 - MP3 nepoužívá zvukк
      str+="|MD:" + String(alarmDuration); 
      str+="|MV:" + String(maxAlarmVolume); 
      if (soundFolder == 0) {      
        str+="|MA:" + String(alarmSound+2);                     // Hodnoty: -1 - ne; 0 - náhodně; 1 a další - soubory; -> Indexy v seznamu: 1 - ne; 2 - náhodou; 3 a dale soubory
        str+="|MB:" + String(dawnSound+2);                       // Hodnoty: -1 - ne; 0 - náhodně; 1 a další - soubory; -> Indexy v seznamu: 1 - ne; 2 - náhodou; 3 a dale soubory
      } else if (soundFolder == 1) {      
        str+="|MB:" + String(dawnSound+2);                      // Hodnoty: -1 - ne; 0 - náhodně; 1 a další - soubory; -> Indexy v seznamu: 1 - ne; 2 - náhodou; 3 a dale soubory
      } else if (soundFolder == 2) {      
        str+="|MA:" + String(alarmSound+2);                      /// Hodnoty: -1 - ne; 0 - náhodně; 1 a další - soubory; -> Indexy v seznamu: 1 - ne; 2 - náhodou; 3 a dale soubory
      }
      str+="|MP:" + String(soundFolder) + '~' + String(soundFile+2); 
      #endif
      str+=";";
      break;
    case 9:  // Nastavení připojení
      str="$18 AU:"; 
      if (useSoftAP) str+="1|AN:["; else str+="0|AN:[";
      str+=String(apName) + "]|AA:[";
      str+=String(apPass) + "]|NW:[";
      str+=String(ssid) + "]|NA:[";
      str+=String(pass) + "]|IP:";
      if (wifi_connected) str += WiFi.localIP().toString(); 
      else                str += String(F("Neni připojeni"));
      str+=";";
      break;
    case 10:  // Nastaveni automatickeho vypinani
      str="$18 AM1T:"+String(AM1_hour)+" "+String(AM1_minute)+"|AM1A:"+String(AM1_effect_id)+
             "|AM2T:"+String(AM2_hour)+" "+String(AM2_minute)+"|AM2A:"+String(AM2_effect_id); 
      str+=";";
      break;
#if (USE_MP3 == 1)
    case 93:  // žadani o seznam alarmových zvuků
      str="$18 S1:[" + String(ALARM_SOUND_LIST).substring(0,UDP_PACKET_MAX_SIZE-12) + "];"; 
      break;
    case 94:  // žadani o zvuky dawn
      str="$18 S2:[" + String(DAWN_SOUND_LIST).substring(0,UDP_PACKET_MAX_SIZE-12) + "];"; 
      break;
#endif      
    case 95:  // odpověd na stav budiku
      str = "$18 AL:"; 
      if ((isAlarming || isPlayAlarmSound) && !isAlarmStopped) str+="1;"; else str+="0;";
      cmd95 = str;
      break;
    case 96:  // odpověd demo režimu
      #if (USE_MP3 == 1)
      str ="$18 MP:" + String(soundFolder) + '~' + String(soundFile+2) + ";"; 
      cmd96 = str;
      #endif
      break;
    case 97:  // žadaní souboru budiku a efektu
      str="$18 LA:[" + String(ALARM_LIST).substring(0,UDP_PACKET_MAX_SIZE-12) + "];"; 
      break;
    case 98:  // žadaní o soubor her
      str="$18 LG:[" + String(GAME_LIST).substring(0,UDP_PACKET_MAX_SIZE-12) + "];"; 
      break;
    case 99:  // žadaní o soubor efektu
      str="$18 LE:[" + String(EFFECT_LIST).substring(0,UDP_PACKET_MAX_SIZE-12) + "];"; 
      break;
  }
  
  if (str.length() > 0) {
    // Odeslat požadované parametry stránky / režimu klientovi
    str.toCharArray(incomeBuffer, str.length()+1);    
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write((const uint8_t*) incomeBuffer, str.length()+1);
    udp.endPacket();
    delay(0);
    Serial.println(String(F("Ответ на ")) + udp.remoteIP().toString() + ":" + String(udp.remotePort()) + " >> " + String(incomeBuffer));
  } else {
    sendAcknowledge();
  }
}

void sendAcknowledge() {
  // Odeslat potvrzení, aby soket klienta přerušil čekání
  String reply = "";
  bool isCmd = false; 
  if (cmd95.length() > 0) { reply += cmd95; cmd95 = ""; isCmd = true;}
  if (cmd96.length() > 0) { reply += cmd96; cmd96 = ""; isCmd = true; }
  reply += "ack" + String(ackCounter++) + ";";  
  reply.toCharArray(replyBuffer, reply.length()+1);    
  udp.beginPacket(udp.remoteIP(), udp.remotePort());
  udp.write((const uint8_t*) replyBuffer, reply.length()+1);
  udp.endPacket();
  delay(0);
  if (isCmd) {
    Serial.println(String(F("Ответ на ")) + udp.remoteIP().toString() + ":" + String(udp.remotePort()) + " >> " + String(replyBuffer));
  }
}

void setSpecialMode(int spc_mode) {
        
  AUTOPLAY = false;
  BTcontrol = true;
  effectsFlag = true;
  gamemodeFlag = false;
  drawingFlag = false;
  runningFlag = false;
  loadingFlag = true;
  isNightClock = false;
  isTurnedOff = false;
  specialModeId = -1;

  String str;
  int8_t tmp_eff = -1;
  specialBrightness = globalBrightness;

  switch(spc_mode) {
    case 0:  // černa obrazovka vypnout;
      tmp_eff = EFFECT_FILL_COLOR;
      specialClock = false;
      setGlobalColor(0x000000);
      specialBrightness = 0;
      isTurnedOff = true;
      break;
    case 1:  // černa obrazovka a hodiny;  
      tmp_eff = EFFECT_FILL_COLOR;
      specialClock = true;
      setGlobalColor(0x000000);
      break;
    case 2:  // bíla barva na celý obraz;
      tmp_eff = EFFECT_FILL_COLOR;
      specialClock = false;
      setGlobalColor(0xffffff);
      break;
    case 3:  // bíla barva + hodiny ;
      tmp_eff = EFFECT_FILL_COLOR;
      specialClock = true;
      setGlobalColor(0xffffff);
      break;
    case 4:  // Černá obrazovka s hodinami minimálního jasu - noční režim;
      tmp_eff = EFFECT_FILL_COLOR;
      specialClock = true;
      setGlobalColor(0x000000);
      COLOR_MODE = 0; // Monochromatický, protože vše, co není nabílené, je minimální. jas černá, bílá - červená.
      specialBrightness = 1;
      isNightClock = true;
      break;
    case 5: // Černá obrazovka s účinkem ohně a hodin (krb);
      tmp_eff = EFFECT_FIRE;
      specialClock = true;
      break;
    case 6:  // Obrazovka zadané barvy;
      tmp_eff = EFFECT_FILL_COLOR;
      str = String(incomeBuffer).substring(6,12); // $14 6 00FFAA;
      specialClock = false;
      setGlobalColor((uint32_t)HEXtoInt(str));
      break;
    case 7:  // Obrazovka zadané barvy s hodinami;
      tmp_eff = EFFECT_FILL_COLOR;
      str = String(incomeBuffer).substring(6,12); // $14 6 00FFAA;
      specialClock = true;
      setGlobalColor((uint32_t)HEXtoInt(str));
      break;
    case 8:  // Светлячки
      tmp_eff = EFFECT_LIGHTERS;
      specialClock = true;
      break;
    case 9:  // Пейнтбол
      tmp_eff = EFFECT_PAINTBALL;
      specialClock = true;
      break;
  }

  if (tmp_eff >=0) {    
    // Zjistit, zda thisMode odpovídá zadanému efektu.
     // Další zobrazení efektu bude provedeno standardním postupem customRoutin ()
    byte b_tmp = mapEffectToMode(tmp_eff);
    if (b_tmp != 255) {
      effect = (byte)tmp_eff;
      thisMode = b_tmp;      
      specialMode = true;
      setTimersForMode(thisMode);
      // Časovač se vrátí do automatického režimu vypnutý    
      idleTimer.setInterval(4294967295);
      idleTimer.reset();
      FastLED.setBrightness(specialBrightness);
      specialModeId = spc_mode;
    }
  }  
  
  setCurrentSpecMode(spc_mode);
  setCurrentManualMode(-1);
}

void resetModes() {

 // Před zapnutím ostatních deaktivujte speciální režim
  specialMode = false;
  isNightClock = false;
  isTurnedOff = false;
  specialModeId = -1;

 // Zakázat VŠECHNY režimy: kreslení / text / efekty / hry  
  effectsFlag = false;        
  runningFlag = false;
  gamemodeFlag = false;
  drawingFlag = false;
  loadingFlag = false;
  controlFlag = false;

  breathBrightness = globalBrightness;
}

void setEffect(byte eff) {

  effect = eff;  
  resetModes();
  loadingFlag = true;
  effectsFlag = true;

 // Zjistit, zda thisMode odpovídá zadanému efektu.
  byte b_tmp = mapEffectToMode(effect);           
  if (b_tmp != 255) {
    thisMode = b_tmp;

    if (!AUTOPLAY) setCurrentManualMode((int8_t)thisMode);    
    setCurrentSpecMode(-1);
  
    if (!useAutoBrightness) 
      FastLED.setBrightness(globalBrightness);      
  }
    
  setTimersForMode(thisMode);
}

void startGame(byte game, bool newGame, bool paused) {
  // Найти соответствие thisMode указанной игре. 
  byte b_tmp = mapGameToMode(game);
  
  if (b_tmp != 255) {
    
    resetModes();
      
    gamemodeFlag = true;
    gamePaused = paused;  
      
    thisMode = b_tmp;

    if (!useAutoBrightness) 
      FastLED.setBrightness(globalBrightness);      

    if (!AUTOPLAY) setCurrentManualMode((int8_t)thisMode);
    setCurrentSpecMode(-1);    
  
    if (newGame) {        
      loadingFlag = true;                                                                  
      FastLED.clear(); 
      FastLED.show(); 
    }    
  } 

  setTimersForMode(thisMode);
}

void startRunningText() {
  runningFlag = true;  
  effectsFlag = false;
  
  // Pokud je barva textu černá, když zapnete režim Chodící linie, zapněte bílou, protože černá na černé není viditelná
  if (globalColor == 0x000000) setGlobalColor(0xffffff);

  if (!useAutoBrightness)
    FastLED.setBrightness(globalBrightness);    
    
  setCurrentSpecMode(-1);
  if (!AUTOPLAY) setCurrentManualMode((int8_t)thisMode);
  
  setTimersForMode(thisMode);
}

void showCurrentIP() {
  resetModes();          
  runningFlag = true;  
  BTcontrol = false;
  AUTOPLAY = true;
  wifi_print_ip = true;
  wifi_current_ip = wifi_connected ? WiFi.localIP().toString() : String(F("Нет подключения к сети WiFi"));
}

void setRandomMode2() {
  byte cnt = 0;
  while (cnt < 10) {
    cnt++;
    byte newMode = random(0, MODES_AMOUNT - 1);
    if (!getUsageForMode(newMode)) continue;

    byte tmp = mapModeToEffect(newMode);
    if (tmp <= MAX_EFFECT) {
      setEffect(tmp);
      break;
    } else {  
      tmp = mapModeToGame(newMode);
      if (tmp <= MAX_GAME) {
        startGame(tmp, true, false);
        break;
      } else if (newMode == DEMO_TEXT_0 || newMode == DEMO_TEXT_1 || newMode == DEMO_TEXT_2) {
        thisMode = newMode;
        loadingFlag = true;
        break;
      }
    }
  }
  if (cnt >= 10) {
    thisMode = 0;  
    setTimersForMode(thisMode);
    AUTOPLAY = true;
    autoplayTimer = millis();
  }
}
