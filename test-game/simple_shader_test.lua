-- Simple shader test that should work without crashing
local shader = nil
local success = false

function love.load()
    print("=== Simple Shader Test ===")
    
    -- Very basic test - try to create shader and handle failure gracefully
    local shaderSuccess, shaderError = pcall(function()
        print("Attempting to create simple shader...")
        
        -- Just try to call newShader with minimal shader code
        shader = love.graphics.newShader([[
            vec4 effect(vec4 color, Image texture, vec2 texture_coords, vec2 screen_coords)
            {
                return vec4(1.0, 0.5, 0.0, 1.0); // Simple orange color
            }
        ]])
        
        success = true
        print("SUCCESS: Basic shader created!")
    end)
    
    if not shaderSuccess then
        print("FAILED: Shader creation failed:", shaderError)
        success = false
    end
    
    love.graphics.setBackgroundColor(0.2, 0.2, 0.2)
end

function love.update(dt)
    -- Nothing here
end

function love.draw()
    love.graphics.setColor(1, 1, 1)
    love.graphics.print("Simple Shader Test", 10, 10)
    
    if success and shader then
        love.graphics.print("Status: SUCCESS", 10, 40)
        love.graphics.print("Shader: LOADED", 10, 70)
        
        -- Test applying the shader
        love.graphics.setColor(1, 1, 1)
        love.graphics.setShader(shader)
        love.graphics.rectangle("fill", 100, 100, 200, 100)
        love.graphics.setShader()
        
    else
        love.graphics.print("Status: FAILED", 10, 40)
        love.graphics.print("Shader: NOT LOADED", 10, 70)
        
        -- Draw without shader
        love.graphics.setColor(0.8, 0.8, 0.8)
        love.graphics.rectangle("fill", 100, 100, 200, 100)
    end
    
    love.graphics.setColor(1, 1, 1)
    love.graphics.print("Press B to exit", 10, 300)
end

function love.keypressed(key)
    if key == "escape" then
        love.event.quit()
    end
end

function love.joystickpressed(joystick, button)
    if button == 2 then -- B button
        love.event.quit()
    end
end

function love.gamepadpressed(joystick, button)
    if button == "b" then
        love.event.quit()
    end
end

function love.quit()
    print("=== Simple Shader Test Exiting ===")
    return false
end
