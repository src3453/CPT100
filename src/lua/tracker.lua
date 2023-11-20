mode=0
cur0=0

function PatternEditor()
    for y=0,15 do
        for x=0,5 do
            print(string.format("%02X",peek(int(0x0F000+cur//96*96+y*6+x))),24+x*24,16+y*16,255)
        end
    end
end

function LOOP()
    cls(0)
    if mode == 0 then
        PatternEditor()
    end
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