#ifndef WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_LITTLEFS_H_
#define WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_LITTLEFS_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
#include <stddef.h>
#include <stdint.h>

#include <cstring>
#include <map>
#include <string>
/*
 * Keeps what was written, so a test can watch a round trip rather than only a call. One
 * map for the whole filesystem, shared by every handle, which is what a filesystem is.
 *
 * A File is a cursor into that map. Deliberately a value rather than a handle: the backend
 * opens, uses and closes within one function, so nothing here has to outlive the call.
 */
inline std::map<std::string, std::string>& littleFsStore() {
  static std::map<std::string, std::string> Store;
  return Store;
}

/*
 * How many more bytes the filesystem takes before a write stops short, as a full or failing
 * flash would make it. Unlimited unless a case sets it.
 */
inline size_t& littleFsWriteBudget() {
  static size_t Budget = static_cast<size_t>(-1);
  return Budget;
}

class File {
 private:
  std::string Path;
  bool Writable{false};
  bool Valid{false};
  size_t Cursor{0u};

 public:
  File() {}
  File(std::string const& sPath, bool sWritable, bool sValid)
      : Path(sPath), Writable(sWritable), Valid(sValid) {}

  explicit operator bool() const { return Valid; }

  size_t size() const {
    auto const Found = littleFsStore().find(Path);
    return (Found == littleFsStore().end()) ? 0u : Found->second.size();
  }

  size_t read(uint8_t* Buffer, size_t Length) {
    std::string const& Content = littleFsStore()[Path];
    size_t const Available = (Cursor < Content.size()) ? (Content.size() - Cursor) : 0u;
    size_t const Taken = (Length < Available) ? Length : Available;
    memcpy(Buffer, Content.data() + Cursor, Taken);
    Cursor += Taken;
    return Taken;
  }

  size_t write(uint8_t const* Buffer, size_t Length) {
    if (!Writable) {
      return 0u;
    }
    size_t const Taken = (Length < littleFsWriteBudget()) ? Length : littleFsWriteBudget();
    littleFsWriteBudget() -= Taken;
    littleFsStore()[Path].append(reinterpret_cast<char const*>(Buffer), Taken);
    return Taken;
  }

  void close() { Valid = false; }
};

/* What the core's FS.h carries, in the two fields the backend reads. */
struct FSInfo {
  uint64_t totalBytes;
  uint64_t usedBytes;
  size_t blockSize;
  size_t pageSize;
  size_t maxOpenFiles;
  size_t maxPathLength;
};

struct LittleFSStub {
  bool Mounted{true};
  // The region the platformio.ini asks for. A case that wants an image refused for want of
  // space sets this to what the board used to have.
  uint64_t Total{1024u * 1024u};

  bool begin() { return Mounted; }

  bool info(FSInfo& Info) {
    size_t Used = 0u;
    for (auto const& Entry : littleFsStore()) {
      Used += Entry.second.size();
    }
    Info = FSInfo{Total, Used, 4096u, 256u, 4u, 32u};
    return true;
  }
  bool mkdir(char const*) { return true; }
  bool exists(char const* Path) { return littleFsStore().count(Path) > 0u; }
  bool remove(char const* Path) { return littleFsStore().erase(Path) > 0u; }
  // Over an existing file, as LittleFS's own rename does.
  bool rename(char const* From, char const* To) {
    auto const Found = littleFsStore().find(From);
    if (Found == littleFsStore().end()) {
      return false;
    }
    littleFsStore()[To] = Found->second;
    littleFsStore().erase(From);
    return true;
  }

  File open(char const* Path, char const* Mode) {
    bool const Writable = (Mode[0] == 'w');
    if (Writable) {
      // Truncating, as the real one does: a shorter blob must not leave the tail of
      // a longer one behind it.
      littleFsStore()[Path].clear();
      return File(Path, true, true);
    }
    if (littleFsStore().count(Path) == 0u) {
      return File();
    }
    return File(Path, false, true);
  }
};
extern LittleFSStub LittleFS;
// NOLINTEND(readability-identifier-naming)
#endif  // WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_LITTLEFS_H_
