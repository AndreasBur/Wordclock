/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       AnimationMatrix.cpp
 *      \brief      Falling code rain, as seen in the Matrix movie
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_MATRIX_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "AnimationMatrix.h"


/******************************************************************************************************************************************************
 *  LOCAL CONSTANT MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  LOCAL FUNCTION MACROS
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  LOCAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Init()
******************************************************************************************************************************************************/
void AnimationMatrix::Init()
{
    Animation::Init(kStateIdle);
    /* all animations share one union, so nothing may rely on member initializers */
    Reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationMatrix::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk && state_ == kStateIdle) {
        /* derives the start frames from the time, so word changes vary the rain pattern */
        seed_ = static_cast<byte>((hour * 60u) + minute);
        frame_counter_ = 0u;
        /* no clear here: the previous time stays visible below the drops and is eaten
           by the rain while it falls */
        state_ = kStateSetTime;
        return_value = kEOk;
    }
    return return_value;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationMatrix::Task()
{
    if(state_ == kStateSetTime) { SetTimeTask(); }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
void AnimationMatrix::Reset()
{
    clock_words_table_.fill(DisplayWords::kWordNone);
    frame_counter_ = 0u;
    seed_ = 0u;
} /* reset */


/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationMatrix::SetTimeTask()
{
    if(frame_counter_ < kNumberOfFrames) {
        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            /* before its start frame the column still shows the previous time */
            const byte start = StartFrame(column);
            if(frame_counter_ >= start) { DrawColumn(column, frame_counter_ - start); }
        }
        frame_counter_++;
    } else {
        /* also puts the letters back into the normal display color, which the drops
           drew over */
        FinishWithClockWords(clock_words_table_);
        Reset();
    }
} /* setTimeTask */


/******************************************************************************************************************************************************
  drawColumn()
******************************************************************************************************************************************************/
void AnimationMatrix::DrawColumn(byte column, byte head)
{
    /* the head is the brightest pixel of a drop, it may already have left the display */
    if(head < DISPLAY_NUMBER_OF_ROWS) { Display::GetInstance().SetPixel(column, head); }

    /* Everything above the head is the trail. Two rows share one fade step, so the
       trail is twice as long as there are intensity steps, and the intensity falls
       with 1/step. Rows below the head are left alone, they still show the previous
       time until the drop arrives. */
    for(byte row = 0u; row < head && row < DISPLAY_NUMBER_OF_ROWS; row++) {
        const byte distance = head - row;
        const byte fade_step = (distance + 1u) / 2u;

        if(IsPixelPartOfClockWords(clock_words_table_, column, row) && fade_step > kRevealFadeStep) {
            /* letter of the new time, deep enough in the trail: it stays from now on
               because it is drawn without a brightness in every following frame */
            Display::GetInstance().SetPixel(column, row);
        } else if(fade_step > kTrailFadeSteps) {
            /* end of the trail, everything above the drop is dark again */
            Display::GetInstance().ClearPixel(column, row);
        } else {
            Display::GetInstance().SetPixel(column, row, static_cast<byte>(kTrailBrightness / fade_step));
        }
    }
} /* drawColumn */


/******************************************************************************************************************************************************
  startFrame()
******************************************************************************************************************************************************/
byte AnimationMatrix::StartFrame(byte column) const
{
    /* Calculated instead of stored: an array of start frames would be the largest
       member of this class and would grow the animation union, while the rain does
       not need real randomness.
       A plain linear formula would not do, because modulo arithmetic turns it into a
       constant step between neighbouring columns, which shows up as a marching
       diagonal instead of rain. Two multiplications with an xor fold in between
       scatter the columns as well as a random generator would. */
    byte hash = static_cast<byte>((column + seed_) * kStartFrameHashFactor);
    hash ^= static_cast<byte>(hash >> 4u);
    hash = static_cast<byte>(hash * kStartFrameHashFactor);

    /* scaled by multiplication instead of a modulo: 256 is not a multiple of
       MaxStartDelay, so a modulo would favour the lower start frames */
    return static_cast<byte>((static_cast<uint16_t>(hash) * kMaxStartDelay) >> 8u);
} /* startFrame */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
