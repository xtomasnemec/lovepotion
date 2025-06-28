-- Comprehensive test game for LOVE Potion on Wii U
local frameCount = 0

-- Test assets
local testFont = nil
local testImage = nil
local testSound = nil
local testShader = nil
local testResults = {
    font = "Not tested",
    image = "Not tested", 
    sound = "Not tested",
    shader = "Not tested"
}

function love.load()
    print("=== OPTIMIZED TEST GAME LOADED ===")
    love.graphics.setBackgroundColor(0, 0.2, 0.5) -- Dark blue background
    
    -- Test font loading
    print("Testing font loading...")
    local fontSuccess, fontError = pcall(function()
        testFont = love.graphics.newFont(16)
        testResults.font = "SUCCESS: Default font loaded"
    end)
    if not fontSuccess then
        testResults.font = "ERROR: " .. tostring(fontError)
        print("Font test failed:", fontError)
    end
    
    -- Test image loading (create a simple colored rectangle as test)
    print("Testing image loading...")
    local imageSuccess, imageError = pcall(function()
        -- Try to create a simple ImageData first
        local imgData = love.image.newImageData(32, 32) -- Smaller for performance
        for x = 0, 31 do
            for y = 0, 31 do
                local r = x / 31
                local g = y / 31
                local b = (x + y) / 62
                imgData:setPixel(x, y, r, g, b, 1)
            end
        end
        testImage = love.graphics.newImage(imgData)
        testResults.image = "SUCCESS: 32x32 test image created"
    end)
    if not imageSuccess then
        testResults.image = "ERROR: " .. tostring(imageError)
        print("Image test failed:", imageError)
    end
    
    -- Test sound loading (try to create a simple tone)
    print("Testing sound loading...")
    local soundSuccess, soundError = pcall(function()
        -- Try to create simple sound data
        local sampleRate = 22050
        local duration = 0.2 -- Shorter for performance
        local samples = math.floor(sampleRate * duration)
        local soundData = love.sound.newSoundData(samples, sampleRate, 16, 1)
        
        for i = 0, samples - 1 do
            local t = i / sampleRate
            local sample = math.sin(440 * 2 * math.pi * t) * 0.3
            soundData:setSample(i, sample)
        end
        
        testSound = love.audio.newSource(soundData, "static")
        testResults.sound = "SUCCESS: 440Hz tone created"
    end)
    if not soundSuccess then
        testResults.sound = "ERROR: " .. tostring(soundError)
        print("Sound test failed:", soundError)
    end
    
    -- Test shader loading with new GLSL support
    print("Testing GLSL shader loading...")
    local shaderSuccess, shaderError = pcall(function()
        local vertexShader = [[
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
                pos.x += sin(time + pos.y * 0.1) * 0.1;
                
                gl_Position = TransformProjectionMatrix * pos;
            }
        ]]
        
        local fragmentShader = [[
            varying vec2 VaryingTexCoord;
            varying vec4 VaryingColor;
            
            uniform float time;
            
            void main()
            {
                vec2 uv = VaryingTexCoord;
                vec3 color = vec3(
                    0.5 + 0.5 * sin(time + uv.x * 6.28),
                    0.5 + 0.5 * sin(time + uv.y * 6.28 + 2.09),
                    0.5 + 0.5 * sin(time + (uv.x + uv.y) * 6.28 + 4.18)
                );
                
                gl_FragColor = vec4(color, 1.0) * VaryingColor;
            }
        ]]
        
        testShader = love.graphics.newShader(vertexShader, fragmentShader)
        testResults.shader = "SUCCESS: GLSL shader compiled"
        print("GLSL shader created successfully!")
    end)
    if not shaderSuccess then
        testResults.shader = "ERROR: " .. tostring(shaderError)
        print("Shader test failed:", shaderError)
        testShader = nil
    end
    
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
        print("  - Available: YES")
    end
end

function love.update(dt)
    frameCount = frameCount + 1
    
    if frameCount > 9999 then
        frameCount = 0
    end
    
    -- No shader updates for maximum performance
    -- But if we have a shader, update its time uniform
    if testShader then
        local time = love.timer.getTime() or (frameCount * 0.016)
        -- Note: uniform setting will be implemented as shader functionality improves
    end
    
    -- Very minimal debug prints
    if frameCount % 1800 == 0 then -- Every 30 seconds at 60fps
        print("Frame:", frameCount, "FPS:", love.timer.getFPS() or "N/A")
    end
end

function love.draw()
    love.graphics.setColor(1, 1, 1, 1)
    
    -- Use custom font if available
    if testFont then
        love.graphics.setFont(testFont)
    end
    
    -- Main title and essential info only
    love.graphics.print("LOVE Potion Test - Optimized", 10, 10)
    love.graphics.print("Frame: " .. frameCount, 10, 40)
    love.graphics.print("FPS: " .. (love.timer.getFPS() or "N/A"), 10, 70)
    
    -- Simplified test results
    love.graphics.print("Font: " .. (testFont and "OK" or "FAIL"), 10, 110)
    love.graphics.print("Image: " .. (testImage and "OK" or "FAIL"), 10, 130)
    love.graphics.print("Sound: " .. (testSound and "OK" or "FAIL"), 10, 150)
    love.graphics.print("Shader: " .. (testShader and "OK" or "NOT SUPPORTED"), 10, 170)
    
    -- Test image drawing (simplified)
    if testImage then
        love.graphics.draw(testImage, 300, 110)
    end
    
    -- Enhanced shader test - now with real GLSL support!
    if testShader then
        love.graphics.setShader(testShader)
        love.graphics.setColor(1, 0.8, 0.6, 1)
        
        -- Draw a rectangle with the animated shader
        love.graphics.rectangle("fill", 400, 110, 80, 80)
        
        -- Draw some triangles to test vertex shader animation
        love.graphics.polygon("fill", 
            500, 110,
            520, 140, 
            480, 140
        )
        
        love.graphics.setShader() -- Reset shader
        love.graphics.setColor(1, 1, 1, 1)
        
        -- Add shader status info
        love.graphics.print("GLSL Shader: ACTIVE", 400, 200)
    else
        -- Draw static shapes if no shader
        love.graphics.setColor(0.5, 0.5, 0.5, 1)
        love.graphics.rectangle("fill", 400, 110, 80, 80)
        love.graphics.setColor(1, 1, 1, 1)
        love.graphics.print("GLSL Shader: FAILED", 400, 200)
    end
    
    -- Very basic joystick info
    local joystickCount = love.joystick.getJoystickCount()
    love.graphics.print("Joysticks: " .. joystickCount, 10, 210)
    
    -- Only show basic joystick state
    local joysticks = love.joystick.getJoysticks()
    if joysticks[1] then
        local joystick = joysticks[1]
        love.graphics.print("GamePad: " .. (joystick:getName() or "Unknown"), 10, 240)
        
        -- Show only pressed buttons (no visual gamepad)
        local pressedButtons = {}
        for btn = 1, 8 do -- Only check first 8 buttons for performance
            if joystick:isDown(btn) then
                table.insert(pressedButtons, tostring(btn))
            end
        end
        
        if #pressedButtons > 0 then
            love.graphics.setColor(1, 0, 0, 1)
            love.graphics.print("BTN: " .. table.concat(pressedButtons, ","), 200, 240)
            love.graphics.setColor(1, 1, 1, 1)
        end
        
        -- Simple stick position text only
        local leftX = joystick:getAxis(1) or 0
        local leftY = joystick:getAxis(2) or 0
        if math.abs(leftX) > 0.2 or math.abs(leftY) > 0.2 then
            love.graphics.print(string.format("Stick: %.1f,%.1f", leftX, -leftY), 10, 270)
        end
    end
    
    -- Minimal moving graphics test
    local time = love.timer.getTime() or (frameCount * 0.016)
    local x = 400 + math.sin(time) * 30
    
    love.graphics.setColor(1, 0.5, 0.2, 1)
    love.graphics.circle("fill", x, 300, 10)
    
    love.graphics.setColor(1, 1, 1, 1)
    
    -- Essential controls only
    love.graphics.print("A - Sound | B - Exit | X - Reload Assets", 10, 350)
    love.graphics.print("Note: Testing GLSL shader support on Wii U!", 10, 370)
end

function love.keypressed(key)
    print("Key pressed:", key)
    if key == "escape" then
        love.event.quit()
    end
end

function love.joystickpressed(joystick, button)
    local success, error = pcall(function()
        print("Joystick button pressed - Joystick:", joystick:getName(), "Button:", button)
        
        -- Wii U GamePad controls (button numbers)
        if button == 1 then -- A button
            if testSound then
                local soundSuccess, soundError = pcall(function()
                    testSound:play()
                end)
                if soundSuccess then
                    print("Sound played successfully")
                else
                    print("Sound play error:", soundError)
                end
            end
        elseif button == 2 then -- B button
            print("Exit requested via gamepad")
            love.event.quit()
        elseif button == 3 then -- X button
            print("Reloading assets...")
            reloadAssets()
        elseif button == 4 then -- Y button
            print("=== SYSTEM INFO ===")
            print("OS:", love.system.getOS())
            print("FPS:", love.timer.getFPS())
            print("Joysticks:", love.joystick.getJoystickCount())
        elseif button == 5 then -- L shoulder
            print("L shoulder pressed - Previous test")
        elseif button == 6 then -- R shoulder
            print("R shoulder pressed - Next test")
        elseif button == 7 then -- ZL
            print("ZL pressed")
        elseif button == 8 then -- ZR
            print("ZR pressed")
        elseif button == 9 then -- + (Plus)
            print("Plus button pressed - Settings")
        elseif button == 10 then -- - (Minus)
            print("Minus button pressed - Menu")
        elseif button == 11 then -- Home
            print("Home button pressed")
        elseif button == 12 then -- Left stick click
            print("Left stick clicked")
        elseif button == 13 then -- Right stick click
            print("Right stick clicked")
        elseif button == 14 then -- D-pad Up
            print("D-pad Up")
        elseif button == 15 then -- D-pad Down
            print("D-pad Down")
        elseif button == 16 then -- D-pad Left
            print("D-pad Left")
        elseif button == 17 then -- D-pad Right
            print("D-pad Right")
        end
    end)
    
    if not success then
        print("Error in joystickpressed:", error)
    end
end

-- Helper function to reload assets
function reloadAssets()
    print("=== RELOADING ASSETS ===")
    
    -- Reset results
    testResults = {
        font = "Reloading...",
        image = "Reloading...", 
        sound = "Reloading...",
        shader = "Reloading..."
    }
    
    -- Reload font
    local fontSuccess, fontError = pcall(function()
        testFont = love.graphics.newFont(16)
        testResults.font = "SUCCESS: Font reloaded"
    end)
    if not fontSuccess then
        testResults.font = "ERROR: " .. tostring(fontError)
    end
    
    -- Reload image
    local imageSuccess, imageError = pcall(function()
        local imgData = love.image.newImageData(64, 64)
        for x = 0, 63 do
            for y = 0, 63 do
                local r = math.random()
                local g = math.random()
                local b = math.random()
                imgData:setPixel(x, y, r, g, b, 1)
            end
        end
        testImage = love.graphics.newImage(imgData)
        testResults.image = "SUCCESS: Random image created"
    end)
    if not imageSuccess then
        testResults.image = "ERROR: " .. tostring(imageError)
    end
    
    -- Reload sound
    local soundSuccess, soundError = pcall(function()
        local sampleRate = 22050
        local duration = 0.3
        local samples = math.floor(sampleRate * duration)
        local soundData = love.sound.newSoundData(samples, sampleRate, 16, 1)
        
        for i = 0, samples - 1 do
            local t = i / sampleRate
            local freq = 880 -- Higher pitch
            local sample = math.sin(freq * 2 * math.pi * t) * 0.3
            soundData:setSample(i, sample)
        end
        
        testSound = love.audio.newSource(soundData, "static")
        testResults.sound = "SUCCESS: 880Hz tone created"
    end)
    if not soundSuccess then
        testResults.sound = "ERROR: " .. tostring(soundError)
    end
    
    print("=== ASSET RELOAD COMPLETE ===")
end

function love.joystickreleased(joystick, button)
    print("Joystick button released - Joystick:", joystick:getName(), "Button:", button)
end

function love.gamepadpressed(joystick, button)
    print("Gamepad button pressed - Joystick:", joystick:getName(), "Button:", button)
    
    -- Handle gamepad buttons by name (if supported)
    if button == "a" then
        if testSound then
            testSound:play()
            print("Sound played via gamepad A")
        end
    elseif button == "b" then
        love.event.quit()
    elseif button == "x" then
        reloadAssets()
    elseif button == "y" then
        print("Gamepad Y pressed - System info")
    elseif button == "back" or button == "select" then
        print("Back/Select pressed")
    elseif button == "start" then
        print("Start pressed")
    elseif button == "leftstick" then
        print("Left stick pressed")
    elseif button == "rightstick" then
        print("Right stick pressed")
    elseif button == "leftshoulder" then
        print("Left shoulder pressed")
    elseif button == "rightshoulder" then
        print("Right shoulder pressed")
    elseif button == "dpup" then
        print("D-pad up")
    elseif button == "dpdown" then
        print("D-pad down")
    elseif button == "dpleft" then
        print("D-pad left")
    elseif button == "dpright" then
        print("D-pad right")
    end
end

function love.gamepadreleased(joystick, button)
    print("Gamepad button released - Joystick:", joystick:getName(), "Button:", button)
end

function love.gamepadaxis(joystick, axis, value)
    -- Only print significant axis changes to avoid spam
    if math.abs(value) > 0.2 then
        print("Gamepad axis - Joystick:", joystick:getName(), "Axis:", axis, "Value:", string.format("%.2f", value))
        
        -- Handle specific axes
        if axis == "leftx" or axis == "lefty" then
            -- Left stick movement
        elseif axis == "rightx" or axis == "righty" then
            -- Right stick movement  
        elseif axis == "triggerleft" then
            print("Left trigger:", string.format("%.2f", value))
        elseif axis == "triggerright" then
            print("Right trigger:", string.format("%.2f", value))
        end
    end
end

function love.quit()
    print("=== COMPREHENSIVE TEST GAME QUITTING ===")
    -- Clean up resources
    if testSound then
        testSound:stop()
    end
    return false -- Allow quit
end
