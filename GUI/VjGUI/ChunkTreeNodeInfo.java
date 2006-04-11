package VjGUI;

import VjConfig.*;

public class ChunkTreeNodeInfo {

  public ConfigChunkDB db;
  public ConfigChunk ch;
  public String s;
  public boolean isdesc;
    public int childchunks;

  public boolean isDescNode() {
    return isdesc;
  }
  public boolean isChunkNode() {
    return (ch != null);
  }
    /*
  public ChunkTreeNodeInfo (ConfigChunkDB _db, ChunkDesc _cd, ConfigChunk _ch) {
    db = _db;
    ch = _ch;
    s = null;
    isdesc = false;
    childchunks = true;
  }
    */

  public ChunkTreeNodeInfo (ConfigChunkDB _db, ConfigChunk _ch) {
    db = _db;
    ch = _ch;
    s = null;
    isdesc = false;
    childchunks = 1;
  }

  public ChunkTreeNodeInfo (ConfigChunkDB _db) {
    db = _db;
    ch = null;
    s = null;
    isdesc = false;
    childchunks = 0;
  }

  public ChunkTreeNodeInfo (ConfigChunkDB _db, String _s) {
    this(_db, _s, false);
    childchunks = 0;
  }

  public ChunkTreeNodeInfo (ConfigChunkDB _db, String _s, boolean _isdesc) {
    /* for orgtree label nodes and chunkdescs(maybe) */
    db = _db;
    ch = null;
    s = _s;
    isdesc = _isdesc;
    childchunks = 0;
  }

  public String toString () {
    if (s != null)
      return s;
    else if (ch != null)
      return ch.getName();
    else if (db != null)
      return db.name;
    else
      return "empty";
  }

}

