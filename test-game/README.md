# LÖVE Potion GLSL Shader Tests

Tato složka obsahuje testy pro nově implementovanou GLSL shader podporu v LÖVE Potion pro Wii U.

## Soubory

- `main.lua` - Kompletní test všech funkcí včetně shader testů (default)
- `main_comprehensive.lua` - Kompletní test všech funkcí včetně shader testů
- `shader_test.lua` - Specializovaný test pouze pro GLSL shadery
- `minimal_shader_test.lua` - Minimální shader test pro debugging
- `main_embedded.lua` - Embedded shader test (standalone)
- `main_shader.lua` - Spouštěč shader testu
- `main_minimal.lua` - Spouštěč minimálního shader testu
- `main_direct.lua` - Direct shader test

## Jak testovat

### Aktuální problémy a řešení

**Problem**: Při spuštění se zobrazuje pouze bílá obrazovka s ikonou
**Příčina**: Chyba při načítání externí modulů (require)
**Řešení**: Použít embedded testy

### Doporučené pořadí testování

```bash
# 1. ZÁKLADNÍ TEST - embedded shader test (doporučeno)
# Přejmenujte main_embedded.lua na main.lua a zkuste znovu

# 2. Pokud osnovní test funguje, zkuste komplexnější:
# Použijte původní main.lua (komplekní test)

# 3. Pro debugging:
# Přejmenujte main_direct.lua na main.lua
```

### Embedded shader test (doporučeno pro první test)

```bash
# Tento test má vše v jednom souboru bez externích závislostí
# main_embedded.lua -> main.lua
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
