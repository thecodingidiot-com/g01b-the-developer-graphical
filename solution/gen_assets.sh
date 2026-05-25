#!/bin/bash
# Generate background PNGs and set up font for g01b.
#
# Each background is 800x600 with four coloured zones:
#   question zone  (0,0)-(559,179)   — prize + question text
#   answer zone    (0,180)-(559,419) — options + lifeline bar
#   help zone      (0,420)-(559,599) — phone hint / audience
#   ladder panel   (562,0)-(799,599) — money tree (always visible)
#
# Font: copies UbuntuMono-R.ttf as assets/font.ttf.
# Replace with Px437 IBM EGA 8x14 from int10h.org (CC BY-SA 4.0, VileR)
# for the intended retro look.

set -e

mkdir -p assets

python3 - <<'EOF'
from PIL import Image, ImageDraw

W, H = 800, 600
DIVIDER_X = 560

def make_bg(path, q_col, ans_col, help_col, lddr_col, div_col):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rectangle([0,   0,   DIVIDER_X - 1, 179], fill=q_col)
    d.rectangle([0,   180, DIVIDER_X - 1, 419], fill=ans_col)
    d.rectangle([0,   420, DIVIDER_X - 1, H - 1], fill=help_col)
    d.rectangle([DIVIDER_X,     0, DIVIDER_X + 1, H - 1], fill=div_col)
    d.rectangle([DIVIDER_X + 2, 0, W - 1,         H - 1], fill=lddr_col)
    img.save(path)
    print(f"  wrote {path}")

make_bg(
    "assets/bg_studio.png",
    q_col    = (0x0e, 0x0e, 0x35),
    ans_col  = (0x09, 0x09, 0x1e),
    help_col = (0x0c, 0x0c, 0x2c),
    lddr_col = (0x0b, 0x15, 0x30),
    div_col  = (0x28, 0x28, 0x70),
)

make_bg(
    "assets/bg_correct.png",
    q_col    = (0x0e, 0x35, 0x0e),
    ans_col  = (0x09, 0x1e, 0x09),
    help_col = (0x0c, 0x2c, 0x0c),
    lddr_col = (0x0b, 0x30, 0x0b),
    div_col  = (0x28, 0x70, 0x28),
)

make_bg(
    "assets/bg_wrong.png",
    q_col    = (0x35, 0x0e, 0x0e),
    ans_col  = (0x1e, 0x09, 0x09),
    help_col = (0x2c, 0x0c, 0x0c),
    lddr_col = (0x30, 0x0b, 0x0b),
    div_col  = (0x70, 0x28, 0x28),
)
EOF

echo "Backgrounds ready. Copy assets/font.ttf from the companion repo."
