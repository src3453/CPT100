
t=0
resolution = 3
dither = {
    {0,8,2,10},
    {12,4,14,6},
    {3,11,1,9},
    {15,7,13,5}
}

function drawcur()
    mx,my,mb=mouse()
    for y=-1,1 do
        for x=-1,1 do
            line(mx+x,my+y,mx+8+x,my+y,0)
            line(mx+x,my+y,mx+x,my+8+y,0)
            line(mx+x,my+y,mx+16+x,my+16+y,0)
        end
    end
    line(mx,my,mx+8,my,255)
    line(mx,my,mx,my+8,255)
    line(mx,my,mx+16,my+16,255)
    
end

-- This is a simple Lua script that initializes a game loop and clears the screen.
frametime = 0
frametime_old = 0
fps_list = {}
t = 0
bufremaining = 0
desiredbuflength = 0x400
wave = "00"
resetgate(0)
function BOOT()
    songfile=io.open("converted.lua","r")
    data=songfile:read("a")
    --trace(data)
    wave=data
    songfile:close()
    for i=0, #wave/2-1 do
        poke(0x0+i, tonumber(string.sub(wave, i*2+1, i*2+2),16))
    end
end
function LOOP()
    cls(0)
    xm,ym,_ = mouse()
    xm = xm - 0
    ym = ym - 0

    --[[for x=0,383,resolution do
        for y=0,287,resolution do
            xa = x - xm
            ya = y - ym
            val = math.max(math.min((math.sin(ya/100*ya/100*ym/50+t/50)+math.cos(xa/100*xa/100*xm/50+t/50))*64+128,255),0)
            fval = 0
            if val%1 >= dither[(y/resolution)%4+1][(x/resolution)%4+1]*(1/16) then
                fval = val//1*1+1
            else
                fval = val//1*1
            end
            fval = math.max(math.min(fval,255),0)
            rect(x,y,resolution,resolution,fval)
        end
    end]]

    

    buf = {}
    

    -- software PCM via DMA
    poke(0x400202,255)
    poke(0x400203,5)
    buflength = 0x500
    if bufremaining + buflength < 4096 then
        for a = 0,buflength-1 do
            t2 = int(t%(#wave//2-1))
            pre = tonumber(string.sub(wave, t2*2+1, t2*2+2),16)
            nxt = tonumber(string.sub(wave, t2*2+3, t2*2+4),16)
            --buf[a+1] = pre+((nxt-pre)*(t%1))--+math.random(-16,16)
            buf[a+1] = pre
            if a<384 then line(a,255-buf[math.max(a,1)],a+1,255-buf[a+1],255) end
            t=t+0.125
        end
    end
    bufremaining = put_dma_buffer(0, buf)
    print(string.format("DMA buffer length: %d",bufremaining),0,12,255) --flush data
    print(string.format("PCM data size: 0x%06x",#wave//2),0,24,255) --flush data
    print(string.format("PCM read head: 0x%06x",t2),0,36,255) --flush data
    

    -- hardware PCM
    
    --[[poke(0x400200,0x00)
    poke(0x400201,0xbb)
    poke(0x400202,255)
    poke(0x400210,0x00)
    poke(0x400211,0x00)
    poke(0x400212,0x00)
    poke(0x400213,#wave/2>>16)
    poke(0x400214,#wave/2>>8)
    poke(0x400215,#wave/2)]]

    function RegisterView()
        for j=0,15 do
            print(string.format("%06X",(0x000000//256)*256+j*16),0,12+j*12,rgb(192,192,255))
            for i=0,15 do
                print(string.format("%02X",peek((0x000000//256)*256+j*16+i)),64+i*20,12+j*12,rgb(192,255,192))
            end
        end
    end
    --RegisterView()

    frametime = time() - frametime_old
    frametime_old = time()
    fps = int(1000/frametime)
    fps_list[#fps_list+1] = fps
    if #fps_list > 60 then
        table.remove(fps_list,1)
    end
    average_fps = 0
    for i=1,#fps_list do
        average_fps = average_fps + fps_list[i]
    end
    average_fps = int(average_fps / #fps_list)
    print(average_fps.." FPS",1,1,rgb(0,0,0))
    print(average_fps.." FPS",0,0,rgb(255,255,255))
    showcur(0)
    drawcur()
    --t=t+1
end