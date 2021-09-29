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
#  define MSG_CMD_DISPLAY_PIXEL_PARSER_INDEX_ARGUMENT_TYPE      MsgParameter::ARGUMENT_TYPE_UINT8
# else
#  define MSG_CMD_DISPLAY_PIXEL_PARSER_INDEX_ARGUMENT_TYPE      MsgParameter::ARGUMENT_TYPE_UINT16
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
    enum StateType {
        STATE_NONE,
        STATE_OFF,
        STATE_ON
    };

    using IndexType = Display::IndexType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;
    byte Index{0u};
    StateType State{STATE_NONE};

    static constexpr char IndexOptionShortName{'I'};
    static constexpr char StateOptionShortName{'S'};

    static constexpr ParameterTableType ParameterTable PROGMEM
    {
        ParameterTableElementType(IndexOptionShortName, MSG_CMD_DISPLAY_PIXEL_PARSER_INDEX_ARGUMENT_TYPE),
        ParameterTableElementType(StateOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
    };

    // functions
    void handleParameter(char ParameterShortName, byte Argument)
    {
        if(ParameterShortName == IndexOptionShortName) { handleParameterIndex(Argument); }
        if(ParameterShortName == StateOptionShortName) { handleParameterState(Argument); }
    }

    void handleParameterIndex(byte Argument)
    {
        Index = Argument;
        if(State == STATE_NONE) { State = getPixelState(Index); }
    }

    void handleParameterState(byte Argument)
    {
        bool pixelValue = Argument;
        State = getPixelState(pixelValue);
    }

    StateType getPixelState(byte Index) const
    {
        bool pixelValue;
        if(Display::getInstance().getPixel(Index, pixelValue) == E_OK) {
            return getPixelState(pixelValue);
        } else {
            return STATE_NONE;
        }
    }

    StateType getPixelState(bool PixelValue) const { return PixelValue ? STATE_ON : STATE_OFF; }
    bool getPixelValue(StateType PixelState) const { return (PixelState == STATE_ON) ? true : false; }

    void sendAnswerIndex(bool AppendSpace) const { sendAnswerParameter(IndexOptionShortName, Index, AppendSpace); }
    void sendAnswerState(bool AppendSpace) const { sendAnswerParameter(StateOptionShortName, Display::getInstance().getPixelFast(Index), AppendSpace); }

    void setPixel() const { if(State != STATE_NONE) { Display::getInstance().writePixelFast(Index, getPixelValue(State)); }}

    void show() const
    {
        StdReturnType returnValue = Display::getInstance().show();
        Error.checkReturnValueAndSend(ErrorMessage::API_DISPLAY_SHOW, returnValue, ErrorMessage::ERROR_DISPLAY_PENDING);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdDisplayPixelParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdDisplayPixelParser() { }

    // get methods

    // set methods

    // methods
    void sendAnswer() const
    {
        if(Display::getInstance().isIndexValid(Index)) {
            sendAnswerIndex(true);
            sendAnswerState(false);
        }
    }

    void process()
    {
        if(!Display::getInstance().isIndexValid(Index)) {
            Error.send(IndexOptionShortName, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNCE, false);
        } else {
            setPixel();
        }
        show();
    }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
