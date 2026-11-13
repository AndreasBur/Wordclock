/*
 * Fuzzes what Persistence::Load() makes of a stored blob that passes its own checks.
 *
 * A blob that fails them is thrown away whole, so fuzzing the bytes as they come would test
 * little more than a checksum: nearly every input would be refused at the door. What is worth
 * finding is a blob the format accepts and the modules should not - a flash cell that flipped
 * and a checksum that happens to agree, or a build that wrote a value this one has no use for.
 * So every input is made into a blob that passes: its bytes are laid over the blob a new clock
 * writes, and then the magic, the version and the checksum are put back.
 *
 * Where those three are is taken from the blob itself rather than from a copy of SettingsType,
 * which lives in Persistence.cpp and is meant to stay there. The magic and the version are the
 * struct's first members and stay where they are; the checksum is the last member, which leaves
 * it at the last byte or, behind a byte of padding, the one before. Which of the two is found
 * out once, by asking Load() which of them it accepts.
 *
 * After the load, each overlay is shown in turn, because the stored date, text and font of an
 * overlay are only read when it draws. For a few ticks each and not for its whole endurance: the
 * first frame is the one that reads them, and a tick is what an input costs - see FuzzCommand.cpp.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "Storage.h"
#include "fuzz.h"

namespace {
// Two uint16_t before it: the magic, the favourites and the two calibration values.
constexpr size_t kVersionOffset{8u};
constexpr unsigned kTicksAfterTheLoad{8u};
constexpr unsigned kTicksPerOverlay{16u};

using BlobType = std::vector<byte>;

BlobType ReadStoredBlob() {
  BlobType blob(Storage::kCapacity);
  std::FILE* file = std::fopen(STORAGE_FILE_NAME, "rb");
  if (file == nullptr) {
    return {};
  }
  blob.resize(std::fread(blob.data(), 1u, blob.size(), file));
  (void)std::fclose(file);
  return blob;
}

byte SumAllBut(BlobType const& blob, size_t skipped) {
  byte sum{0u};
  for (size_t index = 0u; index < blob.size(); index++) {
    if (index != skipped) {
      sum = static_cast<byte>(sum + blob[index]);
    }
  }
  return sum;
}

bool IsAcceptedWithChecksumAt(BlobType blob, size_t checksum_offset) {
  // A byte that is not part of the header, changed so that a checksum in the wrong place
  // could not agree by accident.
  blob[kVersionOffset + 1u] = static_cast<byte>(blob[kVersionOffset + 1u] + 1u);
  blob[checksum_offset] = SumAllBut(blob, checksum_offset);
  return (Storage::GetInstance().Write(blob.data(), blob.size()) == kEOk) &&
         (Persistence::Load() == kEOk);
}

struct LayoutType {
  BlobType fresh_blob;
  size_t checksum_offset;
};

/*
 * Stops the fuzzer when the blob is not what this file expects of it, rather than letting it run
 * on with every input refused - which would look exactly like a fuzzer that finds nothing.
 */
LayoutType FindLayout() {
  StartFromANewClock();
  if (Persistence::Save() != kEOk) {
    std::fputs("FuzzSettings: the store took no blob\n", stderr);
    std::abort();
  }
  BlobType const fresh_blob = ReadStoredBlob();

  for (size_t const from_the_end : {1u, 2u}) {
    if (fresh_blob.size() <= kVersionOffset + from_the_end) {
      break;
    }
    size_t const offset = fresh_blob.size() - from_the_end;
    if (IsAcceptedWithChecksumAt(fresh_blob, offset)) {
      return {fresh_blob, offset};
    }
  }
  std::fputs("FuzzSettings: the stored blob no longer has its checksum at its end\n", stderr);
  std::abort();
}

LayoutType const& GetLayout() {
  static LayoutType const kLayout = FindLayout();
  return kLayout;
}

BlobType ToAcceptedBlob(uint8_t const* data, size_t size) {
  LayoutType const& layout = GetLayout();
  BlobType blob = layout.fresh_blob;

  for (size_t index = 0u; (index < size) && (index < blob.size()); index++) {
    blob[index] = data[index];
  }
  blob[0u] = layout.fresh_blob[0u];
  blob[1u] = layout.fresh_blob[1u];
  blob[kVersionOffset] = layout.fresh_blob[kVersionOffset];
  blob[layout.checksum_offset] = SumAllBut(blob, layout.checksum_offset);
  return blob;
}

void ShowEveryOverlay() {
  Overlays& overlays = Overlays::GetInstance();
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
  (void)overlays.ShowDateNow();
  RunTicks(kTicksPerOverlay);
  (void)overlays.Abort();
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
  (void)overlays.ShowTemperatureNow();
  RunTicks(kTicksPerOverlay);
  (void)overlays.Abort();
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
  (void)overlays.ShowTextNow();
  RunTicks(kTicksPerOverlay);
  (void)overlays.Abort();
#endif
}
}  // namespace

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data,
                                      size_t size)  // NOLINT(readability-identifier-naming)
{
  BlobType const blob = ToAcceptedBlob(data, size);
  StartFromANewClock();

  if (Storage::GetInstance().Write(blob.data(), blob.size()) != kEOk) {
    return 0;
  }
  if (Persistence::Load() != kEOk) {
    std::fputs("FuzzSettings: a blob with the right header and checksum was refused\n", stderr);
    std::abort();
  }
  RunTicks(kTicksAfterTheLoad);
  ShowEveryOverlay();
  return 0;
}
