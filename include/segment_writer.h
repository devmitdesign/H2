#pragma once
#include <Arduino.h>
#include "log_record.h"

class SegmentWriter {
public:
  bool begin();
  void requestPowerFailFlush();
  bool append(const LogRecord32& rec);
  void tick();

private:
  bool ensureLogDir();
  bool loadState();
  bool saveState();
  bool openWip();
  bool finalizeSegment();
  void periodicFlush();

  uint32_t mSegNo = 0;
  uint32_t mBytesInSeg = 0;
  uint32_t mLastFlushMs = 0;
  uint32_t mSinceFlushCount = 0;
  bool mForceFlush = false;
};
