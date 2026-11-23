# ROUND 12 - Animaciones de personajes (sin resto de assets)

Paquete de secuencias animadas para 6 personajes, creado a partir de los sprites originales del GDD.

## Estructura

`personajes/<nombre>/<direccion>/<accion>/frame_00.png` ... `frame_05.png`

Direcciones: `adelante`, `atras`, `derecha`, `izquierda`.

Acciones: `caminar`, `guardia`, `jab`, `recto`, `gancho`, `uppercut`, `esquiva_izquierda`, `esquiva_derecha`.

Cada secuencia tiene **6 cuadros separados**. Los movimientos se pueden interpolar mediante ciclos y los ataques se reproducen una sola vez.

## Diseño técnico

- PNG RGBA transparente, **80×80 píxeles por archivo**; tamaño uniforme para no recortar.
- Ancla común aproximada: `(40,70)` (suelo/centro del personaje).
- Dibujar escalado con nearest-neighbor, con resolución de pantalla 1080×720.
- **No son PNG de pantalla completa 1080×720**: esas dimensiones corresponden al viewport, no al sprite; un archivo 80×80 es mucho más eficiente.
- Sin cambio de identidad, color de guantes, cabello, shorts ni indumentaria. Se reutilizan las regiones originales de los personajes, y se crean cambios de extremidades para expresar movimiento, golpes y esquivas.
- La vista trasera y los cuadros intermedios no estaban en el GDD: son animaciones derivadas nuevas, no capturas originales.
- Para `izquierda` se reflejan los cuadros `derecha`. Desactiva inversión adicional en motor para evitar doble espejo.

## Particularidades

- Carlos Sánchez es un **NPC no combatiente** según el GDD. Sus movimientos de golpes son demostraciones de entrenamiento sin guantes, no significan que ahora sea un boxeador en la historia.
- Este es un **primer set procedural funcional**; los cuadros reutilizan segmentos de personajes y usan extremidades pixel art nuevas. La calidad final de combate deberá revisarse en juego en función de hitboxes, pivot, alcance y velocidad.
- El sistema de estados de tu motor debe combinar desplazamiento con las secuencias. Ningún PNG modifica posición del mundo: traslada al actor según la física y dibuja el cuadro actual.
- El archivo `animaciones.json` documenta nombres, duraciones recomendadas, direcciones y cantidad de cuadros.
