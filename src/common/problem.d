module mdp.common.problem;

import msgpack;

struct ProblemID
{
    public:
    ulong idnum;

    private:
    static ulong next_problem_id; // Should only be on leader

    public:
    this( ulong _id = 0)
    {
        idnum = _id;
    }

    void increment() {
        ++idnum;
    }
}

struct ProblemDescription
{
    public:
    ProblemID problemID;
    
    // the int value to the upper-left of this problem chunk
    int corner;
    int[] top_numbers;
    int[] left_numbers;
    ubyte[] top_genome;
    ubyte[] left_genome;

    this(ProblemDescription other)
    {
        this = other;
    }
    this(ProblemID id, int c, immutable(int)[] tn, immutable(int)[] ln, immutable(ubyte)[] tg, immutable(ubyte)[] lg) immutable
    {
		problemID = id;
		corner = c;
		top_numbers = tn;
		left_numbers = ln;
		top_genome = tg;
		left_genome = lg;
    }

    void opAssign(ProblemDescription x) pure
    {
        problemID = x.problemID;
        corner = x.corner;
        top_numbers = x.top_numbers.dup;
        left_numbers = x.left_numbers.dup;
        top_genome = x.top_genome.dup;
        left_genome = x.left_genome.dup;
    }
    void opAssign(ProblemDescription x) shared pure
    {
        problemID = x.problemID;
        corner = x.corner;
        top_numbers.length = x.top_numbers.length;
        top_numbers[] = x.top_numbers[];
        left_numbers.length = x.left_numbers.length;
        left_numbers[] = x.left_numbers[];
        top_genome.length = x.top_genome.length;
        top_genome[] = x.top_genome[];
        left_genome.length = x.left_genome.length;
        left_genome[] = x.left_genome[];
    }

    @property ProblemDescription dup() inout
    {
        ProblemDescription result;
        result.problemID = problemID;
        result.corner = corner;
        result.top_numbers = top_numbers.dup;
        result.left_numbers = left_numbers.dup;
        result.top_genome = top_genome.dup;
        result.left_genome = left_genome.dup;
        return result;
    }

    @property immutable(ProblemDescription) idup()
    {
        return immutable ProblemDescription(
                problemID, corner,
                top_numbers.idup,
                left_numbers.idup,
                top_genome.idup,
                left_genome.idup
                );
	}

    bool compareID(string op)(ProblemDescription x) inout
    {
        return problemID.opCmp!op(x.problemID);
    }
    bool compareContents(string op)(ProblemDescription other) inout
    {
        if( top_numbers == other.top_numbers )
        {
            if( left_numbers == other.left_numbers )
            {
                if( top_genome == other.top_genome )
                    return left_genome.opCmp!op(other.left_genome);
                else
                    return top_genome.opCmp!op(other.top_genome);
            }
            else
                return left_numbers.opCmp!op(other.left_numbers);
        }
        else
            return top_numbers.opCmp!op(other.top_numbers);
    }

    void toMsgpack(Packer)(ref Packer packer) const
    {
        packer.beginArray(6);
        packer.pack(problemID);
        packer.pack(corner);
        packer.pack(top_numbers);
        packer.pack(left_numbers);
        packer.beginArray(top_genome.length);
        foreach( b; top_genome )
            packer.pack(b);
        packer.beginArray(left_genome.length);
        foreach( b; left_genome )
            packer.pack(b);
    }

    void fromMsgpack(ref Unpacker unpacker)
    {
        auto length = unpacker.beginArray();
        if( length != 6 )
            throw new MessagePackException("The size of the array is mismatched");
        import std.stdio : writeln;
        unpacker.unpack(problemID);
        unpacker.unpack(corner);
        unpacker.unpack(top_numbers);
        unpacker.unpack(left_numbers);
        // the C++ implementation doesn't use a binary array like the D one does,
        // so we need to do it ourselves.
        // TODO when the project is all in D, then we can get rid of this whole method
        top_genome.length = unpacker.beginArray();
        foreach( ref b; top_genome)
            unpacker.unpack(b);
        left_genome.length = unpacker.beginArray();
        foreach(ref b; left_genome)
            unpacker.unpack(b);
    }
    void fromMsgpack(ref Unpacker unpacker) shared
    {
        auto length = unpacker.beginArray();
        if( length != 6 )
            throw new MessagePackException("The size of the array is mismatched");
        import std.stdio : writeln;
        unpacker.unpack(problemID);
        unpacker.unpack(corner);
        unpacker.unpack(top_numbers);
        unpacker.unpack(left_numbers);
        // the C++ implementation doesn't use a binary array like the D one does,
        // so we need to do it ourselves.
        // TODO when the project is all in D, then we can get rid of this whole method
        top_genome.length = unpacker.beginArray();
        foreach( ref b; top_genome)
            unpacker.unpack(b);
        left_genome.length = unpacker.beginArray();
        foreach(ref b; left_genome)
            unpacker.unpack(b);
    }
}
