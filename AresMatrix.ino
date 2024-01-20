


// ************************ Displej *************************

#define BRIGHTNESS 32         // Minimalní až maximalní jas (0-255)
uint16_t CURRENT_LIMIT=5000;  // Omezení proudu v mikroamperech  0 - vypnout omezení

#define DEVICE_TYPE 1         // Použity typ v praci je 1

// ******************* Připojení k siti *******************
                                             // Varovaní!!! Jestli měnite tyto parametry po nahravaní programu do Node mcu je za potřebí změnit hodnotu konstanty v EEprom jinak se změny neuloží
                                             
                                             
#define NETWORK_SSID ""                      // Jmeno sitě Wifi lze změnit i z aplikace 
#define NETWORK_PASS ""                      // Heslo pro sit Wifi
#define DEFAULT_AP_NAME "MatrixAP"           // Jmeno sitě před změnou  ( zakladní jmeno)
#define DEFAULT_AP_PASS "12341234"           // Heslo před změnou (zakladní heslo
#define udp_port 2390                        // Port na který se posilají Upd Packety
byte IP_STA[] = {192, 168, 0, 106};          // Staticka adresa lokalní  Wifi

#define DEFAULT_NTP_SERVER "ru.pool.ntp.org" // NTP Setver zakladně je "time.nist.gov" ale ja použivam tento jelikož zakladní mam na mobilu zablokovaný

// ****************** Piny pro připojení *******************



/*
 * NodeMCU v1.0 (ESP-12E)
 * Fizicke připojení :
 * Displej seriový, z leveho dolniho uhlu, logický pohyb doprava
 * Pin pro pás    - D2
 * Pin Tlačitko   - D6
 * Fotorezistor - A0
 * MP3Player    - D3 к RX, D4 na TX (zkoušel jsem zda je funkční library společně s budikem budik funguje nestabilně 
 * NodeMCU v1.0 (ESP-12E)
 */ 

#define WIDTH 16              // šiřka displeje
#define HEIGHT 16             // výška displeje
#define SEGMENTS 1            // počet diod pro jeden pixel 
#define MATRIX_TYPE 0         // typy 0 - Seriova , 1 - Paralelní
#define CONNECTION_ANGLE 0    // úhel připojení: 0 - vlevo dole, 1 - vlevo nahoře, 2 - vpravo nahoře, 3 - vpravo dole
#define STRIP_DIRECTION 0     // směr pásky od rohu: 0 - doprava, 1 - nahoru, 2 - doleva, 3 - dolů
#define USE_MP3 1             // nastavte 0 jestli nemate nebo nebudete použivat dtf player
#define USE_PHOTO 1           // nastavte 0 jetli neouživate fotorezistor
  
#define PHOTO_PIN 0           // Pin fotorezistoru
#define LED_PIN 2             // pin pasku
#define PIN_BTN 12            // pin tlačitka
#define SRX D4                // fizicky D4 is RX of ESP8266, connect to TX of DFPlayer
#define STX D3                // fizicky D3 is TX of ESP8266, connect to RX of DFPlayer module


 

// Připojeni Knihoven
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

// Připojení použitych knihoven
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include "FastLED.h"
#include "timerMinim.h"
#include "GyverButton.h"
#include "GyverFilters.h"

#if (USE_MP3 == 1)
#include "DFRobotDFPlayerMini.h"      // Nainstalujte standardní knihovnu DFRobotDFPlayerMini do správce knihovny („DFPlayer - mini MP3 přehrávač pro Arduino“)
#include <SoftwareSerial.h>          // Nainstalujte do správce knihovny "EspSoftwareSerial" pro ESP8266 / ESP32 https://github.com/plerup/espsoftwareserial/
#endif

#include "fonts.h"
#include "bitmap1.h"
//#include "bitmap2.h"
//#include "bitmap3.h"
//#include "bitmap4.h"
//#include "bitmap5.h"



#define COLOR_ORDER GRB       // pořadí barev na pásce. Pokud se barva nezobrazuje správně - změňte. Můžete začít s RGB


// ******************** Efekty a režimy ********************

#define EFFECT_LIST F("Sněžení, Míč, Duha, Paintball, Oheň, Matice, Koule, Hodiny, Starfall, Konfety, Diagonální duha, Barevný šum, Mraky, Láva, Plazma, Přetoky duhy, Pruhované přetoky, Zebra, Hlučný les, Moře, Fireflies, Whirlpool , Lampa, Dawn, Animace ") 
#define ALARM_LIST  F("Sněžení, Míč, Duha, Paintball, Oheň, Matice, Koule, Hodiny, Starfall, Konfety, Diagonální duha, Barevný šum, Mraky, Láva, Plazma, Přetoky duhy, Pruhované přetoky, Zebra, Hlučný les, Moře, Fireflies, Whirlpool , Lampa, Dawn, Animace ") 
#define GAME_LIST   F("Snake, Tetris, labyrint, běžec, Arkanoid")

#if (USE_MP3 == 1)

#define ALARM_SOUND_LIST F("One Step Over,In the Death Car")            // Seznam zvuků pro pole se seznamem „Zvuk alarmu“ v aplikaci na smartphonu
#define DAWN_SOUND_LIST F("Птицы,Гроза,Прибой,Дождь,Ручей,Мантра")      // Seznam zvuků pro rozbalovací seznam „Zvuk úsvitu“ v aplikaci ve vašem smartphonu
#endif

#define D_TEXT_SPEED 100      // výchozí rychlost běžícího textu (ms)
#define D_TEXT_SPEED_MIN 10
#define D_TEXT_SPEED_MAX 255

#define D_EFFECT_SPEED 80     // výchozí rychlost efektů (ms)
#define D_EFFECT_SPEED_MIN 0
#define D_EFFECT_SPEED_MAX 255

#define D_GAME_SPEED 250      // výchozí rychlost hry (ms)
#define D_GAME_SPEED_MIN 25
#define D_GAME_SPEED_MAX 375

#define D_GIF_SPEED 100       // Rychlost GIF (ms)
#define DEMO_GAME_SPEED 60    // rychlost her v demo režimu (ms)

boolean AUTOPLAY = 1;         // 0 vypnuto / 1 při automatické změně režimů (vypnout je možné ze smartphonu)
#define AUTOPLAY_PERIOD 30    // čas mezi změnami automatického režimu (sekundy)
#define IDLE_TIME 10          // čas nečinnosti tlačítek nebo Bluetooth (v minutách), po kterém začíná automatická změna režimů a ukázky ve hrách


#define GLOBAL_COLOR_1 CRGB::Green    // zakladní barva pro hry
#define GLOBAL_COLOR_2 CRGB::Orange   // zakladní barva pro hry 2 

#define SCORE_SIZE 0          // velikost znaků v herním režimu. 0 - malý pro 8x8 (písmo 3x5), 1 - velký (písmo 5x7)
#define FONT_TYPE 1           // (0/1) dva typy malých fontů na výstupu z herního režimu
#define SMOOTH_CHANGE 0     // Plynula změna režimů pomocí černé barvy

// ************** Odpojení požností pro testovaní na slabšich procesorech *************


// pozornost! Odpojení modulu neodebíra jeho účinky ze seznamu skladeb!
// Toto musí být provedeno ručně na vlastní kartě, čímž se odstraní zbytečné funkce

#define USE_NOISE_EFFECTS 1 // cool celoobrazovkové efekty (0 ne, 1 ano) Žerou pamet !!!
#define OVERLAY_CLOCK 1     // hodiny na pozadí všech efektů a her. Žere SRAM paměť!


// povolit / zakázat hry
#define USE_SNAKE 1         // hra had (0 ne, 1 ano)
#define USE_TETRIS 1        // hra tetris (0 ne, 1 ano)
#define USE_MAZE 1          // hra labirint (0 ne, 1 ano)
#define USE_RUNNER 1        // hraběhačka (0 ne, 1 ano)(netabilní)
#define USE_ARKAN 1         // hraarkanoid (nestabilní na velkych rychlostech)(0 ne, 1 ano)

// ******************************** Pro vývojaře XD ********************************

#define DEBUG 0
#define NUM_LEDS WIDTH * HEIGHT * SEGMENTS

CRGB leds[NUM_LEDS];


// Orientace zobrazení hodin
byte CLOCK_ORIENT = 0;    
// 0 horizontální, 1 vertikální


// Makro pro vycentrování zobrazení hodin na displeji
#define CLOCK_X_H (byte((WIDTH - (4*3 + 3*1)) / 2.0 + 0.51))  // 4 číslice * (písmo široké 3 px) 3 + mezera mezi číslicemi), / 2 - do středu
#define CLOCK_Y_H (byte((HEIGHT - (1*5)) / 2.0 + 0.51))      // Jeden řádek číslic vysoký 5 pixelů / 2 - do středu
#define CLOCK_X_V (byte((WIDTH - (2*3 + 1)) / 2.0 + 0.51))    // 2 číslice * (písmo široké 3 px) 1 + mezera mezi číslicemi) / 2 - do středu
#define CLOCK_Y_V (byte((HEIGHT - (2*5 + 1)) / 2.0 + 0.51))   // Dva řádky číslic vysoké 5 pixelů + 1 ​​mezera mezi řádky / 2 - do středu


// Makro pro vycentrování zobrazení kalendáře na displeji (nestabilní)
#define CAL_X (byte((WIDTH - (4*3 + 1)) / 2.0))               // 4 číslice * (písmo široké 3 pixely) 1 + mezera mezi číslicemi) / 2 - do středu
#define CAL_Y (byte((HEIGHT - (2*5 + 1)) / 2.0))              // Dva řádky číslic vysoké 5 pixelů + 1 ​​mezera mezi řádky / 2 - do středu


// Pozice zobrazení hodin a kalendáře
byte CALENDAR_X = CAL_X;
byte CALENDAR_Y = CAL_Y;
byte CLOCK_X = CLOCK_X_H;     // Pro vertikální hodiny CLOCK_X_V a CLOCK_Y_V
byte CLOCK_Y = CLOCK_Y_H;


// Lze zobrazit hodiny:
// - vertikální s výškou displeje> = 11 a šířkou> = 7;
// - horizontální se šířkou displeje> = 15 a výškou> = 5
bool allowVertical = WIDTH >= 7 && HEIGHT >= 11;
bool allowHorizontal = WIDTH >= 15 && HEIGHT >= 7;


// Barevný režim hodin
byte COLOR_MODE = 0;
//                              0 - monochromatické barvy definované pro režimy NORMAL_CLOCK_COLOR, CONTRAST_COLOR_1, CONTRAST_COLOR_2, CONTRAST_COLOR_3 v hodinách.ino
//                              1 - posun duhy (každá číslice)
//                              2 - změna duhy (hodiny, body, minuty)
//                              3 - určené barvy (hodiny, body, minuty) - HOUR_COLOR, DOT_COLOR, MIN_COLOR v hodinách.ino

bool useRandomSequence = true;   // Použijte náhodné pořadí efektů


// Pořadí efektů a her v demo režimu (viz customModes () v custom.ino) ID od 0 do MODES_AMOUNT-1
// Pokud chcete změnit pořadí efektů v demo režimu - (stejně jako režimy přidání / odebrání) - upravit
// sekvence zde. Dostupnost - zde a v customModes () v custom.ino
#define DEMO_TEXT_0              0
#define DEMO_TEXT_1              1
#define DEMO_TEXT_2              2
#define DEMO_NOISE_MADNESS       3
#define DEMO_NOISE_CLOUD         4
#define DEMO_NOISE_LAVA          5
#define DEMO_NOISE_PLASMA        6
#define DEMO_NOISE_RAINBOW       7
#define DEMO_NOISE_RAINBOW_STRIP 8
#define DEMO_NOISE_ZEBRA         9
#define DEMO_NOISE_FOREST       10
#define DEMO_NOISE_OCEAN        11
#define DEMO_SNOW               12
#define DEMO_SPARKLES           13
#define DEMO_MATRIX             14
#define DEMO_STARFALL           15
#define DEMO_BALL               16
#define DEMO_BALLS              17
#define DEMO_RAINBOW            18
#define DEMO_RAINBOW_DIAG       19
#define DEMO_FIRE               20
#define DEMO_LIGHTERS           21
#define DEMO_PAINTBALL          22
#define DEMO_SWIRL              23
#define DEMO_SNAKE              24
#define DEMO_TETRIS             25
#define DEMO_MAZE               26
#define DEMO_RUNNER             27
#define DEMO_ARKANOID           28
#define DEMO_CLOCK              29
#define DEMO_FILL_COLOR         30  // displej jednou barvou
#define DEMO_DAWN_ALARM         31  // pokus o budik s světelnou animaci
// ---------------------------------
#define DEMO_ANIMATION_1        32
//#define DEMO_ANIMATION_2        33
//#define DEMO_ANIMATION_3        34
//#define DEMO_ANIMATION_4        35
//#define DEMO_ANIMATION_5        36


// Nezapomeňte zadat počet režimů pro správné přepínání z posledního na první
// počet vlastních režimů (které jsou přepínány samostatně nebo tlačítkem)
#define MODES_AMOUNT 33 // 37

// ---------------------------------
#define DEMO_DAWN_ALARM_SPIRAL 253  
#define DEMO_DAWN_ALARM_SQUARE 254  
// ---------------------------------

// Nepřetržité číslování (ID) efektů ve skupině efektů
#define EFFECT_SNOW                 0
#define EFFECT_BALL                 1
#define EFFECT_RAINBOW              2
#define EFFECT_PAINTBALL            3
#define EFFECT_FIRE                 4
#define EFFECT_MATRIX               5
#define EFFECT_BALLS                6
#define EFFECT_CLOCK                7
#define EFFECT_STARFALL             8
#define EFFECT_SPARKLES             9
#define EFFECT_RAINBOW_DIAG        10
#define EFFECT_NOISE_MADNESS       11
#define EFFECT_NOISE_CLOUD         12
#define EFFECT_NOISE_LAVA          13
#define EFFECT_NOISE_PLASMA        14
#define EFFECT_NOISE_RAINBOW       15
#define EFFECT_NOISE_RAINBOW_STRIP 16
#define EFFECT_NOISE_ZEBRA         17
#define EFFECT_NOISE_FOREST        18
#define EFFECT_NOISE_OCEAN         19
#define EFFECT_LIGHTERS            20
#define EFFECT_SWIRL               21
#define EFFECT_FILL_COLOR          22
#define EFFECT_DAWN_ALARM          23
#define EFFECT_ANIMATION_1         24
//#define EFFECT_ANIMATION_2         25
//#define EFFECT_ANIMATION_3         26
//#define EFFECT_ANIMATION_4         27
//#define EFFECT_ANIMATION_5         28

#define MAX_EFFECT                 25 // 29 // počet efektů definovaných ve firmwaru
#define MAX_SPEC_EFFECT            10       // počet efektů zástupce -> 0..9


// Nepřetržité číslování (ID) her ve skupině
#define GAME_SNAKE               0
#define GAME_TETRIS              1
#define GAME_MAZE                2
#define GAME_RUNNER              3
#define GAME_ARKANOID            4

#define MAX_GAME                 5         // počet her definovaných ve firmwaru


// effect type ID (typ skupiny je text, hry mají jedno ID typu pro všechny podtypy)
#define MC_TEXT                  0
#define MC_CLOCK                 1
#define MC_GAME                  2
#define MC_NOISE_MADNESS         3
#define MC_NOISE_CLOUD           4
#define MC_NOISE_LAVA            5
#define MC_NOISE_PLASMA          6
#define MC_NOISE_RAINBOW         7
#define MC_NOISE_RAINBOW_STRIP   8
#define MC_NOISE_ZEBRA           9
#define MC_NOISE_FOREST         10
#define MC_NOISE_OCEAN          11
#define MC_SNOW                 12
#define MC_SPARKLES             13
#define MC_MATRIX               14
#define MC_STARFALL             15
#define MC_BALL                 16
#define MC_BALLS                17
#define MC_RAINBOW              18
#define MC_RAINBOW_DIAG         19
#define MC_FIRE                 20
#define MC_DAWN_ALARM           21
#define MC_FILL_COLOR           22
#define MC_IMAGE                23
#define MC_PAINTBALL            24
#define MC_SWIRL                25
#define MC_LIGHTERS             26


// Typy efektů (viz výše), ve kterých mohou být hodiny zobrazeny v překryvu
#if (OVERLAY_CLOCK == 1)
byte overlayList[] = {
  MC_NOISE_MADNESS,
  MC_NOISE_CLOUD,
  MC_NOISE_LAVA,
  MC_NOISE_PLASMA,
  MC_NOISE_RAINBOW,
  MC_NOISE_RAINBOW_STRIP,
  MC_NOISE_ZEBRA,
  MC_NOISE_FOREST,
  MC_NOISE_OCEAN,
  MC_SNOW,
  MC_SPARKLES,
  MC_MATRIX,
  MC_STARFALL,
  MC_BALL,
  MC_BALLS,
  MC_RAINBOW,
  MC_RAINBOW_DIAG,
  MC_FIRE,
  MC_PAINTBALL,
  MC_SWIRL,
  MC_LIGHTERS,
  MC_DAWN_ALARM,
  MC_FILL_COLOR,
};
#endif

// ********************** Globalní proměnne **********************


// Korespondence indexů seznamu účinků alarmu ALARM_LIST s indexy seznamu existujících efektů EFFECT_LIST
const byte ALARM_LIST_IDX[] PROGMEM = {EFFECT_SNOW, EFFECT_BALL, EFFECT_RAINBOW, EFFECT_PAINTBALL, EFFECT_FIRE, EFFECT_MATRIX, EFFECT_BALLS,
                                       EFFECT_STARFALL, EFFECT_SPARKLES, EFFECT_RAINBOW_DIAG, EFFECT_NOISE_MADNESS, EFFECT_NOISE_CLOUD,
                                       EFFECT_NOISE_LAVA, EFFECT_NOISE_PLASMA, EFFECT_NOISE_RAINBOW, EFFECT_NOISE_RAINBOW_STRIP,
                                       EFFECT_NOISE_ZEBRA, EFFECT_NOISE_FOREST, EFFECT_NOISE_OCEAN, EFFECT_LIGHTERS, EFFECT_SWIRL,
                                       EFFECT_DAWN_ALARM, EFFECT_ANIMATION_1
                                       //, EFFECT_ANIMATION_2, EFFECT_ANIMATION_3, EFFECT_ANIMATION_4, EFFECT_ANIMATION_5
                                      };

// ---- Подключение к сети

WiFiUDP udp;

bool useSoftAP = false;            // použivat režim přístupového bodu
bool wifi_connected = false;       // true - připojení k WiFi síti je dokončeno
bool ap_connected = false;         // true - pracujeme v režimu přístupového bodu;
bool wifi_print_ip = false;        // Jako běžicí text se zobrazuje ip přistupoveho bodu
String wifi_current_ip = "";


// Buffer pro načítání jmen / hesel z EEPROM aktuálního jména / hesla síťového připojení / přístupového bodu
// Varování: do délky +1 bajtů na \ 0 je znak terminálu.
char apName[11] = "";                    // Název sítě v režimu přístupového bodu
char apPass[17]  = "";                   // Heslo pro připojení k přístupovému bodu
char ssid[25] = "";                      // SSID (název) routeru (nakonfigurováno připojením přes přístupový bod a uložením do EEPROM)
char pass[17] = "";                      // heslo routeru

// ---- Synchronizace hodin se serverem NTP

IPAddress timeServerIP;
#define NTP_PACKET_SIZE 48               // NTP čas v prvních 48 bajtech zprávy
uint16_t SYNC_TIME_PERIOD = 60;          // Období synchronizace v minutách
byte packetBuffer[NTP_PACKET_SIZE];      // vyrovnávací paměť pro ukládání příchozích a odchozích paketů

int8_t timeZoneOffset = 7;               // posunutí časového pásma od UTC
long ntp_t = 0;                          // Čas, který uplynul od vyžádání dat ze serveru NTP (časový limit)
byte ntp_cnt = 0;                        // Počítadlo pokusů o příjem dat ze serveru
bool init_time = false;                  // Označení false - čas není inicializován; je inicializován skutečný čas
bool refresh_time = true;                // Označení true - přišel čas synchronizovat čas se servrem NTP
bool useNtp = true;                      // Použivat časovou synchronizaci s NTP serverem
char ntpServerName[31] = "";             // Použitý server NTP


// ---- Hodiny a budík (budik se v maturitní praci nepouživa ale jadro je naprogramovane)

bool isAlarming = false;           // zapnul se budik "dawn"
bool isPlayAlarmSound = false;     // zapnul se zvuk budiku
bool isAlarmStopped = false;       // budik "dawn" byl vypnióut uživatelm

byte alarmWeekDay = 0;             // Bitová maska ​​dnů v týdnu budiku
byte alarmDuration = 1;            // doba trvaní budiku po efektu dawn

byte alarmHour[7]   = {0, 0, 0, 0, 0, 0, 0}; // hodinove rozmezi budiku
byte alarmMinute[7] = {0, 0, 0, 0, 0, 0, 0}; // minutove rozmezi

int8_t dawnHour = 0; // Sledujte čas zahájení dawn
int8_t dawnMinute = 0; // minuty do dawn
byte dawnWeekDay = 0; // Den v týdnu, čas zahájení dawn (0 - vypnuto, 1,7 - po .. slunce)
byte dawnDuration = 0; // Trvání „dawn“ podle nastavení
byte realDawnDuration = 0; // Doba trvání „dawn“ podle vypočítané doby budíku
byte maxAlarmVolume = 30; // Maximální hlasitost alarmu (1..30)
byte alarmEffect = EFFECT_DAWN_ALARM; // Jaký efekt se používá pro budik za svítání. Mohou existovat běžné efekty - jejich jas se bude postupně zvyšovat


boolean showDateInClock = true; // Zobrazit datum při zobrazování hodin
byte showDateDuration = 5; // po dobu 5 sekund
byte showDateInterval = 20; // každých 20 sekund
byte showDateState = false; // false - zobrazí se hodiny; true - datum se zobrazí
long showDateStateLastChange = 0; // Čas, kdy se zobrazení hodin změnilo na zobrazení kalendáře a naopak
bool overlayEnabled = true; // Nechte hodiny překrývat efekty

int8_t hrs = 0, mins = 0, secs = 0, aday = 1, amnth = 1;
int16_t ayear = 1970;
boolean dotFlag;                   // Příznak pro kreslení oddělovacích bodů v hodinách

// ---- Speciální režimy (když jsou zapnuty speciální režimy, je zakázána automatická změna režimů a ukázky)


int8_t specialModeId = -1; // Číslo aktuálního zvláštního režimu
bool specialMode = false; // Speciální režim, povolený ručně ze smartphonu nebo z tlačítek rychlého režimu
bool specialClock = false; // Speciální režim používá překryvné hodiny
byte specialBrightness = 1; // Jas ve zvláštním režimu
bool isNightClock = false; // Je zapnutý noční režim s minimálním jasem
bool isTurnedOff = false; // Černá obrazovka je zapnutá (tj. Vše je vypnuto)


// ---- Automatické ovládání jasu (jadro je naprogramovane v maturitní práci se nepouživa)

GFilterRA brightness_filter; // filtr pro plynulou změnu jasu
bool useAutoBrightness = false; // Používat automatické ovládání jasu
byte autoBrightnessMin = 1; // Minimální úroveň jasu s automatickým nastavením

// ---- Režimy zahrnuté v daném čase
bool AM1_running = false; // Režim 1 podle času - funguje
byte AM1_hour = 0; // Režim 1 podle času - hodiny
byte AM1_minute = 0; // Režim 1 podle času - minuty
int8_t AM1_effect_id = -5; // Režim 1 podle ID časového efektu nebo -5 - vypnuto; -4 - vypnuty obraz (černá obrazovka); -3 - noční hodiny, -2 - krb s hodinami, -1 - plazivá linka, 0 - náhodná 1 a poté - efekt ALARM_LIST
bool AM2_running = false; // Režim 2 podle času - funguje
byte AM2_hour = 0; // Režim 2 - hodiny
byte AM2_minute = 0; // Režim 2 podle času - minuty
int8_t AM2_effect_id = -5; // Režim 2 podle ID časového efektu nebo -5 - vypnuto; -4 - vypnuty obraz (černá obrazovka); -3 - noční hodiny, -2 - krb s hodinami, -1 - plazivá linka, 0 - náhodná 1 a poté - efekt ALARM_LIST
// ---- Časovače

uint32_t idleTime = ((long)IDLE_TIME * 60 * 1000);      // minuty -> milisekundy
uint32_t autoplayTime = ((long)AUTOPLAY_PERIOD * 1000); // minuty -> milisekundy
uint32_t autoplayTimer;

timerMinim effectTimer (D_EFFECT_SPEED); // Časovač rychlosti efektu (krok provedení efektu)
timerMinim gameTimer (DEMO_GAME_SPEED); // Časovač rychlosti hry (krok hry)
timerMinim scrollTimer (D_TEXT_SPEED); // Časovač efektu řádkování
timerMinim changeTimer (70); // Fade Timer - Fade
timerMinim halfsecTimer (500); // Půlsekundový časový spínač hodin
timerMinim idleTimer (idleTime); // Manuální časovač neaktivity pro automatický přechod a demo režim
timerMinim alarmSoundTimer (4294967295); // Časovač vypnutí zvuku
timerMinim fadeSoundTimer (4294967295); // Časovač pro plynulý zvuk zapnuto / vypnuto
timerMinim autoBrightnessTimer (500); // Časovač pro sledování údajů světelného senzoru, když je zapnutý automatický jas matice
timerMinim saveSettingsTimer (15000); // Nastavení časovače
timerMinim ntpSyncTimer (1000 * 60 * SYNC_TIME_PERIOD); // Časovač pro synchronizaci času se serverem NTP
timerMinim dawnTimer (4294967295); // Časovač kroku Dawn pro alarm úsvitu

// ---- MP3 přehrávač pro přehrávání zvukových alarmů

#if (USE_MP3 == 1)


#if defined (ESP8266)
// SoftwareSerial mp3Serial (SRX, STX); // Tuto volbu použijte, pokud máte jádrovou knihovnu ESP8266 verze 2.5.2
  SoftwareSerial mp3Serial; // Tuto volbu použijte, pokud máte jádrovou knihovnu ESP8266 verze 2.6
#endif
#if defined(ESP32)
  SoftwareSerial mp3Serial;
#endif

DFRobotDFPlayerMini dfPlayer;

int16_t alarmSoundsCount = 0; // Počet zvukových souborů ve složce „01“ na SD kartě
int16_t dawnSoundsCount = 0; // Počet zvukových souborů ve složce „02“ na SD kartě
byte soundFolder = 0;
byte soundFile = 0;
int8_t fadeSoundDirection = 1; // směr změny hlasitosti zvuku: 1 - zvýšení; -1 - pokles
byte fadeSoundStepCounter = 0; // počítadlo kroků ke změně objemu, který je třeba ještě provést
bool useAlarmSound = false; // Používejte zvuky v budíku
int8_t alarmSound = 0; // Zvuk alarmu - číslo souboru ve složce SD: / 01 [-1 nepoužívat; 0 - náhodně; 1..N] číslo souboru
int8_t dawnSound = 0; // Zvuk úsvitu - číslo souboru ve složce SD: / 02 [-1 nepoužívat; 0 - náhodně; 1..N] číslo souboru
#endif

bool isDfPlayerOk = false;


// ---- Ovládací tlačítko fyzického režimu (jadro naprogramovano a otestovano, ale v praci se nepouživa)

//GButton butt(PIN_BTN, LOW_PULL, NORM_OPEN); // pro dotykove tlačitko
GButton butt(PIN_BTN, HIGH_PULL, NORM_OPEN);  // pro normalní tlačitko


#define HOLD_TIMEOUT 2000 // Doba, po kterou je tlačítko drženo před akcí (+ doba odkladu) celkem, je asi 3 sekundy
bool isButtonHold = false; // Tlačítko je stisknuté a přidržené
long hold_start_time = 0; // Čas pro zjištění stavu „Tlačítko je stisknuto a přidrženo“


// ---- Jiné proměnné


byte globalBrightness = BRIGHTNESS; // Aktuální jas
byte dechBrightness; // Jas efektu „Breath“
uint32_t globalColor = 0xffffff; // Barva výkresu při spuštění je bílá


int scrollSpeed ​​= D_TEXT_SPEED; // rychlost posouvání plíživého textu řádku
int gameSpeed ​​= DEMO_GAME_SPEED; // rychlost ve hrách
int effectSpeed ​​= D_EFFECT_SPEED; // rychlost změny efektu

boolean BTcontrol = false; // flag: true - ruční ovládání efektů a her; false - v režimu automatického přehrávání
boolean loadingFlag = true; // flag: inicializace parametrů režimu
boolean runningFlag; // flag: aktuální režim manuální režim - plíživá linka
boolean drawingFlag; // flag: aktuální režim v manuálním režimu - kresba nebo obrázek na matici
booleovské efektyFlag; // flag: aktuální režim v manuálním režimu - efekty
boolean gamemodeFlag = false; // flag: aktuální režim - hra
boolean controlFlag = false; // flag: ovládání hry zachycené pomocí tlačítek nebo smartphonu
boolean gamePaused; // flag: hra je pozastavena
boolean gameDemo = true; // flag: přehrávání v demo režimu
boolean idleState = true; // příznak nečinnosti
boolean fullTextFlag = false; // příznak: text plíživé linie je zobrazen v plném rozsahu (řádek utekl)
boolean eepromModified = false; // flag: data uložená v paměti EEPROM byla změněna a je třeba je uložit
byte game; // index aktuální hry v manuálním režimu
byte effect; // index aktuálního efektu v manuálním režimu

byte modeCode;                     // typ aktuálního efektu: 0 plíživou linii, 1 hodiny, 2 hry, 3 nois šílenství dále, 21 GIF nebo obrázek,
byte thisMode = 0;                 // aktuální režim
byte frameNum;                     // Číslo snímku při přehrávání animace
byte buttons = 4;                 // Ovládání hry: tlačítko stisknuto: 0 - nahoru, 1 - vpravo, 2 - dole, 3 - vlevo, 4 - nestisknuto
#if (SMOOTH_CHANGE == 1)
byte fadeMode = 4;                 // fázová hladká změna režimu
boolean modeDir;                   // směr: útlum / objevení
#endif


// Text pro demo režimy běžicí linky
String TEXT_1 = "";                // Řetězec dané barvy
String TEXT_2 = "";                // Řetězec s hladkou změnou barvy
String TEXT_3 = "";                // Řetězec s různobarevnými písmeny
String runningText = "";           // Text ktery již běží a je zadaný ze smartfonu
byte textColorMode = 0;            // Režim zobrazení barvy běžecké čáry: 0 - globalColor (monochrome); 1 - duha; 2 - barevná písmena;

// Server nemůže zahájit odesílání zprávy klientovi - pouze na základě žádosti klienta
// Následující dvě proměnné ukládají zprávy generované serverem a jsou odesílány v reakci na nejbližší požadavek klienta,
// například v reakci na periodický ping - v příkazu sendAcknowledge ();
String cmd95 = "";                 // Řetězec vytvořený pomocí sendPageParams (95) k odeslání z iniciativy serveru
String cmd96 = "";                // Řetězec vytvořený pomocí sendPageParams (96) k odeslání z iniciativy serveru

static const byte maxDim = max(WIDTH, HEIGHT);

void setup() {

  // Watcdog Timer - 8 sekund
#if defined(ESP8266)
  ESP.wdtEnable(WDTO_8S);
#endif

  // spouštění serioveho portu
  Serial.begin(115200);
  delay(10);

 

  // spouštění  EEPROM a načitaní uloženych parametru
  EEPROM.begin(512);
  loadSettings();

  // Spouštíme generator nahodnyhc čísel
  randomSeed(analogRead(1));

  // první etapa spouštění přehravače
  #if (USE_MP3 == 1)
    InitializeDfPlayer1();
  #endif

  // WiFi vždy zaplý
  #if defined(ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
  #endif

  // připojit se ksití / vytvořit přístupový bod
  connectToNetwork();

  // UDP-client navybranem portu
  udp.begin(udp_port);

  // časoač čekaní
  idleTimer.setInterval(idleTime == 0 ? 4294967295 : idleTime);

  // nastavení pásky
  FastLED.addLeds<WS2812, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(globalBrightness);
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();

  // Druha etapa spouštění přehravače
  #if (USE_MP3 == 1)
    InitializeDfPlayer2();
    if (!isDfPlayerOk) {
      Serial.println(F("MP3 přehravač není dostupný."));
    }
  #endif

  // Kontrola rozměru pásky a rozměru hodin
  checkClockOrigin();

  /*
    butt.setDebounce(50);        // nastavení antí otřesove (v zakladu 80 milisekund)
    butt.setTimeout(300);        // nastavení na dobu zmačknutí (vzakladu 500 milisekund)
    butt.setIncrStep(2);         // nastavení přírůstku, může být záporné (výchozí je 1)
    butt.setIncrTimeout(500);    // nastavení intervalu přírůstku (výchozí je 800 ms)
  */
// Text pro demo režimy běžicí linky
  TEXT_1 = String(F("Matrix na adresovych LED"));              // Řetězec dané barvy
  TEXT_2 = String(F("Vesele vanoce"));                        // Řetězec s hladkou změnou barvy
  TEXT_3 = String(F("Vsechno nejlepši!"));                    // Řetězec s různobarevnými písmeny

// Nastavte koeficient filtru (0,0 ... 1,0). Čím menší je tím hladší filtr
  brightness_filter.setCoef(0.5);

// Pokud byl speciální režim nastaven během předchozí relace matice, povolte ji
// Číslo speciálního režimu je zapamatováno, když je zapnuto, a je resetováno, když je zapnutý normální režim nebo hra
// Toto umožňuje v případě náhlého restartu matice (například wdt), když byl aktivován speciální režim (například noční hodiny nebo off matrix)
// zapněte jej znovu a po zapnutí matice se nezobrazí náhodně náhodně
  int8_t spc_mode = getCurrentSpecMode();
  if (spc_mode >= 0 && spc_mode < MAX_SPEC_EFFECT)
    setSpecialMode(spc_mode);
  else {
    int8_t m_mode = getCurrentManualMode();
    if (m_mode < 0 || AUTOPLAY) {
      setRandomMode2();
    } else {
      thisMode = m_mode;
      while (1) {
        // Pokud je režim označen příkazem „use“ - použijte, jinak vezmeme další
        if (getUsageForMode(thisMode)) break;
        thisMode++;
        if (thisMode >= MODES_AMOUNT) {
          thisMode = 0;
          break;
        }
      }
      setModeByModeId(thisMode); 
      autoplayTimer = millis();
    }
  }
}

void loop() {
  bluetoothRoutine();
}

// -----------------------------------------

void startWiFi() {

  WiFi.disconnect(true);
  wifi_connected = false;
  WiFi.mode(WIFI_STA);

  // Snažime se připojit s routerem k sití
  if (strlen(ssid) > 0) {
    Serial.print(F("\nPřipojení k "));
    Serial.print(ssid);

    if (IP_STA[0] + IP_STA[1] + IP_STA[2] + IP_STA[3] > 0) {
      WiFi.config(IPAddress(IP_STA[0], IP_STA[1], IP_STA[2], IP_STA[3]),  // 192.168.0.106
                  IPAddress(IP_STA[0], IP_STA[1], IP_STA[2], 1),          // 192.168.0.1
                  IPAddress(255, 255, 255, 0),                            // Mask
                  IPAddress(IP_STA[0], IP_STA[1], IP_STA[2], 1),          // DNS1 192.168.0.1
                  IPAddress(8, 8, 8, 8));                                 // DNS2 8.8.8.8                  
    }
    WiFi.begin(ssid, pass);

    // Testovaní připojení (ČASOVAČ 10 sekund)
    for (int j = 0; j < 10; j++ ) {
      wifi_connected = WiFi.status() == WL_CONNECTED;
      if (wifi_connected) {
        // Připojení uspěšne
        Serial.println();
        Serial.print(F("WiFi Připojen. IP adresa: "));
        Serial.println(WiFi.localIP());
        break;
      }
      delay(500);
      Serial.print(".");
    }
    Serial.println();

    if (!wifi_connected)
      Serial.println(F("Chyba připojení k siti WiFi."));
  }
}

void startSoftAP() {
  WiFi.softAPdisconnect(true);
  ap_connected = false;

  Serial.print(F("Vytvoření přistupoveho bodu"));
  Serial.print(apName);

  ap_connected = WiFi.softAP(apName, apPass);

  for (int j = 0; j < 10; j++ ) {
    if (ap_connected) {
      Serial.println();
      Serial.print(F("Přistupový bod vytvořen. Sit: '"));
      Serial.print(apName);
     // Pokud se heslo shoduje s výchozím heslem - vyda informace,
      // pokud se uživatel změnil - nevydavat
      if (strcmp(apPass, "12341234") == 0) {
        Serial.print(F("'. Heslo: '"));
        Serial.print(apPass);
      }
      Serial.println(F("'."));
      Serial.print(F("IP adresa: "));
      Serial.println(WiFi.softAPIP());
      break;
    }

    WiFi.enableAP(false);
    WiFi.softAPdisconnect(true);
    delay(500);

    Serial.print(".");
    ap_connected = WiFi.softAP(apName, apPass);
  }
  Serial.println();

  if (!ap_connected)
    Serial.println(F("Nebylo možne vytvořit přístupovy bod."));
}

void printNtpServerName() {
  Serial.print(F("NTP-Server "));
  Serial.print(ntpServerName);
  Serial.print(F(" -> "));
  Serial.println(timeServerIP);
}

void connectToNetwork() {
  // Připojte se k síti Wi-Fi
  startWiFi();

  // Pokud je použit režim přístupového bodu a nebylo možné se připojit k síti WiFi, vytvořte přístupový bod
  if (!wifi_connected) {
    WiFi.mode(WIFI_AP);
    startSoftAP();
  }

  if (useSoftAP && !ap_connected) startSoftAP();

  // Reporttovat port UDP, ke kterému se očekává připojení
  if (wifi_connected || ap_connected) {
    Serial.print(F("UDP-Server na portu "));
    Serial.println(udp_port);
  }
}
