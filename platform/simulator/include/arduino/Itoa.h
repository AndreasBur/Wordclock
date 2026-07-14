#ifndef _ARDUINO_ITOA_H_
#define _ARDUINO_ITOA_H_

/* -------------------------------------------------------------------------- */
/*  itoa                                                                      */
/* -------------------------------------------------------------------------- */
/*  The Arduino core provides itoa(); GCC/glibc does not. Minimal             */
/*  reimplementation matching Arduino's signature (value, buffer, base),      */
/*  including base-10 sign handling.                                          */
/* -------------------------------------------------------------------------- */
inline char* itoa(int Value, char* Buffer, int Base)
{
    char* p = Buffer;
    bool Negative = (Base == 10 && Value < 0);
    unsigned int u = Negative ? (unsigned int)(-Value) : (unsigned int)Value;

    char* start = p;
    do {
        int Digit = u % (unsigned int)Base;
        *p++ = (Digit < 10) ? char('0' + Digit) : char('a' + Digit - 10);
        u /= (unsigned int)Base;
    } while(u);
    if(Negative) *p++ = '-';
    *p = '\0';

    // reverse in place
    for(char* q = p - 1; start < q; ++start, --q) {
        char t = *start; *start = *q; *q = t;
    }
    return Buffer;
}

#endif // _ARDUINO_ITOA_H_
