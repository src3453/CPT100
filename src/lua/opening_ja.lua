_tick=0
screen(1)

function LOOP()
    if _tick == 30 then
        for addr=0x1c000, 0x100000-1 do
            vpoke(addr,0)
        end
        poke(0x400000,int(1000/256))
        poke(0x400001,1000%256)
        poke(0x400010,255)
        poke(0x400018,0x30)
        poke(0x400021,32)
        poke(0x400023,32)
        poke(0x40001e,1)
        poke(0x40001f,0x80)
        resetgate(0)
        screen(2)
        cls(0)
    end
    if _tick >= 30 then
        colorfg(rgb(0,255,0))
        lc(0,0)
        printlnp(_CPT_PRODUCT_NAME.." セルフテスト")
        printlnp("バージョン ".._CPT_VERSION.."")
        printlnp("(c)2023-2026 src3453 MIT ライセンス")
        colorfg(255)
        local processor_name = "Lua 5.4"
        if _CPT_IS_LUAJIT == 1 then
            processor_name = "LuaJIT 2.1"
        end
        printlnp("プロセッサ ".. processor_name)
        printlnp("メインメモリ ".. string.format("%6d",math.min(_tick*524288,0x1000000)) .." バイト 正常です")
        printlnp("ビデオメモリ ".. string.format("%6d",math.min(_tick*524288,0x100000)) .." バイト 正常です")
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
        printlnp("VDC/SC: 2Dシステム... すべて正常です")
        printlnp("VPU/HPPA: 3Dシステム... すべて正常です")
        printlnp("音源モジュール/ADC: 3HS88PWN4... 正常です (リビジョン C)")
    end
    if _tick >= 40 then
        printlnp("")
        printlnp("システムチェック完了。")
    end

    if _tick>=150 then 
        screen(1)
        lc(0,0)
        _maincall() end
    poke(0x10080,0)
end