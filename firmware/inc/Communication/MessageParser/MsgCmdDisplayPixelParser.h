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
/**     \file       MsgCmdDisplayPixelParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_DISPLAY_PIXEL_PARSER_H_
#define _MSG_CMD_DISPLAY_PIXEL_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"
#include "MsgParameterParser.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdDisplayPixelParser configuration parameter */


/* MsgCmdDisplayPixelParser parameter */
#define MSG_CMD_DISPLAY_PIXEL_PARSER_PARAMETER_TABLE_SIZE       2u

# if (WS2812_NUMBER_OF_LEDS < 255u)
#  define MSG_CMD_DISPLAY_PIXEL_PARSER_INDEX_ARGUMENT_TYPE      MsgParameter::ArgumentTypeType::kUint8
# else
#  define MSG_CMD_DISPLAY_PIXEL_PARSER_INDEX_ARGUMENT_TYPE      MsgParameter::ArgumentTypeType::kUint16
# endif

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdDisplayPixelParser : public MsgParameterParser<MsgCmdDisplayPixelParser, MSG_CMD_DISPLAY_PIXEL_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum class StateType {
        kNone,
        kOff,
        kOn
    };

    using IndexType = Display::IndexType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;
    byte index_{0u};
    StateType state_{StateType::kNone};

    static constexpr char kIndexOptionShortName{'I'};
    static constexpr char kStateOptionShortName{'S'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kIndexOptionShortName, MSG_CMD_DISPLAY_PIXEL_PARSER_INDEX_ARGUMENT_TYPE),
        ParameterTableElementType(kStateOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
    };

    // functions
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kIndexOptionShortName) { HandleParameterIndex(argument); }
        if(parameter_short_name == kStateOptionShortName) { HandleParameterState(argument); }
    }

    void HandleParameterIndex(byte argument)
    {
        index_ = argument;
        if(state_ == StateType::kNone) { state_ = GetPixelState(index_); }
    }

    void HandleParameterState(byte argument)
    {
        bool pixel_value = argument;
        state_ = GetPixelState(pixel_value);
    }

    static StateType GetPixelState(byte index)
    {
        /*
         * Asked before reading rather than read and then judged by a return code: the pixel
         * itself is what this answers with, and an index that is not on the display has no
         * state rather than a false one.
         */
        if(!Display::GetInstance().IsIndexValid(index)) { return StateType::kNone; }

        return GetPixelState(Display::GetInstance().GetPixel(index));
    }

    static StateType GetPixelState(bool pixel_value) { return pixel_value ? StateType::kOn : StateType::kOff; }
    static bool GetPixelValue(StateType pixel_state) { return pixel_state == StateType::kOn; }

    void SendAnswerIndex(bool append_space) const {
        SendAnswerParameter(kIndexOptionShortName, index_, append_space);
    }
    void SendAnswerState(bool append_space) const {
        SendAnswerParameter(kStateOptionShortName, Display::GetInstance().GetPixel(index_), append_space);
    }

    void SetPixel() const { if(state_ != StateType::kNone) { Display::GetInstance().WritePixel(index_, GetPixelValue(state_)); }}

    void Show() const
    {
        StdReturnType return_value = Display::GetInstance().Show();
        error_.CheckReturnValueAndSend(ErrorMessage::ApiType::kDisplayShow, return_value, ErrorMessage::ErrorType::kDisplayPending);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdDisplayPixelParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }
    ~MsgCmdDisplayPixelParser() { }

    // get methods

    // set methods

    // methods
    void SendAnswer() const
    {
        if(Display::GetInstance().IsIndexValid(index_)) {
            SendAnswerIndex(true);
            SendAnswerState(false);
        }
    }

    void Process()
    {
        if(!Display::GetInstance().IsIndexValid(index_)) {
            error_.Send(kIndexOptionShortName, ErrorMessage::ErrorType::kValueOutOfBounds, false);
        } else {
            SetPixel();
        }
        Show();
    }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
