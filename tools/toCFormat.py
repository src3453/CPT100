import re

path = input("path?>")
txt = open(path,"r").read()
txt = txt.split("\n")
res = "source = "
for i in txt:
    res += "\""+re.sub('\"','\\\"',i)+"\\n\"+"
res = res[:-1] + ";"
open("formatted.h","w").write(res)