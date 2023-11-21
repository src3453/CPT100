mode=0
cur0=0

function TrackEditor()
    print("FRM FM1 FM2 FM3 FM4 WT5 WT6",16,4,rgb(192,255,192))
    rectb(15,19+(cur0//6%16)*16,209,12,250)
    rect(47+(cur0%6)*32,19+(cur0//6%16)*16,17,12,250)
    for y=0,15 do
        print(string.format("%02X",int(math.floor(cur0//96)*16+y)),16,20+y*16,rgb(192,192,255))
        for x=0,5 do
            print(string.format("%02X",peek(int(0x0F000+math.floor(cur0//96)*96+y*6+x))),48+x*32,20+y*16,255)
        end
    end
    --print("mouse:("..mx..","..my..","..mb..")",mx,my,255)
end

function PatternEditor()

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
    if mode == 0 then
        TrackEditor()
    end
    if mode == 1 then
        PatternEditor()
    end
    rect(0,275,384,13,255)
    print("Mode:"..modeLabel[mode+1],1,276,0)
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
    if to_key_name(k) == "Z" then

        mode=(mode+1)%2

    end
end