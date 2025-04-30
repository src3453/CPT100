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
    print("Main  RAM ".. string.format("%6d",math.min(_tick*524288,0x1000000)) .." Bytes OK",0,36,255)
    print("Video RAM ".. string.format("%6d",math.min(_tick*524288,0x100000)) .." Bytes OK",0,48,255)
    
    
    _tick=_tick+1
    if _tick == 5 then
        
        resetgate(0)
        poke(0x400000,500//256)
        poke(0x400001,500%256)
        
    end
    if _tick >= 5 then
        print("VPU: Vertex  Processor... OK!",0,60,255)
        print("VPU: Shading Processor... OK!",0,72,255)
        print("Sound Module: 3HS88PWN4... OK! (1x Found; Rev.3)",0,84,255)
    end
    if _tick >= 10 then
        print("System check complete!",0,96,255)
    end

    if _tick>=120 then _maincall() end
    poke(0x10080,0)
end