#include <sstream>

#include "fasta.h"

std::string readFastaString(std::istream& input)
{
    if( input.peek() != '>' ) {
        throw 4;
    }

    while( input && input.get() != '\n' ) ;
    if( !input ) {
        throw 4;
    }

    std::ostringstream strm;
    while( input && input.peek() != '>' )
    {
        char ch = input.get();
        if( isalpha(ch) ) {
            strm << ch;
        }
    }
    return strm.str();
}
