-- Simple GUI system for CPT200

local isMouseEventProessed = false -- This variable is used to track whether a mouse event has been processed in the current frame, to prevent multiple objects from responding to the same click.
local DraggingWindowID = -1 -- This variable is used to track whether a window is currently being dragged, to prevent multiple windows from responding to the same drag.
local nextWID = 0 -- This variable is used to assign unique IDs to windows.

--[[
loadspr(0,"./charset.png",0x80000,false,0)
local function print(text,x,y,_,_)
    for i=1,#text do
        local c = string.byte(text,i)
        local ix = c % 16 * 16
        local iy = math.floor(c / 16) * 16
        bitblt_bb(0x80000,ix,iy,256,256,x+(i-1)*8,y,16,16,0)
    end
end
]]

local function transformCoord(obj)
    local actualX, actualY
    if obj.parent then
        actualX = obj.parent.x + obj.x
        actualY = obj.parent.y + obj.y
    else
        actualX = obj.x
        actualY = obj.y
    end
    return actualX, actualY
end

local Button = {}
Button.new = function(self, x, y, width, height, text)
    local obj = {}
    local int = math.floor
    setmetatable(obj, self)
    self.__index = self
    obj.x = x
    obj.y = y
    obj.width = width
    obj.height = height
    obj.text = text or "Button"
    obj.isClickedFirstFrame = true
    obj.color = rgb(200, 200, 200)
    obj.textColor = rgb(0, 0, 0)
    obj.hoverColor = rgb(220, 220, 220)
    obj.hoverTextColor = rgb(0, 0, 0)
    obj.isHovered = false
    obj.parent = nil
    obj.onClick = function() end  -- Default click handler
    obj.draw = function()
        local color = obj.isHovered and obj.hoverColor or obj.color
        local textColor = obj.isHovered and obj.hoverTextColor or obj.textColor
        local actualX, actualY = transformCoord(obj)
        rect(actualX, actualY, obj.width, obj.height, color)
        rectb(actualX, actualY, obj.width, obj.height, textColor)
        print(obj.text, actualX + int(obj.width / 2) - int((#obj.text / 2)*8), actualY + int(obj.height / 2) - 6, textColor, 1)
    end
    obj.update = function()
        local mx, my, ms = mouse()
        local actualX, actualY = transformCoord(obj)
        obj.isHovered = mx >= actualX and mx <= (actualX + obj.width) and my >= actualY and my <= (actualY + obj.height)
        if obj.isHovered and ms==1 and not isMouseEventProessed then
            isMouseEventProessed = true
            if obj.isClickedFirstFrame then
                obj.isClickedFirstFrame = false
                obj.onClick()  -- Call the click handler if the button is hovered and clicked
            end
        end
        if ms == 0 then
            obj.isClickedFirstFrame = true
        end
    end
    return obj
end

local Label = {}
Label.new = function(self, x, y, text)
    local obj = {}
    setmetatable(obj, self)
    self.__index = self
    obj.x = x
    obj.y = y
    obj.text = text or "Label"
    obj.color = rgb(0, 0, 0)
    obj.parent = nil
    obj.draw = function()
        local actualX, actualY = transformCoord(obj)
        print(obj.text, actualX, actualY, obj.color)
    end
    return obj
end

local Window = {}
Window.new = function(self, x, y, width, height, title)
    local obj = {}
    setmetatable(obj, self)
    self.__index = self
    obj.x = x
    obj.y = y
    obj.width = width
    obj.height = height
    obj.WindowID = nextWID
    nextWID = nextWID + 1 -- Increment for next window
    obj.title = title or "Window"
    obj.color = rgb(180, 180, 180)
    obj.titleColor = rgb(0, 0, 0)
    obj.titleBarHeight = 20
    obj.objects = {}
    obj.parent = nil
    obj.isHovered = false
    obj.isHoveredTitleBar = false
    obj.isDragging = false
    obj.isDraggingFirstFrame = true
    obj.drawingOrder = 0 -- if it's 0 it will be drawn first and it will be assumed active window
    obj.dragOffsetX = 0
    obj.dragOffsetY = 0
    obj.addObject = function(o)
        table.insert(obj.objects, o)
        o.parent = obj
    end
    obj.draw = function()
        local actualX, actualY = transformCoord(obj)
        rect(actualX, actualY, obj.width, obj.height, obj.color)
        rectb(actualX, actualY, obj.width, obj.height, rgb(0, 0, 0))
        rectb(actualX, actualY, obj.width, obj.titleBarHeight, rgb(0, 0, 0))
        print(obj.title, actualX + 4, actualY + 4, obj.titleColor, 1)
        for _, o in ipairs(obj.objects) do
            o.draw()
        end
    end
    obj.update = function()
        -- Bring window to front on click
        local mx, my, ms = mouse() -- Get mouse state (ms = bitfield: left=1, right=2, middle=4)
        local actualX, actualY = transformCoord(obj)
        obj.isHovered = mx >= actualX and mx <= (actualX + obj.width) and my >= actualY and my <= (actualY + obj.height)
        obj.isHoveredTitleBar = obj.isHovered and my <= (actualY + obj.titleBarHeight)
        if ms == 1 and (not isMouseEventProessed and DraggingWindowID == -1) and obj.isHoveredTitleBar or obj.isDragging  then
            isMouseEventProessed = true
            DraggingWindowID = obj.WindowID -- Set the dragging window ID to the current window index
            -- Move this window to the end of the windows list to draw it last (on top)
            for i, w in ipairs(windows) do
                if w == obj then
                    table.remove(windows, i)
                    table.insert(windows, obj)
                    break
                end
            end
            -- Start dragging
            if obj.isDraggingFirstFrame then
                obj.dragOffsetX = mx - actualX
                obj.dragOffsetY = my - actualY
                obj.isDraggingFirstFrame = false
                --trace("Started dragging window: "..obj.title.."dX: "..obj.dragOffsetX.."dY: "..obj.dragOffsetY)
            end
            obj.isDragging = true
            obj.x = mx - obj.dragOffsetX
            obj.y = my - obj.dragOffsetY
            if ms == 0 then
                obj.isDragging = false
                obj.isDraggingFirstFrame = true
                DraggingWindowID = -1 -- Reset dragging window ID when mouse button is released
                --trace("Stopped dragging window: "..obj.title)
            end
        end
        for _, o in ipairs(obj.objects) do
            if o.update then
                o.update()
            end
        end
    end
    return obj
end

-- カーソルの描画関数
local function drawcur()
    local mx,my,mb=mouse()
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

-- setup
function BOOT()
    windows = {}
    screen(0) -- switch to graphic mode
    showcur(0) -- disable OS cursor
    mainWindow = Window:new(40, 40, 304, 208, "Main Window")
    btnHello = Button:new(80, 100, 160, 32, "Click Me!")
    btnHello.onClick = function()
        if (windowSub) then return end -- Prevent opening multiple sub windows
        windowSub = Window:new(60, 60, 200, 100, "Sub Window")
        lblSub = Label:new(10, 40, "Hello from Sub Window!")
        btnClose = Button:new(50, 60, 100, 30, "Close")
        btnClose.onClick = function()
            -- Remove the sub window from the windows list to close it
            for i, w in ipairs(windows) do
                if w == windowSub then
                    table.remove(windows, i)
                    windowSub = nil
                    break
                end
            end
        end
        windowSub.addObject(lblSub)
        windowSub.addObject(btnClose)
        table.insert(windows, windowSub)
    end
    mainWindow.addObject(btnHello)
    local lblInfo = Label:new(10, 40, "CPT200 GUI Demo")
    mainWindow.addObject(lblInfo)
    table.insert(windows, mainWindow)
end

-- Thicken the font by OR-ing each pixel with the pixel to the right
for addr=0x1b300, 0x1b300+12*256-1 do
    val = vpeek(addr)
    val = bit.bor(val,bit.rshift(val, 1))
    vpoke(addr,val)
end

function test()
    for i = 0, 255 do
        local ix = (i % 16) * 8
        local iy = math.floor(i / 16) * 8
        bitblt_bb(0x80000,ix,iy,128,128,ix,iy,8,8,0)
    end
end

-- main loop
function LOOP()
    cls(255) -- clear screen
    isMouseEventProessed = false -- Reset mouse event processed state at the beginning of each frame
    --print("test",10,10,rgb(0,0,0))
    for i, w in ipairs(windows) do
        w.update() -- update all windows and all their child objects
        w.draw()   -- draw all windows and all their child objects
    end
    print("Window(s): "..#windows..", Dragging: "..tostring(DraggingWindowID), 0, 0, rgb(0,0,0), 0)
    for i, w in ipairs(windows) do
        print(i..": \""..w.title.."\",ID: "..tostring(w.WindowID), 0, 0 + i*12, rgb(0,0,0), 0)
    end
    --test()
    drawcur() -- draw custom cursor
end