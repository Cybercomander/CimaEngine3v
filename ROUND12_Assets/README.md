# ROUND 12 - Paquete inicial de recursos 2D de baja resolución

**Fuente de diseño:** documento GDD «Avance 2: Concepto, Personajes, Historia y Temática».
**Resolución del juego:** 320 × 180 píxeles. **Tiles:** 8 × 8 píxeles. **Filtro:** nearest-neighbor (sin interpolación).

## Contenido

- `sprites/personajes/`: personajes en PNG con transparencia; cada combatiente incluye su hoja original de **4 poses** y archivos individuales. Los cinco combatientes están **extraídos de las figuras originales del GDD** y reducidos 5:1 con nearest-neighbor, no reinterpretados. Dos versiones en dirección contraria de los cuadros laterales (espejo horizontal).
- `sprites/personajes/carlos_sanchez/`: entrenador NPC, arte nuevo de 5 poses según su descripción en el GDD.
- `tilesets/`: atlas PNG paletizados a **8×8**, 16 tiles por fila.
- `tilemaps/`: seis mapas TXT de índices de tile, primer renglón ancho y alto en celdas.
- `colisiones/`: capas de colisión de arranque asociadas 1:1 a cada mapa (leyenda abajo).
- `previews/`: reconstrucción exacta del tilemap a resolución nativa y vista con escala nearest-neighbor para inspección. No necesita cargarse en el videojuego.
- `manifest.json`: geometría, nombres, rutas, poses, sentidos de desplazamiento, puntos iniciales ilustrativos y convenciones.

## Formato de tilemaps TXT

Ejemplo:

```
4 2
0 1 1 2
3 3 2 2
```

La primera fila indica `columnas filas`. Cada celda siguiente contiene un **índice decimal de base cero**. Para buscar el tile `(u,v)` dentro del PNG atlas:

```
u = (indice % 16) * 8
v = (indice / 16) * 8
```

El tilemap y su tileset usan los mismos índices. La celda del mundo `(columna,fila)` se dibuja en `(columna*8, fila*8)`. La cámara puede desplazarse en horizontal o vertical (ver `manifest.json`).

## Colisiones TXT

Igual formato rectangular, pero los valores significan:

```
0 = superficie transitable
1 = bloqueado / fuera de recorrido
2 = obstáculo (salto y temporización, prototipo)
```

La colisión es **una base de implementación, pendiente de ajustar al sistema de físicas de tu motor**. El ring y las carreras requerirán reglas de movimiento específicas, así como las rampas y escalones de Santa Fe.

## Hojas de sprites y direcciones

- Los boxeadores tienen **4 poses**, según las Figuras 1–5 del GDD: frente firme, frente guardia, lateral firme y lateral jab.
- Los tamaños de cuadro conservan las diferencias visuales originales. `44×52` para Diego; `44×56` para los adolescentes (Javier y Kevin contienen un margen transparente adicional para respetar el tamaño del GDD); entrenador `44×60` (tamaño propuesto, no especificado en el GDD).
- `spritesheet_4poses.png`: una fila de cuatro cuadros de **44 píxeles** cada uno.
- `spritesheet_izquierda.png`: cuadro lateral y jab mirando hacia la izquierda, derivados por espejo.
- `frames.txt`: `indice nombre x y ancho alto`, con origen superior izquierdo en pixeles.

**Importante:** los cuatro cuadros del documento son **prototipos de poses**, no una animación completa de combate. Faltan por dibujar cuadros intermedios para recto, gancho, uppercut, bloqueo, esquiva, desplazamiento, recibir golpe y caída para construir una animación fluida. No se han inventado animaciones que no estuvieran representadas en el GDD.

## Estilo y geografía

2024 en la historia; imagen pixel-art de los años 80. Los seis ambientes corresponden a lo indicado: Sanchez Boxing GYM (Tijuana, verde/negro), gimnasios regionales, Gimnasio de Box de Mexicali (torneo), Santa Fe (Tijuana, recorrido vertical), playa de Rosarito y circuito atlético de Rosarito. Son **interpretaciones jugables y originales** de la región, no reproducciones arquitectónicas verificadas de sedes reales.

## Uso y optimización en C++

- Carga una vez el tileset y dibuja solo los tiles visibles (culling usando límites de cámara).
- Guarda índices enteros compactos (`uint16_t` si hay más de 255 tiles, `uint8_t` si no).
- El PNG del tileset ya contiene solo **una copia de cada tile único** en el nivel.
- Los sprites usan transparencia y tamaño nativo; desactiva antialiasing y usa escala entera en el render.
- **Atención a la relación de aspecto:** 320×180 (16:9) se escala sin deformación a 1280×720 (4×) o 1920×1080 (6×). No se puede llevar de forma uniforme a **1080×720** (3:2) sin barras o deformación; el GDD menciona ambas resoluciones, pero son proporciones distintas.

**Actualización solicitada:** mantener el mismo arte, separar cada pose en archivos PNG individuales y preparar recursos compatibles con un flujo de trabajo a **1080×720**.

# ACTUALIZACIÓN 1080X720
## Qué cambió

1. **Sprites individuales por pose**
   - Se conservaron las ilustraciones exactamente como estaban.
   - Cada pose está disponible por separado en:
     - `sprites_individuales_nativos/` → PNG individual en su tamaño original optimizado.
     - `sprites_individuales_uniformes/` → PNG individual sobre un canvas uniforme por personaje (útil para animación sin recortar).
     - `sprites_individuales_1080x720/` → PNG individual centrado en lienzo transparente de 1080×720.

2. **Escenarios a 1080×720**
   - Se añadieron versiones en `escenarios_1080x720/`.
   - Cada escenario fue escalado con **nearest-neighbor** y centrado en un lienzo exacto de 1080×720.
   - **No se deformó ni redibujó** la ilustración. Cuando sobró espacio, se rellenó con color de fondo del propio escenario.

3. **Se conservó el paquete base**
   - Tilesets, tilemaps TXT, colisiones y spritesheets originales se mantienen para que sigas usando el motor en modo optimizado.

## Recomendación de uso en C++

- Para animación: usa `sprites_individuales_uniformes/`.
- Si prefieres cargar assets ya adaptados a resolución fija: usa `sprites_individuales_1080x720/` y `escenarios_1080x720/`.
- Si tu prioridad es rendimiento y memoria: usa `sprites_individuales_nativos/` + `tilesets/` + `tilemaps/`.

## Nota sobre 1080×720

1080×720 es una relación **3:2**. El arte original del proyecto y varias vistas previas partían de una lógica retro cercana a **16:9**. Para respetar tu instrucción de **no cambiar la ilustración**, los escenarios se adaptaron a 1080×720 **sin estiramiento**.