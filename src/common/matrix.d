module mdp.common.matrix;

import std.algorithm : max;
import std.conv;
import std.format;

import mdp.common.location;

struct Matrix
{
    private:
    void allocateMatrix();

    int _length;
    int _width;

    public:
    int[][] matrix;

    this(int _l, int _w)
    {
        allocateMatrix();
    }

    this(const(int)[] topNumbers, const(int)[] leftNumbers)
    {
        _length = cast(int)topNumbers.length; // FIXME cast to int loses precision
        _width = cast(int)leftNumbers.length;
        allocateMatrix();
        initialize(topNumbers, leftNumbers);
    }

    this(in Matrix matrix)
    {
        this = matrix;
    }

    private void allocateMatrix()
    {
        matrix.length = _width + 1;
        foreach( ref row; matrix )
        {
            row.length = _length + 1;
        }
    }

    private void initialize(in int[] topNumbers, in int[] leftNumbers)
    {
        assert( topNumbers.length + 1 == matrix[0].length );
        matrix[0][1 .. $] = topNumbers;

        assert( leftNumbers.length + 1 == matrix.length );
        foreach( ulong i; 0 .. leftNumbers.length )
            matrix[i + 1][0] = leftNumbers[i];
    }

    private void resize(int newLength, int newWidth)
    {
        _length = newLength;
        _width = newWidth;
        allocateMatrix();
    }


    @property
    int length() inout { return _length; }
    int width() inout { return _width; }

    void resize(int newLength, int newWidth);

    void opAssign(in Matrix other)
    {
        _width = other._width;
        _length = other._length;
        matrix.length = _width + 1;
        foreach( ulong idx; 0 .. matrix.length )
            matrix[idx] = other.matrix[idx].dup;
    }

    void toString(scope void delegate(const(char)[]) sink, FormatSpec!char fmt) const
    {
        sink(to!string(_length));
        sink(" ");
        sink(to!string(_width));
        sink("\n");

        foreach( row ; matrix )
        {
            foreach( int entry ; row )
            {
                sink(to!string(entry));
                sink("\t");
            }
            sink("\n");
        }
    }

    LocationValuePair localAlignment(const(ubyte)[] topGenome, const(ubyte)[] leftGenome)
    {
        LocationValuePair pair;
        Location location;
        int maxval = 0;
        for( int i = 1; i <= _width; ++i )
        {
            int genomeIndex1 = i -1;
            for( int j = 1; j <= _length; ++j)
            {
                int genomeIndex2 = j-1;
                int V1 = matrix[i-1][j] - 6;
                int V2 = matrix[i][j-1] - 6;
                int V3 = matrix[i-1][j-1];
                char genome1 = leftGenome[genomeIndex1];
                char genome2 = topGenome[genomeIndex2];
                if( genome1 == genome2 )
                    V3 += 2;
                else
                    V3 -= 4;

                //int[] values = [V1, V2, V3, 0];
                int value = max(V1, V2, V3, 0);
                matrix[i][j] = value;

                // maintain the maximum value
                if( maxval < value ) {
                    maxval = value;

                    // NOTE. matrix is padded by an extra row and column in the beginning.
                    // To get the corresponding index within the genomes, You will need to subtract each value by 1.
                    location.row = i;
                    location.column = j;
                }
            }
        }
        pair.value = maxval;
        pair.location = location;

        return pair;
    }
}
