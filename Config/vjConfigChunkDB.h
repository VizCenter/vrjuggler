
#ifndef _VJ_CONFIGCHUNKDB_H_
#define _VJ_CONFIGCHUNKDB_H_

#include <vjConfig.h>
#include <Config/vjConfigChunk.h>
#include <Config/vjChunkDescDB.h>



//------------------------------------------------------------------
//: Database of ConfigChunks
//
// The vjConfigChunkDB is a general-purpose container for 
// ConfigChunks, with functionality for reading/writing 
// files of ConfigChunks and querying for sets of configchunks 
// with specific properties.
//
// @author Christopher Just
// October 1997
//
//------------------------------------------------------------------
class vjConfigChunkDB {


private:

    //: We need access to a ChunkDescDB for the >> operator.
    vjChunkDescDB *descs;

    //: vector of ConfigChunks
    vector<vjConfigChunk*> chunks;

public:

    //: Constructor
    //! PRE: true
    //! POST: self is created. Chunks is empty.
    //! ARGS: d - a pointer to a vjChunkDescDB.  Note that
    //+       files cannot be read until a ChunkDescDB is supplied.
    vjConfigChunkDB (vjChunkDescDB *d = NULL);


    //: Destructor
    //! PRE: true
    //! POST: self is destroyed and its associated memory freed.
    //+       This includes all ConfigChunks stored in self, but
    //+       not any ChunkDescDB
    ~vjConfigChunkDB ();



    //: copy constructor
    vjConfigChunkDB (vjConfigChunkDB& db);



    vjConfigChunkDB& operator = (vjConfigChunkDB& db);



    //: Sets the vjChunkDescDB that self uses for creating Chunks
    //! PRE: d points to a vjChunkDescDB
    //! POST: self uses d for all vjChunkDesc lookups
    //! NOTE: Using this function on a nonempty ConfigChunkDB
    //+       might be dangerous.
    void setChunkDescDB (vjChunkDescDB *d);
  


    //: Returns a pointer to self's ChunkDescDB
    //! PRE: true
    //! RETURNS: d - the value of desc (remember, this could be NULL)
    vjChunkDescDB* getChunkDescDB();



    //! RETURNS: true - if self contains no vjConfigChunks
    //! RETURNS: false - otherwise
    bool isEmpty();



    //: Removes and destroys all vjConfigChunks in self
    //! PRE: true
    //! POST: All ConfigChunks in self have been removed and their
    //+       associated memory freed.
    void removeAll();



    /* accessing individual chunks:  We ought to be able to do this by:
     *   1. giving a name of a specific chunk
     *   2. getting all the chunks of a specific type, or at least the
     *      names of all the chunks of a specific type.
     *      Maybe just a vector of names?
     */



    //: Finds a chunk with a given name
    //! PRE: true
    //! ARGS: name - a non-NULL C string.
    //! RETURNS: p - A pointer to a ConfigChunk in self whose name matches
    //+          the argument, or NULL if no such element exists.
    //! NOTE: The memory associated with the return value belongs to
    //+       the ConfigChunkDB, and should not be delete()d
    vjConfigChunk *getChunk (char *name);



    //: Returns all chunks of a given type.
    //! PRE: true;
    //! POST: true;
    //! ARGS: property - a non-NULL C string, the name of a property.
    //! ARGS: value - value of a property.  non-NULL C string.
    //! RETURNS: p - Pointer to a vector of ConfigChunk* containing
    //+          all vjConfigChunks in self that have the specified
    //+          type (ie vjChunkDesc token).
    //! NOTE: The memory for the vector should be deleted by the
    //+       caller when it is no longer needed.  The individual
    //+       vjConfigChunks in the vector should not be freed.
    vector<vjConfigChunk*>* getMatching (char *mytypename) {
	return getMatching ("type", mytypename);
    }



    //: Returns all chunks with a given property and value.
    //! PRE: true
    //! ARGS: property - a non-NULL C string, the name of a property.
    //! ARGS: value - value of a property.  If char*, must be non-NULL
    //! RETURNS: p - Pointer to a vector of ConfigChunk* containing
    //+          all ConfigChunks in self that have a property whose
    //+          name matches the first argument and which has as
    //+          one of its values the value given in the second
    //+          argument.
    //! NOTE: The memory for the vector should be deleted by the
    //+       caller when it is no longer needed.  The individual
    //+       ConfigChunks in the vector should not be freed.
    vector<vjConfigChunk*>* getMatching (char *property, char *value);
    vector<vjConfigChunk*>* getMatching (char *property, int value);
    vector<vjConfigChunk*>* getMatching (char *property, float value);



    //: Alias for removeAll()
    bool erase ();



    //: Removes the named chunk
    //! POST: If a ConfigChunk with the specified name exists
    //+       in self, it is removed and its memory freed.
    //! RETURNS: true - a matching chunk was found.
    //! RETURNS: false - otherwise.
    int removeNamed (char *name);



    //: Removes all chunks with a matching property and value
    //! PRE: true
    //! POST: All selected ConfigChunks in self are removed and
    //+       their memory freed.  The selection criteria is 
    //+       the same as for getMatching(property, value)
    //! ARGS: property - Name of a property. Non-NULL C string.
    //! ARGS: value - value to match.  If char*, must be non-NULL
    //+       C string.
    //! RETURNS: n - Number of ConfigChunks removed.
    int removeMatching (char *property, int value);
    int removeMatching (char *property, float value);
    int removeMatching (char *property, char *value);



    /* IO functions: */

    //! POST: A text representation of self is written to out
    //! ARGS: out - an output stream
    //! ARGS: self - a ConfigChunkDB
    //! RETURNS: out
    friend ostream& operator << (ostream& out, vjConfigChunkDB& self);



    //! POST: ConfigChunks are read from in and added to self until
    //+       an 'end' token or EOF is read.
    //! ARGS: in - an input stream.
    //! ARGS: self - a ConfigChunkDB.
    //! RETURNS: in
    //! NOTE: Any ConfigChunks in self before the >> operation remain,
    //+       unless they have the same name as a newly read chunk
    //+       in which case they are replaced by the newer chunks.
    friend istream& operator >> (istream& in, vjConfigChunkDB& self);
    


    //: loads ConfigChunks from the given file
    //! ARGS: fname - name of file to read from.
    //! RETURNS: true - file was opened succesfully.
    //! RETURNS: false - otherwise.
    //! NOTE: This function calls operator >> to do its work.
    //! NOTE: The return value only considers opening the file,
    //+       and does not account for parsing or other reading 
    //+       errors.
    bool load (const char *fname);



    //: writes ConfigChunks to the given file
    //! ARGS: fname - name of file to write to.
    //! RETURNS: true - file was opened succesfully.
    //! RETURNS: false - otherwise.
    //! NOTE: This function calls operator << to do its work.
    bool save (const char *fname);
    
};


#endif
