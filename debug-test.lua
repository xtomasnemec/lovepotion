-- Ultra minimal test for debugging
local frame = 0

function love.load()
    print("DEBUG: love.load() called")
    love.graphics.setBackgroundColor(0, 1, 0) -- Green background
end

function love.update(dt)
    frame = frame + 1
    if frame % 60 == 0 then -- Every second at 60fps
        print("DEBUG: Frame " .. frame .. ", dt=" .. dt)
    end
end

function love.draw()
    love.graphics.setColor(1, 1, 1) -- White text
    love.graphics.print("DEBUG TEST", 50, 50)
    love.graphics.print("Frame: " .. frame, 50, 80)
end
