# ROUND 12 — Refactorización al paquete de recursos v3 (escenarios 3/4, capas y animaciones completas) — Plan de implementación

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Que ROUND 12 use el nuevo `ROUND12_Assets` completo. Cada escenario se dibuja en 3 capas: decorado, personajes ordenados por profundidad y primer plano/cuerdas. Personajes con 39 animaciones × 4 direcciones, y diálogos con rostros y retratos. La historia y los controles se mantienen, y ningún recurso nuevo queda sin usar.

**Architecture:**
- Un script de Python empaca los 5 616 cuadros de personaje y los 768 de diálogo, todos de 80×80, en hojas por personaje (`motor/hojas/`).
- La lógica pura nueva vive en `Logica/` y se prueba con `Round12Pruebas`:
  - elegir animación según postura y brazo;
  - dirección de 4 lados;
  - polígonos de piso;
  - camino ovalado del circuito;
  - formato de diálogo.
- `Datos/Escenarios.*` describe cada escenario en coordenadas de 1920×1080: capas, polígonos del piso, ring, objetos y estaciones.
- `EscenaR12` agrega:
  - una cámara con bandas: el escenario completo se ve a 1080×608 y el HUD va en las franjas;
  - una lista de dibujo ordenada por la y de los pies, en la que las cuerdas son un objeto más.
- Las 6 escenas existentes se adaptan; `Escena_Mapa` es nueva.
- Labs vuelve a funcionar copiando a `assets/round12_legado/` los archivos borrados que necesita.

**Tech Stack:** C++17, SFML 3, ImGui-SFML, CMake (Ninja multi-config en `build/`), Python 3 + Pillow (`uv run --with pillow`).

**Spec:**
- `ROUND12_Assets/README.md` (secciones «Asset Pack Completo v3» y «Nota sobre las cuerdas»).
- `ROUND12_Assets/ROUND12_Sprites_Animaciones/animaciones.json` (duraciones y origen).
- `ROUND12_Assets/escenarios/*/mapa.json` y `objetos/objetos.json`.
- `tilesets/tiles.json`.
- GDD Avance 4: historia y controles, sin cambios. Su texto ya está en `Datos/` y en `dialogos/`.

## Context

El usuario reemplazó el paquete de recursos:
- **Escenarios:** pinturas en 3/4 de 1920×1080. La capa `decorado` no trae las cuerdas frontales; esas van en `primer_plano`/`cuerdas_frontales`, que son idénticos byte por byte en 01–03 (cuerdas en y 569–694, x 262–1659). En 04 y 06 el primer plano es transparente. 05 (playa) solo trae la tira lateral `nativo` 960×192 y `zoom` 1920×384.
- **Personajes (6):** 39 acciones × 4 direcciones × 6 cuadros de 80×80, con origen ≈(40,69).
- **Diálogo:**
  - 8 emociones × 4 direcciones × 4 cuadros de cuerpo completo;
  - 8 retratos de 96×96 por personaje;
  - para Diego y Carlos, 30 rostros de 224×224 (5 vistas × 6 expresiones) más una hoja.
- **Objetos:** 26 de 96×128 con pivote (48,120).
- **Tiles:** 32 tiles isométricos de 64×48 (huella 64×32), en `atlas_isometrico_64x48.png` (8×4) y como archivos sueltos.
- **Cuerdas modulares:** 3 tiles de 128×14 más un `stack`.

Ya no existen los recursos viejos que carga el código. Por eso hoy se cierran al cargar:
- el título;
- las cinemáticas;
- el ring y el recorrido (por `spritesheet_npc.png`);
- dos escenas de Labs: `Escena_Round12` y `Escena_Round12Col`.

`GestorAssets::agregarTextura` hace `exit()` cuando falta un archivo.

**Decisiones del usuario:**
- Santa Fe = **circuito de agilidad**.
- Playa = **corredor lateral con la tira y los objetos**, con zona final isométrica armada con tiles.
- Encuadre = **bandas con HUD**; Labs no cambia.
- Recursos viejos y duplicados (`ROUND12_Sprites_Animaciones`, `tilemaps/*.txt` de 8×8, `manifest*.json`) = **no se cargan; se documentan; no se borran**. Labs sigue usando las `hojas/` de `ROUND12_Sprites_Animaciones`.

## Global Constraints

- Historia igual: `secuenciaDemo()` y `secuenciaHistoria()` no cambian sus pasos, ids ni objetivos (se prueba igual que hoy). Solo se agrega `Escena_Mapa` como transición dentro de `irAPaso()`; no es un paso nuevo.
- Controles iguales (GDD):
  - W/A/S/D moverse; J jab, K recto, L gancho, I uppercut; Shift guardia; Q/E esquiva; Esc pausa.
  - En corredor/circuito: W salta, S se agacha, A frena, D acelera.
  - F1 queda para el selector.
  - Único agregado, con teclas que ya existen: **Q+E juntas (≤0.12 s) = agacharse** en el ring y el gimnasio.
- **Coordenadas del mundo = píxeles del escenario 1920×1080.**
  - Cámara del escenario: `sf::View` de 1920×1080 con viewport `{0, 56/720, 1, 608/720}`.
  - HUD en pantalla: banda superior y = 0–56, banda inferior y = 664–720.
- Escalas:
  - `ESCALA_PERSONAJE = 32.f/9.f` (2 px de pantalla por px de arte).
  - `ESCALA_OBJETO = 16.f/9.f` (1:1 en pantalla).
  - Origen de cuadro `(40,69)`; pivote de objeto `(48,120)`.
- Texturas:
  - personajes, objetos, tiles y retratos `setSmooth(false)`;
  - decorados, primer plano y rostros 224 `setSmooth(true)` (se reducen a 0.5625).
- La izquierda **ya viene espejeada** en los archivos: nunca usar escala −1 (README de animaciones).
- Duraciones por cuadro (ms, `animaciones.json`): caminar 110, guardia 120, jab 70, recto 75, gancho 80, uppercut 85, esquivas 90.
  - Supuesto: idle 120, saltar 90, agacharse 90, caer 100, levantarse 100, recibir_golpe 70.
  - Las variantes compuestas (`caminar_jab`…) usan la duración del golpe.
- Namespace `Round12`, claves de assets `r12g_`, sin cambios a `IVJ::NodosBoxeo`.
- En `src/Juego` solo se tocan las rutas de las 2 escenas de laboratorio (Task 1).
- Debug `-Wall -Wextra -Werror`; Debug y Release sin warnings.
- Git:
  - rama nueva `feat/round12-assets-v3` desde `feat/round12-juego-completo`; sin worktrees;
  - commits Conventional **sin** `Co-Authored-By` ni menciones a la IA (CLAUDE.md del usuario manda sobre el recordatorio del sistema);
  - `.gitignore` del usuario sin commitear.
- Patrones: se mantiene la tabla del plan anterior. Este trabajo agrega uso de:
  - Composición: `ListaDibujo` con nodos `Capa`, `Prop`, `Cuerdas` y `Boxeador`.
  - Fábrica: `FabricaR12::prop`, `cuerdas`.
  - Adaptador: `Camino` adapta el `Runner` 1D a la pista ovalada; `Piso` adapta polígonos a movimiento.
  - Estado: postura de `NodosR12`.
  - Iterador: `Dialogo`.
  - Fachada: `Combate`, con la regla de agacharse.

## Review Focus

1. **Diego fuera del ring, al frente**: debe dibujarse encima de las cuerdas; dentro del ring, debajo.
   - Prueba: el orden de `ListaDibujo` con cuerdas en `RING_FRENTE_Y` y pies en 700 y 900 (Task 4).
2. **Doble espejo**: con dirección IZQUIERDA el rect sale de la columna `izquierda` y la escala x es positiva.
   - Prueba: `rectCuadro(accion, IZQUIERDA, f)` y la escala del sprite > 0 (Task 3/4).
3. **Falta un recurso** (el usuario vuelve a cambiar el paquete): el juego no debe cerrarse.
   - `FabricaR12::textura` revisa `std::filesystem::exists`; si falta, imprime una vez `[r12] falta <ruta>` y usa una textura magenta de 8×8.
   - Prueba: `--prueba-selector` con un nombre inexistente forzado en Debug (Task 1).
4. **Agacharse en el ring**:
   - esquiva jab, recto y gancho, pero recibe ×1.5 de uppercut;
   - Q o E sola sigue siendo esquiva lateral;
   - con la tecla repetida del SO no se activa agacharse.
   - Prueba en `Combate` y en `DetectorAgacharse` (Task 3).
5. **Labs**: abrir Round12 y Round12Col desde el selector no debe cerrar el programa.
   - Prueba: `--prueba-selector=20` recorre todas las escenas de Labs y termina con código 0 (Task 1).

---

### Task 0: Rama y commit del paquete de recursos del usuario

**Files:**
- Create: `docs/superpowers/plans/2026-10-31-round12-assets-v3.md` (copia de este plan)

- [ ] **Step 1:** `git checkout -b feat/round12-assets-v3`.
- [ ] **Step 2:** `git add ROUND12_Assets` (altas y bajas del usuario; **no** `.gitignore`) y la copia del plan.
- [ ] **Step 3:** Commit `chore(assets) : paquete de recursos ROUND12 v3`.

### Task 1: Labs sin cierres y texturas faltantes sin `exit`

**Files:**
- Create: `assets/round12_legado/` con los 8 archivos borrados que usa Labs, restaurados desde `HEAD~1` (el commit anterior a Task 0) con `git show <rev>:<ruta> > ...`:
  - `01_sanchez_boxing_gym_1080x720.png`;
  - `spritesheet_4poses.png` × 5 (en `<boxeador>/`);
  - `carlos_sanchez/spritesheet_npc.png`;
  - `tilesets/01_sanchez_boxing_gym.png`.
- Modify:
  - `src/Juego/Escenas/Escena_Round12.cpp:75,79,85` y `Escena_Round12Col.cpp:144`: rutas a `ASSETS "/round12_legado/..."`;
  - `assets/mapas/r12_sanchez_gym_col.txt:2`: atlas en `/round12_legado/tilesets/01_sanchez_boxing_gym.png`.
- Modify: `src/Round12/objetos/FabricaR12.cpp` `textura()`: si el archivo no existe → `cerr` una sola vez y textura magenta 8×8 creada en memoria (`sf::Image({8,8}, sf::Color::Magenta)`).

- [ ] **Step 1:** Ejecutar `./bin/Debug/CimaEngine3v --prueba-selector=20; echo $?`. Expected: se cierra (código ≠ 0) al entrar a `Round12`, `Round12Col` o `R12_Titulo`.
- [ ] **Step 2:** Restaurar los archivos, cambiar las rutas y agregar el respaldo de `textura()`.
- [ ] **Step 3:** Mismo comando en Debug y Release. Expected: `0`, sin `FALLA`. Las escenas R12 pueden mostrar magenta (aún usan rutas viejas); eso se arregla en Tasks 4–9.
- [ ] **Step 4:** Commit `fix(labs) : escenas de laboratorio Round12 con recursos legados`.

### Task 2: Empaquetado de hojas y verificación del paquete

**Files:**
- Create: `tools/round12_hojas.py`.
- Modify: `tools/round12_assets.py`:
  - borrar `mapas_motor()` y lo que lee `tilesets/{nivel}.png`, que ya no existen;
  - borrar `ROUND12_Assets/motor/*.txt`, que eran generados nuestros.
- Modify: `tools/round12_provisionales.py`: dejar de generar lo que reemplaza el paquete y borrar esos archivos:
  - `costal, garrafon, sombrilla, castillo, hielera, sombra, puesto_agua, muelle, publico`;
  - `parallax/*`.
- Se conservan: `botella, isotonica, platano, toalla, manoplas, cuerda, ola, perro, cangrejo, pelota, iconos_golpes, vineta` y el audio.

**Interfaces — Produces (archivos exactos):**
- `ROUND12_Assets/motor/hojas/acciones.txt`: las 39 acciones de `sprites/personajes/diego_ureta/derecha/`, en orden alfabético, una por línea. El renglón *n* es la fila *n* de la hoja.
- `ROUND12_Assets/motor/hojas/<personaje>.png`, 1920×3120:
  - columna = `dir*6 + cuadro`, con dir en orden `adelante, atras, derecha, izquierda` (= `CE::IDireccion` ADELANTE=0…IZQUIERDA=3);
  - fila = índice de la acción.
- `ROUND12_Assets/motor/hojas/<personaje>_dialogo.png`, 1280×640:
  - columna = `dir*4 + cuadro`;
  - fila = emoción en orden `cansado, celebrando, decidido, escuchando, hablando, neutral, sonriendo, sorprendido`.

Validaciones del script (`assert`):
- las 6 carpetas tienen las mismas 39 acciones × 4 direcciones × 6 cuadros, todos de 80×80;
- `atlas_isometrico_64x48.png` recortado en la celda `i` es igual pixel a pixel a `tilesets/<i>_*.png` según `tiles.json`;
- `primer_plano == cuerdas_frontales` en 01–03;
- cada nombre de `objetos.json` existe como PNG de 96×128.

- [ ] **Step 1:** `uv run --with pillow python -I tools/round12_hojas.py`. Expected: termina sin `AssertionError`, 12 PNG + `acciones.txt`; `wc -l acciones.txt` = 39.
- [ ] **Step 2:** Re-ejecutar `tools/round12_provisionales.py`. Expected: sin errores; los archivos retirados ya no están.
- [ ] **Step 3:** Commit `feat(round12) : hojas empacadas de 39 animaciones y diálogo`.

### Task 3: Lógica pura — animación, dirección, piso, camino, agacharse y diálogo

**Files:**
- Create:
  - `src/Round12/Logica/Animaciones.hpp/.cpp`
  - `Logica/Piso.hpp/.cpp`
  - `Logica/Camino.hpp/.cpp`
  - `Logica/LineaDialogo.hpp/.cpp`
- Modify:
  - `Logica/Combate.*` (agachado)
  - `Logica/FiltroTeclas.hpp` (`DetectorAgacharse`)
- Test: `src/Round12/pruebas/pruebas_logica.cpp` (nuevas `void pruebasX()` llamadas desde `main`, macros `CHECK`/`CERCA` existentes).

**Interfaces — Produces:**
- `enum class Accion : uint8_t` con las 39 acciones en el **mismo orden alfabético** que `acciones.txt` (`AGACHARSE=0 … UPPERCUT=38`).
  - `const char* nombre(Accion)`.
  - `float msPorCuadro(Accion)` con los valores de Global Constraints.
  - `bool enCiclo(Accion)`: true para idle, guardia, caminar*, agacharse_guardia, esquiva_*_guardia y saltar_guardia.
- `enum class Postura { PIE, CAMINANDO, AGACHADO, SALTANDO, ESQUIVA_IZQ, ESQUIVA_DER };` `enum class Brazo { NADA, GUARDIA, JAB, RECTO, GANCHO, UPPERCUT };`
  - `Accion elegirAccion(Postura, Brazo)`. Tabla:
    - PIE: NADA→IDLE, GUARDIA→GUARDIA, golpes→JAB…UPPERCUT.
    - CAMINANDO: NADA→CAMINAR, resto→`CAMINAR_<x>`.
    - AGACHADO: NADA→AGACHARSE, resto→`AGACHARSE_<x>`.
    - SALTANDO: NADA→SALTAR, resto→`SALTAR_<x>`.
    - ESQUIVA_IZQ/DER: NADA→`ESQUIVA_<lado>`, resto→`ESQUIVA_<lado>_<x>`.
  - Especiales fuera de la tabla: `CAER`, `LEVANTARSE`, `RECIBIR_GOLPE`.
- `sf`-libre: `struct RectCuadro { int x,y,w,h; }; RectCuadro rectCuadro(Accion, int dir, int cuadro)` → `{(dir*6+cuadro)*80, (int)accion*80, 80, 80}`; `RectCuadro rectDialogo(int emocion, int dir, int cuadro)` → `{(dir*4+cuadro)*80, emocion*80, 80, 80}`.
- `int direccionHacia(float dx, float dy)`:
  - `|dx| >= |dy|` → DERECHA(2)/IZQUIERDA(3); si no, ADELANTE(0) si dy>0, ATRAS(1) si dy<0;
  - con (0,0) regresa -1 (conservar la anterior).
- `struct Pt { float x,y; }; class Piso { void agregar(std::vector<Pt> poligono); void bloquear(Rect); bool dentro(Pt) const; Pt limitar(Pt antes, Pt despues) const; }`
  - `dentro` = dentro de algún polígono y fuera de todos los bloqueos.
  - `limitar` intenta el movimiento completo, luego solo x, luego solo y, y si no, regresa `antes` (deslizarse por la orilla).
- `class Camino { explicit Camino(std::vector<Pt> puntos_cerrados); float largo() const; Pt punto(float s) const; Pt tangente(float s) const; }`: `s` se toma módulo `largo()`.
- `struct DetectorAgacharse { bool presionar(char tecla /*'Q'|'E'*/, float t); }`: regresa true si la otra tecla se presionó ≤0.12 s antes.
- `Combate::recibir(bool a_diego, Golpe, bool bloqueado, bool agachado)` (parámetro nuevo; actualizar los llamadores):
  - agachado + JAB/RECTO/GANCHO → 0 daño y no cuenta como conectado;
  - agachado + UPPERCUT → daño ×1.5.
- `struct LineaDialogo { std::string orador, emocion, fondo, texto; int lado; /*0 izq,1 der*/ }; std::optional<LineaDialogo> parsearLinea(const std::string&)`:
  - formato `orador|emocion|fondo|texto` y opcional `|der`;
  - `#` y líneas vacías → `nullopt`;
  - una emoción fuera de las 8 → `nullopt`.
- `std::string archivoRostro(const std::string& orador, const std::string& emocion, int lado, bool boca_abierta)`:
  - solo `diego`/`carlos`; si no, `""`;
  - vista `perfil_der` si `lado==0` y `perfil_izq` si `lado==1`, salvo `neutral`, que usa `frente`;
  - expresión: hablando → `hablando_abierto`/`hablando_cerrado` según `boca_abierta`; sonriendo/celebrando → `sonriendo`; sorprendido → `sorprendido`; decidido/cansado → `serio`; escuchando/neutral → `neutral`;
  - resultado: `"<orador_completo>_dialogo_<vista>_<expresion>.png"`;
  - la vista `trescuartos_*` se usa para el oyente (`archivoRostroOyente`, mismo mapeo con vista `trescuartos_der/izq`).

- [ ] **Step 1: pruebas que fallan:**
  - `elegirAccion(CAMINANDO,GANCHO)==Accion::CAMINAR_GANCHO`.
  - Cobertura: el conjunto {36 de la tabla} ∪ {CAER, LEVANTARSE, RECIBIR_GOLPE} tiene 39 elementos distintos.
  - Las 39 líneas de `R12_RECURSOS "/motor/hojas/acciones.txt"` coinciden con `nombre(Accion(i))` en minúsculas (`#ifdef` con la ruta, igual que la prueba de diálogos).
  - `rectCuadro(JAB, 3, 2)=={1600, 28*80, 80, 80}` (JAB = 28).
  - `direccionHacia(5,-2)==2`, `(-1,-4)==1`, `(0,0)==-1`.
  - `Piso` con el cuadrado 0..100: `limitar({50,50},{150,60})=={50,60}`; con bloqueo `{40,40,20,20}`, `dentro({50,50})==false`.
  - `Camino` sobre el cuadrado 0..100: `largo()==400`, `punto(450)=={50,0}`.
  - `DetectorAgacharse`: Q en t=0 y E en t=0.1 → true; Q, Q → false; Q en 0 y E en 0.3 → false.
  - `Combate`: con agachado, el jab no quita resistencia y el uppercut quita 1.5× lo de uno sin agacharse.
  - `parsearLinea("carlos|serio|-|hola")` → nullopt (serio no es emoción de retrato); `parsearLinea("carlos|decidido|01_sanchez_boxing_gym_tijuana|Hola|der")->lado==1`.
  - `archivoRostro("diego","hablando",0,true)=="diego_ureta_dialogo_perfil_der_hablando_abierto.png"` y el archivo existe.
- [ ] **Step 2:** `cmake --build build --config Debug --target Round12Pruebas && ctest --test-dir build -C Debug --output-on-failure`. Expected: FAIL (no compila).
- [ ] **Step 3:** Implementar.
- [ ] **Step 4:** Mismo comando. Expected: PASS (las 87 revisiones anteriores + las nuevas).
- [ ] **Step 5:** Commit `feat(round12) : animaciones, piso, camino, agacharse y formato de diálogo`.

### Task 4: Datos de escenarios, capas, orden por profundidad y boxeador nuevo

**Files:**
- Create:
  - `src/Round12/Datos/Escenarios.hpp/.cpp`
  - `src/Round12/objetos/ListaDibujo.hpp/.cpp`
  - `src/Round12/Componentes/` `IAnimR12` (en `ComponentesR12.hpp`)
- Modify:
  - `objetos/FabricaR12.*`
  - `Escenas/EscenaR12.*`
  - `Maquinas/NodosR12.*`
  - `Datos/Niveles.*`: `Escenario` pasa a ser `const EscenarioR12& escenario(id)`; quitar `ring_x0/x1`, `PIES_*` y `mapa()`.
  - Borrar `Logica/MapaColision.*` (ya no hay colisiones TXT), con sus pruebas y llamadores.

**Interfaces — Produces:**
- `struct PropR12 { std::string objeto; Pt pie; bool solido; };` `struct EstacionR12 { std::string tipo; Pt pie; float radio; };`
- `struct EscenarioR12 { std::string carpeta; bool ring; std::vector<std::vector<Pt>> piso; std::vector<Pt> ring_poligono; float ring_frente_y; std::vector<PropR12> props; std::vector<EstacionR12> estaciones; Pt entrada, esquina_diego, esquina_rival, escalera_izq, escalera_der; };`
- `const EscenarioR12& escenario(const std::string& id)` para `gym`, `visitante`, `mexicali`, `santa_fe`, `circuito`. La playa usa su propia escena (Task 7).
- Ring de 01–03 (misma geometría; **medir sobre el decorado con una captura y ajustar**; estos valores son el punto de partida):
  - lona trapecio `{(500,440),(1420,440),(1680,760),(240,760)}`;
  - `ring_frente_y = 770`;
  - escaleras `(300,880)` y `(1620,880)`;
  - esquinas `(330,720)` y `(1590,720)`.
- Props por escenario (todos los 26 objetos de `objetos.json` aparecen al menos una vez; ubicarlos donde no tapen lo pintado y revisarlo en captura):

| Escenario | Objetos |
|---|---|
| gym (01) | `cartel_sanchez`, `silla` (esquina), `caja_madera`, `bote_basura` |
| visitante (02) | `gradas`, `banca`, `lockers`, `costal`, `pesas`, `fuente_agua`, `silla` |
| mexicali (03) | `cartel_mexicali`, `valla`, `banderola`, `silla`, `gradas` |
| santa_fe (04) | `rampa_skate`, `escaleras`, `barandal`, `cono`, `maceta`, `arbol`, `arbusto`, `poste_luz`, `sombra_muro`, `bandera` |
| circuito (06) | `valla`, `cono`, `bandera`, `banderola`, `arbol`, `arbusto`, `maceta`, `sombra_muro` |
| playa (05, Task 7) | `sombrilla`, `torre_salvavidas`, `surf`, `cono`, `caja_madera`, `bote_basura`, `banca`, `banderola`, `fuente_agua`, `bandera` |

- Composición: `class NodoDibujo { virtual float profundidad() const = 0; virtual void dibujar(sf::RenderTarget&) = 0; }`. Nodos:
  - `NodoSprite` (envuelve una `IVJ::Entidad` o un `sf::Sprite`; profundidad = y de los pies);
  - `NodoCuerdas` (textura `primer_plano` de la carpeta; profundidad = `ring_frente_y`).
  - `class ListaDibujo { void agregar(std::shared_ptr<NodoDibujo>); void dibujar(sf::RenderTarget&); }` ordena con `std::stable_sort` por profundidad en cada frame.
  - Orden de render de una escena: `decorado` → `ListaDibujo` → `primer_plano` (solo en 04/06, transparente; si no hay ring, no hay `NodoCuerdas`) → HUD.
- `FabricaR12`:
  - `std::shared_ptr<IVJ::Entidad> boxeador(Personaje, Pt pie, int dir, sf::Color tinte = White)`: textura `motor/hojas/<hoja>.png`, origen (40,69), escala `ESCALA_PERSONAJE`, componente `IAnimR12`.
  - `npc(Personaje::CARLOS, ...)`: agregar `CARLOS` al enum `Personaje` (`Perfiles.hpp`; `hoja()` → `"carlos_sanchez"`, sin atributos de pelea).
  - `std::shared_ptr<NodoDibujo> prop(const PropR12&)`: `objetos/<n>.png`, origen (48,120), escala `ESCALA_OBJETO`.
- `IAnimR12 : CE::IComponentes { Accion accion; int dir; int cuadro; float t; bool termino; void poner(Accion, bool reiniciar=false); void actualizar(float dt, float escala_tiempo); }`. La escena aplica `rectCuadro` al `ISprite` cada frame. Reemplaza el uso de `IVJ::AnimBoxeo` en R12.
- `NodosR12`: cada nodo fija `Postura` + `Brazo` y llama a `elegirAccion`:
  - Idle → PIE/NADA (fuera del ring) o PIE/GUARDIA (en pelea);
  - Caminar → CAMINANDO, con brazo GUARDIA si Shift;
  - Golpe → postura actual (CAMINANDO si hay WASD, con movimiento al 50 %; AGACHADO si está agachado; ESQUIVA_X dentro de la ventana de contragolpe) + brazo del golpe;
  - Esquiva → ESQUIVA_X;
  - `AgachadoR12` (nuevo, 0.5 s, por `DetectorAgacharse`) → AGACHADO;
  - Aturdido → `RECIBIR_GOLPE` (quitar el tinte rojo);
  - Caido → `CAER` y se queda en el cuadro 5 (quitar la rotación);
  - Levantarse → `LEVANTARSE`.
  - Dirección con `direccionHacia(rival - yo)` en pelea y con el movimiento fuera de ella.
- `EscenaR12`:
  - `usarCamara()` sin argumentos crea la vista 1920×1080 con el viewport de bandas;
  - `Hud`/`EnPantalla` dibujan en las bandas;
  - `Vineta` cubre solo el viewport del escenario.
- `SistemasR12`:
  - alcance `{107,107,85,71}` (los valores viejos × 32/9);
  - tolerancia de profundidad `|dy| ≤ 28`;
  - separación de cuerpos 50 px;
  - movimiento con `Piso::limitar`;
  - la velocidad de pasos se multiplica por 32/9.

- [ ] **Step 1: prueba de orden** (en `pruebas_logica.cpp`, solo la parte pura): extraer `std::vector<int> ordenPorProfundidad(const std::vector<float>&)` a `Logica/Animaciones.*`. Con `{900, 770, 700}` → `{2,1,0}` (Diego al frente queda encima de las cuerdas; Review Focus 1). Debe ser estable con empates.
- [ ] **Step 2:** ctest → FAIL; implementar todo el Task; ctest → PASS.
- [ ] **Step 3:** Debug y Release compilan sin warnings; `--prueba-selector=10` → 0.
- [ ] **Step 4:** Commit `feat(round12) : escenarios en capas, orden por profundidad y boxeadores de 39 animaciones`.

### Task 5: Pelea en el ring y en zonas abiertas

**Files:** Modify `Escenas/Escena_Ring.*`, `Maquinas/FasesPelea.*`, `Maquinas/CerebroRival.*`, `objetos/Hud.*`, `Datos/Niveles.cpp` (escenario de cada pelea)

**Comportamiento:**
- Las peleas usan `escenario(cfg.escenario)`:
  - posiciones de salida en las esquinas;
  - movimiento 2D dentro de `ring_poligono` encogido 40 px.
- El HUD va en la banda superior, con el retrato de 96 px de Diego y del rival (`sprites/personajes/<p>/retratos/<emocion>.png`, escala 0.5) junto a sus barras. Emoción por estado:
  - `decidido`: inicio del round o Frontera;
  - `cansado`: aire < 25;
  - `sorprendido`: 0.4 s tras recibir golpe;
  - `celebrando`: cuando el otro cae o al ganar;
  - `escuchando`: en la esquina;
  - `sonriendo`: combo conectado;
  - `hablando`: en la presentación;
  - si no, `neutral`.
- Banda inferior: round, reloj, íconos de combo, ayuda.
- Esquina: Carlos (`npc`) entra caminando desde la escalera de Diego hasta su esquina (`caminar` con la dirección del movimiento) y se queda en `dialogo/<dir>/hablando` mientras da el consejo. Diego en `idle`. La silla es un prop.
- Mexicali: las cartas de jueces se muestran sobre las mesas pintadas.
- Presentación con los dos `idle` mirando a cámara (`adelante`) y luego girando hacia el rival.
- `CerebroRival`:
  - usa agacharse (con `prob_esquiva/2`) contra golpes a la cabeza;
  - lanza golpes en movimiento (`CAMINAR_*`) cuando `agresividad > 0.6`;
  - lanza contragolpes con animación `ESQUIVA_*_<golpe>`.
- El niño (`NINO`) sigue siendo Diego con tinte.

- [ ] **Step 1:** Implementar; ajustar las peleas de `Niveles.cpp` a los escenarios `gym`, `visitante` y `mexicali` como hoy.
- [ ] **Step 2:** `./bin/Debug/CimaEngine3v --r12-paso=pelea:javier --autopiloto --rapido=10`. Expected: termina la pelea y el log muestra `[prueba-r12]`, sin `falta`.
- [ ] **Step 3:** Captura manual (`--r12-paso=pelea:sparring_brandon`):
  - cuerdas al frente de los boxeadores;
  - nadie sale de la lona;
  - los 4 golpes, esquivas, agacharse (Q+E), caer y levantarse se ven con sus animaciones.
- [ ] **Step 4:** Commit `feat(round12) : pelea en escenarios 3/4 con retratos y animaciones completas`.

### Task 6: Recorridos — Sanchez Boxing GYM y llegada a Mexicali

**Files:** Modify `Escenas/Escena_Recorrido.*`, `Datos/Niveles.cpp` (`ConfigRecorrido`), `Datos/Escenarios.cpp` (estaciones)

**Comportamiento:**
- Una sola pantalla y movimiento libre en `piso`.
- El ring está bloqueado hasta terminar las estaciones. Se entra caminando a una escalera: posición en la esquina de Diego y `historia().avanzar()`.
- Estaciones del gym, sobre lo pintado:
  - `costal`: saco rojo de la derecha, pie ≈(1660,330);
  - `agua`: garrafón blanco de la izquierda, ≈(70,430);
  - `platano`: lockers, ≈(1480,260);
  - `cuerda`: piso libre abajo a la izquierda, ≈(450,990);
  - `manoplas`: dentro del ring con Carlos; se entra por la escalera izquierda y es la única estación en el ring.
- Minijuegos (la lógica no cambia; cambia lo que se ve):
  - **Costal:** Diego mira el saco y hace `jab/recto/gancho/uppercut` según la tecla.
  - **Manoplas:** Carlos **hace** cada golpe de la secuencia con su animación (`jab`… en `izquierda`) antes de que Diego la repita; los íconos se quedan como apoyo.
  - **Cuerda:** Diego hace `saltar` en cada pulso y `provisionales/sprites/cuerda.png` se dibuja encima.
  - **Agua y plátano:** `idle` + ítem provisional encima.
- Carlos camina entre estaciones (`caminar` en la dirección del movimiento) y usa `dialogo/<dir>/<emocion>` cuando habla. El texto va en un globo en la banda inferior, con su retrato.
- Llegada a Mexicali:
  - escenario `mexicali`, entrada abajo a la izquierda;
  - los diálogos por punto (vendaje junto a la hielera izquierda, Carlos junto a la escalera) sustituyen a `dialogos_en_x`;
  - Ricardo `idle adelante` en las gradas de la derecha;
  - `publico.wav` en loop.
- `ConfigRecorrido`: `x_ring` y `dialogos_en_x` se reemplazan por `std::vector<std::pair<Pt,std::string>> dialogos_en_punto`.

- [ ] **Step 1:** Implementar; `--r12-paso=recorrido:gym_tutorial --autopiloto --rapido=10` (actualizar el piloto para ir a las estaciones por punto). Expected: completa las 3 estaciones y pasa a la pelea.
- [ ] **Step 2:** Lo mismo con `recorrido:pasillo_mexicali`.
- [ ] **Step 3:** Commit `feat(round12) : recorridos en una pantalla con estaciones sobre el decorado`.

### Task 7: Corredor — Playa (tira lateral + zona isométrica final) y Circuito (pista ovalada)

**Files:**
- Modify: `Escenas/Escena_Runner.*`, `Datos/Niveles.*` (`ConfigRunner` agrega `std::string modo; /*"playa"|"circuito"*/`)
- Create: `src/Round12/objetos/Isometrico.hpp/.cpp`, `src/Round12/Datos/iso/playa_final.txt`

**Interfaces — Produces:**
- `class DioramaIso { bool cargar(const std::string& txt); void dibujar(sf::RenderTarget&, Pt origen); }`:
  - TXT con `cols filas` + matriz de índices del atlas (`-1` vacío);
  - celda (c,f) en pantalla = `origen + ((c-f)*32, (c+f)*16)`;
  - recorta `atlas_isometrico_64x48.png` con `u=(i%8)*64, v=(i/8)*48` y `ESCALA_OBJETO`.
- **Obstáculos del `Runner`** (lógica pura): tipo `BAJO` (se salta), `ALTO` (se pasa agachado; usa la altura del agachado) y `ROMPIBLE` (un golpe a ≤60 px lo quita sin frenar; cuesta aire).
  - Prueba: con agachado, un `ALTO` no choca; con golpe, un `ROMPIBLE` desaparece.

**Playa:**
- Fondo: `05_playa_rosarito_zoom.png` repetido en x, escalado a 1080 de alto. El piso de la carrera va en la y de la arena de la tira (**medir**).
- Diego: `derecha` con `caminar` (velocidad de animación ∝ velocidad), `saltar`, `agacharse`, y `saltar_*`/`agacharse_*`/`caminar_*` al golpear. Al chocar, `recibir_golpe`.
- Objetos según tipo:
  - `BAJO`: `cono`, `caja_madera`, `surf`, `banca`;
  - `ALTO`: `banderola`, `sombrilla`;
  - `ROMPIBLE`: `caja_madera`, `bote_basura`.
- Escenografía:
  - `torre_salvavidas` como fondo cercano;
  - puesto de agua = `fuente_agua` + `bandera`;
  - sombras = `sombrilla` (zona de sombra);
  - ola y animales provisionales se conservan.
- Meta: fundido a la **zona isométrica final** (`DioramaIso` con `playa_final.txt`, que usa los tiles arena, arena_humeda, arena_huellas, espuma_mar, mar, mar_profundo y muelle_madera). Diego camina en 4 direcciones hasta Carlos junto a la `torre_salvavidas` y el nivel termina.
- `05_playa_rosarito_nativo.png` es el fondo de la cinemática `playa_intro` (Task 8).

**Circuito:**
- Escenario `circuito` (06), sin desplazamiento de cámara.
- La `x` del `Runner` es la distancia sobre un `Camino` por el centro del carril (**medir** el óvalo en el decorado), con 3 vueltas.
- Diego se dibuja en `camino.punto(x)` con `direccionHacia(tangente)`, así que usa las 4 direcciones de `caminar`.
- `valla` = `BAJO` en 4 puntos de la vuelta; `cono` = `ROMPIBLE`; `banderola` = `ALTO`.
- `bandera` en la meta; las vueltas se cuentan al cruzarla.

- [ ] **Step 1:** ctest con las pruebas nuevas de `Runner` (FAIL → implementar → PASS).
- [ ] **Step 2:** `--r12-paso=runner:playa --autopiloto --rapido=10` y `runner:circuito`. Expected: llega a la meta y pasa a Resultados (el piloto aprende a agacharse y golpear).
- [ ] **Step 3:** Commit `feat(round12) : playa lateral con zona isométrica y circuito en pista ovalada`.

### Task 8: Santa Fe — circuito de agilidad

**Files:** Modify `Escenas/Escena_SantaFe.*`, `Datos/Niveles.*` (`ConfigSantaFe`: lista de puntos de control, tiempo 120 / 100 s en dif2)

**Comportamiento:**
- Escenario `santa_fe` (04) y movimiento libre en `piso`.
- 8 puntos de control en orden. Cada uno es una `bandera` sobre una rampa, una banca/cornisa o el half-pipe; el siguiente parpadea.
- Para tocarlo hay que **golpearlo** (J/K/L/I a ≤60 px). En el aire sale `saltar_<golpe>`.
- `barandal`, `caja_madera` y las cornisas pintadas son obstáculos bajos: al caminar hacia ellos con aire ≥ 4 Diego los salta solo (`saltar`, aire −4, 0.5 s); si no, lo bloquean.
- `rampa_skate` y `escaleras` lanzan un salto largo.
- `sombra_muro` = zona de regeneración (+12 aire/s).
- `cono`: −10 de aire al tocarlo.
- Al acabar el tiempo se reinicia desde el último punto tocado.
- La recompensa no cambia (`mejorar(resistencia,+1)`).
- Lógica pura nueva: `class Agilidad { Agilidad(int n_puntos, float tiempo); void tick(float dt); bool tocar(int i); int siguiente() const; bool completo() const; bool sinTiempo() const; }`.
  - Prueba: tocar fuera de orden no cuenta; tocar los 8 en orden → `completo()`.

- [ ] **Step 1:** ctest (FAIL → implementar → PASS).
- [ ] **Step 2:** `--r12-paso=santafe:santa_fe --autopiloto --rapido=10`. Expected: completa los 8 puntos.
- [ ] **Step 3:** Commit `feat(round12) : Santa Fe como circuito de agilidad`.

### Task 9: Cinemáticas, título, resultados, pausa y mapa de la historia

**Files:**
- Modify: `Escenas/Escena_Cinematica.*`, `Escena_Titulo.*`, `Escena_Resultados.*`, `JuegoR12.*` (menú de pausa, `irAPaso`), los 15 `Datos/dialogos/*.txt`
- Create: `Escenas/Escena_Mapa.hpp/.cpp`, `Datos/iso/mapa_historia.txt`

**Cinemática** (usa `parsearLinea`, `archivoRostro` y `Dialogo`):
- Fondo = `escenarios/<fondo>/decorado` en el viewport de bandas, o `05_playa_rosarito_nativo` para la playa, o negro con `-`.
- Los personajes de la escena están de pie sobre el piso con su animación de cuerpo `dialogo/<dir>/<emocion>` (4 cuadros en ciclo). El que habla usa su emoción; el oyente usa `escuchando`.
- Caja de texto abajo:
  - Diego y Carlos: rostro de 224 (`archivoRostro`), con la boca alternando `hablando_abierto`/`cerrado` cada 0.12 s mientras se escribe el texto y la emoción final al terminar; el oyente Diego/Carlos con `archivoRostroOyente` del lado contrario.
  - Los demás: retrato de 96 ×2 con su emoción.
- Archivos de diálogo:
  - reescribir la 2.ª columna de pose numérica a emoción;
  - agregar `|der` donde hable el de la derecha;
  - fondos con los nombres nuevos de carpeta (`01_sanchez_boxing_gym_tijuana`, `02_gimnasio_visitante`, `03_gimnasio_mexicali_torneo`, `04_skatepark_santa_fe_tijuana`, `05_playa_rosarito`, `06_circuito_atletico_rosarito`);
  - el texto no cambia.
  - Prueba existente de diálogos: cada línea parsea con `parsearLinea` y cada fondo existe.

**Título:**
- Fondo: decorado del gym.
- `cartel_sanchez` como logo.
- Marco del menú con los tiles `cuerda_roja/blanca/azul_tile_128x14` repetidos (`setRepeated(true)`).
- Diego y Carlos en `idle adelante` sobre la lona.

**Banner «ROUND n»:** `cuerdas_frontales_stack_128.png` estirado en x.

**Pausa:** marco con los tiles de cuerda.

**Resultados:**
- Rostro de Diego de 224: `sonriendo` si mejoró algún atributo, `serio` si perdió (`frente`).
- Diego en `celebrando`/`cansado` (cuerpo de diálogo).

**Mapa de la historia (`R12_Mapa`):**
- Se muestra en `irAPaso()` cuando el paso siguiente cambia de escenario respecto al anterior. Dura 3 s o hasta presionar J.
- Diorama isométrico (`DioramaIso`) de `mapa_historia.txt` que usa **los 32 tiles**: una zona por lugar con sus pisos, más sendero, asfalto, tierra, pasto, seto y mar.
- Cada lugar lleva un marcador:

| Lugar | Marcador |
|---|---|
| gym | `cartel_sanchez` |
| Mexicali | `cartel_mexicali` |
| Santa Fe | `rampa_skate` |
| playa | `torre_salvavidas` |
| circuito | `bandera` |
| visitante | `gradas` |

- Diego camina por el sendero en 4 direcciones del lugar anterior al siguiente.
- Prueba (en `pruebas_logica.cpp`): leer `mapa_historia.txt` y verificar que aparecen los índices 0..31.

- [ ] **Step 1:** ctest (pruebas de diálogo y mapa: FAIL → implementar → PASS).
- [ ] **Step 2:** `--r12-paso=demo --autopiloto --rapido=10` y `--r12-paso=historia --autopiloto --rapido=10`. Expected: corren completos, sin `falta` en el log.
- [ ] **Step 3:** Commit `feat(round12) : cinemáticas con rostros, título, resultados y mapa de la historia`.

### Task 10: Auditoría de cobertura de recursos, README y verificación final

**Files:**
- Create: `tools/round12_cobertura.py`
- Modify: `ROUND12_Assets/README.md`: sección «Uso en el motor», en 10–15 líneas:
  - qué carga cada escena;
  - que `ROUND12_Sprites_Animaciones/` (solo Labs), `tilemaps/`, `manifest*.json` y los tiles sueltos (duplicados del atlas, verificados) no se cargan en R12 y por qué.

**`round12_cobertura.py`:** recorre cada archivo del paquete v3 y falla (código 1, con la lista) si alguno no está cubierto. Cuenta como cubierto:
- **Cuadros de personaje y de diálogo:** empacados en `motor/hojas`.
- **Retratos, rostros, objetos y capas de escenario:** su nombre o patrón aparece en `src/Round12` o en `Datos/`. Los patrones son `retratos/<emocion>`, la tabla de `archivoRostro` y `objetos/<n>` en `Escenarios.cpp`.
  - Los 30 rostros se cubren con la tabla: 5 vistas × 6 expresiones, producidas por `archivoRostro` y `archivoRostroOyente`. `frente` aparece en neutral y en resultados.
  - La hoja de rostros (`dialogo_sheet.png`) se cubre como lámina del menú «Controles» del título: se dibuja de fondo con alfa 40.
- **Tiles:** su índice aparece en `mapa_historia.txt` o en `playa_final.txt`.
- **Cuerdas:** las 5 piezas tienen uso en título, banner, pausa o ring.
- **Excluidos con motivo:** los que el README marca como no cargados.

Pasos:
- [ ] **Step 1:** `python3 -I tools/round12_cobertura.py`. Expected: `cobertura: 0 sin usar`.
- [ ] **Step 2:** Compilar Debug y Release → 0 warnings; `ctest -C Debug` y `-C Release` → PASS.
- [ ] **Step 3:** `--prueba-selector=40` en Debug y Release → `0`, sin `FALLA`.
- [ ] **Step 4:** Manual, con teclado, en Release:
  - lanzador → Labs: Round12, Round12Col y Camaras siguen como antes;
  - F1 → demo completo;
  - comprobar las cuerdas delante y Diego delante de las cuerdas fuera del ring;
  - comprobar que se ve el agachado Q+E;
  - comprobar que el diálogo con Carlos mueve la boca.
- [ ] **Step 5:** Revisión de toda la rama (`superpowers:requesting-code-review`) con el Review Focus de arriba; corregir lo crítico o importante.
- [ ] **Step 6:** Commit `docs(round12) : uso del paquete v3 y auditoría de cobertura`. Sin push ni PR hasta que el usuario lo pida.

## Verificación (resumen)

- Lógica: `cmake --build build --config Debug --target Round12Pruebas && ctest --test-dir build -C Debug --output-on-failure`.
- Recursos: `uv run --with pillow python -I tools/round12_hojas.py` y `python3 -I tools/round12_cobertura.py`.
- Compatibilidad: `./bin/{Debug,Release}/CimaEngine3v --prueba-selector=40` → 0.
- Juego: `--r12-paso=demo|historia --autopiloto --rapido=10`, y jugarlo con teclado (Task 10 Step 4).

## Fuera de alcance / supuestos

- Las posiciones (ring, estaciones, óvalo, piso de la playa, props) se miden sobre las imágenes y se ajustan con capturas; los valores del plan son el punto de partida.
- El balance no cambia, salvo la escala de alcances y velocidades por 32/9 y la regla de agacharse.
- No se arreglan los pendientes menores anteriores, salvo los que esta refactorización resuelve sola: agacharse en la playa, el rival que «cae» rotado y la falta de animaciones de golpe recibido.
- No se agregan teclas nuevas; Q+E juntas usa teclas existentes.

## Ejecución

Aquí mismo (inline), como en el plan anterior. Al final, una revisión independiente de toda la rama.
