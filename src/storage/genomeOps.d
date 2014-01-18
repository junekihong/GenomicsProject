module mdp.storage.genomeOps;

import std.conv : to;
import std.stdio;

import vibe.core.file;

import mdp.common.protocol;

private {
    immutable genomePathEntry = PathEntry("genomes");
    immutable genomeRoot = Path(genomePathEntry);
    GenomeInfo[string] genomes;
    ubyte[][string] genomeData;
}

const GenomeInfo[string] getGenomes()
{
    return genomes;
}

void initializeGenomeSystem()
{
    if( !existsFile(genomeRoot) )
    {
        writeln("Creating new genome directory");
        createDirectory(genomeRoot);
        return;
    }

    if( ! getFileInfo(genomeRoot).isDirectory )
    {
        throw new Exception("file \"" ~ to!string(genomeRoot) ~ "\" exists but cannot"
                " be used as the genome root because it is not a directory");
    }


    // genome root exists, so find the genomes in it
    listDirectory(genomeRoot, delegate (info) { addGenomeFromFile(info); return true; } );

    debug(1) {
        writeln("Genome list (", genomes.length, "):");
        foreach( key, _; genomes)
            writeln(key);
    }
}

void addGenomeFromFile(FileInfo info)
{
    // Ingore hidden files
    if( info.name.length == 0 || info.name[0] == '.')
        return;

    uint length = cast(uint)info.size; // TODO loses precision
    genomes[info.name] = GenomeInfo(info.name, length);

    ubyte[] data = new ubyte[info.size];
    FileStream input = openFile(genomeRoot ~ info.name, FileMode.read);
    input.read(data);
    genomeData[info.name] = data;
    input.close();
}

void createGenome(in CreateNewGenome msg)
{
    genomes[msg.name] = GenomeInfo(msg.name, msg.length);
    FileStream strm = openFile(genomeRoot ~ msg.name, FileMode.createTrunc);
    strm.close();
}

void addGenomeData(in InsertGenomeData args)
{
    FileStream strm = openFile(genomeRoot ~ args.name, FileMode.readWrite);
    strm.seek(args.index);
    strm.write(args.data);
    strm.close();
}

const ref GenomeInfo getGenomeInfo(string name)
{
    // TODO this without both lookups
    if( name !in genomes ) {
        throw new Exception("Couldn't find genome info on " ~ name);
    }
    
    return genomes[name];
}

shared(ubyte)[] getGenomeData(in QueryByName msg)
{
    /*std::ifstream strm((genomeRoot / name).generic_string<std::string>().c_str());
    if( !strm )
        throw std::runtime_error("Error opening the file for genome " + name);
    data.resize(length);
    strm.seekg(index, std::ios::beg);
    strm.read(reinterpret_cast<char*>(data.data()), length);
#ifdef DEBUG
    std::cout << "read data from index " << index << " in file " << (genomeRoot / name).generic_string<std::string>().c_str() << ":\n" << std::string(data.begin(), data.end()) << "\n";
#endif*/
    // Copy into shared memory
    shared(ubyte)[] result = new shared(ubyte)[msg.length];
    result[] = genomeData[msg.name][msg.startIndex .. (msg.startIndex + msg.length)][];
    return result;
}
