
function BOOT()
    screen(0)
    cls(0)
    vpu_init()
    f = io.open("./Debug/suzanne.c3d","rb")
    if f then
        local data = f:read("*all")
        trace("Loaded suzanne.c3d, size: "..#data)
        for i=1,#data do
            vpoke(0x21000 + i - 1, string.byte(data, i))
            --trace("Wrote byte "..i.." : "..string.byte(data, i))
        end
        f:close()
    end
    vpu_update_state()
end

timer_old = 0
t=0
d=0

function ONKEYDOWN(k)
    if to_key_name(k) == "Space" then
        d=d+1
    end
end

function LOOP()
    local timer_new = time()/1000
    local dt = timer_new - timer_old
    local fps = 1 / dt
    --[[for j=1, 255 do
        local r, g, b = S_IMAGE_HSV2RGB((j/256+t/256)%1, 1, 1)
        vpoke(0x1b000+j*3+0, r)
        vpoke(0x1b000+j*3+1, g)
        vpoke(0x1b000+j*3+2, b)
    end]]
    cls(rgb(64,64,64))
    if d%2==0 then
        vpu_draw_debug2(math.sin(timer_new/4)*4+4.0)
    else
        vpu_draw_debug(math.sin(timer_new/4)*4+4.0)
    end
    print("VPU Debug Mesh", 10, 10, 255)
    print("FPS: "..fps, 10, 22, 255)
    timer_old = timer_new
    t=t+1
end