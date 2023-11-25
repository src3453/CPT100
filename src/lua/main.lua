


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
ptr=0x10000

function LOOP()
    cls(0)
    --[[
            for y=0,288 do
        for x=0,384 do
            pix(x,y,int(math.sqrt(x*x+y*y))%256)
        end
    end
    ]]
    rect(int(64+(ptr%16)*20),int(12+(ptr//16)%16*12),16,12,248)
    for j=0,15 do
        print(string.format("0x%05X",(ptr//256)*256+j*16),0,12+j*12,255)
        for i=0,15 do
            print(string.format("%02X",peek((ptr//256)*256+j*16+i)),64+i*20,12+j*12,255)
        end
    end
    --freq=freq+2
    mx,my,mb=mouse()
    print("mouse:("..mx..","..my..","..mb..")",mx,my,255)
    --(^._.^) like a cat 
    poke(0x10000,2*mx//256)
    poke(0x10001,2*mx)
    if mb == 1 then 
        newval=16-my/1.125/16
        if (math.min(mx,383)/6)%2 <= 1 then
            val=peek(0x10090+math.min(mx,383)//12)%16+int(newval)*16
        else
            val=(peek(0x10090+math.min(mx,383)//12)//16)*16+newval
        end
        poke(0x10090+math.min(mx,383)//12,val)
    end
    poke(0x1008c,mx)
end

for i=0,12,4 do
poke(0x10041+i,40)
poke(0x10043+i,40)
end
poke(0x10002,48)
poke(0x10009,255)
poke(0x1000a,64)

function ONKEYDOWN(k)
    if to_key_name(k) == "Up" then
        poke(ptr,(peek(ptr)+1)%256)   
    elseif to_key_name(k) == "Down" then
        poke(ptr,(peek(ptr)-1)%256) 
    elseif to_key_name(k) == "Left" then
        ptr=(ptr-1)
    elseif to_key_name(k) == "Right" then
        ptr=(ptr+1)
    end
end

