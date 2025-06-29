-- Embedded shader test - standalone
local shader = nil
local time = 0
local success = false
local useShader = false

function love.load()
    print("=== EMBEDDED SHADER TEST ===")
    
    -- Simple vertex shader
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
    
    -- Simple fragment shader - red color
    local fragmentCode = [[
        varying vec2 VaryingTexCoord;
        varying vec4 VaryingColor;
        
        void main()
        {
            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    ]]
    
    print("Creating shader...")
    local ok, err = pcall(function()
        shader = love.graphics.newShader(vertexCode, fragmentCode)
        success = true
        print("SUCCESS: Shader created!")
    end)
    
    if not ok then
        print("ERROR creating shader:", err)
        success = false
    end
    
    love.graphics.setBackgroundColor(0.1, 0.1, 0.2)
end

function love.update(dt)
    time = time + dt
end

function love.draw()
    love.graphics.setColor(1, 1, 1)
    love.graphics.print("Embedded Shader Test", 10, 10)
    
    if success then
        love.graphics.print("Shader: OK", 10, 40)
        love.graphics.print("Active: " .. (useShader and "YES" or "NO"), 10, 70)
        
        -- Test the shader
        if useShader and shader then
            love.graphics.setShader(shader)
        end
        
        love.graphics.setColor(1, 1, 1)
        love.graphics.rectangle("fill", 150, 150, 120, 80)
        
        if useShader then
            love.graphics.setShader()
        end
    else
        love.graphics.print("Shader: FAILED", 10, 40)
        love.graphics.setColor(0.5, 0.5, 0.5)
        love.graphics.rectangle("fill", 150, 150, 120, 80)
    end
    
    love.graphics.setColor(1, 1, 1)
    love.graphics.print("Time: " .. string.format("%.1f", time), 10, 300)
    love.graphics.print("A=Toggle B=Exit", 10, 330)
end

function love.joystickpressed(joystick, button)
    if button == 1 then -- A
        useShader = not useShader
        print("Shader:", useShader and "ON" or "OFF")
    elseif button == 2 then -- B
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
