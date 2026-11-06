// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The overlay that shows a configured text.
 */

#ifndef _OVERLAY_TEXT_H_
#define _OVERLAY_TEXT_H_

#include "StandardTypes.h"
#include "Overlay.h"
#include "StringTools.h"

/*!
 * \brief The size of the text overlay's buffer, its terminator included: the longest text has one
 *        character less.
 *
 * Persistence stores the text at this size, so a change of it changes the layout of the stored
 * settings.
 */
#define OVERLAY_TEXT_TEXT_SIZE              50u

/*!
 * \brief The text overlay: scrolls a configured text over the clock.
 *
 * SetText() copies the text in, and the overlay keeps it, up to OVERLAY_TEXT_TEXT_SIZE - 1
 * characters. A new overlay has an empty text. An empty text is still a text, so CanShow() is
 * always true and the overlay starts with it all the same.
 *
 * The text is the overlay's one setting beyond those of Overlay, and ResetToDefaults() empties
 * it.
 */
class OverlayText : public Overlay<OverlayText>
{
  public:
    /*! \brief The length SetText() takes, StringTools::LengthType. */
    using LengthType = StringTools::LengthType;

  private:
    friend class Overlay;
    /*
     * Initialised like the other two overlays' string buffers rather than left to the
     * singleton's static storage to zero. Persistence reads this one before anything has
     * written it, and a terminator that holds only because of where the object lives is
     * one refactor away from not holding.
     */
    char text_[OVERLAY_TEXT_TEXT_SIZE]{0u};

    void SetStateToShow(ClockDate current_date, ClockTime current_time) { SetText(); UNUSED(current_date); UNUSED(current_time); }
    static void SetStateToIdle(ClockDate current_date, ClockTime current_time) { Text::GetInstance().Stop(); UNUSED(current_date); UNUSED(current_time); }

    /*
     * An empty text is still a text - it is what was configured, and shortening the
     * overlay because of it would hide the mistake rather than show it. Only the
     * temperature can have nothing to show.
     */
    static bool IsReady() { return true; }

    /*
     * The only overlay with anything of its own to reset. An empty text rather than the
     * previous one, so a reset clock shows nothing rather than whatever the last owner
     * configured.
     */
    void ResetOwnToDefaults() { text_[0u] = STD_NULL_CHARACTER; }

    void ShowTask() { if(Text::GetInstance().GetState() == Text::StateType::kIdle) { SetText(); } }
    void SetText() { Text::GetInstance().SetTextWithShift(text_, GetFont()); }

  public:
    /*! \brief Makes the text overlay, switched off and with an empty text. */
    constexpr OverlayText() : text_{StringTools::kNullCharacter} { }

    /*! \brief Destroys the text overlay. */
    ~OverlayText() { }

    /*! \brief Returns the text the overlay shows, ended by a null character. */
    const char* GetText() const { return text_; }

    /*!
     * \brief Sets the text the overlay shows: the first `length` - 1 characters of `text`.
     *
     * The overlay copies them, fewer if `text` ends before, and ends the copy with a null
     * character; `length` counts that terminator. `text` is not read again afterwards.
     *
     * \param[in] text The text.
     * \param[in] length The number of characters to take, plus one.
     * \note While the overlay shows, its text scrolls out of the same buffer: a new text can change
     *       the characters still to come in the run under way, and the next run shows it whole.
     * \warning `length` must not exceed OVERLAY_TEXT_TEXT_SIZE, which is not checked: a larger one
     *          writes behind the overlay's buffer. A `length` of 0 leaves the text as it was.
     */
    void SetText(const char* text, LengthType length) { StringTools::StringCopy(text_, text, length); }

    /*!
     * \brief Takes the overlay one second further, as Overlay::Task() does, and starts the text
     *        again each time it has scrolled through.
     *
     * While the overlay shows, it first starts the text anew if the text output is idle, which it
     * is once the previous run has come to its end; then it runs Overlay::Task().
     *
     * \param[in] show_timer_in_seconds The show timer, in seconds.
     * \param[in] current_date The date now.
     * \param[in] current_time The time now.
     * \return The new show timer, in seconds, as Overlay::Task() returns it.
     */
    SecondType Task(SecondType show_timer_in_seconds, ClockDate current_date, ClockTime current_time) {
        if(state_ == StateType::kShow) { ShowTask(); }
        return Overlay::Task(show_timer_in_seconds, current_date, current_time);
    }
};

#endif
