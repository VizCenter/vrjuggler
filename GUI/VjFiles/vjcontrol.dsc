chunk vjcontrol "VjControl Configuration" "Configuration information for the VjControl GUI itself"
  Name String 1 "Name" "Unique name of an instance of this chunk type"
  looknfeel String 1 "Look and Feel" vj_enumeration { "Java" "Motif" "Windows" } "The visual 'Look and Feel' to use for the GUI"
  host String 1 "Default Host" "A remote host that VjControl should try to connect to"
  port Int 1 "Default Port" "Network Port that VjControl should try to connect to"
  descfiles String -1 "Preload ChunkDesc Files" "The names of Chunk Description files that VjControl should load at startup"
  chunkfiles String -1 "Preload ConfigChunk Files" "The names of ConfigChunk files that VjControl should load at startup"
  defaultchunkfiles String -1 "Default ConfigChunk Files" "Files that contain default values for new ConfigChunks"
  fontname String 1 "Font Name" vj_enumeration { "Courier" "Dialog" "DialogInput" "Helvetica" "TimesRoman" } "Name of the font to use for the GUI"
  fontsize Int 1 "Font Size" "Font size to use for GUI"
  windowsize Int 2 "Max Window Size" vj_valuelabels { "Width" "Height" } "Maximum size to open vjcontrol windows"
  end
chunk vjIncludeFile "Include File" "File that should be loaded along with this one"
  Name String 1 "Name" "Unique name of an instance of this chunk type"
  end
chunk PerfData "Performance Data Display" "Information for displaying data from perfbuffers with a matching name"
  Name String 1 "Name" "Unique name of an instance of this chunk type"
  labels String -1 "Labels" "Labels for Perf Buffer phases"
  end
End
