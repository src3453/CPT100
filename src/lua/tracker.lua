mode=0
cur0=0
cur1=0


showcur(0)
startinput()
modeLabel = {
    "Track",
    "Pattern",
    "Insturument",
}

-- 音符を周波数に変換する関数
function note2freq(n)
    concert_pitch = 440;
    f = concert_pitch * 2 ^ ((n - 69)/12)
    return int(f)
end

-- トラックエディターの描画関数
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

-- パターンエディターの描画関数
function PatternEditor()
    notes="C-C#D-D#E-F-F#G-G#A-A#B-"
    print("ROW NOTE INST -EFFECT-",16,4,rgb(192,255,192))
    rectb(15,19+(cur1//4%16)*16,185,12,250)
    rect(47+(cur1%4)*40,19+(cur1//4%16)*16,33,12,250)
    for y=0,15 do
        print(string.format("%02X",int(math.floor(cur1//64)*16+y)),16,20+y*16,rgb(192,192,255))
        note=peek(cur0*256+cur1//64*64+y*4)
        if note==0 then
            print("...",48,20+y*16,rgb(255,255,255))
        elseif note==255 then
            print("===",rgb(255,255,255))
        else
            print(string.sub(notes,note*2%24+1,note*2%24+2)..note//12,48,20+y*16,rgb(255,255,255))
        end
        note=peek(cur0*256+cur1//64*64+y*4+1)
        print(string.format("%02X",note),88,20+y*16,rgb(255,255,255))
        effects="....VOLU"
        note=peek(cur0*256+cur1//64*64+y*4+2)
        print(string.sub(effects,note*4+1,note*4+4),128,20+y*16,rgb(255,255,255))
        note=peek(cur0*256+cur1//64*64+y*4+3)
        print(string.format("%02X",note),168,20+y*16,rgb(255,255,255))
    end
end

-- インストゥルメントエディターの描画関数
function InstEditor()
end

-- カーソルの描画関数
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

-- メインループ関数
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
    if mode == 2 then
        InstEditor()
        print(modeLabel[mode+1].." "..string.format("%02X",cur0),1,276,0)
    end  
    drawcur()
end

inputchar = ""

-- 入力処理関数
function ONINPUT(c)
    if mode == 0 then
        if tonumber(c,16) ~= nil then
            inputchar = inputchar .. c
            poke(int(0x0F000+cur0),tonumber(inputchar,16))
        else
        end
        if #inputchar == 2 then
            inputchar = ""
        end
    end
    if mode == 1 then
        if cur1%4 == 0 then
            if string.find("qwertyu",c) then
                ind = string.find("qwertyu",c)
                local notes = {0,2,4,5,7,9,11}
                poke(cur0*256+cur1,(peek(cur0*256+cur1)//12)*12+notes[ind])
            elseif string.find("0123456789",c) then  
                ind = string.find("0123456789",c)-1
                poke(cur0*256+cur1,peek(cur0*256+cur1)%12+ind*12)
            end
        end
    end
end

-- キーダウン処理関数
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
            cur0=(cur0-1)%256
        end
        if to_key_name(k) == "X" then
            cur0=(cur0+1)%256
        end
    end
    if to_key_name(k) == "Z" then

        mode=(mode+1)%3

    end
end
