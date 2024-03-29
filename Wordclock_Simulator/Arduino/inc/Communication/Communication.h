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
/**     \file       Communication.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Message.h"
#include "ErrorMessage.h"
#include "MsgCmdParser.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Communication configuration parameter */
#define COMMUNICATION_TASK_CYCLE                         10u

/* Communication parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   C O M M U N I C A T I O N
******************************************************************************************************************************************************/
class Communication
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum StateType {
        STATE_MESSAGE_COMPLETE,
        STATE_MESSAGE_INCOMPLETE,
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    Communication() : Error(), IncomingMessage(), State(STATE_MESSAGE_INCOMPLETE) { }
    ~Communication() { }

    static constexpr byte TaskCycle{COMMUNICATION_TASK_CYCLE};
    static constexpr char EndOfMessageChar{'\n'};
    ErrorMessage Error;
    Message IncomingMessage;
    StateType State;

    // private functions
    void addMessagePart();

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Communication& getInstance() {
        static Communication SingletonInstance;
        return SingletonInstance;
    }

    // get methods
    static constexpr char getEndOfMessageChar() { return EndOfMessageChar; }
    static constexpr byte getTaskCycle() { return TaskCycle; }
    StateType getState() const { return State; }

    // set methods

    // methods
    void task();
    bool isMessageComplete() const { return State == STATE_MESSAGE_COMPLETE; }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
