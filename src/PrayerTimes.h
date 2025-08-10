#ifndef PRAYERTIMES_H
#define PRAYERTIMES_H

#include <Arduino.h>

enum AsrMethod : uint8_t { ASR_SHAFII = 1, ASR_HANAFI = 2 };

struct PrayerTimes {
  int fajrMin;     // minutes from 00:00 local
  int sunriseMin;
  int dhuhrMin;
  int asrMin;
  int maghribMin;
  int ishaMin;
};

// Compute prayer times for a given date/location.
// twilight angles are negative degrees, e.g. -15.0
// timezoneHours is local UTC offset (e.g., -6.0 for Saskatoon)
void computePrayerTimes(
    int year, int month, int day,
    double latitudeDeg, double longitudeDeg,
    float timezoneHours,
    double fajrTwilightDeg, double ishaTwilightDeg,
    AsrMethod asrMethod,
    PrayerTimes &out);

inline String toHHMM(int minutesFromMidnight) {
  if (minutesFromMidnight < 0) minutesFromMidnight += 24 * 60;
  minutesFromMidnight %= (24 * 60);
  int hh = minutesFromMidnight / 60;
  int mm = minutesFromMidnight % 60;
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", hh, mm);
  return String(buf);
}

#endif
