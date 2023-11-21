import re
import sys

path = sys.argv[1]
txt = open(path,"r").read()
txt = txt.split("\n")
res = "std::string source = "
for i in txt:
    res += "\""+re.sub('\"','\\\"',i)+"\\n\"\n"
res = res[:-1] + ";"
open(path+".hpp","w").write(res)