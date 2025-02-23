import json
import io
import sys

if len(sys.argv) < 2:
    print("Please specify the opcode type you want to list as a command-line argument")
    exit()

opcodeCategory = sys.argv[1]
print(f"Opcodes for {opcodeCategory}:")

opcode_dbFile = open("opcode_db.json")
opcode_db = json.load(opcode_dbFile)

opcodes = list(opcode_db)

specific_opcodes_names = dict()
specific_opcodes_lengths = dict()

for i in range(len(opcode_db)):
    key = opcodes[i]
    
    if opcodeCategory in opcode_db[key]:
        opcode_info = opcode_db[key][opcodeCategory]
        specific_opcodes_names[opcode_info["id"]] = key
        specific_opcodes_lengths[opcode_info["id"]] = opcode_info["len"]

opcode_dbFile.close()

for i in range(len(specific_opcodes_names)):
    print(specific_opcodes_names[i], end=",\n")
    
print("\nOpcode argument counts:")

for i in range(len(specific_opcodes_lengths)):
    print("[{opcode}] \t = {length},\n".format(opcode = specific_opcodes_names[i], length = specific_opcodes_lengths[i]), end="")
    