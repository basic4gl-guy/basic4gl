//---------------------------------------------------------------------------
/*  Streaming.h

    Created 23-Jan-2005: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
*/

#ifndef StreamingH
#define StreamingH
//---------------------------------------------------------------------------

#include <iostream>
#include <string>

#ifndef byte
typedef unsigned char   byte;
#endif

// Inline streaming functions.
// Should be used by streaming routines where appropriate, as this
// provides a central point for dealing with endian issues (should the
// compiler be ported to a big-endian machine).
// Long and short integers are stored in little-endian format within the
// stream.
inline void WriteLong (std::ostream& stream, long l) {
    stream.write ((char *) &l, sizeof (l));
}
inline void WriteShort (std::ostream& stream, short s) {
    stream.write ((char *) &s, sizeof (s));
}
inline void WriteByte (std::ostream& stream, byte b) {
    stream.write ((char *) &b, sizeof (b));
}
inline void WriteFloat(std::ostream& stream, float f) {
    stream.write((char *) &f, sizeof(f));
}
inline long ReadLong (std::istream& stream) {
    long l;
    stream.read ((char *) &l, sizeof (l));
    return l;
}
inline short ReadShort (std::istream& stream) {
    short s;
    stream.read ((char *) &s, sizeof (s));
    return s;
}
inline byte ReadByte (std::istream& stream) {
    byte b;
    stream.read ((char *) &b, sizeof (b));
    return b;
}
inline float ReadFloat(std::istream& stream) {
    float f;
    stream.read((char *) &f, sizeof(f));
    return f;
}
inline void WriteString (std::ostream& stream, std::string const& s) {
    WriteLong (stream, s.length ());
    stream.write (s.c_str (), s.length ());
}
std::string ReadString (std::istream& stream);

#endif
