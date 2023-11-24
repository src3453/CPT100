_tick=0
function LOOP()
    cls(0)
    print("CPT100 High-spec Fantasy Console",0,0,rgb(0,255,0))
    print("Version ".._CPT_VERSION,0,12,rgb(0,255,0))
    print("(c)2023 src3453 MIT licence",0,24,rgb(0,255,0))
    print("Main  RAM "..0x80000.." Bytes OK",0,36,255)
    print("Video RAM "..0x20000.." Bytes OK",0,48,255)
    print("Sound chip was successfully initialized",0,60,255)
    _tick=_tick+1
    if _tick>=200 then _maincall() end
end