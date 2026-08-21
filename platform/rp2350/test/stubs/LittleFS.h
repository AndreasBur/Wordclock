#ifndef _LITTLEFS_H_
#define _LITTLEFS_H_
#include <stddef.h>
#include <stdint.h>
#include <cstring>
#include <map>
#include <string>
/* Keeps what was written, so a test can watch a round trip rather than only a call. One
   map for the whole filesystem, shared by every handle, which is what a filesystem is.

   A File is a cursor into that map. Deliberately a value rather than a handle: the backend
   opens, uses and closes within one function, so nothing here has to outlive the call. */
inline std::map<std::string, std::string>& littleFsStore()
{
    static std::map<std::string, std::string> Store;
    return Store;
}

class File {
  private:
    std::string Path;
    bool Writable{false};
    bool Valid{false};
    size_t Cursor{0u};

  public:
    File() { }
    File(const std::string& sPath, bool sWritable, bool sValid)
        : Path(sPath), Writable(sWritable), Valid(sValid) { }

    explicit operator bool() const { return Valid; }

    size_t size() const {
        const auto Found = littleFsStore().find(Path);
        return (Found == littleFsStore().end()) ? 0u : Found->second.size();
    }

    size_t read(uint8_t* Buffer, size_t Length) {
        const std::string& Content = littleFsStore()[Path];
        const size_t Available = (Cursor < Content.size()) ? (Content.size() - Cursor) : 0u;
        const size_t Taken = (Length < Available) ? Length : Available;
        memcpy(Buffer, Content.data() + Cursor, Taken);
        Cursor += Taken;
        return Taken;
    }

    size_t write(const uint8_t* Buffer, size_t Length) {
        if(!Writable) { return 0u; }
        littleFsStore()[Path].append(reinterpret_cast<const char*>(Buffer), Length);
        return Length;
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
    /* The region the platformio.ini asks for. A case that wants an image refused for want of
       space sets this to what the board used to have. */
    uint64_t Total{1024u * 1024u};

    bool begin() { return Mounted; }

    bool info(FSInfo& Info) {
        size_t Used = 0u;
        for(const auto& Entry : littleFsStore()) { Used += Entry.second.size(); }
        Info = FSInfo{Total, Used, 4096u, 256u, 4u, 32u};
        return true;
    }
    bool mkdir(const char*) { return true; }
    bool exists(const char* Path) { return littleFsStore().count(Path) > 0u; }
    bool remove(const char* Path) { return littleFsStore().erase(Path) > 0u; }

    File open(const char* Path, const char* Mode) {
        const bool Writable = (Mode[0] == 'w');
        if(Writable) {
            /* Truncating, as the real one does: a shorter blob must not leave the tail of
               a longer one behind it. */
            littleFsStore()[Path].clear();
            return File(Path, true, true);
        }
        if(littleFsStore().count(Path) == 0u) { return File(); }
        return File(Path, false, true);
    }
};
extern LittleFSStub LittleFS;
#endif
