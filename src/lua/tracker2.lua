
startinput()
screen(1)
cls(0)

SCREEN_X = 384
SCREEN_Y = 288

Mode = 0
ModeString = {
    "Tracks",
    "Pattern",
    "Instrument",
}

Tempo = 120
Tick0 = 0
Cx0 = 0
Cy0 = 0
Offset_Tracks = 0x0
CurrentInput = ""

function drawTracks()
    local f = string.format
    lc(0,0)
    colorbg(0)
    colorfg(rgb(0,255,0))
    printp("FRH1H2H3H4H5H6H7H8P1P2P3P4")
    for i=0, 15 do
        lc(0,i+1)
        colorfg(rgb(0,255,0))
        colorbg(0)
        printp(""..f("%02X",i))
        for j=0,11 do
            colorfg(255)
            colorbg(rgb((i+j)%2*64,(i+j)%2*64,(i+j)%2*64))
            lc(2+j*2,i+1)
            local val = peek(Offset_Tracks+i*12+j)
            if val == 0 then
                printp("..")
            else
                printp(f("%02X",val))
            end
        end
    end

end

function drawStatusBar()
    colorfg(0)
    colorbg(255)
    lc(0,23)
    printp(ModeString[Mode+1])
end

function LOOP()
    cls(0)
    drawStatusBar()
    if Mode == 0 then
        drawTracks()
        lc(2+Cx0,1+Cy0)
    end
end

function ONINPUT(c)
    if Mode == 0 then
        if tonumber(c,16)~=nil then
            local val = tonumber(c,16)
            S_COMMON_poke4(Offset_Tracks+Cy0*24+Cx0, val)
            Cx0 = (Cx0 + 1) % 24
        end
    end
end

function ONKEYDOWN(k)
    if to_key_name(k) == "Left" then
        Cx0=(Cx0-1)%24
    end
    if to_key_name(k) == "Right" then
        Cx0=(Cx0+1)%24
    end
    if to_key_name(k) == "Up" then
        Cy0=(Cy0-1)%16
    end
    if to_key_name(k) == "Down" then
        Cy0=(Cy0+1)%16
    end
    if to_key_name(k) == "PageUp" then
        Mode=(Mode+1)%#ModeString
    end
    if to_key_name(k) == "PageDown" then
        Mode=(Mode-1)%#ModeString
    end
end