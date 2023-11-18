
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
ptr=0

function LOOP()
    cls(0)
    rect(int(60+ptr//16*80),int(30+ptr%16*15),80,15,248)
    for ch=0,3 do
        print("CH"..ch,60+ch*80,0,255)
        print(""..peek(0x10000+ch*16)*256+peek(0x10001+ch*16),60+ch*80,15,255)
        print(label[1],0,15,255)
        for i=0,8 do
            print(label[i+2],0,30+i*15,255)
            print(""..peek(0x10000+ch*16+i),60+ch*80,30+i*15,255)
        end
    end
    --freq=freq+2
    mx,my=mouse()
    print("mouse:("..mx..","..my..")",mx-3,my,255)
end

function ONKEYDOWN(k)
    if to_key_name(k) == "Up" then
        poke(0x10000+ptr,(peek(0x10000+ptr)+1)%256)   
    elseif to_key_name(k) == "Down" then
        poke(0x10000+ptr,(peek(0x10000+ptr)-1)%256) 
    elseif to_key_name(k) == "Left" then
        ptr=(ptr-1)%64
    elseif to_key_name(k) == "Right" then
        ptr=(ptr+1)%64
    end
end
