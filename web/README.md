# The two pages

A networked clock serves **two** pages, and which one it hands out at which address is the
whole arrangement: `http://wordclock.local/` is the **panel**, made for the handful of
settings somebody changes, and `http://wordclock.local/console` is the **console**, which
reaches everything else. Nobody types a path - they type the clock's address and take what
comes - so what comes is the page for the frequent things, with a link to the other one in
its header.

Three backends serve them: the [ESP32](../platform/esp32/README.md) and the
[RP2350](../platform/rp2350/README.md) on a wall, and the
[simulator](../platform/simulator/README.md) on a desk, at `http://localhost:8080/`. The
AVR-Dx, having no radio, does not. This directory sits beside `firmware/` rather than inside
any one of them because nothing a browser sees depends on which controller is behind it -
which is the same reason all three answer out of one
[`WebFrontend`](../firmware/inc/Communication/WebFrontend/WebFrontend.h).

Both speak the same commands as the wire, because their web socket is wired straight to the
port `Communication` reads from. Nothing about the protocol is repeated in the browser.

Why two and not one. The panel is **hand-made**: that a colour deserves eight swatches and a
picker, and a brightness a slider, is a judgement about what somebody does often, and no table
carries it - the catalog says "three numbers called Red, Green and Blue, 0 to 255", which is
what a colour *is*. The console is **generated**, and therefore complete by construction: a
command added to the firmware appears in it without a page being touched. One file trying to be
both is how a page ends up neither.

The panel finds its commands by their catalog **label** and not by their number, and that is
not fussiness: `MsgCmdParser::CommandType` is conditional, so a build without the date overlay
gives every command after it a different number. A label that changes hides a card, which is
loud; a number that shifted would wire a slider to the wrong command, which is not.

Above the log it shows the panel itself: the letters come from `GET /display`, which the
clock generates from `DisplayCharacters`, and the colours arrive as binary frames on the
same socket - 330 bytes in the strip's own byte order, at most every 50 ms and only when
they changed. A client that connects to a standing display is sent the current frame at
once, or it would wait for the next change; on a word clock that can be five minutes.

**Nothing shows the colour that went to the strip**, which is worth saying because this
paragraph once claimed the page did and later that nothing could. The wx window renders a
pixel's brightness as a grey level and drops the hue - which is why the colour swap in `Pixel`
could hide there for as long as it did - and both pages take only *whether* a pixel is lit from
the frame: the bytes arrive already dimmed, so a word at low brightness would be a dark grey on
a dark background, unreadable in exactly the case somebody opens the view for. The console
therefore paints every lit pixel one fixed high-contrast colour, and the panel paints its plate
in the colour it *set*, dimmed by the brightness it set - which is the setting read back from
command 2 and not the frame. So a hue that actually left the controller is checked by reading
the bytes, which the tests do, and not by looking.

Three things the panel had to be shaped by rather than shape:

- **Command 9 answers the speed and the favourite of the *selected* animation only.** So the
  animation card is a list to pick from with those two below it, belonging to whatever is
  picked - a column of sixteen sliders would have been fifteen guesses.
- **A window is two times to a reader and four numbers to the protocol.** The night card shows
  two `<input type="time">` and takes the four `-H -M -E -N` apart itself, which is the side of
  that difference the page should carry. The time card is a third of them, on command 4, so the
  taking-apart is one function both use rather than one each.
- **The clock answers when it is asked and not when its minute turns.** Every other card is
  answered because something changed it, so nothing on the page polls - except the time, which
  goes stale on its own and did: the foot under the plate used to show the minute the socket
  opened in, for as long as the page stayed open. It is asked for every half minute now, and
  the field that shows it is left alone while it has the focus, or a poll would put the clock's
  time back over the one being typed in.

The console also carries a command builder, and that form is not written down in the page either:
`GET /commands` serves `MessageCatalog` as JSON, and the page generates the dropdown, the
option rows, the ranges and the named values from it. The same table the simulator's
message builder derives its dialog from - so a command added there appears in both front
ends and on the wire at once, with nothing to keep in step by hand. The built command goes
into the input field rather than straight onto the wire, so it can still be corrected,
exactly as the wx builder's Insert does.

That JSON is written by
[`WebFrontend`](../firmware/inc/Communication/WebFrontend/WebFrontend.h) in the firmware, once
for every backend. It used to be written by each of them, in two functions that were identical
down to their comments - and since a page parses what they produced, a divergence between them
would have shown up in a browser rather than in either backend's tests, which check the bytes a
route answered and not what the page makes of them.

The form and that input are **one tile**, which they were not at first, and the split is worth
recording because it read as harmless: the builder was a folded panel of its own and Use wrote
into the console's, which is folded shut until somebody opens it. So pressing Use closed the
builder, put the line into a field that was not on screen, and moved the focus there - three
things, none of them visible. A button that appears to do nothing is worse than one that is not
offered, so the two are one panel now, in the order somebody works in: pick a command, see what
it builds, take it into the line, send it, read the answer above it.

The console is **only** that, and deliberately so. It carried a hand-made card for the colour
and the brightness for a while - eight swatches, a picker and a slider above the groups - and
that card came out again when the panel arrived: the same controls in two places are two places
to keep in step, and the console's whole worth is that nothing in it is kept by hand. What is
left for a colour here is the generated group, three numbers and a Set, which is what somebody
who is already typing commands expects. The panel is one link away in the header.

Every answer the socket carries is read into the groups' fields, whether this page asked for it
or not - so a colour set on the panel, by hand below, or over the serial line moves them all.
What depends on who asked is only whether the line is *printed*: the sixteen answers a page
collects on load would bury the log it keeps for what somebody typed.

The pages are [`app.html`](app.html) and [`index.html`](index.html), each one
self-contained with its CSS and script inline: the clock has nowhere to fetch anything from.
Which is also why neither uses a downloaded typeface - the panel is the system's own sans with
monospace wherever something is read as data, the console is monospace throughout. They are
**not** uploaded separately. [`../platform/scripts/embed_web.py`](../platform/scripts/embed_web.py)
emits every file the clock serves - both pages, the manifest and the icons, each named in its
asset list - as an array **into the build directory**, so `pio run -t upload` ships page and
firmware together and their versions cannot drift apart - the failure a second partition invites. The generated header is a build product
on purpose; a checked-in one rots the moment someone edits the HTML and forgets to regenerate
it. At the moment the panel is 69 KB of source and 11 KB compressed, the console 57 and 6.7 -
so the panel costs some 10 KB of flash on top of what the console alone did. That pair of
numbers is measured rather than remembered: `tools/documented-sizes.py` holds this paragraph
against the files beside it.

Three of those files are neither page but the **home screen icon**:
`manifest.webmanifest` at `GET /manifest.webmanifest` and two PNGs at `GET /icon-192.png`
and `GET /icon-512.png`, 14 KB of flash between them. They are what makes the console
installable - an icon to tap and a start without an address bar. With one limit worth
knowing before looking for it: Chromium installs only from `https` or `localhost`, and the
clock answers on plain `http`, so today that install works on iOS and on the local
development server and nowhere else. The manifest is served uncompressed, being 485 bytes,
and the icons are as well, a PNG being deflated already.

It follows the system's light or dark preference, with a button in the header that
overrides it and remembers the choice. Light is the base: the clock's own look is amber on
near-black, but the console is usually read next to other light windows.

**Updating the firmware** is reachable from both pages - the console's last folded panel, and
the panel's System tab. It sends the image from a release to `POST /update` as the request
body. Not as a form: a multipart parser in flash would exist only to undo what a `<form>` did
on the way out, and `curl --data-binary @<image> http://wordclock.local/update` is therefore a
first-class way in, which matters on the day the page is the thing that broke. Which file a
release calls the image, and what the clock does with it once the last byte has arrived, is
the backend's business and written down there - [two app slots](../platform/esp32/README.md#updating-the-firmware)
on the ESP32, [a file and a loader](../platform/rp2350/README.md#updating-over-the-network) on
the RP2350. Both keep the property the page relies on: the firmware being written is never the
firmware running, so an upload that stops halfway leaves the clock on what it was running, and
the panel says so rather than reporting a failure.

While the layout is being worked on there is no need to flash: open either page straight from
disk and it asks for the clock's address instead of using its own host. The edit cycle is then
a browser reload.

**The simulator serves them too**, which is the shortest way to see a page with a working
clock behind it: `./build/bin/Wordclock` opens `http://localhost:8080/` beside its window, and
both are the same clock. It answers everything on this page except `/update` - a host process
has no second partition to write an image into.

It answers `/update` as well, and honestly: nothing is installed, because a desktop has no
second partition. What it stands in for is the panel's progress and its two outcomes, which is
the part of that card nothing else reaches without a board.

There used to be a second way - node in front of a host build of the ESP32 backend - and it is
gone. node did all the HTTP and the web socket there, so what it proved was about these pages
and the firmware core, which is exactly what the simulator now does with one binary.
