"""Prepara los assets de ROUND12 para CimaEngine.

1. Empaqueta los 192 PNG sueltos de cada personaje en UNA hoja (480x2560):
   columna = frame (0-5), fila = direccion*8 + accion (orden de abajo).
   Las usa Labs (Escena_Round12Col). ROUND 12 usa tools/round12_hojas.py.

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


if __name__ == "__main__":
    hojas()
