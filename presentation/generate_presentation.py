from pathlib import Path
from PIL import Image
from pptx import Presentation
from pptx.dml.color import RGBColor
from pptx.enum.shapes import MSO_SHAPE, MSO_CONNECTOR
from pptx.enum.text import PP_ALIGN, MSO_ANCHOR
from pptx.enum.dml import MSO_LINE
from pptx.util import Inches, Pt


ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "presentation" / "assets"
OUTPUT = ROOT / "presentation" / "output"
OUTPUT.mkdir(parents=True, exist_ok=True)
WORK = ROOT / "presentation" / ".presentation-work"
WORK.mkdir(parents=True, exist_ok=True)

NAVY = RGBColor(0x17, 0x36, 0x5D)
BLUE = RGBColor(0x26, 0x76, 0xC9)
TEAL = RGBColor(0x16, 0xA8, 0x9A)
TEXT = RGBColor(0x25, 0x34, 0x49)
MUTED = RGBColor(0x5B, 0x6B, 0x7F)
PALE = RGBColor(0xF5, 0xF8, 0xFC)
WHITE = RGBColor(0xFF, 0xFF, 0xFF)
LIGHT_BLUE = RGBColor(0xE8, 0xF1, 0xFA)
LIGHT_TEAL = RGBColor(0xE5, 0xF6, 0xF3)
LINE = RGBColor(0xD5, 0xE0, 0xEC)
TOTAL_SLIDES = 9

prs = Presentation()
prs.slide_width = Inches(13.333)
prs.slide_height = Inches(7.5)
blank = prs.slide_layouts[6]


def box(slide, x, y, w, h, fill=WHITE, line=LINE, radius=True):
    shape = slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE if radius else MSO_SHAPE.RECTANGLE,
                                   Inches(x), Inches(y), Inches(w), Inches(h))
    shape.fill.solid()
    shape.fill.fore_color.rgb = fill
    shape.line.color.rgb = line
    shape.line.width = Pt(1)
    return shape


def text(slide, value, x, y, w, h, size=18, color=TEXT, bold=False,
         align=PP_ALIGN.LEFT, font="Aptos", valign=MSO_ANCHOR.TOP):
    shape = slide.shapes.add_textbox(Inches(x), Inches(y), Inches(w), Inches(h))
    tf = shape.text_frame
    tf.clear()
    tf.word_wrap = True
    tf.margin_left = Inches(0.05)
    tf.margin_right = Inches(0.05)
    tf.margin_top = Inches(0.03)
    tf.margin_bottom = Inches(0.02)
    tf.vertical_anchor = valign
    p = tf.paragraphs[0]
    p.alignment = align
    run = p.add_run()
    run.text = value
    run.font.name = font
    run.font.size = Pt(size)
    run.font.bold = bold
    run.font.color.rgb = color
    return shape


def title(slide, value, kicker=None):
    if kicker:
        text(slide, kicker.upper(), 0.65, 0.22, 4.4, 0.30, 10, TEAL, True)
    text(slide, value, 0.65, 0.55, 12.0, 0.55, 27, NAVY, True)
    line = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0.65), Inches(1.18), Inches(1.05), Inches(0.05))
    line.fill.solid(); line.fill.fore_color.rgb = TEAL; line.line.fill.background()


def footer(slide, n, label="SocketHub-C"):
    text(slide, label, 0.65, 7.15, 2.0, 0.16, 8, MUTED, False)
    text(slide, f"{n:02d} / {TOTAL_SLIDES:02d}", 12.15, 7.15, 0.55, 0.16, 8, MUTED, False, PP_ALIGN.RIGHT)


def add_image(slide, filename, x, y, w, h, caption):
    path = prepare_asset(filename)
    image = Image.open(path)
    ratio = image.width / image.height
    target = w / h
    if ratio > target:
        draw_w, draw_h = w, w / ratio
    else:
        draw_w, draw_h = h * ratio, h
    draw_x, draw_y = x + (w - draw_w) / 2, y + (h - draw_h) / 2
    slide.shapes.add_picture(str(path), Inches(draw_x), Inches(draw_y), width=Inches(draw_w), height=Inches(draw_h))
    text(slide, caption, x, y + h + 0.05, w, 0.28, 10, MUTED, False, PP_ALIGN.CENTER)


def prepare_asset(filename):
    """Create a local cropped working copy while preserving the original captures."""
    source = ASSETS / filename
    output = WORK / filename.replace("/", "_")
    image = Image.open(source).convert("RGB")
    w, h = image.size
    if filename == "screenshots/01_compilation.png":
        image = image.crop((0, int(h * 0.38), w, h))
    elif filename == "screenshots/02_chat_tcp.png":
        image = image.crop((0, int(h * 0.35), w, h))
    elif filename == "screenshots/04_ftp_upload.png":
        image = image.crop((0, int(h * 0.40), w, h))
    elif filename == "cover_network_v3.png":
        # Crop the generated landscape visual to a portrait panel for the cover.
        crop_w = int(h * 0.80)
        left = max(0, (w - crop_w) // 2)
        image = image.crop((left, 0, left + crop_w, h))
    image.save(output)
    return output


def arrow(slide, x1, y1, x2, y2, color=BLUE, width=2.0):
    ln = slide.shapes.add_connector(MSO_CONNECTOR.STRAIGHT, Inches(x1), Inches(y1), Inches(x2), Inches(y2))
    ln.line.color.rgb = color
    ln.line.width = Pt(width)
    ln.line.end_arrowhead = True
    return ln


def bullet_list(slide, items, x, y, w, h, size=15, color=TEXT, gap=0.34):
    for i, item in enumerate(items):
        text(slide, f"{i + 1}", x, y + i * gap, 0.28, 0.25, size - 2, TEAL, True, PP_ALIGN.CENTER)
        text(slide, item, x + 0.35, y + i * gap - 0.01, w - 0.35, 0.30, size, color, False)


def detailed_list(slide, items, x, y, w, gap=0.43):
    for i, (name, explanation) in enumerate(items):
        yy = y + i * gap
        text(slide, f"{i + 1}", x, yy, 0.25, 0.18, 9, TEAL, True, PP_ALIGN.CENTER)
        text(slide, name, x + 0.32, yy - 0.01, w - 0.32, 0.18, 12.5, NAVY, True)
        text(slide, explanation, x + 0.32, yy + 0.18, w - 0.32, 0.16, 8.5, MUTED, False)


# 1. Cover
slide = prs.slides.add_slide(blank); slide.background.fill.solid(); slide.background.fill.fore_color.rgb = PALE
panel = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0), Inches(0), Inches(7.25), Inches(7.5))
panel.fill.solid(); panel.fill.fore_color.rgb = NAVY; panel.line.fill.background()
accent = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(7.25), Inches(0), Inches(0.14), Inches(7.5))
accent.fill.solid(); accent.fill.fore_color.rgb = TEAL; accent.line.fill.background()
box(slide, 0.55, 0.38, 1.25, 1.0, WHITE, WHITE)
slide.shapes.add_picture(str(ASSETS / "logos" / "um5.png"), Inches(0.68), Inches(0.53), width=Inches(0.98))
box(slide, 1.95, 0.38, 1.15, 1.0, WHITE, WHITE)
slide.shapes.add_picture(str(ASSETS / "logos" / "ensam_rabat.png"), Inches(2.18), Inches(0.49), height=Inches(0.78))
text(slide, "PROJET DE FIN D’ÉTUDES", 0.75, 1.72, 5.6, 0.3, 12, RGBColor(0xA9, 0xD8, 0xD2), True)
text(slide, "SocketHub-C", 0.7, 2.08, 6.2, 1.0, 46, WHITE, True)
text(slide, "Programmation réseau en C\nChat TCP et transfert de fichiers FTP", 0.75, 3.22, 5.9, 0.85, 21, RGBColor(0xE5, 0xF6, 0xF3), False)
line = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0.75), Inches(4.25), Inches(1.25), Inches(0.06)); line.fill.solid(); line.fill.fore_color.rgb = TEAL; line.line.fill.background()
text(slide, "Saad Sahraoui\nHassna Aitiaz", 0.75, 4.58, 4.5, 0.72, 17, WHITE, True)
text(slide, "Encadré par : Pr. Sophia ALAMI KAMOURI", 0.75, 5.58, 5.9, 0.28, 12, RGBColor(0xD2, 0xDF, 0xEC), False)
text(slide, "Programmation réseau  •  2026/2027", 0.75, 6.05, 5.9, 0.28, 12, RGBColor(0xD2, 0xDF, 0xEC), False)
cover_path = prepare_asset("cover_network_v3.png")
slide.shapes.add_picture(str(cover_path), Inches(7.39), Inches(0), width=Inches(5.94), height=Inches(7.5))
text(slide, "CLIENT  ↔  SERVEUR", 8.1, 6.62, 4.5, 0.28, 15, WHITE, True, PP_ALIGN.CENTER)
text(slide, "TCP  •  WinSock2  •  Architecture réseau", 8.0, 6.98, 4.7, 0.18, 10, RGBColor(0xC8, 0xF2, 0xEC), True, PP_ALIGN.CENTER)
footer(slide, 1)

# 2. Overview
slide = prs.slides.add_slide(blank); slide.background.fill.solid(); slide.background.fill.fore_color.rgb = PALE
title(slide, "Présentation générale de SocketHub-C", "01 · Étudiant 1")
text(slide, "Deux applications C natives, reliées par TCP, pour pratiquer la communication client/serveur sous WinSock2.", 0.75, 1.42, 11.8, 0.42, 17, TEXT)
box(slide, 0.8, 2.05, 5.85, 3.95, WHITE, LINE)
box(slide, 6.68, 2.05, 5.85, 3.95, WHITE, LINE)
text(slide, "CHAT TCP", 1.15, 2.38, 2.3, 0.3, 16, BLUE, True)
text(slide, "Échange de messages texte", 1.15, 2.82, 4.9, 0.3, 18, NAVY, True)
text(slide, "cadrés par ligne", 1.15, 3.18, 4.9, 0.28, 15, MUTED, False)
box(slide, 1.15, 3.78, 1.75, 0.62, LIGHT_BLUE, LIGHT_BLUE)
box(slide, 4.65, 3.78, 1.65, 0.62, LIGHT_BLUE, LIGHT_BLUE)
text(slide, "Client Chat", 1.28, 3.98, 1.48, 0.2, 14, NAVY, True, PP_ALIGN.CENTER)
text(slide, "Serveur Chat", 4.78, 3.98, 1.38, 0.2, 14, NAVY, True, PP_ALIGN.CENTER)
arrow(slide, 2.95, 3.98, 4.55, 3.98, BLUE); arrow(slide, 4.55, 4.2, 2.95, 4.2, TEAL)
text(slide, "TCP", 3.63, 3.58, 0.5, 0.18, 10, BLUE, True, PP_ALIGN.CENTER)
text(slide, "127.0.0.1:5000", 2.1, 4.88, 3.25, 0.25, 16, NAVY, True, PP_ALIGN.CENTER)
text(slide, "FTP", 7.03, 2.38, 2.3, 0.3, 16, TEAL, True)
text(slide, "Transfert et consultation de fichiers", 7.03, 2.82, 5.0, 0.3, 18, NAVY, True)
text(slide, "LIST  /  STOR  /  RETR", 7.03, 3.18, 5.0, 0.28, 15, MUTED, False)
box(slide, 7.03, 3.78, 1.65, 0.62, LIGHT_TEAL, LIGHT_TEAL)
box(slide, 10.5, 3.78, 1.65, 0.62, LIGHT_TEAL, LIGHT_TEAL)
text(slide, "Client FTP", 7.15, 3.98, 1.42, 0.2, 14, NAVY, True, PP_ALIGN.CENTER)
text(slide, "Serveur FTP", 10.62, 3.98, 1.42, 0.2, 14, NAVY, True, PP_ALIGN.CENTER)
arrow(slide, 8.72, 3.98, 10.4, 3.98, BLUE); arrow(slide, 10.4, 4.2, 8.72, 4.2, TEAL)
text(slide, "Contrôle TCP : 2121", 7.03, 4.88, 5.0, 0.25, 16, NAVY, True, PP_ALIGN.CENTER)
text(slide, "Données : mode passif PASV", 7.03, 5.28, 5.0, 0.25, 14, MUTED, False, PP_ALIGN.CENTER)
box(slide, 0.95, 6.25, 11.45, 0.58, LIGHT_BLUE, LIGHT_BLUE)
text(slide, "Le client initie la communication ; le serveur écoute, accepte la connexion et traite les requêtes.", 1.25, 6.43, 10.85, 0.2, 14, NAVY, True, PP_ALIGN.CENTER)
footer(slide, 2)

# 3. TCP lifecycle
slide = prs.slides.add_slide(blank); slide.background.fill.solid(); slide.background.fill.fore_color.rgb = PALE
title(slide, "Cycle des sockets TCP en C", "02 · Étudiant 1")
text(slide, "send_all() et recv_line() sont les auxiliaires du projet. shutdown() n’est pas appelé.", 4.0, 1.22, 8.1, 0.2, 10, MUTED, False, PP_ALIGN.RIGHT)
box(slide, 0.75, 1.5, 5.85, 5.35, WHITE, LINE)
box(slide, 6.75, 1.5, 5.85, 5.35, WHITE, LINE)
text(slide, "SERVEUR", 1.1, 1.78, 2.0, 0.32, 16, BLUE, True)
text(slide, "CLIENT", 7.1, 1.78, 2.0, 0.32, 16, TEAL, True)
server_steps = [
    ("WSAStartup()", "initialise WinSock"),
    ("socket()", "crée le socket TCP"),
    ("bind()", "associe IP et port"),
    ("listen()", "met le serveur en écoute"),
    ("accept()", "accepte un client"),
    ("recv() / recv_line()", "reçoit les données"),
    ("send() / send_all()", "envoie la réponse"),
    ("closesocket()", "ferme le socket"),
    ("WSACleanup()", "libère WinSock"),
]
client_steps = [
    ("WSAStartup()", "initialise WinSock"),
    ("socket()", "crée le socket TCP"),
    ("connect()", "demande la connexion"),
    ("send() / send_all()", "envoie les messages"),
    ("recv() / recv_line()", "reçoit la réponse"),
    ("closesocket()", "ferme le socket"),
    ("WSACleanup()", "libère WinSock"),
]
detailed_list(slide, server_steps, 1.1, 2.18, 5.05, 0.47)
detailed_list(slide, client_steps, 7.1, 2.18, 5.05, 0.59)
footer(slide, 3)

# 4. Chat sequence
slide = prs.slides.add_slide(blank); slide.background.fill.solid(); slide.background.fill.fore_color.rgb = PALE
title(slide, "Séquence réelle du Chat TCP", "03 · Étudiant 1")
text(slide, "127.0.0.1:5000", 10.75, 0.7, 1.75, 0.25, 12, BLUE, True, PP_ALIGN.RIGHT)
box(slide, 1.65, 1.43, 1.95, 0.43, LIGHT_BLUE, LIGHT_BLUE)
box(slide, 9.65, 1.43, 2.0, 0.43, LIGHT_TEAL, LIGHT_TEAL)
text(slide, "Client Chat", 1.85, 1.55, 1.55, 0.2, 14, NAVY, True, PP_ALIGN.CENTER)
text(slide, "Serveur Chat", 9.85, 1.55, 1.6, 0.2, 14, NAVY, True, PP_ALIGN.CENTER)
client_x, server_x = 2.62, 10.65
lifeline = slide.shapes.add_connector(MSO_CONNECTOR.STRAIGHT, Inches(client_x), Inches(1.88), Inches(client_x), Inches(6.72)); lifeline.line.color.rgb = BLUE; lifeline.line.width = Pt(1.3)
lifeline = slide.shapes.add_connector(MSO_CONNECTOR.STRAIGHT, Inches(server_x), Inches(1.88), Inches(server_x), Inches(6.72)); lifeline.line.color.rgb = TEAL; lifeline.line.width = Pt(1.3)
sequence = [
    ("1", "Serveur : WSAStartup()", "self", TEAL),
    ("2", "Serveur : socket()", "self", TEAL),
    ("3", "Serveur : bind() + listen()", "self", TEAL),
    ("4", "Client : WSAStartup()", "self_client", BLUE),
    ("5", "Client : socket() + connect()", "self_client", BLUE),
    ("6", "Serveur : accept()", "self", TEAL),
    ("7", "send()  Bonjour SocketHub-C", "c2s", BLUE),
    ("8", "recv()  Serveur: reçu -> Bonjour SocketHub-C", "s2c", TEAL),
    ("9", "send()  Test de communication TCP", "c2s", BLUE),
    ("10", "recv()  Serveur: reçu -> Test de communication TCP", "s2c", TEAL),
    ("11", "send()  /quit", "c2s", BLUE),
    ("12", "recv()  Serveur: au revoir !", "s2c", TEAL),
    ("13", "closesocket() + WSACleanup() des deux côtés", "close", NAVY),
]
yy = 2.12
for number, label, direction, col in sequence:
    text(slide, number, 0.95, yy - 0.12, 0.38, 0.18, 9, col, True, PP_ALIGN.RIGHT)
    if direction == "c2s": arrow(slide, client_x, yy, server_x, yy, col); text(slide, label, 3.35, yy - 0.18, 6.55, 0.20, 9.5, TEXT, True, PP_ALIGN.CENTER)
    elif direction == "s2c": arrow(slide, server_x, yy, client_x, yy, col); text(slide, label, 3.35, yy - 0.18, 6.55, 0.20, 9.5, TEXT, True, PP_ALIGN.CENTER)
    elif direction == "self": arrow(slide, server_x, yy, server_x + 0.48, yy, col); text(slide, label, 6.95, yy - 0.18, 3.25, 0.20, 9.2, TEXT, False)
    elif direction == "self_client": arrow(slide, client_x, yy, client_x - 0.48, yy, col); text(slide, label, 2.98, yy - 0.18, 3.25, 0.20, 9.2, TEXT, False, PP_ALIGN.RIGHT)
    else: text(slide, label, 4.05, yy - 0.18, 5.3, 0.20, 9.3, MUTED, False, PP_ALIGN.CENTER)
    yy += 0.36
footer(slide, 4)

# 5. Chat compilation/lancement
slide = prs.slides.add_slide(blank); slide.background.fill.solid(); slide.background.fill.fore_color.rgb = PALE
title(slide, "Exécution réelle du Chat TCP — compilation et lancement", "04 · Étudiant 1")
add_image(slide, "screenshots/01_compilation.png", 0.75, 1.52, 6.0, 3.85, "Compilation GCC réussie pour les quatre exécutables")
add_image(slide, "screenshots/02_chat_tcp.png", 7.0, 1.52, 5.55, 3.85, "Serveur prêt puis client connecté au port 5000")
box(slide, 0.95, 5.92, 11.7, 0.72, LIGHT_TEAL, LIGHT_TEAL)
text(slide, "Quatre exécutables compilés  •  serveur en écoute  •  client connecté", 1.25, 6.15, 11.1, 0.24, 15, NAVY, True, PP_ALIGN.CENTER)
footer(slide, 5)

# 6. Chat exchange
slide = prs.slides.add_slide(blank); slide.background.fill.solid(); slide.background.fill.fore_color.rgb = PALE
title(slide, "Exécution réelle du Chat TCP — échange et fermeture", "05 · Étudiant 1")
add_image(slide, "screenshots/02_chat_tcp.png", 0.75, 1.45, 8.2, 4.55, "Capture réelle des deux terminaux")
box(slide, 9.25, 1.55, 3.25, 4.45, WHITE, LINE)
text(slide, "MESSAGES OBSERVÉS", 9.62, 1.92, 2.5, 0.28, 14, TEAL, True, PP_ALIGN.CENTER)
text(slide, "Bonjour SocketHub-C", 9.62, 2.55, 2.5, 0.42, 16, BLUE, True, PP_ALIGN.CENTER)
text(slide, "Test de communication TCP", 9.48, 3.28, 2.78, 0.5, 15, BLUE, True, PP_ALIGN.CENTER)
text(slide, "/quit", 9.62, 4.12, 2.5, 0.35, 17, NAVY, True, PP_ALIGN.CENTER)
text(slide, "Serveur: au revoir !", 9.55, 4.85, 2.62, 0.4, 15, TEAL, True, PP_ALIGN.CENTER)
box(slide, 1.0, 6.25, 11.35, 0.45, LIGHT_TEAL, LIGHT_TEAL)
text(slide, "Échange bidirectionnel confirmé, puis fermeture propre des sockets.", 1.2, 6.38, 10.95, 0.18, 13, NAVY, True, PP_ALIGN.CENTER)
footer(slide, 6)

# 7. FTP architecture
slide = prs.slides.add_slide(blank); slide.background.fill.solid(); slide.background.fill.fore_color.rgb = PALE
title(slide, "Architecture et authentification FTP", "06 · Étudiant 2")
add_image(slide, "screenshots/03_ftp_authentication.png", 0.75, 1.55, 6.15, 3.9, "Authentification réelle : USER, PASS, 331 puis 230")
box(slide, 7.35, 1.55, 5.2, 3.9, WHITE, LINE)
text(slide, "Deux connexions TCP", 7.75, 1.9, 3.6, 0.3, 18, NAVY, True)
text(slide, "CONTRÔLE", 7.8, 2.55, 1.5, 0.25, 13, BLUE, True)
text(slide, "USER / PASS / PASV\nRéponses 220, 331, 230, 227", 7.8, 2.88, 3.9, 0.65, 15, TEXT)
text(slide, "DONNÉES", 7.8, 3.85, 1.5, 0.25, 13, TEAL, True)
text(slide, "LIST, RETR ou STOR\nRéponses 150 puis 226", 7.8, 4.18, 3.9, 0.6, 15, TEXT)
arrow(slide, 10.3, 3.48, 10.3, 3.82, TEAL)
text(slide, "Le serveur ouvre un port passif dédié après PASV.", 7.75, 5.0, 4.1, 0.25, 11, MUTED)
footer(slide, 7)

# 8. FTP transfer/list
slide = prs.slides.add_slide(blank); slide.background.fill.solid(); slide.background.fill.fore_color.rgb = PALE
title(slide, "Envoi et consultation des fichiers", "07 · Étudiant 2")
add_image(slide, "screenshots/04_ftp_upload.png", 0.75, 1.55, 6.05, 2.65, "STOR : upload vers ftp_root/demo_cc.txt")
add_image(slide, "screenshots/05_ftp_list.png", 6.95, 1.55, 5.6, 2.65, "LIST : contenu du répertoire distant")
text(slide, "STOR", 1.0, 4.65, 1.0, 0.25, 14, BLUE, True)
text(slide, "envoie un fichier local", 2.0, 4.65, 2.2, 0.25, 13, TEXT)
text(slide, "LIST", 7.2, 4.65, 1.0, 0.25, 14, TEAL, True)
text(slide, "affiche le contenu distant", 8.2, 4.65, 2.6, 0.25, 13, TEXT)
text(slide, "Les captures montrent les codes 227, 150 et 226.", 3.4, 5.25, 6.6, 0.22, 12, MUTED, False, PP_ALIGN.CENTER)
footer(slide, 8)

# 9. Download/integrity
slide = prs.slides.add_slide(blank); slide.background.fill.solid(); slide.background.fill.fore_color.rgb = PALE
title(slide, "Téléchargement et vérification des fichiers", "08 · Étudiant 2")
add_image(slide, "screenshots/06_ftp_download.png", 0.75, 1.45, 6.05, 3.65, "RETR : téléchargement du fichier distant")
add_image(slide, "screenshots/07_sha256_verification.png", 6.95, 1.45, 5.6, 3.65, "SHA-256 : comparaison des empreintes")
box(slide, 0.95, 5.45, 11.65, 0.9, LIGHT_TEAL, LIGHT_TEAL)
text(slide, "Chat TCP fonctionnel  ·  Authentification FTP  ·  Upload / download  ·  Listing  ·  Intégrité SHA-256", 1.2, 5.72, 11.1, 0.27, 14, NAVY, True, PP_ALIGN.CENTER)
text(slide, "SocketHub-C : une mise en pratique complète de la programmation réseau en C.", 1.3, 6.55, 10.7, 0.28, 16, NAVY, True, PP_ALIGN.CENTER)
footer(slide, 9)


prs.save(OUTPUT / "SocketHub-C_Presentation_FINAL_v3.pptx")

