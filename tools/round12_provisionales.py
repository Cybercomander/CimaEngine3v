"""Genera arte y audio PROVISIONALES de ROUND 12 (se reemplazan con archivos
del mismo nombre sin tocar el código).

Uso (desde la raiz del repo):
    uv run --with pillow python tools/round12_provisionales.py
Salida: ROUND12_Assets/provisionales/{sprites,parallax,audio}
"""
import math
import random
import struct
import wave
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
SALIDA = RAIZ / "ROUND12_Assets" / "provisionales"
SR = 22050
NEGRO = (20, 20, 28, 255)


# ---------------- sprites ----------------
def lienzo(w, h):
    from PIL import Image
    return Image.new("RGBA", (w, h), (0, 0, 0, 0))


def rect(d, x0, y0, x1, y1, color):
    d.rectangle([x0, y0, x1, y1], fill=color, outline=NEGRO)


def sprites():
    from PIL import ImageDraw
    s = SALIDA / "sprites"
    s.mkdir(parents=True, exist_ok=True)
    hechos = {}

    def guardar(nombre, img):
        img.save(s / nombre)
        hechos[nombre] = img.size

    # ítems 16x16
    img = lienzo(16, 16); d = ImageDraw.Draw(img)
    rect(d, 6, 1, 9, 3, (40, 90, 200, 255)); rect(d, 5, 4, 10, 14, (150, 210, 255, 255))
    guardar("botella.png", img)
    img = lienzo(16, 16); d = ImageDraw.Draw(img)
    rect(d, 6, 1, 9, 3, (230, 230, 230, 255)); rect(d, 5, 4, 10, 14, (60, 220, 120, 255))
    d.line([5, 8, 10, 8], fill=(255, 255, 255, 255))
    guardar("isotonica.png", img)
    img = lienzo(16, 16); d = ImageDraw.Draw(img)
    d.arc([2, 2, 14, 16], 200, 340, fill=(250, 220, 60, 255), width=4)
    guardar("platano.png", img)
    img = lienzo(16, 16); d = ImageDraw.Draw(img)
    rect(d, 2, 3, 13, 12, (245, 245, 245, 255)); d.line([2, 6, 13, 6], fill=(60, 160, 80, 255))
    guardar("toalla.png", img)

    # gimnasio
    img = lienzo(48, 40); d = ImageDraw.Draw(img)
    for i, dx in enumerate((0, 2, -2)):
        x = i * 16
        d.line([x + 8, 0, x + 8 + dx, 4], fill=(150, 150, 150, 255))
        rect(d, x + 3 + dx, 4, x + 12 + dx, 38, (170, 60, 40, 255))
        d.line([x + 3 + dx, 12, x + 12 + dx, 12], fill=NEGRO)
    guardar("costal.png", img)
    img = lienzo(16, 16); d = ImageDraw.Draw(img)
    d.ellipse([2, 2, 13, 13], fill=(230, 60, 50, 255), outline=NEGRO); d.ellipse([5, 5, 10, 10], fill=(250, 250, 250, 255))
    guardar("manoplas.png", img)
    img = lienzo(128, 16); d = ImageDraw.Draw(img)
    for f in range(4):
        y = (2, 8, 14, 8)[f]
        d.line([f * 32 + 1, 8, f * 32 + 16, y, f * 32 + 30, 8], fill=(240, 200, 60, 255), width=1)
    guardar("cuerda.png", img)
    img = lienzo(16, 24); d = ImageDraw.Draw(img)
    rect(d, 2, 4, 13, 18, (120, 190, 255, 230)); rect(d, 4, 19, 11, 23, (90, 90, 90, 255)); rect(d, 6, 0, 9, 3, (60, 120, 220, 255))
    guardar("garrafon.png", img)

    # playa
    img = lienzo(16, 16); d = ImageDraw.Draw(img)
    rect(d, 2, 8, 13, 15, (230, 200, 140, 255)); rect(d, 5, 3, 10, 8, (230, 200, 140, 255))
    guardar("castillo.png", img)
    img = lienzo(16, 16); d = ImageDraw.Draw(img)
    rect(d, 1, 5, 14, 15, (40, 120, 220, 255)); rect(d, 1, 3, 14, 6, (245, 245, 245, 255))
    guardar("hielera.png", img)
    img = lienzo(16, 16); d = ImageDraw.Draw(img)
    d.polygon([(0, 12), (8, 6), (16, 12)], fill=(240, 80, 80, 255), outline=NEGRO); d.line([2, 13, 14, 15], fill=(120, 80, 40, 255))
    guardar("sombrilla.png", img)
    img = lienzo(16, 16); d = ImageDraw.Draw(img)
    d.ellipse([3, 3, 12, 12], fill=(250, 250, 250, 255), outline=NEGRO); d.pieslice([3, 3, 12, 12], 0, 120, fill=(230, 60, 60, 255))
    guardar("pelota.png", img)
    img = lienzo(32, 16); d = ImageDraw.Draw(img)
    for f in range(2):
        x = f * 16
        rect(d, x + 2, 6, x + 12, 11, (150, 100, 60, 255)); rect(d, x + 11, 3, x + 15, 8, (150, 100, 60, 255))
        for p in ((3, 4) if f == 0 else (5, 2)):
            d.line([x + p, 12, x + p, 15], fill=NEGRO)
            d.line([x + p + 7, 12, x + p + 7, 15], fill=NEGRO)
    guardar("perro.png", img)
    img = lienzo(32, 16); d = ImageDraw.Draw(img)
    for f in range(2):
        x = f * 16
        d.ellipse([x + 4, 8, x + 12, 14], fill=(230, 80, 50, 255), outline=NEGRO)
        a = 6 if f == 0 else 4
        d.line([x + 1, a, x + 4, 10], fill=(230, 80, 50, 255)); d.line([x + 15, a, x + 12, 10], fill=(230, 80, 50, 255))
    guardar("cangrejo.png", img)
    img = lienzo(64, 16); d = ImageDraw.Draw(img)
    for x in range(64):
        y = int(8 + 4 * math.sin(x / 64 * math.tau * 2))
        d.line([x, y, x, 15], fill=(80, 170, 230, 220))
        d.point((x, y), fill=(250, 250, 250, 255))
    guardar("ola.png", img)
    img = lienzo(48, 8); d = ImageDraw.Draw(img)
    d.ellipse([0, 0, 47, 7], fill=(0, 0, 0, 128))
    guardar("sombra.png", img)
    img = lienzo(24, 32); d = ImageDraw.Draw(img)
    rect(d, 2, 14, 21, 31, (210, 170, 110, 255)); d.polygon([(0, 14), (12, 2), (23, 14)], fill=(60, 140, 220, 255), outline=NEGRO)
    rect(d, 9, 18, 14, 26, (150, 210, 255, 255))
    guardar("puesto_agua.png", img)
    img = lienzo(64, 48); d = ImageDraw.Draw(img)
    rect(d, 0, 10, 63, 16, (140, 100, 70, 255))
    for x in range(4, 64, 14):
        rect(d, x, 16, x + 3, 47, (110, 80, 55, 255))
    guardar("muelle.png", img)

    # interfaz
    img = lienzo(64, 16); d = ImageDraw.Draw(img)
    colores = [(240, 240, 240, 255), (120, 200, 255, 255), (255, 170, 60, 255), (255, 90, 90, 255)]
    for i in range(4):
        x = i * 16
        rect(d, x + 1, 1, x + 14, 14, colores[i])
        # dígito 1-4 en una retícula 3x5
        DIG = {1: ["010", "110", "010", "010", "111"], 2: ["111", "001", "111", "100", "111"],
               3: ["111", "001", "011", "001", "111"], 4: ["101", "101", "111", "001", "001"]}
        for r, fila in enumerate(DIG[i + 1]):
            for c, v in enumerate(fila):
                if v == "1":
                    d.rectangle([x + 5 + c * 2, 3 + r * 2, x + 6 + c * 2, 4 + r * 2], fill=NEGRO)
    guardar("iconos_golpes.png", img)
    img = lienzo(288, 192)
    px = img.load()
    for y in range(192):
        for x in range(288):
            dx, dy = (x - 144) / 144, (y - 96) / 96
            r = math.sqrt(dx * dx + dy * dy)
            a = max(0.0, min(1.0, (r - 0.55) / 0.5))
            px[x, y] = (0, 0, 0, int(a * 255))
    guardar("vineta.png", img)
    img = lienzo(576, 32); d = ImageDraw.Draw(img)
    rnd = random.Random(12)
    for f in range(2):
        for x in range(0, 288, 8):
            c = rnd.choice([(200, 70, 60), (60, 90, 170), (220, 190, 70), (90, 150, 90)])
            y = 10 + (2 if (x // 8 + f) % 2 else 0)
            d.ellipse([f * 288 + x + 2, y - 6, f * 288 + x + 6, y - 2], fill=(200, 150, 110, 255))
            d.rectangle([f * 288 + x + 1, y, f * 288 + x + 7, 31], fill=c + (255,))
    guardar("publico.png", img)
    return hechos


def parallax():
    from PIL import ImageDraw
    p = SALIDA / "parallax"
    p.mkdir(parents=True, exist_ok=True)
    hechos = {}
    img = lienzo(288, 192); d = ImageDraw.Draw(img)
    for y in range(192):
        t = y / 191
        c = (int(110 + 140 * t), int(60 + 70 * t), int(140 - 60 * t), 255)  # morado -> naranja
        d.line([0, y, 287, y], fill=c)
    d.ellipse([200, 90, 240, 130], fill=(255, 210, 120, 255))
    img.save(p / "cielo.png"); hechos["cielo.png"] = img.size
    img = lienzo(288, 192); d = ImageDraw.Draw(img)
    d.rectangle([0, 112, 287, 140], fill=(50, 90, 160, 255))
    for x in range(0, 288, 12):
        d.line([x, 116 + (x // 12) % 3 * 6, x + 5, 116 + (x // 12) % 3 * 6], fill=(170, 200, 240, 255))
    img.save(p / "mar.png"); hechos["mar.png"] = img.size
    img = lienzo(288, 192); d = ImageDraw.Draw(img)
    d.polygon([(0, 112), (40, 84), (90, 104), (150, 76), (210, 100), (288, 86), (288, 112)], fill=(90, 60, 100, 255))
    img.save(p / "cerros.png"); hechos["cerros.png"] = img.size
    img = lienzo(288, 192); d = ImageDraw.Draw(img)
    for x in (30, 130, 230):
        d.line([x, 140, x + 6, 80], fill=(90, 60, 40, 255), width=3)
        for a in range(0, 360, 60):
            d.line([x + 6, 80, x + 6 + 18 * math.cos(math.radians(a)), 80 + 10 * math.sin(math.radians(a))], fill=(40, 120, 60, 255), width=3)
    img.save(p / "palmeras.png"); hechos["palmeras.png"] = img.size
    return hechos


# ---------------- audio ----------------
def nota(n):
    return 440.0 * 2 ** ((n - 69) / 12)


def escribir(nombre, muestras):
    ruta = SALIDA / "audio" / nombre
    with wave.open(str(ruta), "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(SR)
        w.writeframes(b"".join(struct.pack("<h", int(max(-1, min(1, m)) * 12000)) for m in muestras))
    return ruta


def cuadrada(f, dur, vol=0.5, duty=0.5, caida=0.0):
    n = int(SR * dur)
    out = []
    for i in range(n):
        if f <= 0:
            out.append(0.0); continue
        fase = (i * f / SR) % 1.0
        env = vol * (1 - caida * i / n)
        out.append(env if fase < duty else -env)
    return out


def ruido(dur, vol=0.5, caida=1.0, semilla=1):
    r = random.Random(semilla)
    n = int(SR * dur)
    return [vol * (1 - caida * i / n) * (r.random() * 2 - 1) for i in range(n)]


def mezclar(*pistas):
    n = max(len(p) for p in pistas)
    return [sum(p[i] for p in pistas if i < len(p)) / len(pistas) * 1.6 for i in range(n)]


def tema(bpm, compases, raiz, escala, semilla):
    """trap latino en 8 bits muy simple: bajo, melodía y hi-hat en semicorcheas"""
    r = random.Random(semilla)
    negra = 60.0 / bpm
    bajo, mel, hat = [], [], []
    for c in range(compases):
        grado = [0, 5, 3, 4][c % 4]
        for b in range(4):
            bajo += cuadrada(nota(raiz - 12 + escala[grado % len(escala)]), negra, 0.6, 0.5, 0.6)
        for s in range(8):
            n = raiz + 12 + escala[(grado + r.choice([0, 2, 4, 7])) % len(escala)]
            mel += cuadrada(nota(n) if r.random() > 0.25 else 0, negra / 2, 0.35, 0.25, 0.4)
        for s in range(16):
            hat += ruido(negra / 4, 0.25 if s % 4 else 0.45, 1.0, semilla + s)
    return mezclar(bajo, mel, hat)


def audio():
    (SALIDA / "audio").mkdir(parents=True, exist_ok=True)
    menor = [0, 2, 3, 5, 7, 8, 10]
    hechos = {}
    temas = {"musica_menu.wav": (100, 8, 57, 1), "musica_gym.wav": (100, 8, 57, 2),
             "musica_playa.wav": (130, 12, 62, 3), "musica_pelea.wav": (140, 12, 55, 4)}
    for nombre, (bpm, comp, raiz, sem) in temas.items():
        escribir(nombre, tema(bpm, comp, raiz, menor, sem)); hechos[nombre] = 60 / bpm * 4 * comp
    # cuerda: 120 BPM exactos, golpe de bombo justo en cada pulso (t = 0, 0.5, 1.0 ...)
    pulso = []
    for i in range(32):
        golpe = cuadrada(nota(36), 0.08, 0.9, 0.5, 1.0) + [0.0] * int(SR * 0.42)
        pulso += golpe
    mel = tema(120, 8, 60, menor, 5)
    escribir("musica_cuerda.wav", mezclar(pulso, mel[:len(pulso)])); hechos["musica_cuerda.wav"] = 16.0
    efectos = {
        "campana.wav": mezclar(cuadrada(nota(88), 0.8, 0.6, 0.5, 1.0), cuadrada(nota(95), 0.8, 0.4, 0.5, 1.0)),
        "golpe_suave.wav": ruido(0.08, 0.6, 1.0, 7),
        "golpe_fuerte.wav": mezclar(ruido(0.18, 0.9, 1.0, 8), cuadrada(nota(40), 0.18, 0.8, 0.5, 1.0)),
        "bloqueo.wav": cuadrada(nota(52), 0.06, 0.6, 0.5, 1.0),
        "caida.wav": mezclar(ruido(0.4, 0.8, 1.0, 9), cuadrada(nota(31), 0.4, 0.8, 0.5, 1.0)),
        "publico.wav": ruido(1.5, 0.35, 0.2, 10),
        "beat.wav": cuadrada(nota(81), 0.05, 0.5, 0.5, 1.0),
        "menu.wav": cuadrada(nota(76), 0.06, 0.4, 0.5, 1.0),
    }
    for nombre, m in efectos.items():
        escribir(nombre, m); hechos[nombre] = len(m) / SR
    return hechos


def verificar(sprites_h, parallax_h, audio_h):
    from PIL import Image
    esperado = {"botella.png": (16, 16), "isotonica.png": (16, 16), "platano.png": (16, 16),
                "toalla.png": (16, 16), "costal.png": (48, 40), "manoplas.png": (16, 16),
                "cuerda.png": (128, 16), "garrafon.png": (16, 24), "castillo.png": (16, 16),
                "hielera.png": (16, 16), "sombrilla.png": (16, 16), "pelota.png": (16, 16),
                "perro.png": (32, 16), "cangrejo.png": (32, 16), "ola.png": (64, 16),
                "sombra.png": (48, 8), "puesto_agua.png": (24, 32), "muelle.png": (64, 48),
                "iconos_golpes.png": (64, 16), "vineta.png": (288, 192), "publico.png": (576, 32)}
    for nombre, tam in esperado.items():
        with Image.open(SALIDA / "sprites" / nombre) as img:
            assert img.size == tam, f"{nombre} mide {img.size}, se esperaba {tam}"
    for nombre in ("cielo.png", "mar.png", "cerros.png", "palmeras.png"):
        with Image.open(SALIDA / "parallax" / nombre) as img:
            assert img.size == (288, 192), nombre
    for nombre in audio_h:
        with wave.open(str(SALIDA / "audio" / nombre)) as w:
            assert w.getframerate() == SR and w.getnframes() > 0, nombre
    with wave.open(str(SALIDA / "audio" / "musica_cuerda.wav")) as w:
        assert abs(w.getnframes() / SR - 16.0) < 0.01, "la cuerda debe durar 32 pulsos de 0.5 s"
    print(f"provisionales: {len(sprites_h)} sprites, {len(parallax_h)} parallax, {len(audio_h)} audios")


if __name__ == "__main__":
    verificar(sprites(), parallax(), audio())
