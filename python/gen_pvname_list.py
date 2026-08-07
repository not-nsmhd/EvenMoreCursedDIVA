import json
import io
import sys
import array
import os
import xml.etree.ElementTree as XmlET

class PVName:
    ID = 0;
    Name = "";
    
    def __init__(self, _id, name):
        self.ID = _id;
        self.Name = name;

if len(sys.argv) < 3:
    print(f"Usage: {sys.argv[0]} <pv_db.txt file> <pvlist.xml output file>")
    exit()

# ---------------

pvDbFile_path = sys.argv[1]
outputFilePath = sys.argv[2]

# ---------------

pvDbFile = open(pvDbFile_path, "r", encoding='utf-8', newline='\n')

pvNames = [];

while True:
    pvPrefix = f"pv_"
    pvIndex = 0
    
    line = pvDbFile.readline()
    if (len(line) == 0):
        break
        
    if line.startswith(pvPrefix) == True:
        pvIndex = int(line[3:6]);
        
    propNameEnd = line.find('=');
    if line[7:propNameEnd] == "song_name":
        pvPropNameAndValue = line.split('=', 1);
        pvNameText = pvPropNameAndValue[1].rstrip('\n')
        pvNames.append(PVName(pvIndex, pvNameText))
    
    if line[7:propNameEnd] == "song_name_en":
        pvPropNameAndValue = line.split('=', 1);
        pvNameText = pvPropNameAndValue[1].rstrip('\n')
        pvNames[len(pvNames) - 1].Name = pvNameText

pvDbFile.close()

# ---------------

rootElement = XmlET.Element("PVList")

for pv in pvNames:
    pvElement = XmlET.SubElement(rootElement, "PV",
    {
        "ID": "{0:03d}".format(pv.ID),
        "Name": pv.Name
    })
    
    XmlET.indent(rootElement, space="\t")
    
xmlLyricsTree = XmlET.ElementTree(rootElement)

with open(outputFilePath, mode='wb') as xmlOutFile:
    xmlLyricsTree.write(xmlOutFile, encoding="utf-8")

