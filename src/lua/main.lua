
function BOOT()
    screen(1)
    cls(0)
    colorfg(255)
    init_sound_input(256)
    
    --[[for j=0, 255 do
        vpoke(0x1b000+j*3+0, j)
        vpoke(0x1b000+j*3+1, j)
        vpoke(0x1b000+j*3+2, j)
    end]]

end

i = 0

function LOOP()
    local samples = acquire_sound_input()
    if #samples > 0 then
        for j=1,#samples do
            char = (samples[j]*128+127)
            vpoke(0x1c000+i*3, char)--samples[j]*32768%0x80
            vpoke(0x1c000+i*3+1, char)
            vpoke(0x1c000+i*3+2, 0)
            trace(i)
            i=(i+1)%(48*24)
        end
    end
end