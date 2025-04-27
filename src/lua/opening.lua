_tick=0
poke(0x400000,1000//256)
poke(0x400001,1000%256)
poke(0x400010,255)
poke(0x400018,0x30)
poke(0x400021,32)
poke(0x400023,32)
poke(0x40001e,1)
poke(0x40001f,0x80)
resetgate(0)
function LOOP()
    cls(0)
    print("CPT200 High-spec Fantasy Console",0,0,rgb(0,255,0))
    print("Version ".._CPT_VERSION,0,12,rgb(0,255,0))
    print("(c)2025 src3453 MIT licence",0,24,rgb(0,255,0))
    print("Main  RAM ".. string.format("%6d",math.min(_tick*131072,0x1000000)) .." Bytes OK",0,36,255)
    print("Video RAM ".. string.format("%6d",math.min(_tick*131072,0x100000)) .." Bytes OK",0,48,255)
    print("Sound chip was successfully initialized",0,60,255)
    _tick=_tick+1
    if _tick == 5 then
        poke(0x400000,500//256)
        poke(0x400001,500%256)
        resetgate(0)
    end
    if _tick>=200 then _maincall() end
    poke(0x10080,0)
end