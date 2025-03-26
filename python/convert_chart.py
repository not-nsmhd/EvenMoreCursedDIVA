import json
import io
import sys
import array
import xml.etree.ElementTree as XmlET

dscToXml_NoteNames = [
"Triangle",
"Circle",
"Cross",
"Square",
"Star"
]

opcode_dbFile = open("opcode_db.json")
opcode_db = json.load(opcode_dbFile)

opcodes = list(opcode_db)
opcodeSizes = dict()

for i in range(len(opcode_db)):
    key = opcodes[i]
    
    if "info_A12" in opcode_db[key]:    
        opcode_info = opcode_db[key]["info_A12"]
        opcodeSizes[int(opcode_info["id"])] = int(opcode_info["len"])

opcode_dbFile.close()

# ---------------

dscFile = open("test_dt1.dsc", "rb")

dscFile.seek(0, io.SEEK_END)
dscDataSize = dscFile.tell()
dscFile.seek(0, io.SEEK_SET)

dscData = array.array("i")
dscData.fromfile(dscFile, (int)(dscDataSize / 4))

dscFile.close()

xmlChart = XmlET.Element("Chart")

opcodeIdx = 0
nextCommandTime_divaTime = 0
while True:
    if opcodeIdx >= (int)(dscDataSize / 4):
        break

    opcode = dscData[opcodeIdx]
    
    match opcode:
        case 0: # END
            xmlChartNote = XmlET.SubElement(xmlChart, "Event", 
            {
                "Time": "{0:.3f}".format(nextCommandTime_divaTime / 100000.0),
                "Type": "SongEnd"
            })
            
            XmlET.indent(xmlChart, space="\t")
        case 1: # TIME
            nextCommandTime_divaTime = dscData[opcodeIdx + 1]
        case 6: # TARGET
            shape = dscData[opcodeIdx + 1]
            
            tgtX = dscData[opcodeIdx + 2]
            tgtY = dscData[opcodeIdx + 3]
            
            angle = dscData[opcodeIdx + 4]
            distance = dscData[opcodeIdx + 5]
            amplitude = dscData[opcodeIdx + 6]
            frequency = dscData[opcodeIdx + 7]
            
            xmlChartNote = XmlET.SubElement(xmlChart, "Note", 
            {
                "Time": "{0:.3f}".format(nextCommandTime_divaTime / 100000.0),
                "ReferenceIndex": "-1",
                
                "Shape": dscToXml_NoteNames[shape],
                "X": "{0:.3f}".format((tgtX * 960.0 / 480000.0) + 160.0),
                "Y": "{0:.3f}".format((tgtY * 540.0 / 272000.0) + 90.0),
                
                "Angle": "{0:.3f}".format(angle / 1000.0),
                "Frequency": "{0}".format(frequency),
                "Amplitude": "{0}".format(amplitude),
                "Distance": "{0:.3f}".format(distance / 1000.0 * 4)
            })
            
            XmlET.indent(xmlChart, space="\t")
        case 25: # MUSIC_PLAY
            xmlChartNote = XmlET.SubElement(xmlChart, "Event", 
            {
                "Time": "{0:.3f}".format(nextCommandTime_divaTime / 100000.0),
                "Type": "PlayMusic"
            })
            
            XmlET.indent(xmlChart, space="\t")
        case 26: # MODE_SELECT
            mode = dscData[opcodeIdx + 1]
        
            if mode == 1:
                xmlChartNote = XmlET.SubElement(xmlChart, "Event", 
                {
                    "Time": "{0:.3f}".format(nextCommandTime_divaTime / 100000.0),
                    "Type": "ChanceTimeStart"
                })
            elif mode == 3:
                xmlChartNote = XmlET.SubElement(xmlChart, "Event", 
                {
                    "Time": "{0:.3f}".format(nextCommandTime_divaTime / 100000.0),
                    "Type": "ChanceTimeEnd"
                })
            
            XmlET.indent(xmlChart, space="\t")
        case 28: # BAR_TIME_SET
            bpm = dscData[opcodeIdx + 1]
            beatsPerBar = dscData[opcodeIdx + 2]
        
            xmlChartNote = XmlET.SubElement(xmlChart, "Event", 
            {
                "Time": "{0:.3f}".format(nextCommandTime_divaTime / 100000.0),
                "Type": "SetBPM",
                
                "BPM": "{0}".format(bpm),
                "BeatsPerBar": "{0}".format(beatsPerBar + 1),
            })
            
            XmlET.indent(xmlChart, space="\t")
            
    opcodeIdx += opcodeSizes[opcode] + 1
    
xmlChartTree = XmlET.ElementTree(xmlChart)
xmlChartTree.write("test_dt1.xml")
