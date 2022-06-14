
/*
*******************************************************************************
Fernando Cesar Mercado <fernando.mercado@gmail.com>

Display random Seinfeld (TV show) Quotes, using the M5Staack display.
*******************************************************************************
*/


#include <M5GFX.h>
#include <Arduino.h>
#include <WiFiMulti.h>
#include <Arduino_JSON.h>
#include <HTTPClient.h>
#define USE_SERIAL Serial

WiFiMulti wifiMulti;
M5GFX display;
M5Canvas canvas(&display);

char text[512];
static constexpr size_t textlen = sizeof(text) / sizeof(text[0]);
int textpos = 0;
int scrollstep = 3;
int cycles = 0;

void setup(void)
{
  display.begin();
  display.setColorDepth(8);

  if (display.isEPD())
  {
    scrollstep = 16;
    display.setEpdMode(epd_mode_t::epd_fastest);
    display.invertDisplay(true);
    display.clear(TFT_BLACK);
  }
  if (display.width() < display.height())
  {
    display.setRotation(display.getRotation() ^ 1);
  }

  canvas.setColorDepth(1); // mono color
  canvas.setFont(&fonts::lgfxJapanMinchoP_32);
  canvas.setTextWrap(false);
  canvas.setTextSize(2);
  canvas.createSprite(display.width() + 64, 72);

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  wifiMulti.addAP("Tafirol", "Tafirol1");
}

void loop(void)
{
  int32_t cursor_x = canvas.getCursorX() - scrollstep;
  if (cursor_x <= 0)
  {
    textpos = 0;
    cursor_x = display.width();
    cycles = cycles + 1;
  }

  // Cycles defines the amount of times a Quote is repeated. After n full repetitions a new quote is downloaded.
  
  if (cycles >= 2) {
    cycles = 0;
    if ((wifiMulti.run() == WL_CONNECTED)) {

      HTTPClient http;
      http.begin("https://seinfeld-quotes.herokuapp.com/random"); //HTTP
      int httpCode = http.GET();

      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          JSONVar myObject = JSON.parse(payload);
          strncpy(text, myObject["quote"], 512);
        }
      }
      http.end();
    }
  }

  canvas.setCursor(cursor_x, 0);
  canvas.scroll(-scrollstep, 0);
  while (textpos < textlen && cursor_x <= display.width())
  {
    canvas.print(text[textpos++]);
    cursor_x = canvas.getCursorX();
  }
  display.waitDisplay();
  int y = (display.height() - canvas.height()) >> 1;
  canvas.pushSprite(&display, 0, y);

}
