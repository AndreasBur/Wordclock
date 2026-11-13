// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the clock knows about the temperature.
 *
 * The same arrangement as Illuminance and its BH1750: the core owns the driver, reaches it by
 * header name, and every platform supplies its own DS3231.h behind that name.
 *
 * There is no reading until the chip has answered once, and a build with no chip on the bus never
 * gets one. That is why the value comes with a return code rather than as a number: the overlay has
 * to be able to stay away, and a temperature of zero is a perfectly ordinary reading it must not be
 * confused with.
 */

#ifndef WORDCLOCK_SRC_TEMPERATURE_TEMPERATURE_H_
#define WORDCLOCK_SRC_TEMPERATURE_TEMPERATURE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "DS3231.h"

/*!
 * \brief The temperature: the last reading of the DS3231, as a number and as text.
 *
 * There is no reading until the chip has answered once, so every function that gives one says
 * whether there is.
 */
class Temperature {
 public:
  /*! \brief A temperature in tenths of a degree Celsius, signed. */
  using TemperatureType = DS3231::TemperatureType;

  /*! \brief The tenths in a degree, the unit of a TemperatureType. */
  static constexpr TemperatureType kTenthsPerDegree{DS3231::kTenthsPerDegree};

  /*!
   * \brief The length of the buffer GetTemperatureString() writes to.
   *
   * "-12.3C" and its terminator, which is the longest a reading the chip can produce makes: it
   * measures from -40 to +85 degrees.
   */
  static constexpr byte kStringLength{6u + 1u};

  /*!
   * \brief The length of the buffer GetTemperatureStringToShow() writes to.
   *
   * The shown form carries the degree sign as well, which is one byte more - it is Latin-1
   * here, the same single-byte encoding the font tables are indexed by.
   */
  static constexpr byte kStringToShowLength{kStringLength + 1u};

 private:
  static constexpr char kDegreeUnit{'C'};
  /*
   * Latin-1 0xB0, which is what the font tables index by and what the letter table's
   * umlauts already are. It stays out of the reported form on purpose - see
   * GetTemperatureString() below.
   */
  static constexpr char kDegreeSign{'\xB0'};
  static constexpr char kDecimalPoint{'.'};

  constexpr Temperature() {}
  ~Temperature() = default;

  DS3231 sensor_;

  /*
   * Sign, whole part and tenth are taken apart before they are written, so that the
   * digits themselves are always those of a positive number - a remainder of a negative
   * value is negative in C, and would print a second minus in the middle of the
   * reading.
   */
  static char* AppendSign(TemperatureType tenths, char* string) {
    if (tenths >= 0) {
      return string;
    }

    return AppendChar('-', string);
  }
  static TemperatureType WholeDegrees(TemperatureType tenths) {
    return static_cast<TemperatureType>(Absolute(tenths) / kTenthsPerDegree);
  }
  static TemperatureType TenthOfDegree(TemperatureType tenths) {
    return static_cast<TemperatureType>(Absolute(tenths) % kTenthsPerDegree);
  }
  static constexpr TemperatureType Absolute(TemperatureType value) {
    return (value < 0) ? static_cast<TemperatureType>(-value) : value;
  }

  static char* AppendChar(char character, char* string) {
    string[0u] = character;
    return &string[1u];
  }
  static char* AppendDegreeSign(bool wanted, char* string) {
    if (!wanted) {
      return string;
    }

    return AppendChar(kDegreeSign, string);
  }

  StdReturnType AppendReading(char* string, bool with_degree_sign) const;
  static char* AppendNumber(TemperatureType value, char* string) {
    itoa(value, string, 10u);
    return &string[DigitsOfNumber(value)];
  }

 public:
  /*! \brief Returns the one temperature. */
  static Temperature& GetInstance() {
    static Temperature singleton_instance;
    return singleton_instance;
  }

  /*! \brief Returns the scheduler ticks between two calls of Task(). */
  byte GetTaskCycle() const { return sensor_.GetTaskCycle(); }

  /*!
   * \brief Gives the temperature last read.
   *
   * \param[out] temperature The temperature, in tenths of a degree Celsius.
   * \return kEOk, or kENotOk while there is no reading, which is the answer a build without the
   *         chip keeps giving; `temperature` is then left as it was.
   */
  StdReturnType GetTemperature(TemperatureType& temperature) const {
    return sensor_.GetTemperature(temperature);
  }

  /*! \brief Returns whether there is a reading. */
  bool IsTemperatureAvailable() const {
    TemperatureType unused{0};
    return GetTemperature(unused) == kEOk;
  }

  /*!
   * \brief Writes the reading as it is reported, "23.4C".
   *
   * Written into the caller's buffer rather than kept here, because the overlay needs it to
   * stand still while its text scrolls, and the status command needs it once.
   *
   * No degree sign in this one, and that is the point of the split: this string travels the
   * serial line and the web socket, and a web socket text frame has to be valid UTF-8. A raw
   * Latin-1 0xB0 in it makes the browser close the connection with a protocol error rather
   * than draw one character oddly.
   *
   * \param[out] string A buffer of kStringLength.
   * \return kEOk, or kENotOk if there is no reading; `string` is then an empty string - the degree
   *         it would otherwise print is the one thing a reader must not mistake for a measurement.
   */
  StdReturnType GetTemperatureString(char* string) const { return AppendReading(string, false); }

  /*!
   * \brief Writes the same reading as it is shown, "23.4°C".
   *
   * The display indexes the font tables by Latin-1 byte, so the degree sign can be one here:
   * the byte 0xB0.
   *
   * \param[out] string A buffer of kStringToShowLength.
   * \return kEOk, or kENotOk if there is no reading; `string` is then an empty string.
   */
  StdReturnType GetTemperatureStringToShow(char* string) const {
    return AppendReading(string, true);
  }

  /*! \brief Lets the DS3231 driver of the platform read the temperature. */
  void Task() { sensor_.Task(); }
};

#endif  // WORDCLOCK_SRC_TEMPERATURE_TEMPERATURE_H_
