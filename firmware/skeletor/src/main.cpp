/**
 * Skeletor Firmware for Cardputer ADV (M5Stack ESP32-S3)
 * 
 * Displays random catchphrases with skull animation
 * Reads phrases from SPIFFS or uses built-in fallback
 * Optional OTA updates when WiFi credentials provided
 */

#include <M5Unified.h>
#include <SPIFFS.h>
#include <vector>
#include <string>

// Conditionally include secrets for OTA if available
#ifdef __has_include
  #if __has_include("secrets.h")
    #define HAS_SECRETS
    #include "secrets.h"
  #endif
#endif

// OTA includes - only used if WiFi credentials available
#ifdef HAS_SECRETS
  #if defined(WIFI_SSID) && defined(WIFI_PASSWORD)
    #define OTA_ENABLED
    #include <WiFi.h>
    #include <ArduinoOTA.h>
  #endif
#endif

// Configuration
const unsigned long PHRASE_CHANGE_INTERVAL = 5000; // 5 seconds
const unsigned long ANIMATION_FRAME_TIME = 500;     // 500ms per animation frame
const char* PHRASES_FILE = "/data/catchphrases.txt";

// Built-in fallback catchphrases
const char* FALLBACK_PHRASES[] = {
  "Myah! You'll never defeat me!",
  "I have the power!",
  "Curse you, heroes!",
  "Foolish mortals!",
  "Bow before Skeletor!",
  "Evil will triumph!",
  "I am invincible!",
  "Your puny efforts amuse me!",
  "Witness my supremacy!",
  "Snake Mountain shall prevail!",
  "Pathetic weaklings!",
  "Kneel before your master!",
  "The universe is mine!",
  "Tremble at my might!"
};
const int FALLBACK_COUNT = sizeof(FALLBACK_PHRASES) / sizeof(FALLBACK_PHRASES[0]);

// Animation state
enum AnimationFrame { IDLE = 0, TALK1 = 1, TALK2 = 2 };
AnimationFrame currentFrame = IDLE;

// Runtime data
std::vector<String> catchphrases;
unsigned long lastPhraseChange = 0;
unsigned long lastFrameChange = 0;
int currentPhraseIndex = 0;

/**
 * Load catchphrases from SPIFFS file
 * Returns true if successful, false otherwise
 */
bool loadPhrasesFromFile() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed!");
    return false;
  }
  
  File file = SPIFFS.open(PHRASES_FILE, "r");
  if (!file) {
    Serial.println("Failed to open catchphrases file, using fallback");
    return false;
  }
  
  Serial.println("Loading catchphrases from SPIFFS...");
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      catchphrases.push_back(line);
    }
  }
  file.close();
  
  Serial.printf("Loaded %d catchphrases from file\n", catchphrases.size());
  return catchphrases.size() > 0;
}

/**
 * Load built-in fallback catchphrases
 */
void loadFallbackPhrases() {
  Serial.println("Using built-in fallback catchphrases");
  catchphrases.clear();
  for (int i = 0; i < FALLBACK_COUNT; i++) {
    catchphrases.push_back(String(FALLBACK_PHRASES[i]));
  }
  Serial.printf("Loaded %d fallback phrases\n", catchphrases.size());
}

/**
 * Initialize OTA if WiFi credentials are available
 */
void initOTA() {
#ifdef OTA_ENABLED
  Serial.println("WiFi credentials found, enabling OTA...");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    ArduinoOTA.setHostname("skeletor-cardputer");
    ArduinoOTA.onStart([]() {
      Serial.println("OTA Update Starting...");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nOTA Update Complete!");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("OTA Error[%u]: ", error);
    });
    
    ArduinoOTA.begin();
    Serial.println("OTA ready");
  } else {
    Serial.println("\nWiFi connection failed");
  }
#else
  Serial.println("OTA disabled - no WiFi credentials provided");
  Serial.println("To enable OTA: copy src/secrets.h.example to src/secrets.h");
#endif
}

/**
 * Display skull animation frame
 */
void displaySkullFrame(AnimationFrame frame) {
  const char* frameFiles[] = {
    "/data/assets/skull_idle.bmp",
    "/data/assets/skull_talk1.bmp",
    "/data/assets/skull_talk2.bmp"
  };
  
  // Placeholder: In a real implementation, load and display BMP
  // For now, just show frame indicator on screen
  M5.Display.fillRect(10, 10, 60, 60, BLACK);
  M5.Display.drawRect(10, 10, 60, 60, WHITE);
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(20, 35);
  
  switch(frame) {
    case IDLE:
      M5.Display.print("IDLE");
      break;
    case TALK1:
      M5.Display.print("TALK1");
      break;
    case TALK2:
      M5.Display.print("TALK2");
      break;
  }
}

/**
 * Display current catchphrase on screen
 */
void displayCatchphrase() {
  if (catchphrases.empty()) return;
  
  // Clear phrase area
  M5.Display.fillRect(0, 80, M5.Display.width(), M5.Display.height() - 80, BLACK);
  
  // Display current phrase
  M5.Display.setTextColor(GREEN);
  M5.Display.setTextSize(2);
  M5.Display.setTextWrap(true);
  M5.Display.setCursor(10, 90);
  M5.Display.println(catchphrases[currentPhraseIndex]);
  
  Serial.print("Displaying: ");
  Serial.println(catchphrases[currentPhraseIndex]);
}

/**
 * Update animation frame
 */
void updateAnimation() {
  unsigned long now = millis();
  
  if (now - lastFrameChange >= ANIMATION_FRAME_TIME) {
    // Cycle through animation frames: IDLE -> TALK1 -> TALK2 -> TALK1 -> IDLE
    switch(currentFrame) {
      case IDLE:
        currentFrame = TALK1;
        break;
      case TALK1:
        currentFrame = TALK2;
        break;
      case TALK2:
        currentFrame = IDLE;
        break;
    }
    
    displaySkullFrame(currentFrame);
    lastFrameChange = now;
  }
}

/**
 * Change to next random catchphrase
 */
void updateCatchphrase() {
  unsigned long now = millis();
  
  if (now - lastPhraseChange >= PHRASE_CHANGE_INTERVAL) {
    // Select random phrase
    if (!catchphrases.empty()) {
      currentPhraseIndex = random(0, catchphrases.size());
      displayCatchphrase();
    }
    lastPhraseChange = now;
  }
}

void setup() {
  // Initialize M5Unified
  auto cfg = M5.config();
  M5.begin(cfg);
  
  // Initialize display
  M5.Display.begin();
  M5.Display.setRotation(1);
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 10);
  M5.Display.println("Skeletor Init...");
  
  // Initialize serial
  Serial.begin(115200);
  Serial.println("\n=== Skeletor Firmware Starting ===");
  Serial.println("Cardputer ADV (ESP32-S3)");
  
  // Load catchphrases
  if (!loadPhrasesFromFile()) {
    loadFallbackPhrases();
  }
  
  // Initialize random seed
  randomSeed(esp_random());
  
  // Initialize OTA if available
  initOTA();
  
  // Clear screen and show initial state
  M5.Display.fillScreen(BLACK);
  displaySkullFrame(IDLE);
  
  // Display first phrase
  if (!catchphrases.empty()) {
    currentPhraseIndex = random(0, catchphrases.size());
    displayCatchphrase();
  }
  
  lastPhraseChange = millis();
  lastFrameChange = millis();
  
  Serial.println("=== Initialization Complete ===\n");
}

void loop() {
  M5.update();
  
  // Handle OTA
#ifdef OTA_ENABLED
  ArduinoOTA.handle();
#endif
  
  // Update animation frame
  updateAnimation();
  
  // Update catchphrase periodically
  updateCatchphrase();
  
  delay(10);
}
