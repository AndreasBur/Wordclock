// The conversions of StringTools at the ends of their ranges.

#include "check.h"
#include "cases.h"

#include <cstring>
#include <limits>

#include "StringTools.h"

using ResultType = StringTools::ResultType;

/*
 * Converts `text` into a fresh value and reports whether the result and the value are the ones
 * expected. The value starts at `start`, so that a conversion that leaves it alone shows.
 */
template<typename T>
bool Converts(const char* text, ResultType result, T expected, T start = T{42})
{
    T value{start};
    StringTools::PositionType position{0u};
    return (StringTools::StringTo(text, position, value) == result) && (value == expected);
}

// The one number whose magnitude an int32_t cannot hold: it used to be negated as an int32_t.
void TestItoaWritesEveryInt32()
{
    char text[12];

    char* end = StringTools::Itoa(std::numeric_limits<int32_t>::min(), text);
    Expect(std::strcmp(text, "-2147483648") == 0, "Itoa writes INT32_MIN");
    Expect(end == text + 11, "Itoa returns the terminator after INT32_MIN");

    StringTools::Itoa(std::numeric_limits<int32_t>::max(), text);
    Expect(std::strcmp(text, "2147483647") == 0, "Itoa writes INT32_MAX");
    StringTools::Itoa(0, text);
    Expect(std::strcmp(text, "0") == 0, "Itoa writes 0");
    StringTools::Itoa(-7, text);
    Expect(std::strcmp(text, "-7") == 0, "Itoa writes a negative digit");
}

// An empty string used to make the index of its last character wrap to the largest unsigned.
void TestStringReverseTakesAnyLength()
{
    char empty[4]{"\0xy"};
    StringTools::StringReverse(empty);
    Expect(std::memcmp(empty, "\0xy", 4) == 0, "an empty string reverses to itself, nothing after it moves");

    char one[]{"a"};
    StringTools::StringReverse(one);
    Expect(std::strcmp(one, "a") == 0, "one character reverses to itself");

    char even[]{"abcd"};
    StringTools::StringReverse(even);
    Expect(std::strcmp(even, "dcba") == 0, "an even length reverses");

    char odd[]{"abc"};
    StringTools::StringReverse(odd);
    Expect(std::strcmp(odd, "cba") == 0, "an odd length reverses");
}

// A size of 0 has no room for the terminator, and used to write it before the buffer.
void TestStringCopyIntoNoRoom()
{
    char buffer[4]{"xyz"};
    StringTools::StringCopy(buffer + 1, "abc", 0);
    Expect(std::strcmp(buffer, "xyz") == 0, "a copy of size 0 writes nothing");

    StringTools::StringCopy(buffer, "abcdef", sizeof(buffer));
    Expect(std::strcmp(buffer, "abc") == 0, "a copy is cut to the buffer and terminated");
}

// Beyond the type but within a long, a number used to be cast: 200 into an int8_t was -56.
void TestSignedConversionKeepsToTheType()
{
    Expect(Converts<int8_t>("127", ResultType::kOk, 127), "127 fits an int8_t");
    Expect(Converts<int8_t>("-128", ResultType::kOk, -128), "-128 fits an int8_t");
    Expect(Converts<int8_t>("200", ResultType::kOverflow, 127), "200 is beyond an int8_t, at its top");
    Expect(Converts<int8_t>("-200", ResultType::kOverflow, -128), "-200 is beyond an int8_t, at its bottom");
    Expect(Converts<int16_t>("40000", ResultType::kOverflow, 32767), "40000 is beyond an int16_t");
    Expect(Converts<int64_t>("99999999999999999999", ResultType::kOverflow, std::numeric_limits<int64_t>::max()),
           "a number beyond a long sets an int64_t to its top, not leaves it");
    Expect(Converts<int8_t>("x", ResultType::kNoValue, 42), "no number leaves the value");
}

// strtoul() negates a negative number as an unsigned long, which made -1 a valid uint32_t on the AVR.
void TestUnsignedConversionRefusesANegativeNumber()
{
    Expect(Converts<uint32_t>("-1", ResultType::kOverflow, 0u), "-1 is below a uint32_t");
    Expect(Converts<uint8_t>(" -5", ResultType::kOverflow, 0u), "-5 is below a uint8_t, past white space");
    Expect(Converts<uint8_t>("-0", ResultType::kOk, 0u), "-0 is 0");
    Expect(Converts<uint8_t>("+7", ResultType::kOk, 7u), "a plus sign is taken");
    Expect(Converts<uint8_t>("255", ResultType::kOk, 255u), "255 fits a uint8_t");
    Expect(Converts<uint8_t>("256", ResultType::kOverflow, 255u), "256 is beyond a uint8_t");
}

// A negative number out of range used to set the smallest positive value, and a float beyond its
// range but within a double's was cast to infinity and taken.
void TestFloatConversionKeepsToTheType()
{
    Expect(Converts<float>("1.5", ResultType::kOk, 1.5f), "1.5 fits a float");
    Expect(Converts<float>("1e39", ResultType::kOverflow, std::numeric_limits<float>::max()),
           "1e39 is beyond a float, at its top");
    Expect(Converts<float>("-1e39", ResultType::kOverflow, std::numeric_limits<float>::lowest()),
           "-1e39 is beyond a float, at its bottom");
    Expect(Converts<double>("-1e999", ResultType::kOverflow, std::numeric_limits<double>::lowest()),
           "-1e999 is beyond a double, at its bottom");
    Expect(Converts<double>("inf", ResultType::kOverflow, std::numeric_limits<double>::max()),
           "inf is beyond a double");
    Expect(Converts<double>("1e-999", ResultType::kOverflow, 0.0), "1e-999 is too close to zero, and 0");
}
