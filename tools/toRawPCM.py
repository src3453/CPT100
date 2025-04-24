import itertools
import numpy as np
from pydub import AudioSegment


path = input("path?> ")
FRAME_RATE = 6000

def rle(seq:str) -> str:
    x=0
    tmp=""
    for i,v in enumerate(seq):
        if v == (seq+" ")[i+1] and x<15:
            x+=1
        else:
            #x=0
            tmp+=f"{v}{x:01x}"
            x=0
    return tmp
sounds = AudioSegment.from_file(path, path.split(".")[-1])
#sounds2 = AudioSegment.from_file(path, path.split(".")[-1])

#sounds2 = sounds2.set_frame_rate(TIC_FRAME_RATE*2)

# 基本情報の表示
print(f'[-] Number of channel(s): {sounds.channels}')
print(f'[-] Frame rate: {sounds.frame_rate}')
print(f'[-] Duration: {sounds.duration_seconds} second(s)')

# チャンネルが2 （ステレオ) の場合，交互にデータが入っているので，二つおきに読み出す。

#sig2 = np.array(sounds2.get_array_of_samples())[::sounds.channels]
print(sounds.array_type)
bit = lambda x=sounds.array_type: 30 if x == "i" else 6 if x=="b" else 14 if x=="h" else 1

FACTOR = 127
wave = []
length=2**10
sounds = sounds.set_frame_rate(FRAME_RATE*length)
sig = np.array(sounds.get_array_of_samples())[::sounds.channels]
print(length)
    #val = np.array_split(sig,len(sig)/length)
val = sig
tmp = [val[i:i+length] for i in range(0,len(val),length)]
tmp = [np.mean(i) for i in tmp]
    #print(tmp)
wave.append(np.clip(((np.array(tmp)/2**bit())*FACTOR+FACTOR),0,255).astype(int))
writer = open("converted.lua","w")
#wave2 = np.clip((np.array(list(itertools.chain.from_iterable(([sig2[i:i+31] for i in range(0,len(sig2),64)]))))/2**bit()*FACTOR+8),0,15).astype(int)
#writer = open("converted.lua","w")
sep='\",\"'
val = ",".join(['"'+''.join(["{:02x}".format(i) for i in j])+'"' for j in wave])
out = f"wave={val}"
                  #sep+(''.join([hex(i)[2:] for i in wave2])))
writer.write(out)
writer.close()
print("[*] Converted!")