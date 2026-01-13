import re
import sys

path = sys.argv[1]
txt = open(path,"r",encoding="utf-8").read()
txt = txt.split("\n")
res = f"std::string {re.sub('.lua','',path.split('/')[-1])}_source = "
for i in txt:
    res += "\""+re.sub('\"','\\\"',re.sub(r"\n",r"\\n",re.sub(r"\\",r"\\\\",i)))+"\\n\"\n"
res = res[:-1] + ";"
open(path+".hpp","w",encoding="utf-8").write(res)
print(f"Formatted {path}.")