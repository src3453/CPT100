mode=0
cur0=0
cur1=0

function TrackEditor()
    print("FRM FM1 FM2 FM3 FM4 WT5 WT6",16,4,rgb(192,255,192))
    rectb(15,19+(cur0//6%16)*16,209,12,250)
    rect(47+(cur0%6)*32,19+(cur0//6%16)*16,17,12,250)
    for y=0,15 do
        print(string.format("%02X",int(math.floor(cur0//96)*16+y)),16,20+y*16,rgb(192,192,255))
        for x=0,5 do
            print(string.format("%02X",peek(int(0x0F000+math.floor(cur0//96)*96+y*6+x))),48+x*32,20+y*16,rgb(255,255,255))
        end
    end
    --print("mouse:("..mx..","..my..","..mb..")",mx,my,255)
end

function PatternEditor()
    notes="C-C#D-D#E-F-F#G-G#A-A#B-"
    print("ROW NOTE INST -EFFECT-",16,4,rgb(192,255,192))
    rectb(15,19+(cur1//4%16)*16,185,12,250)
    rect(47+(cur1%4)*40,19+(cur1//4%16)*16,33,12,250)
    for y=0,15 do
        print(string.format("%02X",int(math.floor(cur1//64)*16+y)),16,20+y*16,rgb(192,192,255))
        note=peek(cur0*256+y*4)
        print(string.sub(notes,note*2%24,note*2%24+2)..note//12,48,20+y*16,rgb(255,255,255))
    end
end

function drawcur()
    mx,my,mb=mouse()
    for y=-1,1 do
        for x=-1,1 do
            line(mx+x,my+y,mx+8+x,my+y,0)
            line(mx+x,my+y,mx+x,my+8+y,0)
            line(mx+x,my+y,mx+16+x,my+16+y,0)
        end
    end
    line(mx,my,mx+8,my,255)
    line(mx,my,mx,my+8,255)
    line(mx,my,mx+16,my+16,255)
    
end

showcur(0)
startinput()
modeLabel = {
    "Track",
    "Pattern",
}
function LOOP()
    cls(0)
    rect(0,275,384,13,255)
    if mode == 0 then
        TrackEditor()
        print(modeLabel[mode+1],1,276,0)
    end
    if mode == 1 then
        PatternEditor()
        print(modeLabel[mode+1].." "..string.format("%02X",cur0),1,276,0)
    end  
    drawcur()
end

inputchar = ""

function ONINPUT(c)
    if tonumber(c,16) ~= nil then
        inputchar = inputchar .. c
        poke(int(0x0F000+cur0),tonumber(inputchar,16))
    else

    end
    if #inputchar == 2 then
        inputchar = ""
    end
end

function ONKEYDOWN(k)
    if mode == 0 then
        if to_key_name(k) == "Up" then
            cur0=(cur0-6)%1536
        end
        if to_key_name(k) == "Down" then
            cur0=(cur0+6)%1536
        end
        if to_key_name(k) == "Left" then
            cur0=(cur0-1)%1536
        end
        if to_key_name(k) == "Right" then
            cur0=(cur0+1)%1536
        end
    end
    if mode == 1 then
        if to_key_name(k) == "Up" then
            cur1=(cur1-4)%256
        end
        if to_key_name(k) == "Down" then
            cur1=(cur1+4)%256
        end
        if to_key_name(k) == "Left" then
            cur1=(cur1-1)%256
        end
        if to_key_name(k) == "Right" then
            cur1=(cur1+1)%256
        end
        if to_key_name(k) == "S" then
            cur0=(cur0-1)%128
        end
        if to_key_name(k) == "X" then
            cur0=(cur0+1)%128
        end
    end
    if to_key_name(k) == "Z" then

        mode=(mode+1)%2

    end
end