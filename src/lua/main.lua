
t=0
resolution = 1
screen(0)
-- index, filepath, offset, Dithered, transparentColorIndex
loadspr(0,"../sprite.png",0x20000,false,0)
trace("Sprite loaded")

timer_old = 0
fps = 0
-- This is a simple Lua script that initializes a game loop and clears the screen.
function LOOP()
    local timer_new = time()/1000
    local dt = timer_new - timer_old
    fps = 1 / dt
    cls(0)
    xm,ym,_ = mouse()
    for x=0,383,resolution do
        for y=0,287,resolution do
            val = math.max(math.min((math.sin(y/100*ym/50+t/50)+math.cos(x/100*xm/50+t/50))*64+128,255),0)
            val = math.max(math.min(val,255),0)
            rect(x,y,resolution,resolution,rgbd(x,y,0,val,0))
        end
    end
    -- index, enabled, x, y, rotation
    spr(0, true, xm, ym, 0)
    t=t+1
    timer_old = timer_new
end

function POSTDRAW()
    -- called after sprite rendering
    print("FPS:"..fps,1,1,rgb(0,0,0))
    print("FPS:"..fps,0,0,rgb(255,255,255))
end