# ROUND 12 — Juego completo en `src/Round12` + selector Labs/Round12 — Plan de implementación

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implementar ROUND 12 completo (demo de 3 niveles + resto de la historia del GDD) en `src/Round12`, con un selector en Debug y Release para cambiar entre `src/Juego` (labs) y `src/Round12` sin que se trabe.

**Architecture:** Un `Selector` (Proxy de `CE::GameLayer`) en `src/Main` contiene `IVJ::Juego` y `Round12::JuegoR12`, los inicializa en orden fijo (Juego primero) y reenvía eventos/update/render al juego activo; F1, un menú ImGui y una pantalla de lanzador cambian de juego. ROUND 12 vive en `namespace Round12`, reutiliza las clases base que el motor exige (`IVJ::Entidad`, `IVJ::FSM`, `IVJ::TileMap`, `IVJ::AnimBoxeo`, `IVJ::ColaGolpes`, `SistemaColAABBMid`) y tiene su propia lógica pura (sin SFML) en `src/Round12/Logica`, probada con un ejecutable de pruebas.

**Tech Stack:** C++17, SFML 3, ImGui 1.91 docking + ImGui-SFML, CMake ≥3.20 (Ninja multi-config en `build/`), Python 3 + Pillow (`uv run --with pillow`) para assets provisionales.

**Spec:** `/Users/ariel/Library/CloudStorage/OneDrive-Personal/Documentos/02 ESCOLARES/03 UNIVERSIDAD/UABC/07 Semestre/Videojuegos/Avance 4 GDD.docx` (texto extraído: `textutil -convert txt -stdout "<ruta>"`).

## Context

El GDD (Avance 4) define ROUND 12: boxeo 2D pixel art, 4 barras físicas (resistencia, aire, sed, concentración), 3 atributos (poder, velocidad, defensa), controles WASD/J/K/L/I/Shift/Q/E/Esc, demo de 3 niveles (Sanchez Boxing GYM, Playa de Rosarito, Mexicali vs Javier) y una historia con Santa Fe, Circuito, gimnasios visitantes, Kevin, semifinal imposible contra Ricardo, epílogo y revancha. Hoy solo existen escenas de laboratorio (`Escena_Round12`, `Escena_Round12Col`) dentro de `src/Juego`. Decisiones del usuario: alcance **demo + historia**, recursos **se leen desde `ROUND12_Assets`** (no se mueven), selector **lanzador + menú ImGui + F1**, arte/audio faltante **se genera provisional** con script.

## Global Constraints

- No borrar ni romper escenas de `src/Juego`; no modificar `IVJ::NodosBoxeo`, `Escena_Round12*`. Cambios en `src/Juego` = 0 líneas (salvo que una prueba lo exija y se justifique).
- Código del juego en `src/Round12/` con la misma estructura que `src/Juego/` (`Escenas/`, `Componentes/`, `Sistemas/`, `Maquinas/`, `objetos/`) + `Logica/`, `Datos/`, `pruebas/`. Namespace `Round12` (no `R12`: choca con la macro `#define R12` de `Escena_Round12*.cpp`).
- Claves de escena con prefijo `R12_`; claves de `GestorAssets` con prefijo `r12g_` (Juego ya usa `r12_` y `r12a_`).
- Recursos: `#define R12_RECURSOS ASSETS "/../ROUND12_Assets"` en `src/Round12/Rutas.hpp`; generados en `ROUND12_Assets/provisionales/` y `ROUND12_Assets/motor/`.
- Debug compila con `-Wall -Wextra -Werror`; ambas configuraciones deben compilar sin advertencias.
- Texturas de ROUND 12: `setSmooth(false)` tras cargar (pixel art; `GestorAssets` pone `true`).
- Vista de juego: mundo nativo en píxeles de 8×8 tiles, vista `288×192` (3:2, zoom 3.75 sobre 1080×720), igual que `Escena_Round12Col`.
- Textos en español con acentos: construir con `sf::String::fromUtf8`.
- Controles (GDD): W/A/S/D moverse, J jab(1), K recto(2), L gancho(3), I uppercut(4), Shift guardia, Q/E esquiva, Esc pausa. F1 = lanzador (reservada al selector).
- Escala de estadísticas 1–10. Tabla GDD (Diego, Brandon, Javier, Kevin, Ricardo): Resistencia 6,7,7,5,9 · Aire 7,5,5,8,9 · Sed 6,6,5,7,8 · Concentración 7,5,4,7,9 · Velocidad 7,5,4,9,8 · Defensa 5,6,5,6,9. **Poder** (no está en la tabla; supuesto): 8,5,9,5,9. Niño del barrio: todo 3.
- Patrones (CLAUDE.md): Observador, Decorador, Fábrica, Singleton, Comando, Adaptador, Fachada, Método plantilla, Iterador, Composición, Estado, Proxy — cada uno con su lugar en la tabla de abajo; no agregar ninguno fuera de ahí.
- Git: rama nueva `feat/round12-juego-completo` desde la actual; sin worktrees; commits Conventional (`feat(round12) : ...`) **sin** líneas `Co-Authored-By` ni menciones a la IA.

## Patrones de diseño (dónde vive cada uno)

| Patrón | Clase |
|---|---|
| Proxy | `Selector` (src/Main) delante de `IVJ::Juego` y `Round12::JuegoR12` |
| Singleton | `Round12::Partida` (progreso), además de los gestores CE existentes |
| Observador | `Round12::Eventos` (bus): HUD, audio y público se suscriben a golpe/caída/campana |
| Decorador | cadena `ModDanio` (poco aire → bloqueo → combo → contragolpe → Frontera) |
| Fábrica | `FabricaR12::boxeador/item/obstaculo` |
| Comando | teclas → bits `BotonBox` en `CE::IControl`; opciones de menús/pausa como `std::function<void()>` |
| Adaptador | `CerebroRival` (IA → `CE::IControl`) y `MapaColision` (formato TXT de ROUND12_Assets → rectángulos) |
| Fachada | `Round12::Combate` (stats, golpes, jueces, conteo) usada por `Escena_Ring` |
| Método plantilla | `EscenaR12` (base de todas las escenas R12) |
| Iterador | `Historia` (pasos), `Dialogo` (líneas de cinemática) |
| Composición | `Hud` (árbol de `Widget`: `Barra`, `Etiqueta`, `Panel`) |
| Estado | nodos FSM `NodosR12` y fases de pelea `FasePelea` |

## Estructura de archivos

```
src/Main/Selector.hpp/.cpp          Proxy GameLayer, lanzador ImGui, F1, --prueba-selector
src/Main/main.cpp                   (mod) argumentos --juego= y --prueba-selector=
src/Motor/GUI/GDock.cpp             (mod) usar WorkPos/WorkSize (deja lugar a la barra de menú)
src/Motor/Primitivos/GestorAssets.* (mod) un sf::SoundBuffer por clave (bug: hoy se comparte uno)
src/CMakeLists.txt                  (mod) add_subdirectory(Round12)
src/Round12/
  CMakeLists.txt, Rutas.hpp, JuegoR12.hpp/.cpp   GameLayer: registra escenas, pausa (Esc), clamp de dt
  Logica/      Stats, Combate, ModDanio, Combos, Conteo, Jueces, Runner, MapaColision, Historia, Partida, Eventos   (sin SFML)
  Componentes/ ComponentesR12.hpp/.cpp   IBoxeador, IZona, IItem, IObstaculo
  Sistemas/    SistemasR12.hpp/.cpp      alcance de golpe, recoger ítems, zonas
  Maquinas/    NodosR12.hpp/.cpp, CerebroRival.hpp/.cpp, FasesPelea.hpp/.cpp
  objetos/     FabricaR12, Hud, Audio, Vineta
  Escenas/     EscenaR12 (base), Escena_Titulo, Escena_Cinematica, Escena_Resultados,
               Escena_Recorrido (gym/pasillo), Escena_Ring, Escena_Runner (playa/circuito), Escena_SantaFe
  Datos/       Perfiles.hpp (tabla GDD), Niveles.hpp (configs), dialogos/*.txt
  pruebas/     pruebas_logica.cpp      ejecutable Round12Pruebas (ctest)
tools/round12_provisionales.py     sprites/parallax/audio provisionales
tools/round12_assets.py            (mod) convierte los 6 tilemaps al formato del motor
```

## Review Focus

1. **F1 con una tecla sostenida o música sonando** → al volver a Labs no debe seguir la música de R12 ni Diego caminar solo. Prueba: `--prueba-selector` verifica `Audio::sonando()==false` y `IControl` en cero tras cada cambio (Task 1/6).
2. **Orden de cámaras**: `Escena_Camara` de Labs usa índices fijos 1–4; si R12 agregara cámaras antes, se rompen. Prueba: `--prueba-selector` revisa que las cámaras 1–4 sean las de `IVJ::Juego` (`getListaCamaras().size()>=5` justo después de `IVJ::Juego::OnInit` y antes de cualquier escena R12) (Task 1).
3. **Repetición de tecla del SO** (`setKeyRepeatEnabled` está activo): sostener J no debe contar como machacar en el conteo ni disparar combos. Prueba: `Conteo` con J,J,J,J = 1 pulsación válida (Task 4).
4. **Semifinal contra Ricardo (derrota obligatoria)** no debe reiniciar la pelea en bucle; la historia avanza. Prueba: `Partida::alTerminarPelea(PERDER_FORZADO, derrota)` avanza al siguiente paso (Task 5).
5. **dt enorme** (primer frame del motor = 0.166 s, o tras cambiar de juego) → los temporizadores saltan y el corredor atraviesa obstáculos. `JuegoR12::OnUpdate` limita `dt` a `1/30`. Prueba: `Runner` con dt 1/30 detecta un obstáculo de 8 px a velocidad máxima (Task 5).

---

### Task 0: Rama, copia del plan y esqueleto de `src/Round12`

**Files:**
- Create: `docs/superpowers/plans/2026-10-31-round12-juego-completo.md` (copia de este plan), `src/Round12/CMakeLists.txt`, `src/Round12/Rutas.hpp`, `src/Round12/JuegoR12.hpp/.cpp`, `src/Round12/Escenas/CMakeLists.txt`, `src/Round12/Escenas/Escena_Titulo.hpp/.cpp` (versión mínima: texto "ROUND 12")
- Modify: `src/CMakeLists.txt` (`add_subdirectory(Round12)`)

**Interfaces:**
- Produces: `class Round12::JuegoR12 : public CE::GameLayer` con `OnInit()` (registra escenas `R12_*`, no cambia de escena), `void activar()` (cambia a `R12_Titulo` o a la última escena R12), `void desactivar()` (para audio, suelta controles), `bool pideLanzador()` / `void limpiarPeticion()`. Mismo despacho de `InputBuffer` → `Escena::onInputs` que `IVJ::Juego::OnInputs(CE::InputBuffer&)`.

- [ ] **Step 1:** `git checkout -b feat/round12-juego-completo` (el `.gitignore` modificado se queda sin commitear salvo que el usuario diga). Copiar el plan a `docs/superpowers/plans/`.
- [ ] **Step 2:** Crear el esqueleto; cada subcarpeta con `target_sources(CimaEngine3v PRIVATE ...)` como `src/Juego/*/CMakeLists.txt`.
- [ ] **Step 3:** Compilar ambas: `cmake --build build --config Debug && cmake --build build --config Release` → sin errores ni warnings. (Aún no se usa `JuegoR12` en `main`.)
- [ ] **Step 4:** Commit `chore(round12) : esqueleto de src/Round12`.

### Task 1: Selector Labs/Round12 + pruebas de compatibilidad

**Files:**
- Create: `src/Main/Selector.hpp/.cpp` (agregar a `src/Main/CMakeLists.txt` con `target_sources`)
- Modify: `src/Main/main.cpp`, `src/Motor/GUI/GDock.cpp`, `src/Motor/Primitivos/GestorAssets.hpp/.cpp`

**Interfaces:**
- Consumes: `IVJ::Juego`, `Round12::JuegoR12` (Task 0).
- Produces:
  - `enum class Modo { LANZADOR, LABS, ROUND12 };`
  - `class Selector : public CE::GameLayer { explicit Selector(Modo inicial, int ciclos_prueba = 0); void cambiar(Modo m); int fallas() const; }`
  - `GestorAssets::agregarSonido` guarda un `std::shared_ptr<sf::SoundBuffer>` por clave en `hashBuffers`.

Comportamiento:
- `OnInit`: `labs->OnInit()` y **después** `r12->OnInit()` (orden fijo, Review Focus 2); luego `cambiar(inicial)`.
- `cambiar`: guarda `GestorEscenas::getEscenaActual().nombre` del juego que sale; si sale R12 → `r12->desactivar()`; al entrar a LABS hace `cambiarEscena(guardada o "Init")`; a ROUND12 `r12->activar()`. LANZADOR no actualiza ningún juego.
- `OnInputs(InputBuffer&)`: saca cada evento; `Closed` → `termino=true`; `KeyPressed F1` → LANZADOR (o regresa al juego previo si ya está en él); en LANZADOR teclas `1`/`2` eligen; el resto se copia a un `InputBuffer filtrado{32}` que se pasa al juego activo. Eventos posteriores a un cambio en el mismo frame se descartan.
- `OnUpdate/OnRender`: delega; si `r12->pideLanzador()` → `cambiar(LANZADOR)`. `EstaCorriendo()` = `!termino && activo->EstaCorriendo()`.
- ImGui (Debug **y** Release): `BeginMainMenuBar` con menú "Juego" → "Lanzador (F1)", "CimaEngine Labs", "ROUND 12"; en LANZADOR una ventana centrada "Elige juego" con dos botones grandes + texto "1 / 2".
- `GDock`: `viewport->WorkPos`/`WorkSize` en lugar de `Pos`/`Size`.
- `main.cpp`: `--juego=labs|round12` (sin argumento → LANZADOR), `--prueba-selector=N`. Guardar `Selector*` antes de moverlo al `Motor`; `int r = motor.OnRun(); return sel->fallas() ? 1 : r;`.
- `--prueba-selector=N`: alterna LABS↔ROUND12 cada 30 frames N veces y dentro de R12 recorre todas las escenas `R12_*` registradas (una por ciclo); al terminar `termino=true`. Cada cambio verifica y cuenta en `fallas` (imprime `[prueba-selector] FALLA: <motivo>`):
  - la escena actual empieza con `R12_` ⇔ modo ROUND12;
  - `Round12::Audio::sonando()==false` en LABS (esta verificación se agrega en Task 6, cuando existe `Audio`);
  - cámaras 1–4 siguen siendo las creadas por `IVJ::Juego` (guardar sus `shared_ptr` tras `labs->OnInit()` y comparar);
  - `GestorEscenas::getKeys().size()` constante.

- [ ] **Step 1:** Implementar `Selector`, cambios de `main`, `GDock`, `GestorAssets`.
- [ ] **Step 2:** Debug: `cmake --build build --config Debug && ./bin/Debug/CimaEngine3v --prueba-selector=20; echo $?` → `0` y ninguna línea `FALLA`.
- [ ] **Step 3:** Release: mismo comando con `Release` → `0`.
- [ ] **Step 4:** Manual: arrancar sin argumentos → lanzador; `1` → Labs, recorrer sus escenas como antes; F1 → `2` → título R12; menú "Juego" visible en ambas configuraciones.
- [ ] **Step 5:** Commit `feat(selector) : selector entre CimaEngine Labs y ROUND 12`.

### Task 2: Assets provisionales y tilemaps convertidos

**Files:**
- Create: `tools/round12_provisionales.py`
- Modify: `tools/round12_assets.py` (convertir los 6 tilemaps de `ROUND12_Assets/tilemaps` + `tilesets` al formato `[Atlas_info]…[ID_Matrix]` del motor, todos los tiles `'n'`; la colisión la lee `MapaColision`, Task 5)

Salida (nombres exactos; el código de tasks siguientes los usa):
- `ROUND12_Assets/motor/0{1..6}_<escenario>.txt` (mismos nombres base que `tilemaps/`).
- `ROUND12_Assets/provisionales/sprites/`: `botella.png, isotonica.png, platano.png, toalla.png` (16×16); `costal.png` (16×40, 3 cuadros de vaivén en fila), `manoplas.png` (16×16), `cuerda.png` (32×16, 4 cuadros), `garrafon.png` (16×24); `castillo.png, hielera.png, sombrilla.png, pelota.png` (16×16), `perro.png, cangrejo.png` (16×16, 2 cuadros), `ola.png` (64×16), `sombra.png` (48×8, alfa 50%), `puesto_agua.png` (24×32), `muelle.png` (64×48); `iconos_golpes.png` (4×16×16: 1,2,3,4); `vineta.png` (288×192, centro transparente → bordes negros alfa 255); `publico.png` (288×32, 2 cuadros).
- `ROUND12_Assets/provisionales/parallax/`: `cielo.png, mar.png, cerros.png, palmeras.png` (288×192, atardecer naranja/morado; capas con transparencia salvo cielo).
- `ROUND12_Assets/provisionales/audio/` (WAV mono 22050 Hz, onda cuadrada, solo stdlib `wave`): `musica_menu.wav` (100 BPM), `musica_gym.wav` (100 BPM), `musica_cuerda.wav` (120 BPM exactos, pulso cada 0.5 s desde t=0), `musica_playa.wav` (130 BPM), `musica_pelea.wav` (140 BPM), 16–32 s cada una en loop; efectos `campana.wav, golpe_suave.wav, golpe_fuerte.wav, bloqueo.wav, caida.wav, publico.wav, beat.wav, menu.wav`.

- [ ] **Step 1:** Escribir el script; al final un `assert` por archivo (existe y tamaño esperado con Pillow / `wave`).
- [ ] **Step 2:** `uv run --with pillow python tools/round12_provisionales.py && uv run --with pillow python tools/round12_assets.py` → termina sin `AssertionError`; `ls ROUND12_Assets/motor | wc -l` = 6.
- [ ] **Step 3:** Commit `feat(round12) : assets provisionales y tilemaps convertidos`.

### Task 3: Lógica — estadísticas y daño (Decorador)

**Files:**
- Create: `src/Round12/Logica/Stats.hpp/.cpp`, `src/Round12/Logica/ModDanio.hpp/.cpp`, `src/Round12/Datos/Perfiles.hpp`, `src/Round12/Logica/CMakeLists.txt`, `src/Round12/pruebas/pruebas_logica.cpp`
- Modify: `CMakeLists.txt` raíz (`enable_testing()` antes de `add_subdirectory(src)`)

**Interfaces:**
- Produces:
  - `struct Atributos { int resistencia, aire, sed, concentracion, velocidad, defensa, poder; };` (1–10)
  - `enum class Golpe { JAB=0, RECTO, GANCHO, UPPERCUT };`
  - `struct Barras { float resistencia, resistencia_max, aire=100, sed=0, concentracion; };`
  - `Barras barrasIniciales(const Atributos&)` → `resistencia_max = 40 + 8*resistencia`, `concentracion = 50 + 5*concentracion`.
  - `float costoAire(Golpe, const Atributos&)` = base{5,7,11,12} × `(1.3 - 0.05*aire)`; `float costoEsquiva(const Atributos&)` = 6 × mismo factor.
  - `float regenAire(const Barras&, const Atributos&, bool guardia, bool moviendo)` por segundo = `(8 + 1.2*aire)` × (guardia 1.5 / quieto 1 / moviendo 0.7) × (sed>70 ? 0.5 : 1).
  - `float factorSed(const Atributos&)` = `1.5 - 0.1*sed`; sed sube `0.5*factor`/s y `0.6*factor` por golpe lanzado (tope 100).
  - `float escalaAnimacion(const Barras&, const Atributos&)` = `(1.5 - 0.05*velocidad) * (aire<25 ? 1.3 : 1)`; `float velocidadPasos(const Atributos&)` = `30 + 3*velocidad` px/s.
  - `struct ContextoGolpe { Golpe g; const Atributos& atk; const Barras& b_atk; const Atributos& def; bool bloqueado, cierra_combo, contragolpe, frontera; };`
  - Decorador: `class ModDanio { virtual float calcular(const ContextoGolpe&) const = 0; }`, `DanioBase` (base{3,5,8,9} × `(0.5+0.1*poder)`), `ModPocoAire` (aire<25 → ×0.5), `ModBloqueo` (×`(1-0.07*defensa)`), `ModCombo` (×1.25), `ModContragolpe` (×2), `ModFrontera` (×2). `std::unique_ptr<ModDanio> cadenaDanio()` arma la cadena en ese orden.
  - `Perfiles.hpp`: `enum class Personaje { DIEGO, BRANDON, JAVIER, KEVIN, RICARDO, NINO };` `Atributos perfil(Personaje)` con la tabla de Global Constraints; `const char* hoja(Personaje)` → `"diego_ureta"`, … (`NINO` usa `"diego_ureta"` con tinte).

- [ ] **Step 1: pruebas que fallan** en `pruebas_logica.cpp` (framework: `assert`-like propio `CHECK(cond)` que imprime archivo:línea y cuenta fallas; `main` regresa ≠0 si hubo):
  - `barrasIniciales(perfil(DIEGO)).resistencia_max == 88`
  - jab de Diego sin modificadores = `3*(0.5+0.8)` = `3.9`
  - gancho de Diego bloqueado por Ricardo (def 9) = `8*1.3*(1-0.63)` ≈ `3.848` (±0.01)
  - contragolpe duplica: `jab contragolpe == 2*jab`
  - con aire 20 el jab = `1.95`
  - `regenAire` con sed 80 es la mitad que con sed 0
- [ ] **Step 2:** `cmake --build build --config Debug --target Round12Pruebas && ctest --test-dir build -C Debug --output-on-failure` → FAIL (no compila / falla).
- [ ] **Step 3:** Implementar. En `Logica/CMakeLists.txt`: `set(R12_LOGICA ...)`, `target_sources(CimaEngine3v PRIVATE ${R12_LOGICA})`, `add_executable(Round12Pruebas ${R12_LOGICA} ../pruebas/pruebas_logica.cpp)`, include `${CMAKE_SOURCE_DIR}/src`, `-Wall -Wextra -Werror`, `add_test(NAME round12_logica COMMAND Round12Pruebas)`.
- [ ] **Step 4:** Mismo comando → PASS.
- [ ] **Step 5:** Commit `feat(round12) : estadísticas y cálculo de daño del GDD`.

### Task 4: Lógica — combos, conteo y jueces

**Files:** Create `src/Round12/Logica/Combos.hpp/.cpp`, `Conteo.hpp/.cpp`, `Jueces.hpp/.cpp`; Test: `pruebas_logica.cpp`

**Interfaces:**
- Produces:
  - `class DetectorCombos { void reiniciar(); Combo registrar(Golpe g, float t); }` con `enum class Combo { NINGUNO, C112, C123, C34, FRONTERA };` ventana entre golpes `0.45 s`; reconoce sufijos `1-1-2`, `1-2-3`, `3-4`, `1-2-3-4` (FRONTERA tiene prioridad). FRONTERA solo cuenta si `concentracion>=100` (parámetro de `registrar` — firma: `Combo registrar(Golpe g, float t, bool concentracion_maxima)`).
  - `class Conteo { explicit Conteo(int caidas_previas); void tick(float dt); void pulsar(char tecla /* 'J' o 'K' */); bool levantado() const; bool fuera() const; int numero() const; }` — cuenta 1..10, 1 s por número; necesita `10 + 6*caidas_previas` pulsaciones **alternadas** (misma tecla seguida no cuenta); `fuera()` al llegar a 10 sin levantarse.
  - `float probLevantarRival(const Atributos&, int caidas_previas)` = `clamp(resistencia/12.f - 0.2f*caidas_previas, 0, 0.95)`.
  - `struct RoundStats { int conectados_diego, conectados_rival; float danio_diego, danio_rival; int caidas_diego, caidas_rival; };`
  - `enum class Veredicto { DIEGO_UNANIME, DIEGO_DIVIDIDA, RIVAL_UNANIME, RIVAL_DIVIDIDA, EMPATE };`
  - `Veredicto decidir(const std::vector<RoundStats>&, std::mt19937& rng)` — 3 jueces; por round cada juez da puntos `conectados + danio/10 + 3*caidas_provocadas` + ruido uniforme ±1; ganador del round 10, perdedor 9 (8 si cayó en ese round); suma por juez.
- [ ] **Step 1: pruebas que fallan:** `1,1,2` → `C112`; `1,2,3,4` con concentración máxima → `FRONTERA`, sin ella → `NINGUNO` en el 4; golpe tras 0.5 s reinicia; `Conteo(0)` con `J,J,J,J` → 1 pulsación válida (Review Focus 3); `J,K`×5 antes de 10 s → `levantado()`; 10 s sin pulsar → `fuera()`; `decidir` con 3 rounds claramente de Diego y rng semilla 1 → `DIEGO_UNANIME`; rounds iguales → `EMPATE` o dividida (determinista con semilla).
- [ ] **Step 2:** ctest → FAIL. **Step 3:** implementar. **Step 4:** ctest → PASS.
- [ ] **Step 5:** Commit `feat(round12) : combinaciones, conteo y decisión de jueces`.

### Task 5: Lógica — progreso, historia, eventos, corredor y mapas de colisión

**Files:** Create `src/Round12/Logica/Eventos.hpp`, `Historia.hpp/.cpp`, `Partida.hpp/.cpp`, `Runner.hpp/.cpp`, `MapaColision.hpp/.cpp`; Test: `pruebas_logica.cpp`

**Interfaces:**
- Produces:
  - Observador: `enum class Evento { GOLPE_CONECTADO, GOLPE_FUERTE, BLOQUEO, CAIDA, CAMPANA, ITEM, NIVEL_COMPLETO };` `class Eventos { static Eventos& Get(); int suscribir(std::function<void(Evento,float)>); void desuscribir(int); void emitir(Evento, float valor=0); }`.
  - `enum class Habilidad : uint8_t { COMBOS=1, ESQUIVA=2, CONTRAGOLPE=4, SEGUNDO_AIRE=8, FRONTERA=16 };`
  - `enum class TipoPaso { CINEMATICA, RECORRIDO, PELEA, RUNNER, SANTA_FE, RESULTADOS, FIN };` `enum class Objetivo { SOBREVIVIR, GANAR, PERDER_FORZADO };` `struct Paso { TipoPaso tipo; std::string id; };` (el `id` indexa `Datos/Niveles.hpp`, Task 12).
  - Iterador: `class Historia { explicit Historia(std::vector<Paso>); const Paso& actual() const; bool avanzar(); void reiniciarPaso(); bool fin() const; }`.
  - Singleton: `class Partida { static Partida& Get(); void nueva(bool demo); Atributos& diego(); Atributos antes_del_nivel; bool tiene(Habilidad) const; void desbloquear(Habilidad); void mejorar(int Atributos::*campo, int delta /* tope 10 */); Historia& historia(); void alTerminarPelea(Objetivo, bool gano, bool de_pie); void alTerminarNivel(); }` — `alTerminarPelea`: SOBREVIVIR avanza si `de_pie`; GANAR avanza si `gano`; PERDER_FORZADO avanza siempre; si no avanza → `reiniciarPaso()`. `alTerminarNivel` copia `diego()` a `antes_del_nivel` tras mostrar resultados.
  - `class Runner { struct Config { float meta, tiempo, x_checkpoints[3]; bool olas; }; void paso(float dt, int ritmo /* -1 frena, 0, +1 acelera */, bool saltar, bool agachar); bool chocaCon(float x, float ancho, float alto) ...; float x, y, vel, aire, sed, tiempo; }` — velocidades 40/60/85 px/s; aire −6/s a 85, +1/s a 60, +4/s a 40 (sombra +8/s); sed +0.8/s, sed>70 regen ×0.5; salto `vy=-160`, `g=420`; agachado mitad de alto; choque: vel 30 por 1 s y aire −10; ola: vel ×0.5 por 1.5 s, aire −8; arena mojada ×0.8. Colisión barrida (de `x_prev` a `x`) para no atravesar con dt grande.
  - Adaptador: `std::vector<Rect> MapaColision::cargar(const std::string& ruta, int valor, int tile=8)` lee `ROUND12_Assets/colisiones/*.txt` (`cols filas` + matriz) y une corridas horizontales del mismo `valor` en un rectángulo. `struct Rect { float x,y,w,h; };`
- [ ] **Step 1: pruebas que fallan:** `alTerminarPelea(PERDER_FORZADO,false,false)` avanza (Review Focus 4); `GANAR` perdido → mismo paso; `mejorar` no pasa de 10; `Runner` a velocidad máxima con dt=1/30 choca con obstáculo de 8 px (Review Focus 5); aire llega a 0 tras ~17 s a 85 px/s con aire inicial 100; `MapaColision` sobre un archivo temporal `3 2 / 1 1 0 / 0 1 1` → 2 rectángulos `{0,0,16,8}` y `{8,8,16,8}`; `Eventos` notifica a 2 suscriptores y no al desuscrito.
- [ ] **Step 2:** ctest → FAIL. **Step 3:** implementar. **Step 4:** ctest → PASS.
- [ ] **Step 5:** Commit `feat(round12) : progreso, historia, eventos, corredor y colisiones`.

### Task 6: Base de escenas, HUD, audio, pausa, título, cinemática y resultados

**Files:**
- Create: `src/Round12/Escenas/EscenaR12.hpp/.cpp`, `Escena_Cinematica.*`, `Escena_Resultados.*`; `src/Round12/objetos/Hud.*`, `Audio.*`, `Vineta.*`, `FabricaR12.*`; `src/Round12/Componentes/ComponentesR12.*`; `src/Round12/Datos/dialogos/` (archivo de prueba `prueba.txt`)
- Modify: `Escena_Titulo.*`, `JuegoR12.*`

**Interfaces:**
- Consumes: `Partida`, `Eventos`, `Historia` (Task 5).
- Produces:
  - Método plantilla: `class EscenaR12 : public CE::Escena` con `onInit() final` → (primera vez) registra teclas del GDD como `Escena_Round12Col::onInit` + `cargar()` + `crear()`; siempre `reiniciar()`, pone su cámara y `Audio::musica(musica())`. `onUpdate(dt) final` → `actualizar(dt)` + `hud.actualizar(dt)`. `onRender() final` → `dibujarMundo()` y luego HUD en espacio de pantalla (guardar `view`, `setView(getDefaultView())` de `Render::Get().GetTextura()`, dibujar, restaurar). `onFinal() final` → `Audio::detener()`, `soltarControles()`. Ganchos virtuales: `cargar()`, `crear()`, `reiniciar()`, `actualizar(float)`, `dibujarMundo()`, `const char* musica()`, `void alPresionar(const std::string& accion, bool presionado)`. Utilidades: `int crearCamara(CE::Vector2D vista)`, `CE::IControl control` del jugador.
  - `JuegoR12`: Esc alterna pausa (no llama `onUpdate` de la escena); menú de pausa (Comando) "Continuar / Reiniciar / Menú principal" con W/S + J/Enter; `dt = std::min(dt, 1.f/30.f)` (Review Focus 5); método `irAPaso()` que traduce `Partida::historia().actual()` a `GestorEscenas::cambiarEscena("R12_<Tipo>")` tras configurar la escena con su `id`.
  - Composición: `class Widget { virtual void actualizar(float); virtual void dibujar(sf::RenderTarget&, sf::Vector2f tam); std::vector<std::unique_ptr<Widget>> hijos; }`, `Barra(etiqueta, const float* valor, const float* maximo, sf::Color)`, `Etiqueta(std::function<std::string()>)`, `Panel`. Colores: resistencia rojo, aire azul cielo, sed naranja, concentración amarillo.
  - `namespace Audio { void musica(const char* clave); void efecto(const char* clave); void detener(); bool sonando(); }` — carga perezosa desde `R12_RECURSOS "/provisionales/audio/<clave>.wav"`; si el archivo no existe imprime una vez `[audio] falta <ruta>` y no hace nada (nunca `getMusica` sobre clave inexistente). Suscrito a `Eventos` para `GOLPE_FUERTE`→`golpe_fuerte`+`publico`, `CAMPANA`, `CAIDA`, `BLOQUEO`.
  - `Vineta::dibujar(target, concentracion)`: `vineta.png` a pantalla completa con alfa `= concentracion<40 ? (40-concentracion)/40*255 : 0`.
  - Fábrica: `std::shared_ptr<IVJ::Entidad> FabricaR12::boxeador(Personaje, float x, float pies_y, int dir, sf::Color tinte = sf::Color::White)` (misma construcción que `crearBoxeador` de `Escena_Round12Col.cpp`: hoja 80×80, origen (40,44), `IBoundingBox{14,52}`, `IDireccion`, `IMaquinaEstado`, `IControl`, `IBoxeador`); `item(TipoItem, x, y)`, `obstaculo(TipoObstaculo, x, y)`.
  - `class IBoxeador : public CE::IComponentes { Personaje quien; Atributos atr; Barras barras; bool segundo_aire_usado; float ventana_contragolpe; Golpe ultimo; bool golpe_activo; }`.
  - `Escena_Titulo` (`R12_Titulo`): "ROUND 12", opciones Demo / Historia / Controles / Volver al lanzador (pide `JuegoR12::pideLanzador`); Demo/Historia → `Partida::nueva(demo)` → `irAPaso()`.
  - `Escena_Cinematica` (`R12_Cinematica`): lee `Datos/dialogos/<id>.txt`, líneas `orador|pose|fondo|texto` (`#` comentario); Iterador `Dialogo` sobre líneas; máquina de escribir 40 car/s; J completa/avanza; Esc salta todo; fondo de `escenarios_1080x720/<fondo>.png` o negro (`-`); sprite del orador (`carlos` usa `sprites/personajes/carlos_sanchez/spritesheet_npc.png` 44×60, pose 0–4; boxeadores su `spritesheet_4poses.png`). Al terminar → `Partida::historia().avanzar()` + `irAPaso()`.
  - `Escena_Resultados` (`R12_Resultados`): tabla de los 7 atributos `antes → ahora` (+n en verde), habilidades nuevas, tiempo/veredicto del nivel; J continúa → `alTerminarNivel()`.
- [ ] **Step 1:** Implementar. Datos de prueba: `dialogos/prueba.txt` con 2 líneas con acentos.
- [ ] **Step 2:** Debug/Release compilan; `--prueba-selector=10` → 0 (ahora recorre también Cinemática y Resultados).
- [ ] **Step 3:** Manual: título navega; Esc pausa y congela; F1 en pausa no deja la música sonando; acentos se ven bien.
- [ ] **Step 4:** Commit `feat(round12) : escenas base, HUD, audio, pausa, cinemáticas y resultados`.

### Task 7: Máquina de estados del boxeador e IA de rivales

**Files:** Create `src/Round12/Maquinas/NodosR12.hpp/.cpp`, `CerebroRival.hpp/.cpp`, `CMakeLists.txt`

**Interfaces:**
- Consumes: `IVJ::FSM`, `IVJ::AnimBoxeo`, `IVJ::AccionBox`, `IVJ::BotonBox`, `IVJ::ColaGolpes` (de `src/Juego/Maquinas/Boxeadores/NodosBoxeo.hpp`, solo lectura); `IBoxeador`, `Stats` (Tasks 3, 6).
- Produces (todos `: public IVJ::FSM`, `onInputs(const CE::IControl&)`; jugador **y** rivales usan los mismos nodos):
  - `IdleR12, CaminarR12, GuardiaR12, PrepararR12(Golpe, float t)` (wind-up; jugador t=0, rival `0.5 - 0.02*velocidad` y el sprite parpadea blanco si la concentración del **jugador** ≥70 — señal del GDD), `GolpeR12(Golpe)` (marca `golpe_activo` en el cuadro 3 de 6; cobra aire al entrar; si no alcanza el aire, no golpea), `EsquivaR12(bool izq)` (requiere `Habilidad::ESQUIVA` en Diego; cobra aire), `AturdidoR12` (0.25 s, sprite tintado rojo), `CaidoR12` (sprite rotado 90° sobre la lona; sale solo por la escena), `LevantarseR12` (0.6 s).
  - Animación: `anim.actualizar(dt / escalaAnimacion(...))` usando `IBoxeador`.
  - Adaptador: `struct EstiloIA { float prob_guardia, prob_esquiva, distancia_ideal, agresividad; std::array<float,4> pesos_golpe; };` `EstiloIA estilo(Personaje)` — Brandon {0.6, 0.1, 32, 0.4, {0.7,0.2,0.05,0.05}}, Javier {0.1, 0.05, 20, 0.9, {0.15,0.15,0.5,0.2}}, Kevin {0.2, 0.35, 36, 0.5, {0.4,0.4,0.1,0.1}}, Ricardo {0.35, 0.4, 28, 0.7, {0.4,0.25,0.2,0.15}}, Niño {0.2, 0.05, 26, 0.4, {0.4,0.3,0.2,0.1}}. `class CerebroRival { CerebroRival(EstiloIA, std::mt19937&); void pensar(float dt, const IVJ::Entidad& yo, const IVJ::Entidad& diego, CE::IControl& salida); }` decide cada `0.25 s`: acercarse/alejarse a `distancia_ideal`, guardia, esquivar cuando Diego está en `PrepararR12/GolpeR12`, o golpe ponderado por `pesos_golpe`. Javier con aire<30 baja `agresividad` a 0.3 (se cansa).
- [ ] **Step 1:** Prueba en `pruebas_logica.cpp` solo de la parte pura: extraer `int elegirGolpe(const std::array<float,4>&, float u)` a `Logica/` y probar `elegirGolpe({0.7,0.2,0.05,0.05}, 0.75)==RECTO`.
- [ ] **Step 2:** ctest FAIL → implementar → PASS.
- [ ] **Step 3:** Commit `feat(round12) : estados del boxeador e IA por estilo de rival`.

### Task 8: Escena del ring (Fachada + fases)

**Files:** Create `src/Round12/Logica/Combate.hpp/.cpp` (fachada pura), `src/Round12/Maquinas/FasesPelea.hpp/.cpp`, `src/Round12/Escenas/Escena_Ring.hpp/.cpp`, `src/Round12/Sistemas/SistemasR12.hpp/.cpp`, `src/Round12/Datos/Niveles.hpp` (configs de pelea)

**Interfaces:**
- Produces:
  - `struct ConfigPelea { Personaje rival; int rounds; float segundos; Objetivo objetivo; std::string escenario; bool jueces; int dificultad; sf::Color tinte; };` en `Niveles.hpp`: `const ConfigPelea& pelea(const std::string& id)`; escenarios `"gym"` (tilemap motor 01, ring igual que `Escena_Round12Col`: Diego x=120, rival x=330, pies 154), `"visitante"` (02), `"mexicali"` (03); para 02/03 los límites del ring se miden de su archivo de colisiones y se fijan como constantes `RingRect{x0,x1,y0,y1}` en `Niveles.hpp`.
  - Fachada `class Combate { Combate(Atributos diego, Atributos rival, ConfigPelea); void lanzar(bool de_diego, Golpe, bool cierra_combo, bool contragolpe, bool frontera); void recibir(bool a_diego, Golpe, bool bloqueado); void tick(float dt, EstadoFisico diego, EstadoFisico rival); Barras& barras(bool diego); RoundStats& round(); Veredicto veredicto(std::mt19937&); }` — aplica `cadenaDanio`, aire, sed, concentración (golpe a la cabeza = gancho/uppercut no bloqueado: −`(14-concentracion)`; aire<20: −2/s; guardia +2/s; combo recibido −10), **segundo aire** (si Diego tiene la habilidad, una vez: aire<10 → +50), emite `Eventos`.
  - Sistema `bool SistemaAlcance(const IVJ::Entidad& atk, const IVJ::Entidad& def, Golpe)`: `|dx| <= {30,30,24,20}[g]`, `|dy| <= 8`, mirando hacia el rival.
  - Estado: `class FasePelea { virtual FasePelea* actualizar(Escena_Ring&, float dt) = 0; virtual void entrar(Escena_Ring&); }` con `Presentacion` (3 s "ROUND n", campana), `EnRound` (reloj `segundos`), `Caida` (usa `Conteo`; Diego machaca J/K; rival usa `probLevantarRival` y se levanta entre 6 y 8), `Esquina` (toalla automática: `concentracion = max(concentracion, 60)`; elegir **agua** (sed=0) o **consejo** (concentración +30 y durante los primeros 10 s del round siguiente se muestra el ícono del próximo golpe del rival); 10 s o J), `Decision` (si `jueces`: tarjetas de 3 jueces con `decidir`; si no, solo "Sparring terminado"), `FinPelea` (→ `Partida::alTerminarPelea` + `irAPaso()` o reinicio).
  - Reglas GDD: 3 caídas de Diego en un round → pelea perdida (TKO) y reinicia; `fuera()` en el conteo → KO; Diego derriba al rival y no se levanta → victoria. Contragolpe: golpe dentro de `0.4 s` tras esquiva exitosa → ×2 (requiere `Habilidad::CONTRAGOLPE`). Combos dan +25% y −10 de concentración al rival (requiere `COMBOS`). FRONTERA (requiere habilidad + concentración 100): ×2 y si conectan los 4 golpes → caída del rival. `PERDER_FORZADO` (Ricardo semifinal): resistencia del rival nunca baja de 1, Diego no puede ser noqueado (resistencia mínima 1 tras levantarse siempre), decisión forzada `RIVAL_UNANIME`.
  - HUD de pelea: barras de Diego a la izquierda y del rival a la derecha, round y reloj al centro, `Vineta`; con concentración <40 el rival se dibuja 2 veces extra desplazado ±2 px con alfa 80 (desenfoque).
- [ ] **Step 1: pruebas de la fachada** (`pruebas_logica.cpp`): `Combate` Diego vs Brandon: un jab bloqueado resta menos que uno limpio; Diego con aire 3 no puede lanzar uppercut (`lanzar` no cobra ni daña); segundo aire se dispara una sola vez; `PERDER_FORZADO` deja al rival en ≥1 tras 100 uppercuts.
- [ ] **Step 2:** ctest FAIL → implementar → PASS.
- [ ] **Step 3:** Agregar temporalmente a la historia demo el paso `PELEA sparring_brandon` y jugar: 3 rounds, esquina, conteo con J/K, sin jueces.
- [ ] **Step 4:** `--prueba-selector=10` Debug y Release → 0.
- [ ] **Step 5:** Commit `feat(round12) : combate en el ring con rounds, esquina, conteo y jueces`.

### Task 9: Recorrido del Sanchez Boxing GYM (estaciones) y pasillo de Mexicali

**Files:** Create `src/Round12/Escenas/Escena_Recorrido.hpp/.cpp`; Modify `Niveles.hpp` (configs `gym_tutorial`, `pasillo_mexicali`)

**Interfaces:**
- Produces: `struct ConfigRecorrido { std::string tilemap; std::vector<Estacion> estaciones; float x_ring; std::string dialogo_inicio; std::vector<std::pair<float,std::string>> dialogos_en_x; };` `enum class Estacion { COSTAL, MANOPLAS, CUERDA, AGUA };`
  - Vista lateral, Diego camina con WASD por la franja del piso (colisiones de `MapaColision` valor 1 + casilleros/costales sólidos), cámara como `Escena_Round12Col` (foco con x limitada al mapa). HUD con las 4 barras.
  - Zonas (`IZona`, radio 24 px) en x: entrada 40 (diálogo de controles), plátano en casilleros 60, costales 160, manoplas 280 (Carlos con manoplas), cuerda 400, agua 480, ring `x_ring=560`. Mensaje "J: entrenar". El ring está cerrado (pared invisible en x=520, Carlos dice "Primero termina las estaciones") hasta completar costal, manoplas y cuerda.
  - Mini-juegos como sub-estados de la escena:
    - **Costal:** fases J, K, L, I; 5 indicaciones cada una (ícono de `iconos_golpes.png`); acierto en ≤1.5 s mueve el costal; fallo repite la indicación. Al terminar: `mejorar(&Atributos::poder, +1)`.
    - **Manoplas:** secuencias `1-1-2`, `1-2-3`, `3-4`; Carlos las muestra 1.5 s; el jugador tiene 0.6 s entre teclas; cada una 2 veces correctas. Al terminar: `desbloquear(COMBOS)`, `mejorar(velocidad,+1)`.
    - **Cuerda:** `musica_cuerda.wav`, pulso cada 0.5 s; 32 pulsos; J dentro de ±0.12 s = acierto; ≥22 aciertos o se repite. Al terminar: `mejorar(aire,+1)`.
    - **Agua:** J → sed 0. **Plátano:** +25% de `resistencia_max`.
  - Al llegar al ring: `historia().avanzar()` (el siguiente paso es la pelea). En el paso de Brandon, Carlos enseña la esquiva: `desbloquear(ESQUIVA)` al iniciar ese sparring; al completar el nivel 1 `desbloquear(CONTRAGOLPE)`.
  - Pasillo Mexicali: tilemap 03, sin estaciones; diálogos en x (vendaje, Carlos), `publico.wav` en loop; `x_ring` al final.
- [ ] **Step 1:** Implementar; prueba pura de la ventana de ritmo en `Logica/` (`bool enPulso(float t, float periodo=0.5f, float tolerancia=0.12f)`): `enPulso(1.08)`=true, `enPulso(1.2)`=false → ctest.
- [ ] **Step 2:** Manual: completar las 3 estaciones; el ring se abre; la sed baja con el garrafón.
- [ ] **Step 3:** Commit `feat(round12) : nivel 1 Sanchez Boxing GYM con estaciones de entrenamiento`.

### Task 10: Corredor — Playa de Rosarito y Circuito atlético

**Files:** Create `src/Round12/Escenas/Escena_Runner.hpp/.cpp`; Modify `Niveles.hpp` (configs `playa`, `playa_dif2`, `circuito`, `circuito_dif2`)

**Interfaces:**
- Consumes: `Runner` (Task 5), `FabricaR12::obstaculo/item`.
- Produces: `struct ConfigRunner { std::string tilemap; float meta, tiempo; int vueltas; float meta_vuelta; bool olas; unsigned semilla; };`
  - Playa: meta 9000 px, tiempo 180 s (dif2: 150), tramos 0–3000 arena seca (1 obstáculo/300 px: castillo, hielera, sombrilla), 3000–6000 orilla (1/220, olas cada 6 s durante 2 s, arena mojada ×0.8), 6000–9000 rumbo al muelle (1/150: pelota, perro, cangrejo móviles). Puestos de agua (checkpoint + sed 0) en 3000 y 6000; muelle en 9000. Sombras de 48 px cada ~900 px; 2 isotónicas a la altura del salto sobre una hielera (aire +30, sed −40); 2 plátanos. Obstáculos generados con `std::mt19937(semilla)` (determinista).
  - Tilemap 05 dibujado repetido en x (`for` con `setPosition(i*ancho,0)` solo los visibles). Parallax: cielo 0.0, mar 0.2, cerros 0.4, palmeras 0.7 de la x de cámara.
  - Controles: A frena, D acelera, W salta, S se agacha. Falla (tiempo 0 o aire 0) → reinicia en el último puesto con `tiempo = max(tiempo_guardado, 60)`.
  - Resultado: tiempo restante ≥60 s → aire +2, resistencia +1; si no → aire +1. Al terminar la playa la primera vez: `desbloquear(SEGUNDO_AIRE)`.
  - Circuito: tilemap 06, pista de 1500 px × 3 vueltas, sin olas, límite 100 s (dif2: 85); todas las vueltas ≤26 s → velocidad +1.
- [ ] **Step 1:** Implementar; `--prueba-selector=10` → 0.
- [ ] **Step 2:** Manual: terminar la playa en <3 min; perder por aire regresa al puesto de agua.
- [ ] **Step 3:** Commit `feat(round12) : nivel 2 Playa de Rosarito y circuito atlético`.

### Task 11: Parque de skate de Santa Fe (recorrido vertical)

**Files:** Create `src/Round12/Escenas/Escena_SantaFe.hpp/.cpp`; Modify `Niveles.hpp` (`santa_fe`, `santa_fe_dif2`)

**Interfaces:**
- Produces: plataformas = `MapaColision::cargar(".../colisiones/04_skate_santa_fe_colisiones.txt", 1)` y obstáculos valor 2 (dañan: aire −10). Mundo 320×640, cámara vertical (sigue la y de Diego, x fija). A/D mueve, W salta (aire −4, `vy=-170`, `g=420`), meta: llegar a la fila superior en 120 s (dif2: 100). 2 puntos de agua (checkpoint) a 1/3 y 2/3 de altura; caer debajo del checkpoint → reaparece en él. Resultado: `mejorar(resistencia,+1)`.
- [ ] **Step 1:** Implementar; manual: subir hasta arriba; caer reaparece en checkpoint.
- [ ] **Step 2:** Commit `feat(round12) : parque de skate de Santa Fe`.

### Task 12: Historia completa, diálogos y demo

**Files:** Modify `src/Round12/Datos/Niveles.hpp`, `Partida.cpp` (secuencias); Create `src/Round12/Datos/dialogos/*.txt`

**Interfaces:**
- Produces: `std::vector<Paso> secuenciaDemo()` y `secuenciaHistoria()`.
  - **Demo (GDD "Gameplay del demo"):** CINE `prologo` → RECORRIDO `gym_tutorial` → PELEA `sparring_brandon` (3×60, SOBREVIVIR, sin jueces, gym) → RESULTADOS → CINE `playa_intro` → RUNNER `playa` → RESULTADOS → CINE `mexicali_llegada` → RECORRIDO `pasillo_mexicali` → PELEA `javier` (3×60, GANAR, jueces, mexicali; desbloquea FRONTERA al empezar) → RESULTADOS → CINE `ricardo_observa` → FIN (vuelve al título).
  - **Historia:** CINE `prologo` → RECORRIDO `gym_tutorial` → CINE `cap1` → SANTA_FE `santa_fe` → RUNNER `playa` → RUNNER `circuito` → PELEA `sparring_ninos` (NINO tinte (180,140,110), 1×60, GANAR, sin jueces, gym) → RESULTADOS → CINE `cap2` → PELEA `sparring_brandon` → PELEA `torneo_local_1` y `torneo_local_2` (visitante; NINO con tinte y atributos 4–6 al azar con la semilla de la partida, 2×60, GANAR, jueces) → RESULTADOS → CINE `cap3` → RECORRIDO `pasillo_mexicali` → PELEA `javier` → PELEA `kevin` (3×60, GANAR) → RESULTADOS → CINE `cap4` → PELEA `semifinal_ricardo` (3×60, PERDER_FORZADO) → CINE `epilogo` (fondo negro, hospital, "30 horas después") → CINE `segunda_mitad` → SANTA_FE `santa_fe_dif2` → RUNNER `playa_dif2` → RUNNER `circuito_dif2` → PELEA `revancha_javier` y `revancha_kevin` (dificultad 2: atributos del rival +1, tope 10) → RESULTADOS → CINE `final_previa` → PELEA `final_ricardo` (**12 rounds × 45 s** — supuesto por el título "Round 12"; GANAR; dificultad 2) → CINE `creditos` → FIN.
  - Diálogos: un `.txt` por id, redactados a partir de la sección del GDD que lleva su nombre (Prólogo, Capítulos 1–4, Epílogo, descripción de cada nivel); `prologo` incluye la frase de Carlos «Aquí nadie pelea hasta que aprende a entrenar»; `ricardo_observa` muestra a Ricardo en las gradas de Mexicali. 4–10 líneas cada uno.
- [ ] **Step 1:** Prueba: `secuenciaDemo()` empieza en `prologo`, contiene exactamente 3 pasos RECORRIDO/RUNNER/PELEA "de nivel" (gym, playa, javier) y termina en FIN; cada `id` de PELEA/RUNNER/RECORRIDO/SANTA_FE existe en `Niveles.hpp` (recorrer ambas secuencias) → ctest.
- [ ] **Step 2:** ctest FAIL → implementar → PASS; quitar el paso temporal de Task 8.
- [ ] **Step 3:** Commit `feat(round12) : historia completa, demo y diálogos`.

### Task 13: Verificación final

- [ ] **Step 1:** `cmake --build build --config Debug && cmake --build build --config Release` → 0 warnings.
- [ ] **Step 2:** `ctest --test-dir build -C Debug --output-on-failure` y `-C Release` → todo PASS.
- [ ] **Step 3:** `./bin/Debug/CimaEngine3v --prueba-selector=40; echo $?` y lo mismo en `bin/Release` → `0`, sin líneas `FALLA`.
- [ ] **Step 4:** Recorrido manual (Release y Debug): lanzador → Labs (Init, Menu, Camaras con sus 4 cámaras, Sprites, Round12Col) → F1 → ROUND 12 Demo completo (~20 min: estaciones, sparring de pie al final del 3er round, playa <3 min, victoria a Javier, adelanto de Ricardo) → F1 a Labs y regreso: la escena R12 se reanuda, sin música en Labs. Historia: probar al menos Santa Fe, circuito, un torneo local, Kevin y que la semifinal contra Ricardo avance a la cinemática del epílogo.
- [ ] **Step 5:** Revisión con `superpowers:requesting-code-review` sobre la rama; corregir lo que salga.
- [ ] **Step 6:** Commit final si hubo correcciones. No hacer push ni PR sin pedirlo.

## Verificación (resumen)

- Pruebas de lógica: `cmake --build build --config Debug --target Round12Pruebas && ctest --test-dir build -C Debug --output-on-failure`.
- Compatibilidad del selector (ambas configuraciones): `./bin/<Config>/CimaEngine3v --prueba-selector=40` → código 0.
- Juego: recorrido manual de Task 13 Step 4.

## Fuera de alcance / supuestos

- Sin guardado de partida (el GDD no lo pide). Sin control/gamepad (GDD: solo teclado).
- "Poder" de cada boxeador, todos los números de balance y el final a 12×45 s son supuestos marcados; viven como constantes en `Logica/` y `Datos/` para ajustarlos.
- Arte y audio provisionales generados; se reemplazan con archivos del mismo nombre sin tocar código.
- No hay animación de caída en las hojas: se rota el sprite.

## Ejecución recomendada

Subagent-driven: son 14 tareas con interfaces encadenadas (Logica → Maquinas → Escenas) y un error en la fachada de combate o en el selector se propaga a todo lo demás, así que conviene un revisor por tarea.
