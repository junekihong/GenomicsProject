module mdp.client.fasta;

import std.uni;

import vibe.core.file;

const(ubyte)[] readFastaString(FileStream input)
{
    // TODO rewrite with some kind of appender?
    const(ubyte)[] result = [];

    if( input.peek()[0] != '>' )
        throw new Exception("Unexpected character to start FASTA input.");

    ubyte[1] char_buf;
    while( input.read(char_buf), char_buf[0] != '\n' ) { }

    while( input.peek()[0] != '>' )
    {
        input.read(char_buf);
        if( isAlpha(char_buf[0]) )
            result ~= char_buf;
    }
    return result;
}
