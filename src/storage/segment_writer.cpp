#include "segment_writer.h"
#include "config.h"
#include "event_bus.h"
#include "crc16.h"

#include <SPI.h>
#include <SD.h>
#include <stddef.h>

static SPIClass gSpiSD(VSPI);
static File gFile;

static String segWipPath(uint32_t n) {
  char buf[48];
  snprintf(buf, sizeof(buf), "/log/seg_%06lu.wip", (unsigned long)n);
  return String(buf);
}
static String segDonePath(uint32_t n) {
  char buf[48];
  snprintf(buf, sizeof(buf), "/log/seg_%06lu.bin", (unsigned long)n);
  return String(buf);
}
static const char* STATE_PATH = "/log/state.txt";

bool SegmentWriter::ensureLogDir() {
  if (!SD.exists("/log")) {
    if (!SD.mkdir("/log")) {
      Serial.println("SD: mkdir /log failed");
      return false;
    }
  }
  return true;
}

bool SegmentWriter::loadState() {
  if (!SD.exists(STATE_PATH)) { mSegNo = 0; return true; }
  File f = SD.open(STATE_PATH, FILE_READ);
  if (!f) return false;
  String s = f.readStringUntil('\n');
  f.close();
  s.trim();
  mSegNo = (uint32_t)s.toInt();
  return true;
}

bool SegmentWriter::saveState() {
  File f = SD.open(STATE_PATH, FILE_WRITE);
  if (!f) return false;
  f.seek(0);
  f.print(String(mSegNo));
  f.print("\n");
  f.flush();
  f.close();
  return true;
}

bool SegmentWriter::openWip() {
  if (gFile) gFile.close();
  String path = segWipPath(mSegNo);
  gFile = SD.open(path, FILE_APPEND);
  if (!gFile) {
    Serial.printf("SD: open failed %s\n", path.c_str());
    return false;
  }
  mBytesInSeg = (uint32_t)gFile.size();
  mLastFlushMs = millis();
  mSinceFlushCount = 0;
  mForceFlush = true;
  Serial.printf("SD: logging to %s (size=%lu)\n", path.c_str(), (unsigned long)mBytesInSeg);
  return true;
}

bool SegmentWriter::begin() {
  gSpiSD.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (!SD.begin(PIN_SD_CS, gSpiSD, SD_SPI_HZ)) {
    Serial.println("SD.begin failed");
    return false;
  }
  if (!ensureLogDir()) return false;
  if (!loadState()) return false;

  // Safety: rotate partial WIP on boot (size not multiple of record size)
  const String wip = segWipPath(mSegNo);
  if (SD.exists(wip)) {
    File f = SD.open(wip, FILE_READ);
    if (f) {
      uint32_t sz = (uint32_t)f.size();
      f.close();
      if (sz % sizeof(LogRecord32) != 0) {
        String bad = wip + ".bad";
        SD.rename(wip, bad);
        Serial.printf("SD: rotated partial WIP to %s\n", bad.c_str());
        mSegNo++;
        saveState();
      }
    }
  }

  if (!openWip()) return false;

  xEventGroupSetBits(gSysEvents, EV_SD_READY);
  return true;
}

void SegmentWriter::requestPowerFailFlush() {
  mForceFlush = true;
}

bool SegmentWriter::finalizeSegment() {
  if (!gFile) return false;
  gFile.flush();
  gFile.close();

  const String wip = segWipPath(mSegNo);
  const String done = segDonePath(mSegNo);
  if (SD.exists(done)) SD.remove(done);

  if (!SD.rename(wip, done)) {
    Serial.println("SD: rename wip->bin failed");
    return false;
  }

  Serial.printf("SD: finalized seg_%06lu\n", (unsigned long)mSegNo);

  mSegNo++;
  saveState();
  return openWip();
}

void SegmentWriter::periodicFlush() {
  if (!gFile) return;

  const uint32_t now = millis();
  const bool timeFlush  = (now - mLastFlushMs) >= SD_FLUSH_EVERY_MS;
  const bool countFlush = mSinceFlushCount >= SD_FLUSH_EVERY_N_RECORDS;

  if (mForceFlush || timeFlush || countFlush) {
    gFile.flush();
    mLastFlushMs = now;
    mSinceFlushCount = 0;
    mForceFlush = false;
  }
}

bool SegmentWriter::append(const LogRecord32& inRec) {
  if (!gFile) return false;

  LogRecord32 rec = inRec;
  rec.crc16 = 0;
  rec.rsv = 0;
  rec.crc16 = crc16_ccitt_false(reinterpret_cast<const uint8_t*>(&rec), offsetof(LogRecord32, crc16));

  size_t wr = gFile.write(reinterpret_cast<const uint8_t*>(&rec), sizeof(rec));
  if (wr != sizeof(rec)) {
    Serial.println("SD: write failed");
    return false;
  }

  mBytesInSeg += (uint32_t)wr;
  mSinceFlushCount++;

  if (mBytesInSeg >= SEGMENT_MAX_BYTES) {
    return finalizeSegment();
  }
  return true;
}

void SegmentWriter::tick() {
  periodicFlush();
}
