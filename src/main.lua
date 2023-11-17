
--[[function LOOP()
    for y=0,100 do
        for x=0,100 do
            pix(x,y,from_rgb(math.floor(x*(math.cos(time()/1000)*2+2)),math.floor(y*(math.sin(time()/1000)*2+2)),math.floor(math.sqrt(x*x+y*y))*4))
        end
    end
end]]

j=0
k=16
poke(0x10002,255)
poke(0x10003,16)
poke(0x10004,0)
poke(0x10005,16)
poke(0x10006,0)
poke(0x10007,16)
poke(0x10008,0)
label = {
    "F1_U",
    "F1_L",
    "V1",
    "F*2",
    "V2",
    "F*3",
    "V3",
    "F*4",
    "V4" 
}
function LOOP()
    cls(0)
    for ch=0,3 do
        print("CH"..ch,60+ch*80,0,255)
        for i=0,8 do
            print(label[i+1],0,30+i*15,255)
            print(""..peek(0x10000+ch*16+i),60+ch*80,30+i*15,255)
        end
    end
    --print(""..j,100,0,255)
    poke(0x10000,j//256)
    poke(0x10001,j)
    poke(0x10004,math.floor(k))
    --k=k+1
    j=j+1
end
