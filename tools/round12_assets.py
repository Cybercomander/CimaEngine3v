"""Prepara los assets de ROUND12 para CimaEngine.

1. Empaqueta los 192 PNG sueltos de cada personaje en UNA hoja (480x2560):
   columna = frame (0-5), fila = direccion*8 + accion (orden de abajo).
3. Convierte los 6 tilemaps a ROUND12_Assets/motor/ (ROUND 12 en src/Round12).
2. Convierte el tilemap del Sanchez Boxing GYM al formato del motor
   ([Atlas_info] ... [ID_Matrix] con 'c'/'n' por tile). La colisión es el ring:
   un marco de tiles 'c' (cuerdas a los lados, límites de carril al fondo y al
   frente) pensado para cajas de colisión del cuerpo completo (cabeza a pies).

Uso (desde la raiz del repo):
    uv run --with pillow python tools/round12_assets.py
"""
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
R12 = RAIZ / "ROUND12_Assets"
ANIM = R12 / "ROUND12_Sprites_Animaciones"

# mismo orden que usa Juego/Maquinas/Boxeadores/NodosBoxeo.hpp
DIRECCIONES = ["adelante", "atras", "derecha", "izquierda"]
ACCIONES = ["caminar", "guardia", "jab", "recto", "gancho", "uppercut",
            "esquiva_izquierda", "esquiva_derecha"]
FRAMES = 6
TAM = 80


def hojas():
    from PIL import Image

    salida = ANIM / "hojas"
    salida.mkdir(exist_ok=True)
    for personaje in sorted(p for p in (ANIM / "personajes").iterdir() if p.is_dir()):
        hoja = Image.new("RGBA", (FRAMES * TAM, len(DIRECCIONES) * len(ACCIONES) * TAM))
        for d, direccion in enumerate(DIRECCIONES):
            for a, accion in enumerate(ACCIONES):
                fila = d * len(ACCIONES) + a
                for f in range(FRAMES):
                    png = personaje / direccion / accion / f"frame_{f:02d}.png"
                    with Image.open(png) as img:
                        assert img.size == (TAM, TAM), f"{png} no mide {TAM}x{TAM}"
                        hoja.paste(img.convert("RGBA"), (f * TAM, fila * TAM))
        hoja.save(salida / f"{personaje.name}.png")
        print("hoja:", salida / f"{personaje.name}.png", hoja.size)


def leer_matriz(ruta):
    tokens = ruta.read_text().split()
    cols, filas = int(tokens[0]), int(tokens[1])
    valores = [int(t) for t in tokens[2:]]
    assert len(valores) == cols * filas, f"{ruta}: se esperaban {cols*filas} celdas"
    return cols, filas, [valores[r * cols:(r + 1) * cols] for r in range(filas)]


# Ring del Sanchez Boxing GYM en tiles de 8px (medido sobre previews/..._nativo.png):
# postes en x=48-56 y x=584-592, lona del ring en y=151-159.
RING_COL_IZQ, RING_COL_DER = 6, 73   # columnas de los postes: paredes laterales
RING_FILA_FONDO = 11                 # y=88-96: con cuerpo de 52px los pies quedan en y>=148
RING_FILA_FRENTE = 20                # y=160-168: los pies no bajan de la lona (y<=160)


def mapa_gym():
    from PIL import Image

    nivel = "01_sanchez_boxing_gym"
    cols, filas, tiles = leer_matriz(R12 / "tilemaps" / f"{nivel}.txt")
    with Image.open(R12 / "tilesets" / f"{nivel}.png") as atlas:
        aw, ah = atlas.size

    def es_ring(r, c):
        dentro_x = RING_COL_IZQ <= c <= RING_COL_DER
        dentro_y = RING_FILA_FONDO <= r <= RING_FILA_FRENTE
        return (dentro_x and r in (RING_FILA_FONDO, RING_FILA_FRENTE)) or \
               (dentro_y and c in (RING_COL_IZQ, RING_COL_DER))

    lineas = ["[Atlas_info]",
              f"/../ROUND12_Assets/tilesets/{nivel}.png",  # relativo a ASSETS
              f"{aw} {ah}", "8 8", f"{filas} {cols}", "[ID_Matrix]"]
    for r in range(filas):
        lineas.append(" ".join(f"{tiles[r][c]} {'c' if es_ring(r, c) else 'n'}" for c in range(cols)))
    destino = RAIZ / "assets" / "mapas" / "r12_sanchez_gym_col.txt"
    destino.write_text("\n".join(lineas) + "\n")
    n = sum(es_ring(r, c) for r in range(filas) for c in range(cols))
    print("mapa:", destino, f"({n} tiles con colision)")


def mapas_motor():
    """Los 6 tilemaps de ROUND12_Assets al formato del motor (todos 'n'):
    src/Round12 lee la colisión directo de colisiones/*.txt (MapaColision)."""
    from PIL import Image

    salida = R12 / "motor"
    salida.mkdir(exist_ok=True)
    for ruta in sorted((R12 / "tilemaps").glob("*.txt")):
        cols, filas, tiles = leer_matriz(ruta)
        with Image.open(R12 / "tilesets" / f"{ruta.stem}.png") as atlas:
            aw, ah = atlas.size
        lineas = ["[Atlas_info]", f"/../ROUND12_Assets/tilesets/{ruta.stem}.png",
                  f"{aw} {ah}", "8 8", f"{filas} {cols}", "[ID_Matrix]"]
        lineas += [" ".join(f"{t} n" for t in fila) for fila in tiles]
        (salida / ruta.name).write_text("\n".join(lineas) + "\n")
        print("motor:", salida / ruta.name)
    assert len(list(salida.glob("*.txt"))) == 6


if __name__ == "__main__":
    hojas()
    mapa_gym()
    mapas_motor()
