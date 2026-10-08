/*
 * Config.cpp
 *
 *       Created on: 24.11.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

#include "Config.h"
#include "Frame.h"


const int captionsSizeIndex = 0;
const int timeZoneIndex = 1;
const int digitalModeIndex = 2;
const int closeWarningIndex = 3;
const int additionalHeightIndex = 4;
const int lastSetTimeIndex = 5;
const int maxDigitalClockSizeTimeIndex = 6;
const int maxDigitalClockSizeStopwatchIndex = 7;
const int volumeIndex = 8;

const std::string CONFIG_TAGS[] = {"captionsSize",
                                   "timeZone",
                                   "digitalMode",
                                   "closeWarning",
                                   "additionalHeight",
                                   "lastSetTime",
                                   "maxDigitalClockSize(time)",
                                   "maxDigitalClockSize(stopwatch)",
                                   "volume"};

Config::Config() {
  timeZone = 3;

  /* set timezone in config file because got invalid time for
   * in summer for Moscow (actually no saving daylight)
   * For Moscow use "3" which means utc+3 without daylight savings
   */
  if (timeZone == DEFAULT_TIME_ZONE) {
    tz = g_time_zone_new_local();
  } else {
    // 3 is full width with sign
    std::string s = std::format("{:+03}", timeZone);
    tz = g_time_zone_new_identifier(s.c_str());
  }
}

void Config::init() {
  std::string s;

  timeZone = DEFAULT_TIME_ZONE;
  digitalMode = 0;
  closeWarning = 1;
  maxDigitalClockSize[0] = {800, 800, true};
  maxDigitalClockSize[1] = {1800, 1800, false};
  soundVolume = 0x3000;

  readConfig(CONFIG_TAGS, captionsSize, timeZone, digitalMode, closeWarning,
             additionalHeight, s, maxDigitalClockSize[0],
             maxDigitalClockSize[1], soundVolume);

  /* set timezone in config file because got invalid time for
   * in summer for Moscow (actually no saving daylight)
   * For Moscow use "3" which means utc+3 without daylight savings
   */
  if (timeZone == DEFAULT_TIME_ZONE) {
    tz = g_time_zone_new_local();
  } else {
    // 3 is full width with sign
    s = std::format("{:+03}", timeZone);
    tz = g_time_zone_new_identifier(s.c_str());
  }
}

void Config::write() {
  std::string s;
  int i;
  if (frame.isTime() && !frame.beepTime.empty()) {
    i = 0;
    for (auto v : frame.beepTime) {
      if (i++) {
        s += " ";
      }
      s += std::to_string(v);
    }
  } else {
    s = "";
  }

  writeConfig(CONFIG_TAGS, captionsSize, timeZone, digitalMode, closeWarning,
              additionalHeight, s, maxDigitalClockSize[0],
              maxDigitalClockSize[1], soundVolume);
}

VString Config::getArguments() const {
  VString r;

  if (!read) {
    return r;
  }

  // diffDays
  DateTime dt;
  BeepTimeType now = DateTime().toBeepTime();
  for (auto v : config.lastSetTime) {
    if (now < v) {
      dt.fromBeepTime(v);
      int d = dt.diffDays(now);
      auto s = std::to_string(getHHMM(v));
      if (d > 0) {
        s += "+";
        if (d > 1) {
          s += std::to_string(d);
        }
      }
      r.push_back(s);
    }
  }

  if (!r.empty()) {
    r.insert(r.begin(), "tm");
  }

  return r;
}
