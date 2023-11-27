mode=0
cur0=0
cur1=0
preview_tick = 32
pattern_tick = 32
track_tick = {32,32}
play_target=0

showcur(0)
startinput()
modeLabel = {
    "Track",
    "Pattern",
    "FM Insturument",
    "WT Insturument",
    "Sound Registers"
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
            if peek(int(0x0F000+math.floor(cur0//96)*96+y*6+x))==0 then
                txt=".."
            else 
                txt=string.format("%02X",peek(int(0x0F000+math.floor(cur0//96)*96+y*6+x)))
            end
            print(txt,48+x*32,20+y*16,rgb(255,255,255))
        end
    end
    --print("mouse:("..mx..","..my..","..mb..")",mx,my,255)
end
currentinst=0
-- パターンエディターの描画関数
function PatternEditor()
    _cur0=cur0+1
    notes="C-C#D-D#E-F-F#G-G#A-A#B-"
    print("ROW NOTE INST -EFFECT-",16,4,rgb(192,255,192))
    rectb(15,19+(cur1//4%16)*16,185,12,250)
    rect(47+(cur1%4)*40,19+(cur1//4%16)*16,33,12,250)
    for y=0,15 do
        print(string.format("%02X",int(math.floor(cur1//64)*16+y)),16,20+y*16,rgb(192,192,255))
        note=peek(_cur0*256+cur1//64*64+y*4)
        if note==0 then
            print("...",48,20+y*16,rgb(255,255,255))
        elseif note==255 then
            print("===",48,20+y*16,rgb(255,255,255))
        else
            print(string.sub(notes,note*2%24+1,note*2%24+2)..note//12,48,20+y*16,rgb(255,255,255))
        end
        note=peek(_cur0*256+cur1//64*64+y*4+1)
        if peek(_cur0*256+cur1//64*64+y*4)==0 then
            print("..",88,20+y*16,rgb(255,255,255))
        else
            print(string.format("%02X",note),88,20+y*16,rgb(255,255,255))
        end
        effects="....VOLUARPELEGASLIDDELYJUMPTEMP"
        note=peek(_cur0*256+cur1//64*64+y*4+2)
        print(string.sub(effects,note*4+1,note*4+4),128,20+y*16,rgb(255,255,255))
        note=peek(_cur0*256+cur1//64*64+y*4+3)
        print(string.format("%02X",note),168,20+y*16,rgb(255,255,255))
    end
    
end

-- インストゥルメントエディターの描画関数
function FMInstEditor()
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
function WTInstEditor()
    print("Mode",8,4,rgb(192,255,192))
    val=peek(int(0x08000+cur0*128+cur1))
    print(string.sub("8Bit  4Bit  Noise PNoisePCM   ",val*6+1,val*6+6),48,4,rgb(255,255,255))
    mx,my,mb=mouse()
    rect(8,34,256,68,248)
    for i=0,30 do
        val=peek(0x08020+cur0*128+i)//4
        val2=peek(0x08020+cur0*128+i+1)//4
        line(12+i*8,36+64-val,20+i*8,36+64-val2,255)
    end
    for i=0,31 do
        val=peek(0x08020+cur0*128+i)//4
        rect(10+i*8,34+64-val,4,4,rgb(255,64,64))
        rectb(10+i*8,34+64-val,4,4,rgb(255,128,128))
    end
    if mx>8 and mx<8+256 and my>34 and my<34+64 and mb==1 then
        poke(0x08020+cur0*128+math.min(mx-8,256)//8,((34+63)-my)*4)
    end
    rect(8,114,256,64,248)
    for i=0,31 do
        val=peek(0x08040+cur0*128+i)//4
        rect(8+i*8,114+64-val,8,val,rgb(64,255,64))
        rectb(8+i*8,114+64-val,8,val,rgb(128,255,128))
    end
    if mx>8 and mx<8+256 and my>114 and my<114+64 and mb==1 then
        poke(0x08040+cur0*128+math.min(mx-8,256)//8,((114+63)-my)*4)
    end
    rect(8,194,256,64,248)
    for i=0,31 do
        val=peek(0x08060+cur0*128+i)//4
        rect(8+i*8,194+64-val,8,val,rgb(64,64,255))
        rectb(8+i*8,194+64-val,8,val,rgb(128,128,255))
    end
    if mx>8 and mx<8+256 and my>194 and my<194+64 and mb==1 then
        poke(0x08060+cur0*128+math.min(mx-8,256)//8,((194+63)-my)*4)
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
function PlayWTInst(ch,num,freq,tick)
    for i=0,31 do
        poke(0x10090+i+ch*32,peek(0x08020+num*128+i))
    end
    poke(0x10084+ch*2,freq//256)
    poke(0x10085+ch*2,freq%256)
    poke(0x1008a+ch,peek(0x08000+num*128))
    poke(0x10088+ch,peek(0x08040+num*128+math.min(tick,31)))
    poke(0x1008c+ch,peek(0x08060+num*128+math.min(tick,31)))
    if tick==0 then
        wtsync(ch)
    end
end
function RegisterView()
    for j=0,15 do
        print(string.format("%05X",(0x10000//256)*256+j*16),0,12+j*12,rgb(192,192,255))
        for i=0,15 do
            print(string.format("%02X",peek((0x10000//256)*256+j*16+i)),64+i*20,12+j*12,rgb(192,255,192))
        end
    end
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
tempo=0
speed=4
lastplaytime=0
currentoctave=4
g_playing=0
g_lastplaytime=0
track_note={0,0}
pattern_inst=0
track_inst={0,0}
preview2_tick=32
-- メインループ関数
function LOOP()
    cls(0)
    rect(0,275,384,13,255)
    tempo=peek(0xefff)
    if tempo==0 then
        poke(0xefff,120)
    end
    if mode == 0 then
        TrackEditor()
        print(modeLabel[mode+1],1,276,0)
        print("Tempo "..peek(0xefff),304,276,0)
    end
    if mode == 1 then
        PatternEditor()
        print(modeLabel[mode+1].." "..string.format("%02X",cur0+1),1,276,0)
        print("Target "..string.sub("FMWT",play_target*2+1,play_target*2+2),304,276,0)
    end
    if mode == 2 then
        FMInstEditor()
        print(modeLabel[mode+1].." "..string.format("%02X",cur0),1,276,0)
    end
    if mode == 3 then
        WTInstEditor()
        print(modeLabel[mode+1].." "..string.format("%02X",cur0),1,276,0)
    end  
    if mode == 4 then
        RegisterView()
        print(modeLabel[mode+1],1,276,0)
    end  
    drawcur()
    if playing == 1 then 
        local _cur0=cur0+1
        if play_target==0 then
            if (time()-lastplaytime)%((60/tempo/speed)*1000) <= 30 then
                local gate,val
                val = peek(_cur0*256+(time()-lastplaytime)//((60/tempo/speed)*1000)*4%256)
                val2 = peek(_cur0*256+((time()-lastplaytime)//((60/tempo/speed)*1000)*4%256)+1)
                    if val == 255 then
                        poke(0x10080,0)
                    else
                        gate = 1
                        if val ~= 0 then
                            PlayInst(0,val2,note2freq(val),gate)
                        end
                    end
                if mode == 1 then
                    cur1 = int((time()-lastplaytime)//((60/tempo/speed)*1000)*4%256)
                end
            end
        else 
            local val
            val=peek(_cur0*256+(time()-lastplaytime)//((60/tempo/speed)*1000)*4%256)
            val2 = peek(_cur0*256+((time()-lastplaytime)//((60/tempo/speed)*1000)*4%256)+1)
            if val ~= 0 then
                pattern_note = val
                pattern_inst = val2
            end
            
            if (time()-lastplaytime)%((60/tempo/speed)*1000) <= 30 and val ~= 0 and playing == 1 then
                pattern_tick=0
            end
            if val == 255 then
                pattern_tick=31
            end
            if pattern_tick<32 then
                PlayWTInst(0,pattern_inst,note2freq(pattern_note),pattern_tick)
            end

            if mode == 1 then
                cur1 = int((time()-lastplaytime)//((60/tempo/speed)*1000)*4%256)
            end
        end
    end

    if g_playing == 1 then 
        playcur = int((time()-g_lastplaytime)//((60/tempo/speed)*1000)//64*6%1536)
        if (time()-g_lastplaytime)%((60/tempo/speed)*1000) <= 30 then
            for ch=0,3 do
                local gate,val
                
                val = peek(peek(int(0x0F000+math.floor(playcur//6)*6+ch))*256+((time()-g_lastplaytime)//((60/tempo/speed)*1000)*4%256))
                val2 = peek((peek(int(0x0F000+math.floor(playcur//6)*6+ch))*256+((time()-g_lastplaytime)//((60/tempo/speed)*1000)*4%256)+1))
                if val == 255 then
                    poke(0x10080+ch,0)
                else
                    gate = 1
                    if val ~= 0 then
                        PlayInst(ch,val2,note2freq(val),gate)
                    end
                end
            end
        end
        for ch=0,1 do
            local val
            val=peek(peek(int(0x0F000+math.floor(playcur//6)*6+ch+4))*256+((time()-g_lastplaytime)//((60/tempo/speed)*1000)*4%256))
            val2=peek((peek(int(0x0F000+math.floor(playcur//6)*6+ch+4))*256+((time()-g_lastplaytime)//((60/tempo/speed)*1000)*4%256)+1))
            if val ~= 0 then
                track_note[ch+1]=val
                track_inst[ch+1]=val2
            end
            
            if (time()-g_lastplaytime)%((60/tempo/speed)*1000) <= 30 and val ~= 0 and g_playing == 1 then
                track_tick[ch+1]=0
            end
            if val == 255 then
                track_tick[ch+1]=31
            end
            ind = int(track_tick[ch+1])
            if ind<32 then
                PlayWTInst(ch,track_inst[ch+1],note2freq(track_note[ch+1]),ind)
            end
            
        end
        if mode == 0 then
            cur0 = playcur
        end
    end
    if playing==0 and g_playing==0 and preview_tick>32 and preview2_tick>32 then
        for i=0,9 do
        poke(0x10080+i,0)
        end
    end
    if preview_tick<32 then
        PlayWTInst(0,cur0,note2freq(60),preview_tick)
    end
    if preview2_tick<32 then
        PlayWTInst(0,peek((cur0+1)*256+cur1+1),note2freq(peek((cur0+1)*256+cur1)),preview2_tick)
    end
    preview_tick=preview_tick+1
    preview2_tick=preview2_tick+1
    pattern_tick=pattern_tick+1
    track_tick[1]=track_tick[1]+1
    track_tick[2]=track_tick[2]+1
    --trace(""..pattern_tick)
    --trace(track_tick[1].." "..track_tick[2])
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
        local _cur0=cur0+1
        if cur1%4 == 0 then
            if string.find("qwertyu",c) then
                ind = string.find("qwertyu",c)
                local notes = {0,2,4,5,7,9,11}
                poke(_cur0*256+cur1,currentoctave*12+notes[ind])
            elseif string.find("0123456789",c) then  
                currentoctave = string.find("0123456789",c)-1
                poke(_cur0*256+cur1,peek(_cur0*256+cur1)%12+currentoctave*12)
            end
            if string.find("0123456789qwertyuh",c) then
                if play_target==0 then
                    PlayInst(0,peek(_cur0*256+cur1+1),note2freq(peek(_cur0*256+cur1)),1)
                else
                    preview2_tick=0
                end 
            end
            poke(int(_cur0*256+cur1+1),currentinst)
        end
        if cur1%4 == 1 then
            if tonumber(c,16) ~= nil then
                inputchar = inputchar .. c
                poke(int(_cur0*256+cur1),tonumber(inputchar,16))
            else
            end
            currentinst = tonumber(inputchar,16)
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
    if mode == 3 then
        if tonumber(c,16) ~= nil then
            inputchar = inputchar .. c
            poke(int(0x08000+cur0*128+cur1),tonumber(inputchar,16))
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
        if to_key_name(k) == "S" then
            poke(int(0x0EFFF),(peek(int(0x0EFFF))+1)%256)
        end
        if to_key_name(k) == "X" then
            poke(int(0x0EFFF),math.max(1,peek(int(0x0EFFF))-1%256))
        end
    end
    if mode == 1 then
        local _cur0=cur0+1
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
            cur0=(cur0-1)%127
        end
        if to_key_name(k) == "X" then
            cur0=(cur0+1)%127
        end
        if to_key_name(k) == "Space" then
            playing=(playing+1)%2
            lastplaytime=time()
        end
        if to_key_name(k) == "Backspace" then
            if peek(_cur0*256+cur1) == 0 then
                poke(_cur0*256+cur1,255)
            else
                poke(_cur0*256+cur1,0)
            end
        end
        if to_key_name(k) == "H" then
            local notes = {0,2,5,7,9}
            local sharp_notes = {1,3,6,8,10}
            if table_contains(notes,peek(_cur0*256+cur1)%12) then
                poke(_cur0*256+cur1,peek(_cur0*256+cur1)+1)
            elseif table_contains(sharp_notes,peek(_cur0*256+cur1)%12) then
                poke(_cur0*256+cur1,peek(_cur0*256+cur1)-1)
            end
        end
        if to_key_name(k) == "C" then
            play_target=(play_target+1)%2
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
    if mode == 3 then
        if to_key_name(k) == "Up" then
            poke(int(0x08000+cur0*128+cur1),(peek(int(0x08000+cur0*128+cur1))+1)%5)
        end
        if to_key_name(k) == "Down" then
            poke(int(0x08000+cur0*128+cur1),(peek(int(0x08000+cur0*128+cur1))-1)%5)
        end
        if to_key_name(k) == "Left" then
            cur1=(cur1-1)%1
        end
        if to_key_name(k) == "Right" then
            cur1=(cur1+1)%1
        end
        if to_key_name(k) == "S" then
            cur0=(cur0-1)%64
        end
        if to_key_name(k) == "X" then
            cur0=(cur0+1)%64
        end
        if to_key_name(k) == "Space" then
            preview_tick=0
        end
    end
    if to_key_name(k) == "Z" then

        mode=(mode+1)%5
        cur0=0
        cur1=0

    end
    if to_key_name(k) == "P" then
        if _CPT_IS_WASM == 0 then
            local songfile=io.open("song.cptm","w+")
            local memdata={}
            for i=0,0xffff do
                memdata[i+1] = peek(i)
            end
            local data=""
            for i=0,0xffff do
                data=data..string.format("%02x",memdata[i+1])
            end
            songfile:write(data)
            songfile:flush()
            songfile:close()
        end
    end
end

function BOOT()
    if _CPT_IS_WASM == 0 then
        local songfile=io.open("song.cptm","r")
        data=songfile:read("a")
        for i=0,0xffff do
            poke(i,tonumber(string.sub(data,i*2+1,i*2+2),16))
        end
        songfile:close()
    end
end