import sys
import io
import subprocess
from datetime import datetime
from pathlib import Path

def main():
    scriptPath = Path(sys.argv[0])
    outputFile = sys.argv[1]

    genDate = datetime.now()
    
    genDateString = genDate.strftime("%Y.%m.%dT%H:%M:%S")
    
    process = subprocess.run("git rev-parse --abbrev-ref HEAD", capture_output=True, encoding="ASCII")
    gitBranch = process.stdout
    
    process = subprocess.run("git rev-parse --short=7 HEAD", capture_output=True, encoding="ASCII")
    gitCommitHash = process.stdout

    # Writing
    buildDefFile = open(outputFile, "wb")
    buildDefFileWriter = io.TextIOWrapper(buildDefFile, newline='\n')
    
    # Header
    buildDefFileWriter.write("#pragma once\n")
    buildDefFileWriter.write("//------------------------------------------------------------\n")
    buildDefFileWriter.write("// This file was auto-generated by \"{scriptName}\" script.\n".format(scriptName=scriptPath.name))
    buildDefFileWriter.write("// Generated on: {date} at {time}\n".format(date=genDate.date(), time=genDate.time()))
    buildDefFileWriter.write("//\n")
    buildDefFileWriter.write("// DO NOT EDIT THIS FILE!!!\n")
    buildDefFileWriter.write("//------------------------------------------------------------\n\n")
    
    buildDefFileWriter.write("namespace BuildInfo\n")
    buildDefFileWriter.write("{\n")
    
    # Variables
    buildDefFileWriter.write("\tstatic constexpr const char* BuildDateString = \"{genDate}\";\n".format(genDate=genDateString))
    buildDefFileWriter.write("\tstatic constexpr const char* GitBranchName = \"{branch}\";\n".format(branch=gitBranch[:-1]))
    buildDefFileWriter.write("\tstatic constexpr const char* GitCommitHashString = \"{hash}\";\n".format(hash=gitCommitHash[:-1]))
    
    buildDefFileWriter.write("}\n")
    buildDefFileWriter.close()

if (len(sys.argv) < 2):
    print("Output header file must be specified")
    exit()

main()