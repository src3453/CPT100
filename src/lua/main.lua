
function BOOT()
    screen(0)
    cls(0)
    init_sound_input(1024)
end

function LOOP()
    local samples = acquire_sound_input()
    if #samples > 0 then
        cls(0)
        x = 0
        for i=1,#samples,3 do
            pix(x, 144 - math.floor(samples[i]*144*1), rgb(0,255,0))
            x = x + 1
            if x >= 384 then break end
        end
    end
end