#ifndef _INC_EVERY_X_TIME_H
#define _INC_EVERY_X_TIME_H

#include <Arduino.h>

#define _LIB_STATIC_INLINE_ __attribute__ ((unused)) static inline

/// Helper routine to divide a 32-bit value by 1024, returning
/// only the low 16 bits. You'd think this would be just
///   result = (in32 >> 10) & 0xFFFF;
/// and on ARM, that's what you want and all is well.
/// But on AVR that code turns into a loop that executes
/// a four-byte shift ten times: 40 shifts in all, plus loop
/// overhead. This routine gets exactly the same result with
/// just six shifts (vs 40), and no loop overhead.
/// Used to convert millis to 'binary seconds' aka bseconds:
/// one bsecond == 1024 millis.
_LIB_STATIC_INLINE_ uint16_t div_by_1024( uint32_t in32)
{
  uint16_t out16;
#if defined(__AVR__)
  asm volatile (
    "  lsr %D[in]  \n\t"
    "  ror %C[in]  \n\t"
    "  ror %B[in]  \n\t"
    "  lsr %D[in]  \n\t"
    "  ror %C[in]  \n\t"
    "  ror %B[in]  \n\t"
    "  mov %B[out],%C[in] \n\t"
    "  mov %A[out],%B[in] \n\t"
    : [in] "+r" (in32),
    [out] "=r" (out16)
  );
#else
  out16 = (in32 >> 10) & 0xFFFF;
#endif
  return out16;
}

/// Return the current seconds since boot in a 16-bit value.  Used as part of the
/// "every N time-periods" mechanism
_LIB_STATIC_INLINE_ uint16_t seconds_16b()
{
  uint32_t ms = millis();
  uint16_t s16;
  s16 = ms / 1000;
  return s16;
}
/// bseconds_16b returns the current time-since-boot in
/// "binary seconds", which are actually 1024/1000 of a
/// second long.
_LIB_STATIC_INLINE_ uint16_t bseconds_16b()
{
  uint32_t ms = millis();
  uint16_t s16;
  s16 = div_by_1024( ms);
  return s16;
}

/// Return the current minutes since boot in a 16-bit value.  Used as part of the
/// "every N time-periods" mechanism
_LIB_STATIC_INLINE_ uint16_t minutes_16b()
{
  uint32_t ms = millis();
  uint16_t m16;
  m16 = (ms / (60000L)) & 0xFFFF;
  return m16;
}

/// Return the current hours since boot in an 8-bit value.  Used as part of the
/// "every N time-periods" mechanism
_LIB_STATIC_INLINE_ uint8_t hours_8b()
{
  uint32_t ms = millis();
  uint8_t h8;
  h8 = (ms / (3600000L)) & 0xFF;
  return h8;
}

#define INSTANTIATE_EVERY_X_TIME_PERIODS(NAME,TIMETYPE,TIMEGETTER) \
  class NAME {    \
    public: \
      TIMETYPE mPrevTrigger;  \
      TIMETYPE mPeriod;   \
      \
      NAME(TIMETYPE period=1, bool TrigAtStart=true) { \
        setPeriod(period); \
        if(TrigAtStart) \
          trigger(); \
        else \
          reset(); \
        };    \
      void setPeriod( TIMETYPE period) { mPeriod = period; }; \
      TIMETYPE getTime() { return (TIMETYPE)(TIMEGETTER()); };    \
      TIMETYPE getPeriod() { return mPeriod; };   \
      TIMETYPE getElapsed() { return getTime() - mPrevTrigger; }  \
      TIMETYPE getRemaining() { return mPeriod - getElapsed(); }  \
      TIMETYPE getLastTriggerTime() { return mPrevTrigger; }  \
      bool ready() { \
        bool isReady = (getElapsed() >= mPeriod);   \
        if( isReady ) { reset(); }  \
        return isReady; \
      }   \
      void reset() { mPrevTrigger = getTime(); }; \
      void trigger() { mPrevTrigger = getTime() - mPeriod; }; \
      \
      operator bool() { return ready(); } \
  };

INSTANTIATE_EVERY_X_TIME_PERIODS(CEveryXMicros, uint32_t, micros);
INSTANTIATE_EVERY_X_TIME_PERIODS(CEveryXMillis, uint32_t, millis);
INSTANTIATE_EVERY_X_TIME_PERIODS(CEveryXSeconds, uint16_t, seconds_16b);
INSTANTIATE_EVERY_X_TIME_PERIODS(CEveryXBSeconds, uint16_t, bseconds_16b);
INSTANTIATE_EVERY_X_TIME_PERIODS(CEveryXMinutes, uint16_t, minutes_16b);
INSTANTIATE_EVERY_X_TIME_PERIODS(CEveryXHours, uint8_t, hours_8b);

#define CONCAT_HELPER( x, y ) x##y
#define CONCAT_MACRO( x, y ) CONCAT_HELPER( x, y )
#define EVERY_X_MICROS(...) EVERY_X_MICROS_NAMED(CONCAT_MACRO(EVERY_X_, __COUNTER__ ),__VA_ARGS__)
#define EVERY_X_MICROS_NAMED(NAME,...) static CEveryXMicros NAME(__VA_ARGS__); if( NAME )
#define EVERY_X_MILLIS(...) EVERY_X_MILLIS_NAMED(CONCAT_MACRO(EVERY_X_, __COUNTER__ ),__VA_ARGS__)
#define EVERY_X_MILLIS_NAMED(NAME,...) static CEveryXMillis NAME(__VA_ARGS__); if( NAME )
#define EVERY_X_SECONDS(...) EVERY_X_SECONDS_NAMED(CONCAT_MACRO(EVERY_X_, __COUNTER__ ),__VA_ARGS__)
#define EVERY_X_SECONDS_NAMED(NAME,...) static CEveryXSeconds NAME(__VA_ARGS__); if( NAME )
#define EVERY_X_BSECONDS(...) EVERY_X_BSECONDS_NAMED(CONCAT_MACRO(EVERY_X_, __COUNTER__ ),__VA_ARGS__)
#define EVERY_X_BSECONDS_NAMED(NAME,...) static CEveryXBSeconds NAME(__VA_ARGS__); if( NAME )
#define EVERY_X_MINUTES(...) EVERY_X_MINUTES_NAMED(CONCAT_MACRO(EVERY_X_, __COUNTER__ ),__VA_ARGS__)
#define EVERY_X_MINUTES_NAMED(NAME,...) static CEveryXMinutes NAME(__VA_ARGS__); if( NAME )
#define EVERY_X_HOURS(...) EVERY_X_HOURS_NAMED(CONCAT_MACRO(EVERY_X_, __COUNTER__ ),__VA_ARGS__)
#define EVERY_X_HOURS_NAMED(NAME,...) static CEveryXHours NAME(__VA_ARGS__); if( NAME )

#endif