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
    Text = ""
    
    def __init__(self, start, end, color, text):
        StartTime = start
        EndTime = end
        Color = color
        Text = text

if len(sys.argv) < 2:
    print("Please specify the chart file you want to convert as a command-line argument")
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

chartFile = sys.argv[1]
dscFile = open(chartFile, "rb")

dscFile.seek(0, io.SEEK_END)
dscDataSize = dscFile.tell()
dscFile.seek(0, io.SEEK_SET)

dscData = array.array("i")
dscData.fromfile(dscFile, (int)(dscDataSize / 4))

dscFile.close()

xmlLyrics = XmlET.Element("Lyrics")
firstLyric = True

opcodeIdx = 1
nextCommandTime_divaTime = 0

while True:
    if opcodeIdx >= (int)(dscDataSize / 4):
        break

    opcode = dscData[opcodeIdx]
    
    match opcode:
        case 1: # TIME
            nextCommandTime_divaTime = dscData[opcodeIdx + 1]
        case 24: # LYRIC
            lyricIndex = dscData[opcodeIdx + 1]
            color = dscData[opcodeIdx + 2] + 2**32
            
            prevLyricStart = xmlPrevLyric.get("Start")
            xmlPrevLyric.set("Start", prevLyricStart)
            xmlPrevLyric.set("End", "{0:.3f}".format(nextCommandTime_divaTime / 100000.0 - 0.001))
            
            if lyricIndex >= 0:
                if firstLyric == True:
                    xmlPrevLyric.set("Start", "{0:.3f}".format(nextCommandTime_divaTime / 100000.0))
                    xmlPrevLyric.set("End", "{0:.3f}".format(nextCommandTime_divaTime / 100000.0 - 0.001))
                    xmlPrevLyric.set("Color", "{0:04x}".format(color))
                    xmlPrevLyric.text = "dsadadasdsa"
                else:
                    xmlPrevLyric = XmlET.SubElement(xmlLyrics, "Lyric")
                    xmlPrevLyric.set("Start", "{0:.3f}".format(nextCommandTime_divaTime / 100000.0))
                    xmlPrevLyric.set("End", "0.000")
                    xmlPrevLyric.set("Color", "{0:04x}".format(color))
                    xmlPrevLyric.text = "dsadadasdsa"
            
                XmlET.indent(xmlLyrics, space="\t")
                firstLyric = False
            
    opcodeIdx += opcodeSizes[opcode] + 1
    
xmlLyricsTree = XmlET.ElementTree(xmlLyrics)

xmlLyricsName = os.path.splitext(chartFile)[0] + ".xml"
xmlLyricsTree.write(xmlLyricsName)
