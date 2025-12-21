_tick=0

screen(1)
function LOOP()
    if _tick == 30 then
        for addr=0x1c000, 0x100000-1 do
            vpoke(addr,0)
        end
        cls(0)
        poke(0x400000,int(1000/256))
        poke(0x400001,1000%256)
        poke(0x400010,255)
        poke(0x400018,0x30)
        poke(0x400021,32)
        poke(0x400023,32)
        poke(0x40001e,1)
        poke(0x40001f,0x80)
        resetgate(0)
    end
    if _tick >= 30 then
        colorfg(rgb(0,255,0))
        lc(0,0)
        printlnp("CPT200 High-spec Fantasy Console")
        printlnp("Version ".._CPT_VERSION.."")
        printlnp("(c)2025 src3453 MIT licence")
        colorfg(255)
        printlnp("Main  RAM ".. string.format("%6d",math.min(_tick*524288,0x1000000)) .." Bytes OK")
        printlnp("Video RAM ".. string.format("%6d",math.min(_tick*524288,0x100000)) .." Bytes OK")
    end
    
    _tick=_tick+1
    --vpoke(0x1c000+_tick, math.random(0,255))
    if _tick == 35 then
        
        resetgate(0)
        poke(0x400000,int(500/256))
        poke(0x400001,500%256)
        
    end
    if _tick >= 35 then
        printlnp("")
        printlnp("VDC/SC: 2D System... ALL OK!")
        printlnp("VPU/HPPA: 3D System... ALL OK!")
        printlnp("Sound Module/ADC: 3HS88PWN4... OK! (Rev.C)")
    end
    if _tick >= 40 then
        printlnp("")
        printlnp("System check complete!")
    end

    if _tick>=150 then 
        screen(1)
        lc(0,0)
        _maincall() end
    poke(0x10080,0)
end