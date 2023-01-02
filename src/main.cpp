#include <M5StickCPlus.h>
#include <BH1750.h>
#include <Wire.h>
#include <map>
#include <string>
#include <math.h>
#include <algorithm>

using namespace std;

BH1750 lightMeter(0x23);
TFT_eSprite sprite = TFT_eSprite(&M5.Lcd);

float lux;
double ev;
tuple<float, string> settings;
bool stop_flag = false;
const int device_iso = 400;

const std::map<float, float> device_av = {
    {1.7, 1.8},
    {2, 2},
    {3, 2.8},
    {4, 4},
    {5, 5.6},
    {6, 8},
    {7, 11},
    {8, 16},
};

const std::map<int, string> device_tv = {
    {0, "1"},
    {1, "1/2"},
    {2, "1/4"},
    {3, "1/8"},
    {4, "1/15"},
    {5, "1/30"},
    {6, "1/60"},
    {7, "1/125"},
    {8, "1/250"},
    {9, "1/500"},
};

const std::map<int, tuple<float, string>> program_line = {
    {5, {1.8, "1/8"}}, // 本当は4.7
    {6, {2, "1/15"}},
    {7, {2, "1/30"}},
    {8, {2.8, "1/30"}},
    {9, {2.8, "1/60"}},
    {10, {4, "1/60"}},
    {11, {4, "1/125"}},
    {12, {5.6, "1/125"}},
    {13, {5.6, "1/250"}},
    {14, {8, "1/250"}},
    {15, {8, "1/500"}},
    {16, {11, "1/500"}},
};

void setBrightness(float lux)
{
  if (lux < 100)
  {
    M5.Axp.ScreenBreath(8);
  }
  else if (100 <= lux < 1000)
  {
    M5.Axp.ScreenBreath(9);
  }
  else if (1000 <= lux)
  {
    M5.Axp.ScreenBreath(10);
  }
}

double calcEV(float lux)
{
  double n = 0.32;
  double sv = log2(device_iso * n);

  double nc = n * 340;
  double iv = log2(lux / nc);

  return iv + sv;
}

tuple<float, string> findSettings(double ev)
{
  int rounded_ev = (int)round(ev);
  int minEv = program_line.begin()->first;
  int maxEv = program_line.rbegin()->first;

  tuple<float, string> settings;

  if (rounded_ev <= minEv)
  {
    settings = {
        get<0>(program_line.at(minEv)),
        get<1>(program_line.at(minEv)),
    };
  }
  else if (rounded_ev >= maxEv)
  {
    settings = {
        get<0>(program_line.at(maxEv)),
        get<1>(program_line.at(maxEv)),
    };
  }
  else
  {
    settings = {
        get<0>(program_line.at(rounded_ev)),
        get<1>(program_line.at(rounded_ev)),
    };
  }

  return settings;
}

void setup()
{
  M5.begin();
  M5.Axp.ScreenBreath(8);
  M5.Lcd.setRotation(1);
  M5.Lcd.setSwapBytes(false);
  M5.Lcd.setTextSize(2);

  M5.Lcd.println("Exposure Meter Beta");

  sprite.setColorDepth(8);
  sprite.setTextSize(2);
  sprite.createSprite(M5.Lcd.width(), M5.Lcd.height());

  M5.Lcd.println("Sensor begin.....");
  Wire.begin(0, 26);
  lightMeter.begin();
  lux = lightMeter.readLightLevel();
  M5.Lcd.println("Sensor activated");
  delay(300);
}

void loop()
{
  sprite.fillScreen(BLACK);
  sprite.setCursor(0, 0);
  sprite.setTextSize(2);

  M5.update();
  if (M5.BtnA.wasPressed() && !stop_flag)
  {
    stop_flag = true;
  }
  else if (M5.BtnA.wasPressed() && stop_flag)
  {
    stop_flag = false;
  }

  if (stop_flag)
  {
    setBrightness(lightMeter.readLightLevel());

    sprite.println("Stoped");
  }
  else
  {
    lux = lightMeter.readLightLevel();
    setBrightness(lux);
    ev = calcEV(lux);
    settings = findSettings(ev);

    sprite.println("Looping");
  }

  sprite.print("ISO ");
  sprite.println(device_iso);

  sprite.print(lux);
  sprite.println(" lx");

  sprite.print(ev);
  sprite.println(" EV");

  sprite.setTextSize(3);
  sprite.print("F ");
  sprite.println(get<0>(settings));
  sprite.print(get<1>(settings).c_str());
  sprite.println(" s");

  sprite.pushSprite(0, 0);

  delay(300);
}