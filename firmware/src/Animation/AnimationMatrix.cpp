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
  init()
******************************************************************************************************************************************************/
void AnimationMatrix::init()
{
    Animation::init(STATE_IDLE);
    /* all animations share one union, so nothing may rely on member initializers */
    reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationMatrix::setTime(byte Hour, byte Minute)
{
    StdReturnType returnValue{E_NOT_OK};

    if(Clock::getInstance().getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        /* derives the start frames from the time, so word changes vary the rain pattern */
        Seed = static_cast<byte>((Hour * 60u) + Minute);
        FrameCounter = 0u;
        /* no clear here: the previous time stays visible below the drops and is eaten
           by the rain while it falls */
        State = STATE_SET_TIME;
        returnValue = E_OK;
    }
    return returnValue;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationMatrix::task()
{
    if(State == STATE_SET_TIME) { setTimeTask(); }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
void AnimationMatrix::reset()
{
    ClockWordsTable.fill(DisplayWords::WORD_NONE);
    FrameCounter = 0u;
    Seed = 0u;
} /* reset */


/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationMatrix::setTimeTask()
{
    if(FrameCounter < NumberOfFrames) {
        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            /* before its start frame the column still shows the previous time */
            const byte start = startFrame(column);
            if(FrameCounter >= start) { drawColumn(column, FrameCounter - start); }
        }
        FrameCounter++;
    } else {
        /* also puts the letters back into the normal display color, which the drops
           drew over */
        finishWithClockWords(ClockWordsTable);
        reset();
    }
} /* setTimeTask */


/******************************************************************************************************************************************************
  drawColumn()
******************************************************************************************************************************************************/
void AnimationMatrix::drawColumn(byte Column, byte Head)
{
    /* the head is the brightest pixel of a drop, it may already have left the display */
    if(Head < DISPLAY_NUMBER_OF_ROWS) { Display::getInstance().setPixel(Column, Head); }

    /* Everything above the head is the trail. Two rows share one fade step, so the
       trail is twice as long as there are intensity steps, and the intensity falls
       with 1/step. Rows below the head are left alone, they still show the previous
       time until the drop arrives. */
    for(byte row = 0u; row < Head && row < DISPLAY_NUMBER_OF_ROWS; row++) {
        const byte distance = Head - row;
        const byte fadeStep = (distance + 1u) / 2u;

        if(isPixelPartOfClockWords(ClockWordsTable, Column, row) && fadeStep > RevealFadeStep) {
            /* letter of the new time, deep enough in the trail: it stays from now on
               because it is drawn without a brightness in every following frame */
            Display::getInstance().setPixel(Column, row);
        } else if(fadeStep > TrailFadeSteps) {
            /* end of the trail, everything above the drop is dark again */
            Display::getInstance().clearPixel(Column, row);
        } else {
            Display::getInstance().setPixel(Column, row, static_cast<byte>(TrailBrightness / fadeStep));
        }
    }
} /* drawColumn */


/******************************************************************************************************************************************************
  startFrame()
******************************************************************************************************************************************************/
byte AnimationMatrix::startFrame(byte Column) const
{
    /* Calculated instead of stored: an array of start frames would be the largest
       member of this class and would grow the animation union, while the rain does
       not need real randomness.
       A plain linear formula would not do, because modulo arithmetic turns it into a
       constant step between neighbouring columns, which shows up as a marching
       diagonal instead of rain. Two multiplications with an xor fold in between
       scatter the columns as well as a random generator would. */
    byte hash = static_cast<byte>((Column + Seed) * StartFrameHashFactor);
    hash ^= static_cast<byte>(hash >> 4u);
    hash = static_cast<byte>(hash * StartFrameHashFactor);

    /* scaled by multiplication instead of a modulo: 256 is not a multiple of
       MaxStartDelay, so a modulo would favour the lower start frames */
    return static_cast<byte>((static_cast<uint16_t>(hash) * MaxStartDelay) >> 8u);
} /* startFrame */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
