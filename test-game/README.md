# LÖVE Potion GLSL Shader Tests

Tato složka obsahuje testy pro nově implementovanou GLSL shader podporu v LÖVE Potion pro Wii U.

## Soubory

- `main_comprehensive.lua` - Kompletní test všech funkcí včetně shader testů
- `shader_test.lua` - Specializovaný test pouze pro GLSL shadery
- `minimal_shader_test.lua` - Minimální shader test pro debugging
- `main_shader.lua` - Spouštěč shader testu
- `main_minimal.lua` - Spouštěč minimálního shader testu

## Jak testovat

### Kompletní test

```bash
# Použijte main_comprehensive.lua jako hlavní soubor
# Testuje fonty, obrázky, zvuky a nově GLSL shadery
```

### Pouze shader test

```bash
# Použijte main_shader.lua jako hlavní soubor
# Zaměřuje se pouze na GLSL shader funkcionalitet
```

### Minimální shader test (pro debugging)

```bash
# Použijte main_minimal.lua jako hlavní soubor
# Jednoduchý test pro debugging crash problémů
```

## Očekávané výsledky

### GLSL Shader Test Features:
- ✅ `love.graphics.newShader()` - Vytvoření shader z GLSL kódu
- ✅ `love.graphics.setShader()` - Nastavení aktivního shaderu
- ✅ `love.graphics.getShader()` - Získání aktuálního shaderu
- ✅ Vertex shader s animací
- ✅ Fragment shader s barevnými efekty
- ✅ Správné přepínání mezi shader a výchozím renderováním

### Ovládání:
- **A button** - Toggle shader on/off (v shader testu)
- **B button** - Exit
- **X button** - Reload assets (v comprehensive testu)

## Implementované shader funkce

### Vertex Shader:
- Animace pozice vrcholů pomocí sin funkce
- Podpora pro time uniform
- Standardní atributy (pozice, UV, barva)

### Fragment Shader:
- Animované barvy založené na UV souřadnicích
- Time-based animace
- Podpora pro textury a barvy

## Technické detaily

GLSL podpora byla implementována v:
- `wrap_Graphics.cpp` - Lua API binding
- `ShaderCompiler.cpp` - GLSL kompilace pipeline
- `ShaderStage.cpp` - Shader loading a detekce
- `Graphics.cpp` - Backend implementace

### Momentální stav:
- ✅ Lua API kompletní
- ✅ C++ backend připravený
- ⚠️ GLSL-to-GX2 kompilace (stubs, vyžaduje dokončení)
- ✅ Error handling a logging

## Debugging

Pokud shader selhává, zkontrolujte logy pro:
- Chyby kompilace GLSL
- Problémy s alokací paměti
- GX2 shader registrace

Logování je aktivní v `DebugLogger` a `ShaderCompiler`.
