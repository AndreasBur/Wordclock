/* Stands in for the Arduino core's Update object, which is what writes the other app
   partition. What the cases here can check is the sequence the backend drives it through -
   begun with the announced size, written the bytes that arrived, ended only when they all
   did, aborted when they did not - and that is the part the backend owns. Whether the
   flash write lands is the framework's, and no host test can say anything about it.

   The refusal is settable, because the interesting failure is the one that happens before
   any byte is written: an image larger than the partition, which is what picking the factory
   file instead of the OTA one looks like from here. */
#ifndef _UPDATE_H_
#define _UPDATE_H_

#include <stddef.h>
#include <stdint.h>

class UpdateClass
{
  public:
    /* what the case sets */
    bool RefuseBegin{false};
    const char* Error{"none"};

    /* what the case reads */
    bool Begun{false};
    bool Ended{false};
    bool Aborted{false};
    size_t Announced{0u};
    size_t Written{0u};

    void forget()
    {
        RefuseBegin = false;
        Error = "none";
        Begun = false;
        Ended = false;
        Aborted = false;
        Announced = 0u;
        Written = 0u;
    }

    bool begin(size_t Size)
    {
        Announced = Size;
        Begun = !RefuseBegin;
        return Begun;
    }

    size_t write(uint8_t*, size_t Length)
    {
        Written += Length;
        return Length;
    }

    /* The framework refuses an image that is shorter than the size begin() was given, which
       is what a dropped connection leaves behind - so the stub refuses it too, or the case
       for a truncated upload would pass on a backend that never checked. */
    bool end(bool)
    {
        Ended = (Written == Announced);
        return Ended;
    }

    void abort() { Aborted = true; }

    const char* errorString() { return Error; }
};

extern UpdateClass Update;

#endif // _UPDATE_H_
