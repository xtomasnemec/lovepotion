-- Simple Test for GLSL Shader Support
print("=== LOADING LOVE POTION TEST ===")

local shader = nil
local time = 0
local success = false
local useShader = false

function love.load()
    print("=== SIMPLE SHADER TEST LOADED ===")
    
    -- Basic test without shader first
    love.graphics.setBackgroundColor(0.1, 0.1, 0.1)
    
    -- Only set window title on non-Wii U platforms
    if love.system.getOS() ~= "cafe" then
        love.window.setTitle("LOVE Potion Comprehensive Test")
    else
        print("Wii U detected - skipping window title setting")
    end
    
    -- Test asset loading
    testAssets()
    
    -- Check for joysticks
    local joystickCount = love.joystick.getJoystickCount()
    print("Number of joysticks detected:", joystickCount)
    
    local joysticks = love.joystick.getJoysticks()
    for i, joystick in ipairs(joysticks) do
        print("Joystick", i, ":", joystick:getName())
        if joystick.getGamepadType then
            print("  - Type:", joystick:getGamepadType())
        end
        print("  - GUID:", joystick:getGUID())
        -- Skip isConnected() as it's not implemented on Wii U
        print("  - Available: YES")
    end
end

function testAssets()
    print("=== TESTING ASSET LOADING ===")
    
    -- Test Font Loading
    testFont = loadTestFont()
    
    -- Test Image Loading (create a test image)
    testImage = loadTestImage()
    
    -- Test Sound Loading (create a test sound)
    testSound = loadTestSound()
    
    -- Test Shader Loading
    testShader = loadTestShader()
    
    print("=== ASSET TESTING COMPLETE ===")
end

function loadTestFont()
    print("Testing font loading...")
    local success, result = pcall(function()
        -- Try to create a font (using default size)
        local font = love.graphics.newFont(24)
        if font then
            love.graphics.setFont(font)
            return font
        else
            error("Font creation returned nil")
        end
    end)
    
    if success then
        testResults.font = "SUCCESS"
        print("Font loading: SUCCESS")
        return result
    else
        testResults.font = "FAILED: " .. tostring(result)
        print("Font loading FAILED:", result)
        return nil
    end
end

function loadTestImage()
    print("Testing image loading...")
    local success, result = pcall(function()
        -- Create a simple test image data
        local imageData = love.image.newImageData(64, 64)
        
        -- Fill with a gradient pattern
        for y = 0, 63 do
            for x = 0, 63 do
                local r = x / 63
                local g = y / 63
                local b = (x + y) / 126
                imageData:setPixel(x, y, r, g, b, 1)
            end
        end
        
        -- Create texture from image data
        local image = love.graphics.newImage(imageData)
        return image
    end)
    
    if success then
        testResults.image = "SUCCESS"
        print("Image loading: SUCCESS")
        return result
    else
        testResults.image = "FAILED: " .. tostring(result)
        print("Image loading FAILED:", result)
        return nil
    end
end

function loadTestSound()
    print("Testing sound loading...")
    local success, result = pcall(function()
        -- Try to create a simple sound source
        -- Note: LOVE Potion may have limited sound support
        local soundData = love.sound.newSoundData(22050, 22050, 16, 1) -- 1 second, mono
        
        -- Generate a simple sine wave
        for i = 0, soundData:getSampleCount() - 1 do
            local t = i / soundData:getSampleRate()
            local sample = math.sin(2 * math.pi * 440 * t) * 0.3 -- 440 Hz tone
            soundData:setSample(i, sample)
        end
        
        local source = love.audio.newSource(soundData, "static")
        return source
    end)
    
    if success then
        testResults.sound = "SUCCESS"
        print("Sound loading: SUCCESS")
        return result
    else
        testResults.sound = "FAILED: " .. tostring(result)
        print("Sound loading FAILED:", result)
        return nil
    end
end

function loadTestShader()
    print("Testing shader loading...")
    local success, result = pcall(function()
        -- Simple vertex and fragment shader
        local vertexShader = [[
        vec4 position(mat4 transform_projection, vec4 vertex_position)
        {
            return transform_projection * vertex_position;
        }
        ]]
        
        local fragmentShader = [[
        vec4 effect(vec4 color, Image texture, vec2 texture_coords, vec2 screen_coords)
        {
            vec4 texcolor = Texel(texture, texture_coords);
            return texcolor * color * vec4(1.0, 0.8, 0.6, 1.0); // Warm tint
        }
        ]]
        
        local shader = love.graphics.newShader(fragmentShader, vertexShader)
        return shader
    end)
    
    if success then
        testResults.shader = "SUCCESS"
        print("Shader loading: SUCCESS")
        return result
    else
        testResults.shader = "FAILED: " .. tostring(result)
        print("Shader loading FAILED:", result)
        return nil
    end
end

function love.update(dt)
    -- IMPORTANT: Pump events to make joysticks work!
    love.event.pump()
    
    frameCount = frameCount + 1
    
    -- Prevent frameCount from getting too high (fix for freeze at 1000)
    if frameCount > 999 then
        frameCount = 0
    end
    
    -- Print debug info every 60 frames (1 second at 60 FPS)
    if frameCount % 60 == 0 then
        print("Frame:", frameCount, "DT:", dt)
        
        -- Also check joysticks periodically
        local joystickCount = love.joystick.getJoystickCount()
        print("Joysticks available:", joystickCount)
    end
end

function love.draw()
    -- Set white color for text
    love.graphics.setColor(1, 1, 1, 1)
    
    -- Draw header
    love.graphics.print("LOVE Potion Comprehensive Test", 50, 20)
    love.graphics.print("Frame: " .. frameCount .. " | FPS: " .. love.timer.getFPS(), 50, 50)
    love.graphics.print("OS: " .. love.system.getOS(), 50, 80)
    
    -- Display asset test results
    love.graphics.print("=== ASSET TEST RESULTS ===", 50, 120)
    love.graphics.print("Font: " .. testResults.font, 50, 150)
    love.graphics.print("Image: " .. testResults.image, 50, 180)
    love.graphics.print("Sound: " .. testResults.sound, 50, 210)
    love.graphics.print("Shader: " .. testResults.shader, 50, 240)
    
    -- Test the loaded assets visually
    if testFont then
        local oldFont = love.graphics.getFont()
        love.graphics.setFont(testFont)
        love.graphics.setColor(0, 1, 0, 1) -- Green
        love.graphics.print("Custom Font Test!", 400, 150)
        love.graphics.setFont(oldFont)
        love.graphics.setColor(1, 1, 1, 1) -- Reset
    end
    
    if testImage then
        love.graphics.setColor(1, 1, 1, 1)
        love.graphics.draw(testImage, 400, 180)
        love.graphics.print("Test Image (64x64)", 470, 200)
    end
    
    if testSound then
        love.graphics.setColor(1, 1, 0, 1) -- Yellow
        love.graphics.print("Sound ready (press S to play)", 400, 250)
        love.graphics.setColor(1, 1, 1, 1) -- Reset
    end
    
    if testShader then
        -- Draw something with the shader
        love.graphics.setShader(testShader)
        love.graphics.setColor(1, 0.5, 0.2, 1) -- Orange
        love.graphics.rectangle("fill", 400, 280, 100, 50)
        love.graphics.setShader() -- Reset shader
        love.graphics.setColor(1, 1, 1, 1)
        love.graphics.print("Shader test rectangle", 510, 295)
    end
    
    -- Display joystick info
    love.graphics.print("=== JOYSTICK INFO ===", 50, 300)
    local joystickCount = love.joystick.getJoystickCount()
    love.graphics.print("Joysticks: " .. joystickCount, 50, 330)
    
    local joysticks = love.joystick.getJoysticks()
    for i, joystick in ipairs(joysticks) do
        local name = joystick:getName() or "Unknown"
        love.graphics.print("J" .. i .. ": " .. name, 50, 330 + i * 30)
        
        -- Show if any buttons are pressed
        local buttonPressed = false
        for btn = 1, 16 do
            if joystick:isDown(btn) then
                love.graphics.print("BTN" .. btn .. " DOWN", 250, 330 + i * 30)
                buttonPressed = true
                break
            end
        end
        
        -- Show axis values if gamepad (LOVE Potion compatible)
        if joystick:isGamepad() then
            local axisCount = joystick:getAxisCount() or 0
            if axisCount > 0 then
                local axisValues = {}
                for a = 1, math.min(4, axisCount) do
                    local value = joystick:getAxis(a) or 0
                    table.insert(axisValues, string.format("%.2f", value))
                end
                love.graphics.print("Axes: " .. table.concat(axisValues, ","), 400, 330 + i * 30)
            end
        end
    end
    
    -- Draw animated elements
    local time = love.timer.getTime()
    local x = 650 + math.sin(time) * 100
    local y = 400 + math.cos(time * 0.5) * 50
    
    love.graphics.setColor(1, 0.5, 0.2, 1) -- Orange color
    love.graphics.circle("fill", x, y, 30)
    
    -- Draw a rectangle
    love.graphics.setColor(0.2, 1, 0.3, 1) -- Green color
    love.graphics.rectangle("fill", 600, 500, 200, 100)
    
    -- Reset color
    love.graphics.setColor(1, 1, 1, 1)
    
    -- Instructions
    love.graphics.print("Press ESC to quit | S to play sound | R to reload assets", 50, 650)
end

function love.keypressed(key)
    print("Key pressed:", key)
    if key == "escape" then
        love.event.quit()
    elseif key == "s" and testSound then
        print("Playing test sound...")
        testSound:play()
    elseif key == "r" then
        print("Reloading assets...")
        testAssets()
    end
end

function love.joystickpressed(joystick, button)
    print("Joystick button pressed - Joystick:", joystick:getName(), "Button:", button)
end

function love.joystickreleased(joystick, button)
    print("Joystick button released - Joystick:", joystick:getName(), "Button:", button)
end

function love.gamepadpressed(joystick, button)
    print("Gamepad button pressed - Joystick:", joystick:getName(), "Button:", button)
    
    -- Play sound on gamepad button press if available
    if button == "a" and testSound then
        testSound:play()
    end
end

function love.gamepadreleased(joystick, button)
    print("Gamepad button released - Joystick:", joystick:getName(), "Button:", button)
end

function love.gamepadaxis(joystick, axis, value)
    -- Only print significant axis changes to avoid spam
    if math.abs(value) > 0.1 then
        print("Gamepad axis - Joystick:", joystick:getName(), "Axis:", axis, "Value:", value)
    end
end

function love.quit()
    print("=== COMPREHENSIVE TEST GAME QUITTING ===")
    return false -- Allow quit
end
