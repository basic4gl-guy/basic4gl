//---------------------------------------------------------------------------
#pragma hdrstop

#include "streaming.h"

//---------------------------------------------------------------------------

std::string ReadString (std::istream& stream) {

    // Get length of string
    long length = ReadLong (stream);
    std::string s;

    // Reserve enough space to store it
    s.reserve (length);

    // If longer than 4096 characters, read in chunks
    char buffer [4097];
    while (length > 4096) {
        stream.read (buffer, 4096);
        buffer [4096] = 0;              // Terminate string
        s = s + buffer;
        length -= 4096;
    }

    // Read < 4096 bit
    stream.read (buffer, length);
    buffer [length] = 0;                // Terminate string
    s = s + buffer;

    return s;
}

#pragma package(smart_init)
