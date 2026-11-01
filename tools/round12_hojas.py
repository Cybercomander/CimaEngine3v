"""Empaca y verifica el paquete de recursos ROUND12 v3 para el motor.

Salida en ROUND12_Assets/motor/hojas/:
  acciones.txt            39 acciones en orden alfabético (renglón n = fila n).
  <personaje>.png         1920x3120: columna = dir*6 + cuadro, fila = acción.
  <personaje>_dialogo.png 1280x640:  columna = dir*4 + cuadro, fila = emoción.
dir en orden adelante, atras, derecha, izquierda (= CE::IDireccion).

Además revisa el paquete: tamaños de cuadro, que el atlas isométrico coincida
con los tiles sueltos, que primer_plano == cuerdas_frontales y los objetos.

Uso (desde la raíz del repo):
    uv run --with pillow python -I tools/round12_hojas.py
"""
import json
from pathlib import Path

from PIL import Image, ImageChops

RAIZ = Path(__file__).resolve().parent.parent
R12 = RAIZ / "ROUND12_Assets"
PERS = R12 / "sprites" / "personajes"
SALIDA = R12 / "motor" / "hojas"

DIRECCIONES = ["adelante", "atras", "derecha", "izquierda"]
EMOCIONES = ["cansado", "celebrando", "decidido", "escuchando",
             "hablando", "neutral", "sonriendo", "sorprendido"]
PERSONAJES = ["diego_ureta", "brandon_salazar", "javier_castaneda",
              "kevin_ramirez", "ricardo_oropeza", "carlos_sanchez"]
TAM = 80


def cuadro(png):
    img = Image.open(png).convert("RGBA")
    assert img.size == (TAM, TAM), f"{png} mide {img.size}"
    return img


def hojas():
    acciones = sorted(p.name for p in (PERS / "diego_ureta" / "derecha").iterdir() if p.is_dir())
    assert len(acciones) == 39, f"se esperaban 39 acciones, hay {len(acciones)}"
    SALIDA.mkdir(parents=True, exist_ok=True)
    (SALIDA / "acciones.txt").write_text("\n".join(acciones) + "\n")
    for p in PERSONAJES:
        hoja = Image.new("RGBA", (len(DIRECCIONES) * 6 * TAM, len(acciones) * TAM))
        for d, direccion in enumerate(DIRECCIONES):
            propias = sorted(x.name for x in (PERS / p / direccion).iterdir() if x.is_dir())
            assert propias == acciones, f"{p}/{direccion} no tiene las mismas acciones"
            for a, accion in enumerate(acciones):
                for f in range(6):
                    hoja.paste(cuadro(PERS / p / direccion / accion / f"frame_{f:02d}.png"),
                               ((d * 6 + f) * TAM, a * TAM))
        hoja.save(SALIDA / f"{p}.png")
        dialogo = Image.new("RGBA", (len(DIRECCIONES) * 4 * TAM, len(EMOCIONES) * TAM))
        for d, direccion in enumerate(DIRECCIONES):
            for e, emocion in enumerate(EMOCIONES):
                for f in range(4):
                    dialogo.paste(cuadro(PERS / p / "dialogo" / direccion / emocion / f"frame_{f:02d}.png"),
                                  ((d * 4 + f) * TAM, e * TAM))
        dialogo.save(SALIDA / f"{p}_dialogo.png")
        for e in EMOCIONES:
            assert Image.open(PERS / p / "retratos" / f"{e}.png").size == (96, 96), f"{p} retrato {e}"
        print("hojas:", p, hoja.size, dialogo.size)
    return acciones


def verificar_paquete():
    tiles = json.loads((R12 / "tilesets" / "tiles.json").read_text())
    atlas = Image.open(R12 / "tilesets" / "atlas_isometrico_64x48.png").convert("RGBA")
    for i, t in tiles["indices"].items():
        i = int(i)
        celda = atlas.crop(((i % 8) * 64, (i // 8) * 48, (i % 8) * 64 + 64, (i // 8) * 48 + 48))
        suelto = Image.open(R12 / "tilesets" / t["file"]).convert("RGBA")
        assert ImageChops.difference(celda, suelto).getbbox() is None, f"tile {i} difiere del atlas"
    assert len(tiles["indices"]) == 32
    for e in ("01_sanchez_boxing_gym_tijuana", "02_gimnasio_visitante", "03_gimnasio_mexicali_torneo"):
        d = R12 / "escenarios" / e
        assert (d / "primer_plano_1920x1080.png").read_bytes() == (d / "cuerdas_frontales_1920x1080.png").read_bytes(), e
    objetos = json.loads((R12 / "objetos" / "objetos.json").read_text())
    for n in objetos["nombres"]:
        assert Image.open(R12 / "objetos" / f"{n}.png").size == (96, 128), n
    for p in ("diego_ureta", "carlos_sanchez"):
        rostros = list((PERS / p / "dialogo_rostros").glob("*_dialogo_*_*.png"))
        assert len([r for r in rostros if "sheet" not in r.name]) == 30, p
    print("paquete v3 verificado: 32 tiles, 3 cuerdas, %d objetos, 60 rostros" % len(objetos["nombres"]))


if __name__ == "__main__":
    hojas()
    verificar_paquete()
