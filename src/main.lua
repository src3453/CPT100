
--[[function LOOP()
    for y=0,100 do
        for x=0,100 do
            pix(x,y,from_rgb(int(x*(math.cos(time()/1000)*2+2)),int(y*(math.sin(time()/1000)*2+2)),int(math.sqrt(x*x+y*y))*4))
        end
    end
end]]

--[]
label = {
    "F1",
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

freq=200
j=0

function LOOP()
    cls(0)
    for ch=0,3 do
        print("CH"..ch,60+ch*80,0,255)
        print(""..peek(0x10000+ch*16)*256+peek(0x10001+ch*16),60+ch*80,15,255)
        print(label[1],0,15,255)
        for i=0,8 do
            print(label[i+2],0,30+i*15,255)
            print(""..peek(0x10000+ch*16+i),60+ch*80,30+i*15,255)
        end
    end
    
    for i=1,15 do
        if (j%60)==0 then 
            poke(0x10000+i,math.random(0,255))        
        end
    end
    j=j+1
    --[[
    poke(0x10000,int(freq//256))
    poke(0x10001,int(freq%256))
    poke(0x10002,255)
    poke(0x10003,16)
    poke(0x10004,64)
    poke(0x10005,16)
    poke(0x10006,16)
    poke(0x10007,192)
    poke(0x10008,4)
    ]]
    --freq=freq+2
end
