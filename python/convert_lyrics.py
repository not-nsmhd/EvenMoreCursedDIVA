import json
import io
import sys
import array
import os
import xml.etree.ElementTree as XmlET

class SongLyric:
    StartTime = 0.0
    EndTime = 0.0
    Color = 0
    Text = None
    
    def __init__(self, start, end, color, text):
        self.StartTime = start
        self.EndTime = end
        self.Color = color
        self.Text = text

if len(sys.argv) < 4:
    print(f"Usage: {sys.argv[0]} <pv_db.txt file> <pv_db entry index> <DSC script file>")
    exit()

opcode_dbFile = open("opcode_db.json")
opcode_db = json.load(opcode_dbFile)

opcodes = list(opcode_db)
opcodeSizes = dict()

for i in range(len(opcode_db)):
    key = opcodes[i]
    
    if "info_f" in opcode_db[key]:    
        opcode_info = opcode_db[key]["info_f"]
        opcodeSizes[int(opcode_info["id"])] = int(opcode_info["len"])

opcode_dbFile.close()

# ---------------

pvDbFile_path = sys.argv[1]
pvIndexString = sys.argv[2]
chartFile = sys.argv[3]

# ---------------

pvDbFile = open(pvDbFile_path, "r", encoding='utf-8', newline='\n')
pvIndex = int(pvIndexString)
pvLyricPrefix = f"pv_{pvIndex:03d}.lyric"

pvLyrics = [];

while True:
    line = pvDbFile.readline()
    if (len(line) == 0):
        break
        
    if line.startswith(pvLyricPrefix) == False:
        continue
        
    pvLyricNameAndValue = line.split('=', 1);
    pvLyricString = pvLyricNameAndValue[1].rstrip('\n')
    pvLyrics.append(pvLyricString)

pvDbFile.close()

# ---------------

dscFile = open(chartFile, "rb")

dscFile.seek(0, io.SEEK_END)
dscDataSize = dscFile.tell()
dscFile.seek(0, io.SEEK_SET)

dscData = array.array("i")
dscData.fromfile(dscFile, (int)(dscDataSize / 4))

dscFile.close()

# ---------------

convLyrics = []

opcodeIdx = 1
nextCommandTime_divaTime = 0

lyricIndex = 0

while True:
    if opcodeIdx >= (int)(dscDataSize / 4):
        break

    opcode = dscData[opcodeIdx]
    
    match opcode:
        case 1: # TIME
            nextCommandTime_divaTime = dscData[opcodeIdx + 1]
        case 24: # LYRIC
            pvLyricIndex = dscData[opcodeIdx + 1] - 1
            
            color = dscData[opcodeIdx + 2] + 2**32
            
            lyric = SongLyric(nextCommandTime_divaTime / 100000.0, 0.0, color, None)
            
            if (pvLyricIndex != -1):
                lyric.Text = pvLyrics[pvLyricIndex]
            
            if lyricIndex > 0:
                convLyrics[lyricIndex - 1].EndTime = nextCommandTime_divaTime / 100000.0 - 0.001
            
            prevLyric = lyric
            convLyrics.append(lyric)
            lyricIndex += 1
            
    opcodeIdx += opcodeSizes[opcode] + 1
    
# ---------------

xmlLyricsList = XmlET.Element("Lyrics")

for l in convLyrics:
    if (l.Text == None or len(l.Text) == 0):
        continue
        
    xmlLyric = XmlET.SubElement(xmlLyricsList, "Lyric",
    {
        "Start": "{0:.3f}".format(l.StartTime),
        "End": "{0:.3f}".format(l.EndTime),
        "Color": "{0:X}".format(l.Color)
    })
    
    xmlLyric.text = l.Text
    
    XmlET.indent(xmlLyricsList, space="\t")
    
xmlLyricsTree = XmlET.ElementTree(xmlLyricsList)

xmlLyricsName = f"pv_{pvIndex}_lyrics.xml"
with open(xmlLyricsName, mode='wb') as xmlOutFile:
    xmlLyricsTree.write(xmlOutFile, encoding="utf-8")

