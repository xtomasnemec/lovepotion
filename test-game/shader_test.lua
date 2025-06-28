-- Simple GLSL shader test for LOVE Potion on Wii U
local shader = nil
local time = 0
local success = false

function love.load()
    print("=== GLSL Shader Test ===")
    
    -- Simple vertex shader
    local vertexCode = [[
        attribute vec4 VertexPosition;
        attribute vec2 VertexTexCoord;
        attribute vec4 VertexColor;
        
        varying vec2 VaryingTexCoord;
        varying vec4 VaryingColor;
        
        uniform mat4 TransformProjectionMatrix;
        uniform float time;
        
        void main()
        {
            VaryingTexCoord = VertexTexCoord;
            VaryingColor = VertexColor;
            
            vec4 pos = VertexPosition;
            pos.x += sin(time + pos.y * 2.0) * 0.05;
            
            gl_Position = TransformProjectionMatrix * pos;
        }
    ]]
    
    -- Simple fragment shader with color animation
    local fragmentCode = [[
        varying vec2 VaryingTexCoord;
        varying vec4 VaryingColor;
        
        uniform float time;
        
        void main()
        {
            vec2 uv = VaryingTexCoord;
            
            float r = 0.5 + 0.5 * sin(time + uv.x * 3.14159);
            float g = 0.5 + 0.5 * sin(time + uv.y * 3.14159 + 2.0);
            float b = 0.5 + 0.5 * sin(time + (uv.x + uv.y) * 3.14159 + 4.0);
            
            vec3 color = vec3(r, g, b);
            gl_FragColor = vec4(color, 1.0) * VaryingColor;
        }
    ]]
    
    -- Try to create the shader
    local shaderSuccess, shaderError = pcall(function()
        shader = love.graphics.newShader(vertexCode, fragmentCode)
        success = true
        print("GLSL shader created successfully!")
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
    love.graphics.print("GLSL Shader Test", 10, 10)
    
    if success and shader then
        love.graphics.print("Status: SUCCESS - Shader loaded", 10, 40)
        love.graphics.print("Press A to toggle shader", 10, 70)
        
        -- Test shader with different shapes
        love.graphics.setShader(shader)
        
        -- Rectangle
        love.graphics.setColor(1, 1, 1)
        love.graphics.rectangle("fill", 100, 100, 100, 100)
        
        -- Circle
        love.graphics.setColor(1, 0.8, 0.6)
        love.graphics.circle("fill", 300, 150, 50)
        
        -- Triangle
        love.graphics.setColor(0.8, 1, 0.6)
        love.graphics.polygon("fill", 
            450, 100,
            500, 200,
            400, 200
        )
        
        love.graphics.setShader()
        
        -- Show time
        love.graphics.setColor(1, 1, 1)
        love.graphics.print("Time: " .. string.format("%.2f", time), 10, 300)
        
    else
        love.graphics.print("Status: FAILED - Could not load shader", 10, 40)
        love.graphics.print("Falling back to simple graphics", 10, 70)
        
        -- Draw simple shapes without shader
        love.graphics.setColor(0.5, 0.5, 0.5)
        love.graphics.rectangle("fill", 100, 100, 100, 100)
        love.graphics.circle("fill", 300, 150, 50)
        love.graphics.polygon("fill", 450, 100, 500, 200, 400, 200)
    end
    
    love.graphics.setColor(1, 1, 1)
    love.graphics.print("Press B to exit", 10, 350)
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
            print("Shader object exists:", shader)
            print("Testing setShader/getShader...")
            
            local currentShader = love.graphics.getShader()
            print("Current shader:", currentShader)
            
            if currentShader == shader then
                love.graphics.setShader()
                print("Shader disabled")
            else
                love.graphics.setShader(shader)
                print("Shader enabled")
            end
        end
    elseif button == 2 then -- B button
        love.event.quit()
    end
end

function love.gamepadpressed(joystick, button)
    if button == "a" then
        -- Same as joystick button 1
        love.joystickpressed(joystick, 1)
    elseif button == "b" then
        love.event.quit()
    end
end

function love.quit()
    print("=== Shader Test Exiting ===")
    return false
end
