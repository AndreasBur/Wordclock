// The simulator's port speaks the firmware's Latin-1, as the hardware's ports do.

#include "check.h"
#include "cases.h"

#include "sim/SerialShim.h"

#include <cstring>

/*
 * An answer with an umlaut, as the text overlay can hold one. The port took the firmware's
 * Latin-1 bytes for the locale's multibyte text, which under a UTF-8 locale left nothing of the
 * answer, and handed the line on as UTF-8, which the web front end then encoded a second time.
 */
void TestSerialShimSpeaksLatin1() {
  char const* const answer =
      "7 T=Gl\xFC"
      "ck";
  wxString const text = SerialShim::FromFirmware(answer);

  Expect(text == wxString::FromUTF8("7 T=Gl\xC3\xBC"
                                    "ck"),
         "an answer with an umlaut must convert whole");
  Expect(strcmp(SerialShim::ToFirmware(text).data(), answer) == 0,
         "and must reach the line sink as the same Latin-1 bytes");
  Expect(SerialShim::FromFirmware('\xFC') == wxString::FromUTF8("\xC3\xBC"),
         "a single umlaut must convert as well");
}
