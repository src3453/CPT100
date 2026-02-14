import re
import sys

path = sys.argv[1]
outpath = sys.argv[2]
txt = open(path,"r",encoding="utf-8").read()
txt_len = len(txt)
txt = txt.split("\n")
res = f"std::string {re.sub('.hpp','',re.sub('.lua','',outpath.split('/')[-1]))}_source = "
for i in txt:
    res += "\""+re.sub('\"','\\\"',re.sub(r"\n",r"\\n",re.sub(r"\\",r"\\\\",i)))+"\\n\"\n"
res = res[:-1] + ";"
open(outpath,"w",encoding="utf-8").write(res)
print(f"Formatted {path} -> {outpath}. Script length: {txt_len}")