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
function table_contains(tbl, x)
    local found = false
    for _, v in pairs(tbl) do
        if v == x then 
            found = true 
        end
    end
    return found
end
-- 音符を周波数に変換する関数
function note2freq(n)
    concert_pitch = 440;
    f = concert_pitch * 2 ^ ((n-69+12)/12)
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
            print("===",48,20+y*16,rgb(255,255,255))
        else
            print(string.sub(notes,note*2%24+1,note*2%24+2)..note//12,48,20+y*16,rgb(255,255,255))
        end
        note=peek(cur0*256+cur1//64*64+y*4+1)
        if peek(cur0*256+cur1//64*64+y*4)==0 then
            print("..",88,20+y*16,rgb(255,255,255))
        else
            print(string.format("%02X",note),88,20+y*16,rgb(255,255,255))
        end
        effects="....VOLUARPELEGASLIDDELYJUMP"
        note=peek(cur0*256+cur1//64*64+y*4+2)
        print(string.sub(effects,note*4+1,note*4+4),128,20+y*16,rgb(255,255,255))
        note=peek(cur0*256+cur1//64*64+y*4+3)
        print(string.format("%02X",note),168,20+y*16,rgb(255,255,255))
    end
end

-- インストゥルメントエディターの描画関数
function InstEditor()
    print(" F*  V   A   D   S   R",32,4,rgb(192,255,192))
    rect(39+(cur1%6)*32,19+(cur1//6%6)*20,25,13,250)
    for y=0,3 do
        print("OP"..y+1,8,20+y*20,rgb(192,192,255))
        for x=0,5 do
            note=peek(0x0b000+cur0*24+y*6+x)
            if x==0 and y==0 then
                print("---",40+x*32,20+y*20,rgb(255,255,255))
            else
                print(string.format("%02X",note),40+x*32,20+y*20,rgb(255,255,255))
            end
        end
    end
end

function PlayInst(ch,num,freq,gate)
    
    for i=0,3 do
        if i ~= 0 then
            poke(0x10000+1+i+ch*16,peek(0x0b000+num*24+i*6+0))
        end
        poke(0x10000+9+i+ch*16,peek(0x0b000+num*24+i*6+1))
        for j=0,3 do
            poke(0x10040+i*4+j+ch*16,peek(0x0b000+num*24+i*6+2+j)) 
        end
    end
    poke(0x10000+ch*16,freq//256)
    poke(0x10001+ch*16,freq%256)
    if gate==1 then
        resetgate(ch)
    end
    poke(0x10080+ch,gate)
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

playing=0
tempo=120
speed=4
lastplaytime=0
currentoctave=4
g_playing=0
g_lastplaytime=0
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
    if playing == 1 then 
        if (time()-lastplaytime)%((60/tempo/speed)*1000) <= 20 then
            local gate,val
            val = peek((time()-lastplaytime)//((60/tempo/speed)*1000)*4%256)
            val2 = peek(1+(time()-lastplaytime)//((60/tempo/speed)*1000)*4%256)
            if val == 255 then
                poke(0x10080,0)
            else
                gate = 1
                if val ~= 0 then
                    PlayInst(val2,0,note2freq(val),gate)
                end
            end
            if mode == 1 then
                cur1 = int((time()-lastplaytime)//((60/tempo/speed)*1000)*4%256)
            end
        end
    end
    if g_playing == 1 then 
        if (time()-g_lastplaytime)%((60/tempo/speed)*1000) <= 20 then
            for ch=0,3 do
                local gate,val
                
                val = peek(peek(int(0x0F000+math.floor(cur0//6)*6+ch))*256+((time()-g_lastplaytime)//((60/tempo/speed)*1000)*4%256))
                val2 = peek(1+peek(int(0x0F000+math.floor(cur0//6)*6+ch))*256+((time()-g_lastplaytime)//((60/tempo/speed)*1000)*4%256))
                if val == 255 then
                    poke(0x10080+ch,0)
                else
                    gate = 1
                    if val ~= 0 then
                        PlayInst(ch,val2,note2freq(val),gate)
                    end
                end
                if mode == 0 then
                    cur0 = int((time()-g_lastplaytime)//((60/tempo/speed)*1000)//64*6%1536)
                end
            end
        end
    end
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
                poke(cur0*256+cur1,currentoctave*12+notes[ind])
            elseif string.find("0123456789",c) then  
                currentoctave = string.find("0123456789",c)-1
                poke(cur0*256+cur1,peek(cur0*256+cur1)%12+currentoctave*12)
            end
        end
        if cur1%4 == 1 then
            if tonumber(c,16) ~= nil then
                inputchar = inputchar .. c
                poke(int(0x0F000+cur0),tonumber(inputchar,16))
            else
            end
            if #inputchar == 2 then
                inputchar = ""
            end
        end
    end
    if mode == 2 then
        if tonumber(c,16) ~= nil then
            inputchar = inputchar .. c
            poke(int(0x0b000+cur0*24+cur1),tonumber(inputchar,16))
        else
        end
        if #inputchar == 2 then
            inputchar = ""
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
        if to_key_name(k) == "Space" then
            g_playing=(g_playing+1)%2
            g_lastplaytime=time()
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
        if to_key_name(k) == "Space" then
            playing=(playing+1)%2
            lastplaytime=time()
        end
        if to_key_name(k) == "Backspace" then
            if peek(cur0*256+cur1) == 0 then
                poke(cur0*256+cur1,255)
            else
                poke(cur0*256+cur1,0)
            end
        end
        if to_key_name(k) == "H" then
            local notes = {0,2,5,7,9}
            local sharp_notes = {1,3,6,8,10}
            if table_contains(notes,peek(cur0*256+cur1)%12) then
                poke(cur0*256+cur1,peek(cur0*256+cur1)+1)
            elseif table_contains(sharp_notes,peek(cur0*256+cur1)%12) then
                poke(cur0*256+cur1,peek(cur0*256+cur1)-1)
            end
        end
    end
    if mode == 2 then
        if to_key_name(k) == "Up" then
            cur1=(cur1-6)%24
        end
        if to_key_name(k) == "Down" then
            cur1=(cur1+6)%24
        end
        if to_key_name(k) == "Left" then
            cur1=(cur1-1)%24
        end
        if to_key_name(k) == "Right" then
            cur1=(cur1+1)%24
        end
        if to_key_name(k) == "S" then
            cur0=(cur0-1)%64
        end
        if to_key_name(k) == "X" then
            cur0=(cur0+1)%64
        end
        if to_key_name(k) == "Space" then
            PlayInst(0,cur0,note2freq(60),1)
        end
    end
    if to_key_name(k) == "Z" then

        mode=(mode+1)%3

    end
end
