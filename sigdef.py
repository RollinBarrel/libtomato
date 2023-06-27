import re
from sys import argv
from enum import Enum

class SigType(Enum):
    ENUMS = 0
    IN = 1
    OUT = 2

class ArgType(Enum):
    INT = 0
    BOOL = 1
    FLOAT = 2
    ENUM = 3

class Arg:
    def __init__(self, name):
        self.type : ArgType = 0
        self.enumtype : str = ""
        
        self.name : str = name
        self.descriptions : dict[str, str] = {}

class Signal:
    def __init__(self, name):
        self.name : str = name
        self.descriptions : dict[str, str] = {}

        self.args : list[Arg] = []
        
rENUMS = "Enums:"
rIN = "In:"
rOUT = "Out:"

rSIG = "(\S+) {"
# rSIGEND = "}"
rSIGDESC = "\t(\w\w): (.+)"

rENUMARG = "\t(\S+)"
rSIGARG = "\t(\S+) (\S+)"
rARGDESC = "\t\t(\w\w): (.+)"

sigtype : SigType = None
signal : Signal = None
arg : Arg = None
insidesignal = False

languages : set[str] = set()
enums : list[Signal] = []
insigs : list[Signal] = []
outsigs : list[Signal] = []

with open(argv[1]) as file:
    line = "1"
    while line != "":
        line = file.readline()
        if line == "\n":
            continue

        # vscode really like its spaces huh
        line = line.replace("    ", "\t")

        if re.match(rENUMS, line):
            sigtype = SigType.ENUMS
            continue
        if re.match(rIN, line):
            sigtype = SigType.IN
            continue
        if re.match(rOUT, line):
            sigtype = SigType.OUT
            continue

        if not insidesignal:
            m = re.match(rSIG, line)
            if m != None:
                signal = Signal(m[1])
                if sigtype == SigType.ENUMS:
                    enums.append(signal)
                elif sigtype == SigType.IN:
                    insigs.append(signal)
                elif sigtype == SigType.OUT:
                    outsigs.append(signal)

                insidesignal = True
                continue
            
            m = re.match(rSIGDESC, line)
            if m != None:
                languages.add(m[1])
                signal.descriptions[m[1]] = m[2]
                continue
        
        if line == "}\n":
            insidesignal = False
            continue

        if sigtype == SigType.ENUMS:
            m = re.match(rENUMARG, line)
            if m:
                arg = Arg(m[1])
                signal.args.append(arg)
                continue
        else:
            m = re.match(rSIGARG, line)
            if m:
                arg = Arg(m[2])
                if m[1] == "int":
                    arg.type = ArgType.INT
                elif m[1] == "float":
                    arg.type = ArgType.FLOAT
                elif m[1] == "bool":
                    arg.type = ArgType.BOOL
                else:
                    arg.type = ArgType.ENUM
                    arg.enumtype = m[1]
                signal.args.append(arg)
                continue
        
        m = re.match(rARGDESC, line)
        if m:
            languages.add(m[1])
            arg.descriptions[m[1]] = m[2]

with open(argv[2], "w") as file:
    file.write("#pragma once\n")
    file.write("#include <tomato/message.h>\n")

    def wdescr(descr : dict[str, str]):
        if "en" in descr:
            encoded = bytes(descr["en"], "utf8").decode("unicode_escape")
            file.write(f"/*\n{encoded}\n*/\n")
    
    # defines

    numcount = 0
    for num in enums:
        wdescr(num.descriptions)
        file.write(f"#define TSIG_ENUM_{num.name} {numcount}\n")
        numcount += 1

        valcount = 0
        for val in num.args:
            wdescr(val.descriptions)
            file.write(f"#define TSIG_{num.name}_{val.name} {valcount}\n")
            valcount += 1

        file.write('\n')
    
    incount = 0
    for sig in insigs:
        wdescr(sig.descriptions)
        file.write(f"#define TSIG_IN_{sig.name} {incount}\n")
        incount += 1
    
    outcount = 0
    for sig in outsigs:
        wdescr(sig.descriptions)
        file.write(f"#define TSIG_OUT_{sig.name} {outcount}\n")
        outcount += 1
    
    # languages

    file.write("static Tomato::Msg::LanguageCode languages[] = {\n")
    for language in languages:
        file.write(f"\tTomato::Msg::LanguageCode('{language[0]}', '{language[1]}'),\n")
    file.write("};\n")

    # descriptions
    
    def wdescr(name : str, list : dict[str, str]):
        if len(list) > 0:
            file.write(f"static const char *descr_{name}[] = {{\n")
            for l in languages:
                if list[l] == None:
                    raise Exception(f"No localized description for {name} in language {l}")
                
                file.write(f"\t\"{list[l]}\",\n")
                file.write("};\n")


    for num in enums:
        wdescr(f"ENUM_{num.name}", num.descriptions)
        for arg in num.args:
            wdescr(f"ENUM_{num.name}_{arg.name}", arg.descriptions)
    
    for sig in insigs:
        wdescr(f"IN_{sig.name}", sig.descriptions)
        for arg in sig.args:
            wdescr(f"IN_{sig.name}_{arg.name}", arg.descriptions)

    for sig in outsigs:
        wdescr(f"OUT_{sig.name}", sig.descriptions)
        for arg in sig.args:
            wdescr(f"OUT_{sig.name}_{arg.name}", arg.descriptions)

    #init

    file.write("inline void InitSignalDefs(Tomato::Msg::SignalList* list, Tomato::Msg::LanguageCode language) {\n")

    file.write("\tif (list->Initialized) return;\n")

    file.write("\tint l;\n")
    file.write("\tfor (l = 0; l < sizeof(languages) / sizeof(language); ++l) {if (language == languages[l]) break;}\n")

    if len(enums) > 0:
        file.write(f"\tlist->Enums.reserve({len(enums)});\n")
    if len(insigs) > 0:
        file.write(f"\tlist->In.reserve({len(insigs)});\n")
    if len(outsigs) > 0:
        file.write(f"\tlist->Out.reserve({len(outsigs)});\n")

    for num in enums:
        file.write("\t{\n")

        file.write("\t\tauto &num = list->Enums.emplace_back();\n")
        file.write(f"\t\tnum.Name = \"{num.name}\";\n")
        if len(num.descriptions) > 0:
            file.write(f"\t\tnum.Description = descr_ENUM_{num.name}[l];\n")

        if len(num.args) > 0:
            file.write(f"\t\tnum.Values.reserve({len(num.args)});\n")
        
        for val in num.args:
            file.write("\t\t{\n")

            file.write("\t\t\tauto &val = num.Values.emplace_back();\n")
            file.write(f"\t\t\tval.Name = \"{val.name}\";\n")
            if len(val.descriptions) > 0:
                file.write(f"\t\t\tval.Description = descr_ENUM_{num.name}_{val.name}[l];\n")

            file.write("\t\t}\n")

        file.write("\t}\n")

    for sig in insigs:
        file.write("\t{\n")

        file.write("\t\tauto &signal = list->In.emplace_back();\n")
        file.write(f"\t\tsignal.Name = \"{sig.name}\";\n")
        if len(sig.descriptions) > 0:
            file.write(f"\t\tsignal.Description = descr_IN_{sig.name}[l];\n")
        
        if len(sig.args) > 0:
            file.write(f"\t\tsignal.Args.reserve({len(sig.args)});\n")

        for arg in sig.args:
            file.write("\t\t{\n")

            file.write("\t\t\tauto &arg = signal.Args.emplace_back();\n")
            file.write(f"\t\t\targ.Type = {arg.type.value};\n")
            if arg.type == ArgType.ENUM:
                file.write(f"\t\t\targ.Param.EnumId = TSIG_ENUM_{arg.enumtype};\n")
            file.write(f"\t\t\targ.Name = \"{arg.name}\";\n")
            if len(arg.descriptions) > 0:
                file.write(f"\t\t\targ.Description = descr_IN_{sig.name}_{arg.name}[l];\n")

            file.write("\t\t}\n")

        file.write("\t}\n")

    for sig in outsigs:
        file.write("\t{\n")

        file.write("\t\tauto &signal = list->Out.emplace_back();\n")
        file.write(f"\t\tsignal.Name = \"{sig.name}\";\n")
        if len(sig.descriptions) > 0:
            file.write(f"\t\tsignal.Description = descr_OUT_{sig.name}[l];\n")

        if len(sig.args) > 0:
            file.write(f"\t\tsignal.Args.reserve({len(sig.args)});\n")
        
        for arg in sig.args:
            file.write("\t\t{\n")

            file.write("\t\t\tauto &arg = signal.Args.emplace_back();\n")
            file.write(f"\t\t\targ.Type = {arg.type.value};\n")
            if arg.type == ArgType.ENUM:
                file.write(f"\t\t\targ.Param.EnumId = TSIG_ENUM_{arg.enumtype};\n")
            file.write(f"\t\t\targ.Name = \"{arg.name}\";\n")
            if len(arg.descriptions) > 0:
                file.write(f"\t\t\targ.Description = descr_OUT_{sig.name}_{arg.name}[l];\n")

            file.write("\t\t}\n")

        file.write("\t}\n")

    file.write("\tlist->Initialized = true;\n")

    file.write("}\n")