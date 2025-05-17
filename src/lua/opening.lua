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
screen(1)
function LOOP()
    printp("CPT200 High-spec Fantasy Console",0,0,rgb(0,255,0),0)
    printp("Version ".._CPT_VERSION,0,1,rgb(0,255,0),0)
    printp("(c)2025 src3453 MIT licence",0,2,rgb(0,255,0),0)
    printp("Main  RAM ".. string.format("%6d",math.min(_tick*524288,0x1000000)) .." Bytes OK",0,3,255,0)
    printp("Video RAM ".. string.format("%6d",math.min(_tick*524288,0x100000)) .." Bytes OK",0,4,255,0)
    
    
    _tick=_tick+1
    --vpoke(0x1c000+_tick, math.random(0,255))
    if _tick == 5 then
        
        resetgate(0)
        poke(0x400000,500//256)
        poke(0x400001,500%256)
        
    end
    if _tick >= 5 then
        printp("VPU: Vertex  Processor... OK!",0,5,255,0)
        printp("VPU: Shading Processor... OK!",0,6,255,0)
        printp("Sound Module: 3HS88PWN4... OK! (1x Found; Rev.3)",0,7,255,0)
    end
    if _tick >= 10 then
        printp("System check complete!",0,8,255,0)
    end

    if _tick>=120 then 
        screen(0)
        _maincall() end
    poke(0x10080,0)
end