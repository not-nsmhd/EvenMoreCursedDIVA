import json
import io
import sys
import array
import os
import xml.etree.ElementTree as XmlET

def GetNoteShapeString(value):
    match value:
        case 0 | 4 | 8:
            return "Triangle"
        case 1 | 5 | 9:
            return "Circle"
        case 2 | 6 | 10:
            return "Cross"
        case 3 | 7 | 11:
            return "Square"
        case 12 | 15 | 22 | 23:
            return "Star"
       
    print("Unknown shape value: {0}".format(value));
    return "Circle"
  
def GetNoteTypeString(value):
    if (value <= 3 or value == 12 or value == 15):
        return "Normal"
    elif (value <= 7):
        return "Double"
    elif (value <= 11):
        return "HoldStart"
        
    print("Unknown type value: {0}".format(value));
    return "Normal"
    
if len(sys.argv) < 3:
    print(f"Usage: {sys.argv[0]} <script format> <script file path>")
    print()
    print("Script formats:")
    print("\tinfo_A12 - Dreamy Theater 1st/Arcade")
    print("\tinfo_f - F/Dreamy Theater 2nd/Dreamy Theater Extend")
    print("\tinfo_F2 - F 2nd")
    exit()

opcode_dbFile = open("opcode_db.json")
opcode_db = json.load(opcode_dbFile)

opcodes = list(opcode_db)
opcodeSizes = dict()

scriptFormat = sys.argv[1]

for i in range(len(opcode_db)):
    key = opcodes[i]
    
    if scriptFormat in opcode_db[key]:    
        opcode_info = opcode_db[key][scriptFormat]
        opcodeSizes[int(opcode_info["id"])] = int(opcode_info["len"])

opcode_dbFile.close()

# ---------------

chartFile = sys.argv[2]
dscFile = open(chartFile, "rb")

dscFile.seek(0, io.SEEK_END)
dscDataSize = dscFile.tell()
dscFile.seek(0, io.SEEK_SET)

dscData = array.array("i")
dscData.fromfile(dscFile, (int)(dscDataSize / 4))

dscFile.close()

xmlChart = XmlET.Element("Chart")

opcodeIdx = 1
nextCommandTime_divaTime = 0

if (scriptFormat == "info_F2"):
    opcodeIdx = 18 # 72 / 4

prevShapeString = ""
prevTypeString = ""
prevNoteTime = 0
while True:
    if opcodeIdx >= (int)(dscDataSize / 4):
        break

    opcode = dscData[opcodeIdx]
    if (opcode == 0x43464f45):
        break
    
    match opcode:
        case 0: # END
            xmlChart.attrib["Duration"] = "{0}".format(nextCommandTime_divaTime / 100000.0)
        case 1: # TIME
            nextCommandTime_divaTime = dscData[opcodeIdx + 1]
        case 6: # TARGET
            shape = dscData[opcodeIdx + 1]
            
            shapeString = GetNoteShapeString(shape)
            typeString = GetNoteTypeString(shape)
            
            if (prevTypeString == "HoldStart" and typeString == "HoldStart" and prevShapeString == shapeString):
                typeString = "HoldEnd"
            
            prevShapeString = shapeString
            prevTypeString = typeString
            
            tgtX = dscData[opcodeIdx + 4]
            tgtY = dscData[opcodeIdx + 5]
            
            angle = dscData[opcodeIdx + 6]
            frequency = dscData[opcodeIdx + 7]
            distance = dscData[opcodeIdx + 8]
            amplitude = dscData[opcodeIdx + 9]
            noteTime = dscData[opcodeIdx + 10]
            
            if (prevNoteTime != noteTime):
                xmlChartNote = XmlET.SubElement(xmlChart, "SetNoteTime", 
                {
                    "Time": "{0}".format(nextCommandTime_divaTime / 100000.0),
                    "Value": "{0}".format(noteTime / 1000.0)
                })
                XmlET.indent(xmlChart, space="\t")
                
            prevNoteTime = noteTime
            
            xmlChartNote = XmlET.SubElement(xmlChart, "Note", 
            {
                "Time": "{0}".format(nextCommandTime_divaTime / 100000.0),
                
                "Shape": shapeString,
                "Type": typeString,
                "X": "{0}".format((tgtX * 960.0 / 480000.0) + 160.0),
                "Y": "{0}".format((tgtY * 540.0 / 272000.0) + 90.0),
                
                "Angle": "{0}".format(angle / 1000.0),
                "Frequency": "{0}".format(frequency),
                "Amplitude": "{0:.0f}".format((amplitude * 272) / 540),
                "Distance": "{0:.0f}".format((distance * 720) / 272000)
            })
            
            XmlET.indent(xmlChart, space="\t")
        case 25: # MUSIC_PLAY
            xmlChart.attrib["MusicStart"] = "{0}".format(nextCommandTime_divaTime / 100000.0)
        case 26: # MODE_SELECT
            difficulty = dscData[opcodeIdx + 1]
            mode = dscData[opcodeIdx + 2]
        
            if difficulty & 1 != 0:
                eventName = ""
                match mode:
                    case 1:
                        eventName = "ChanceTimeStart"
                    case 3:
                        eventName = "ChanceTimeEnd"
                    #case 8:
                        #eventName = "TechnicalZoneStart"
                    #case 9:
                        #eventName = "TechnicalZoneEnd"
                    case _:
                        eventName = ""
                
                if len(eventName) > 0:
                     xmlEvent = XmlET.SubElement(xmlChart, eventName, 
                    {
                        "Time": "{0}".format(nextCommandTime_divaTime / 100000.0),
                    })
            
            XmlET.indent(xmlChart, space="\t")
        case 28: # BAR_TIME_SET
            bpm = dscData[opcodeIdx + 1]
            beatsPerBar = dscData[opcodeIdx + 2]
        
            xmlChartNote = XmlET.SubElement(xmlChart, "SetNoteTime", 
            {
                "Time": "{0}".format(nextCommandTime_divaTime / 100000.0),
                "Value": "{0}".format(60 / bpm * 4)
            })
            
            XmlET.indent(xmlChart, space="\t")
            
    opcodeIdx += opcodeSizes[opcode] + 1
    
xmlChartTree = XmlET.ElementTree(xmlChart)

xmlChartName = os.path.splitext(chartFile)[0] + ".xml"
xmlChartTree.write(xmlChartName)
