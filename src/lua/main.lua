-- [AdvFramework - Text based Adventure Game Framework for CPT200]
AdvFramework = {}
local af = AdvFramework -- shorthand
af.SCREEN_X = 48 -- text width in characters
af.SCREEN_Y = 24 -- text height in characters
af.IS_SCREEN3 = false -- Controls rendering method and charsets (false: cp437, true: utf-16be)
af.DEBUG = false -- debug mode
af.init = function ()
    screen(1) -- set to PCG mode
    cls(0)
    colorfg(255)
    colorbg(0)
    lc(0,0) 
    setpwrap(false) -- disable print wrapping
    showcurp(0) -- hide cursor
end
af.draw = {}
af.draw.genericWindow = function (type, x, y, w, h, color1, color2)
    color1 = color1 or 255
    color2 = color2 or 0
    colorfg(color1)
    colorbg(color2)
    for xi = x, x+w do -- horizontal lines 
        lc(xi,y)
        printp(0xc4)
        lc(xi,y+h)
        printp(0xc4)
    end
    for yi = y, y+h do -- vertical lines
        lc(x,yi)
        printp(0xb3)
        lc(x+w,yi)
        printp(0xb3)
    end
    lc(x,y) -- corners
    printp(0xda)
    lc(x+w,y)
    printp(0xbf)
    lc(x,y+h)
    printp(0xc0)
    lc(x+w,y+h)
    printp(0xd9)
    for xi = x+1, x+w-1 do
        for yi = y+1, y+h-1 do
            lc(xi,yi)
            printp(0x20)
        end
    end
end
af.draw.dialogWindow = function (title, content, color1, color2)
    color1 = color1 or 255
    color2 = color2 or 0
    af.draw.genericWindow("dialog", 0, af.SCREEN_Y-8, af.SCREEN_X-1, 7, color1, color2)
    lc(1, af.SCREEN_Y-8) -- title
    --colorbg(color2)
    --colorfg(color1)
    printp(title)
    lc(2, af.SCREEN_Y-6) -- contents
    --colorbg(color2)
    --colorfg(color1)
    printp(content)
    lc(af.SCREEN_X-2,af.SCREEN_Y-2) -- page feed mark
    printp(0x1f)
end

function BOOT()
    af.init()
    af.draw.dialogWindow("Test", "This is a test")
    af.draw.genericWindow("generic",2,2,5,3,0,255)
end

function LOOP()
    --showcur(0)
    --mx, my, _ = mouse()
    --cls(0)
    --af.draw.genericWindow("generic",2,2,int(mx/8-2),int(my/12-2),255,0)
    --lc(2,2)
end