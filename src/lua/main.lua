-- CPT200 Disk Operating System

Button = {}
Button.new = function(self, x, y, width, height, text)
    local obj = {}
    setmetatable(obj, self)
    self.__index = self
    obj.x = x
    obj.y = y
    obj.width = width
    obj.height = height
    obj.text = text or "Button"
    obj.color = rgb(200, 200, 200)
    obj.textColor = rgb(0, 0, 0)
    obj.hoverColor = rgb(220, 220, 220)
    obj.hoverTextColor = rgb(0, 0, 0)
    obj.isHovered = false
    obj.onClick = function() end  -- Default click handler
    obj.draw = function()
        local color = obj.isHovered and obj.hoverColor or obj.color
        local textColor = obj.isHovered and obj.hoverTextColor or obj.textColor
        rect(obj.x, obj.y, obj.x + obj.width, obj.y + obj.height, color)
        print(obj.text, obj.x + (obj.width // 2) - (#obj.text // 2), obj.y + (obj.height // 2) - 4, textColor)
    end
    obj.checkHover = function()
        local mx, my, _ = mouse()
        obj.isHovered = mx >= obj.x and mx <= (obj.x + obj.width) and my >= obj.y and my <= (obj.y + obj.height)
    end
    return obj
end

function BOOT()
    screen(0)
    cls(rgb(64,64,64))
    button1 = Button:new(10, 10, 100, 30, "Start")
    
end

function LOOP()
    button1.checkHover()
    button1.draw()
end