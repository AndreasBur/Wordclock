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
/**     \file       Animation.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _ANIMATION_H_
#define _ANIMATION_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Animation configuration parameter */


/* Animation parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N   C O M M O N
******************************************************************************************************************************************************/
class Animation
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum class StateType {
        kNone,
        kUninit,
        kIdle,
        kClearTime,
        kSetTime
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:


/******************************************************************************************************************************************************
 *  P R O T E C T E D   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  protected:
    /* odd factor of the hash in SelectByTime(), see there */
    static constexpr byte kSelectHashFactor{181u};

    StateType state_{StateType::kNone};

    // functions
    void Init(StateType state) { state_ = state; }
    /* Every animation whose set phase places the letters itself ends through one of
       these two instead of just going idle: drawing the target once more makes the last
       frame authoritative, so a pixel the animation skipped or hit twice cannot leave a
       word of the previous time behind. AnimationFade is the one animation outside this,
       because its set phase only ramps the brightness of letters already in place. */
    void FinishWithClockWords(const ClockWords::WordsListType& clock_words_table) {
        Display::GetInstance().Clear();
        Clock::GetInstance().SetTime(clock_words_table);
        state_ = StateType::kIdle;
    }
    /* For the animations that kept the time rather than the word list. */
    void FinishWithTime(byte hour, byte minute) {
        Display::GetInstance().Clear();
        Clock::GetInstance().SetTime(hour, minute);
        state_ = StateType::kIdle;
    }
    static byte SelectByTime(byte, byte, byte);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr Animation() : state_{StateType::kNone} { }
    ~Animation() { }

    // get methods
    StateType GetState() const { return state_; }

    // set methods

    // methods
    static StdReturnType Show() { return Display::GetInstance().Show(); }
    static bool IsPixelPartOfClockWords(ClockWords::WordsListType, byte, byte);
    static bool IsPixelPartOfClockWords(ClockWords::WordsListType, byte);
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
