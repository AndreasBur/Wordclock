// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the clock can say about itself.
 *
 * Everything a remote procedure call cannot answer. Its answer is "RpcId=<id> Error=<code>" and
 * carries no value, so the readings that were reachable nowhere else - what the light sensor
 * measures, what the clock chip measures, which firmware is running - are a command with the same
 * query semantics as every other one instead.
 *
 * It is read-only, and its parameter table is empty rather than filled with fields that are
 * ignored: an option sent to it is answered with ErrorType::kParameterUnknown by the base parser,
 * which is what a caller trying to set a reading should hear. The field names still exist - in the
 * message catalog, marked read-only - so a front end can read the answer back through them without
 * offering them as inputs.
 */

#ifndef WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDSTATUSPARSER_H_
#define WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDSTATUSPARSER_H_

#include "Arduino.h"
#include "Illuminance.h"
#include "MsgParameterParser.h"
#include "StandardTypes.h"
#include "System.h"
#include "Temperature.h"
#include "Uptime.h"
#include "Version.h"

/*!
 * \brief The number of options in the parameter table of the status command: none, as the command
 *        takes nothing; see the file header.
 */
#define MSG_CMD_STATUS_PARSER_PARAMETER_TABLE_SIZE 0u

/*!
 * \brief The parser of command 12, the status command: answers what the clock can say about itself.
 *
 * It takes no option. Its parameter table is empty, so MsgParameterParser answers every option
 * with `Error=3:<option>`, ErrorType::kParameterUnknown, as `12 -V1` is answered with
 * `Error=3:V`, and the private `HandleParameter()` is never called. MsgCmdParser makes the parser
 * and calls Parse(), Process() and SendAnswer() in this order, so an error answered for an option
 * stands in the line before the fields of the answer.
 *
 * The number holds while every overlay is built in; one whose `OVERLAYS_SUPPORT_*` switch is
 * STD_OFF moves the commands behind it down by one.
 */
class MsgCmdStatusParser
    : public MsgParameterParser<MsgCmdStatusParser, MSG_CMD_STATUS_PARSER_PARAMETER_TABLE_SIZE> {
 private:
  friend class MsgParameterParser;

  static constexpr char kVersionShortName{'V'};
  static constexpr char kUptimeDaysShortName{'U'};
  static constexpr char kUptimeHoursShortName{'H'};
  static constexpr char kUptimeMinutesShortName{'N'};
  static constexpr char kIlluminanceShortName{'I'};
  static constexpr char kTemperatureShortName{'T'};
  static constexpr char kAddressShortName{'A'};
  static constexpr char kLinkQualityShortName{'Q'};
  static constexpr char kFreeMemoryShortName{'M'};

  static constexpr ParameterTableType kParameterTable PROGMEM{};

  // Never called - the table above has nothing to match an option against - but the base
  // parser instantiates both, so both have to exist.
  static void HandleParameter(char parameter_short_name, char const* argument,
                              PositionType length) {
    UNUSED(parameter_short_name);
    UNUSED(argument);
    UNUSED(length);
  }
  static void HandleParameter(char parameter_short_name, byte argument) {
    UNUSED(parameter_short_name);
    UNUSED(argument);
  }

 public:
  /*!
   * \brief Makes the parser for the options in `parameter`.
   *
   * \param[in] parameter The parameter string, terminated by a null character; the parser keeps the
   *                      pointer and reads it in Parse().
   */
  explicit constexpr MsgCmdStatusParser(char const* parameter)
      : MsgParameterParser(kParameterTable, parameter) {}

  /*! \brief Destroys the parser. */
  ~MsgCmdStatusParser() = default;

  /*!
   * \brief Sends a number field of the answer, or the field empty where there is no number.
   *
   * \param[in] short_name The short name of the field.
   * \param[in] return_value What the getter of `value` returned: kEOk sends `value`, kENotOk an
   *                         empty field.
   * \param[in] value The number.
   * \param[in] append_space True to send a space after the field.
   */
  static void SendAnswerNumber(char short_name, StdReturnType return_value, uint16_t value,
                               bool append_space = true) {
    if (return_value == kEOk) {
      SendAnswerParameter(short_name, value, append_space);
    } else {
      SendAnswerParameter(short_name, "", append_space);
    }
  }

  /*!
   * \brief Sends the answer: the fields that follow the command number.
   *
   * It sends `V=<version> U=<days> H=<hours> N=<minutes> I=<lux> T=<temperature> A=<address>
   * Q=<link quality> M=<free memory>`:
   * - `V`: the firmware version, WORDCLOCK_VERSION.
   * - `U`, `H` and `N`: the uptime, as whole days, the hours beyond them and the minutes beyond
   *   those.
   * - `I`: the illuminance the light sensor read last, in lux.
   * - `T`: the temperature of the clock chip, as `23.4C`; empty while there is no reading.
   * - `A`: the address of the clock in its network; empty while it has none.
   * - `Q`: the received signal strength, in dBm; empty while there is no network.
   * - `M`: the free memory, in kibibytes; empty where System cannot tell, as in the simulator.
   *
   * Every field that can be absent is sent empty rather than as a zero that reads like a value:
   * no chip, no network, no answer. The getters write the terminator either way, so what is sent
   * after a refusal is the empty string.
   *
   * A space follows each field but the last; MsgCmdParser ends the line.
   */
  static void SendAnswer() {
    System const& system = System::GetInstance();

    char temperature_string[Temperature::kStringLength];
    char address_string[System::kAddressStringLength];
    char link_quality_string[System::kLinkQualityStringLength];
    uint16_t free_memory{0u};

    /*
     * Every value is fetched before anything is sent, rather than inside the call that
     * sends it: an out parameter and the return code that says whether it was written
     * are two arguments of the same call, and which of them is evaluated first is not
     * the compiler's to be trusted with - it reported a zero uptime where a fetched
     * one was right beside it.
     */
    Temperature::GetInstance().GetTemperatureString(temperature_string);
    StdReturnType const free_memory_return = system.GetFreeMemoryInKibibytes(free_memory);
    StdReturnType const address_return = system.GetNetworkAddress(address_string);
    StdReturnType const link_quality_return = system.GetLinkQuality(link_quality_string);

    SendAnswerParameter(kVersionShortName, WORDCLOCK_VERSION);
    // Counted in the core rather than asked of the platform, so there is no return code
    // to weigh: every build has an uptime, and it is always an answer.
    Uptime const& uptime = Uptime::GetInstance();
    SendAnswerParameter(kUptimeDaysShortName, uptime.GetDays());
    SendAnswerParameter(kUptimeHoursShortName, uptime.GetHours());
    SendAnswerParameter(kUptimeMinutesShortName, uptime.GetMinutes());
    SendAnswerParameter(kIlluminanceShortName, Illuminance::GetInstance().GetIlluminance());
    SendAnswerParameter(kTemperatureShortName, temperature_string);
    SendAnswerParameter(kAddressShortName, (address_return == kEOk) ? address_string : "");
    SendAnswerParameter(kLinkQualityShortName,
                        (link_quality_return == kEOk) ? link_quality_string : "");
    // Last field before the command parser's terminating println(), so no trailing
    // separator space.
    SendAnswerNumber(kFreeMemoryShortName, free_memory_return, free_memory, false);
  }

  /*!
   * \brief Completes the command once Parse() has read its options.
   *
   * Nothing is left to do: the command takes no option.
   */
  static void Process() {}
};

#endif  // WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDSTATUSPARSER_H_
