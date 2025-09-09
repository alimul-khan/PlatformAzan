#include "PrayerTimes.h"
#include <math.h>

// ----- helpers -----
static inline double degToRad(double d) { return d * M_PI / 180.0; }
static inline double radToDeg(double r) { return r * 180.0 / M_PI; }
static inline double norm360(double x)  { while (x < 0) x += 360; while (x >= 360) x -= 360; return x; }
static inline double clamp(double x)    { if (x < -1.0) return -1.0; if (x > 1.0) return 1.0; return x; }

// minutes from local midnight helper
static inline int toMin(double hours) { return (int)round(hours * 60.0); }


void computePrayerTimes(
    int year, int month, int day,
    double latDeg, double lonDeg,
    float tzHours,
    double fajrTwilightDeg, double ishaTwilightDeg,
    AsrMethod asrMethod,
    PrayerTimes &out)
{
  // 4) Export (minutes from local midnight) – using fixed custom values
  out.fajrMin    = 19 *60 + 47;  // 04:15
  out.sunriseMin = 19 *60 +  48;  // 06:00
  out.dhuhrMin   = 19 *60 + 50;  // 13:15
  out.asrMin     = 19 *60 +  53;  // 18:05
  out.maghribMin = 19 *60 + 56;  // 20:23
  out.ishaMin    = 19 *60 + 59;  // 22:10
}



// // Core computation (cleaned from your original)

// void computePrayerTimes(
//     int year, int month, int day,
//     double latDeg, double lonDeg,
//     float tzHours,
//     double fajrTwilightDeg, double ishaTwilightDeg,
//     AsrMethod asrMethod,
//     PrayerTimes &out)
// {
//   // 1) Solar position prelims
//   // Days since 2000-01-01 12:00 (approx; matches your constant)
//   double D = (367 * year)
//            - (int)((7 * (year + (int)((month + 9)/12.0))) / 4.0)
//            + (int)(275 * month / 9.0)
//            + day - 730531.5;

//   double L = norm360(280.461 + 0.9856474 * D);        // mean longitude
//   double M = norm360(357.528 + 0.9856003 * D);        // mean anomaly
//   double Lambda = L + 1.915 * sin(degToRad(M)) + 0.02 * sin(degToRad(2*M));
//   Lambda = norm360(Lambda);                            // ecliptic longitude

//   double Obliq = 23.439 - 0.0000004 * D;              // obliquity
//   // RA (Alpha)
//   double Alpha = radToDeg(atan( cos(degToRad(Obliq)) * tan(degToRad(Lambda)) ));
//   Alpha = norm360(Alpha);
//   Alpha = Alpha - 360.0 * floor(Alpha / 360.0);
//   Alpha = Alpha + 90.0 * (floor(Lambda/90.0) - floor(Alpha/90.0));

//   // Apparent sidereal time at Greenwich
//   double ST = 100.46 + 0.985647352 * D;

//   // Declination
//   double Dec = radToDeg(asin( sin(degToRad(Obliq)) * sin(degToRad(Lambda)) ));

//   // Sunrise/sunset solar altitude = -0.8333°
//   double lat = latDeg;
//   double hourAngleSun = radToDeg(acos( clamp( (sin(degToRad(-0.8333)) - sin(degToRad(Dec))*sin(degToRad(lat))) /
//                                                (cos(degToRad(Dec))*cos(degToRad(lat))) )));
//   double Noon = norm360(Alpha - ST);
//   double UT_Noon = Noon - lonDeg; // degrees
//   double localNoonH = UT_Noon / 15.0 + tzHours; // hours

//   // 2) Prayer arcs
//   // Asr altitude per school (Shafi‘i k=1, Hanafi k=2)
//   double k = (asrMethod == ASR_HANAFI) ? 2.0 : 1.0;
//   double AsrAlt = radToDeg(atan(k + tan(degToRad(fabs(lat - Dec)))));
//   double AsrHA  = radToDeg(acos( clamp( (sin(degToRad(90.0 - AsrAlt)) - sin(degToRad(Dec))*sin(degToRad(lat))) /
//                                          (cos(degToRad(Dec))*cos(degToRad(lat))) ))) / 15.0;

//   double IshaHA = radToDeg(acos( clamp( (sin(degToRad(ishaTwilightDeg)) - sin(degToRad(Dec))*sin(degToRad(lat))) /
//                                          (cos(degToRad(Dec))*cos(degToRad(lat))) ))) / 15.0;

//   double FajrHA = radToDeg(acos( clamp( (sin(degToRad(fajrTwilightDeg)) - sin(degToRad(Dec))*sin(degToRad(lat))) /
//                                          (cos(degToRad(Dec))*cos(degToRad(lat))) ))) / 15.0;

//   double sunriseHA = hourAngleSun / 15.0;

//   // 3) Times (hours local)
//   double fajrH    = localNoonH - FajrHA;           // Fajr
//   double sunriseH = localNoonH - sunriseHA;        // Sunrise
//   double dhuhrH   = localNoonH;                    // Dhuhr (local noon)
//   double asrH     = localNoonH + AsrHA;            // Asr
//   double maghribH = localNoonH + sunriseHA;        // Maghrib
//   double ishaH    = localNoonH + IshaHA;           // Isha

//   // small empirical adjustments (your original had +5 / +3 minutes on some)
//   // Feel free to tweak or remove if undesired:
//   // fajrH    += 5.0/60.0;
//   // dhuhrH   += 5.0/60.0;
//   // maghribH += 3.0/60.0;

//   // 4) Export (minutes from local midnight)
//   out.fajrMin    = toMin(fajrH);
//   out.sunriseMin = toMin(sunriseH);
//   out.dhuhrMin   = toMin(dhuhrH);
//   out.asrMin     = toMin(asrH);
//   out.maghribMin = toMin(maghribH);
//   out.ishaMin    = toMin(ishaH);
// }
