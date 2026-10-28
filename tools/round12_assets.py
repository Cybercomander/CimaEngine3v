"""Prepara los assets de ROUND12 para CimaEngine.

1. Empaqueta los 192 PNG sueltos de cada personaje en UNA hoja (480x2560):
   columna = frame (0-5), fila = direccion*8 + accion (orden de abajo).
2. Convierte el tilemap del Sanchez Boxing GYM + su capa de colisiones al formato
   del motor ([Atlas_info] ... [ID_Matrix] con 'c'/'n' por tile).

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


def mapa_gym():
    from PIL import Image

    nivel = "01_sanchez_boxing_gym"
    cols, filas, tiles = leer_matriz(R12 / "tilemaps" / f"{nivel}.txt")
    c2, f2, col = leer_matriz(R12 / "colisiones" / f"{nivel}_colisiones.txt")
    assert (cols, filas) == (c2, f2), "tilemap y colisiones con distinto tamaño"
    with Image.open(R12 / "tilesets" / f"{nivel}.png") as atlas:
        aw, ah = atlas.size

    def borde(r, c):
        # solo los bloqueados que tocan suelo transitable: los del fondo nunca se alcanzan
        if col[r][c] == 0:
            return False
        vecinos = [(r - 1, c), (r + 1, c), (r, c - 1), (r, c + 1)]
        return any(0 <= rr < filas and 0 <= cc < cols and col[rr][cc] == 0 for rr, cc in vecinos)

    lineas = ["[Atlas_info]",
              f"/../ROUND12_Assets/tilesets/{nivel}.png",  # relativo a ASSETS
              f"{aw} {ah}", "8 8", f"{filas} {cols}", "[ID_Matrix]"]
    for r in range(filas):
        lineas.append(" ".join(f"{tiles[r][c]} {'c' if borde(r, c) else 'n'}" for c in range(cols)))
    destino = RAIZ / "assets" / "mapas" / "r12_sanchez_gym_col.txt"
    destino.write_text("\n".join(lineas) + "\n")
    n = sum(borde(r, c) for r in range(filas) for c in range(cols))
    print("mapa:", destino, f"({n} tiles con colision)")


if __name__ == "__main__":
    hojas()
    mapa_gym()
