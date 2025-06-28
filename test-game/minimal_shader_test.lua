-- Minimal GLSL shader test that doesn't immediately use the shader
local shader = nil
local time = 0
local success = false
local useShader = false

function love.load()
    print("=== Minimal GLSL Shader Test ===")
    
    -- Very simple shader without complex uniforms
    local vertexCode = [[
        attribute vec4 VertexPosition;
        attribute vec2 VertexTexCoord;
        attribute vec4 VertexColor;
        
        varying vec2 VaryingTexCoord;
        varying vec4 VaryingColor;
        
        uniform mat4 TransformProjectionMatrix;
        
        void main()
        {
            VaryingTexCoord = VertexTexCoord;
            VaryingColor = VertexColor;
            gl_Position = TransformProjectionMatrix * VertexPosition;
        }
    ]]
    
    -- Simple fragment shader without uniforms
    local fragmentCode = [[
        varying vec2 VaryingTexCoord;
        varying vec4 VaryingColor;
        
        void main()
        {
            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Just red color
        }
    ]]
    
    -- Try to create the shader
    local shaderSuccess, shaderError = pcall(function()
        shader = love.graphics.newShader(vertexCode, fragmentCode)
        success = true
        print("Simple GLSL shader created successfully!")
    end)
    
    if not shaderSuccess then
        print("Failed to create shader:", shaderError)
        success = false
    end
    
    love.graphics.setBackgroundColor(0.1, 0.1, 0.1)
end

function love.update(dt)
    time = time + dt
end

function love.draw()
    love.graphics.setColor(1, 1, 1)
    love.graphics.print("Minimal GLSL Test", 10, 10)
    
    if success and shader then
        love.graphics.print("Status: SUCCESS - Shader loaded", 10, 40)
        love.graphics.print("Press A to toggle shader on/off", 10, 70)
        love.graphics.print("Shader active: " .. (useShader and "YES" or "NO"), 10, 100)
        
        -- Only use shader if explicitly enabled
        if useShader then
            love.graphics.setShader(shader)
            love.graphics.setColor(1, 1, 1) -- White, but shader will make it red
        else
            love.graphics.setColor(0.5, 0.5, 0.5) -- Gray when no shader
        end
        
        -- Simple rectangle
        love.graphics.rectangle("fill", 200, 150, 100, 100)
        
        if useShader then
            love.graphics.setShader() -- Reset shader
        end
        
    else
        love.graphics.print("Status: FAILED - Could not load shader", 10, 40)
        love.graphics.print("Drawing without shader", 10, 70)
        
        -- Draw simple shape without shader
        love.graphics.setColor(0.5, 0.5, 0.5)
        love.graphics.rectangle("fill", 200, 150, 100, 100)
    end
    
    love.graphics.setColor(1, 1, 1)
    love.graphics.print("Time: " .. string.format("%.1f", time), 10, 300)
    love.graphics.print("Press A - Toggle Shader | B - Exit", 10, 350)
end

function love.keypressed(key)
    if key == "escape" then
        love.event.quit()
    end
end

function love.joystickpressed(joystick, button)
    print("Button pressed:", button)
    
    if button == 1 then -- A button
        if shader then
            useShader = not useShader
            print("Shader toggled to:", useShader and "ON" or "OFF")
        else
            print("No shader available to toggle")
        end
    elseif button == 2 then -- B button
        love.event.quit()
    end
end

function love.gamepadpressed(joystick, button)
    if button == "a" then
        love.joystickpressed(joystick, 1)
    elseif button == "b" then
        love.event.quit()
    end
end

function love.quit()
    print("=== Minimal Shader Test Exiting ===")
    return false
end
