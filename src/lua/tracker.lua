mode=0
cur0=0

function PatternEditor()
    print("FRM FM1 FM2 FM3 FM4 WT5 WT6",16,8,255)
    rectb(15,23+(cur0//6%16)*16,209,12,248)
    rect(47+(cur0%6)*32,23+(cur0//6%16)*16,17,12,248)
    for y=0,15 do
        print(string.format("%02X",int(math.floor(cur0//96)*16+y)),16,24+y*16,255)
        for x=0,5 do
            print(string.format("%02X",peek(int(0x0F000+math.floor(cur0//96)*96+y*6+x))),48+x*32,24+y*16,255)
        end
    end
    --print("mouse:("..mx..","..my..","..mb..")",mx,my,255)
end

function LOOP()
    mx,my,mb=mouse()
    cls(0)
    if mode == 0 then
        PatternEditor()
    end
    line(mx,my,mx+8,my,255)
    line(mx,my,mx,my+8,255)
    line(mx,my,mx+16,my+16,255)
end

function ONKEYDOWN(k)
    if to_key_name(k) == "Up" then
        if mode == 0 then
            cur0=(cur0-6)%1536
        end
    end
    if to_key_name(k) == "Down" then
        if mode == 0 then
            cur0=(cur0+6)%1536
        end
    end
    if to_key_name(k) == "Left" then
        if mode == 0 then
            cur0=(cur0-1)%1536
        end
    end
    if to_key_name(k) == "Right" then
        if mode == 0 then
            cur0=(cur0+1)%1536
        end
    end
end