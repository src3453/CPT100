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
        rectb(obj.x, obj.y, obj.x + obj.width, obj.y + obj.height, textColor)
        print(obj.text, obj.x + (obj.width // 2) - int((#obj.text / 2)*8), obj.y + (obj.height // 2) - 5, textColor)
    end
    obj.update = function()
        local mx, my, ms = mouse()
        obj.isHovered = mx >= obj.x and mx <= (obj.x + obj.width) and my >= obj.y and my <= (obj.y + obj.height)
        if obj.isHovered and ms==1 then
            obj.onClick()  -- Call the click handler if the button is hovered and clicked
        end
    end
    return obj
end

_PODOS_VERSION="0.1"
CurX = 0
CurY = 2

function BOOT()
    screen(1)
    cls(0)
    printp("PoDOS Version ".._PODOS_VERSION, 0, 0, 255, 0)
    printp("(c) 2025 src3453", 0, 1, 255, 0)
end



function LOOP()
    
end