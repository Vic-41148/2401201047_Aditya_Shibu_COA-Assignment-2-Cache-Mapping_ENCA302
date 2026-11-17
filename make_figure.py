"""Draw the cache-mapping layout figure for the COA Q2 report."""
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch, FancyBboxPatch

EDGE = "#333333"
FILL = "#f2f2f2"
ACCENT = "#d9d9d9"

fig, ax = plt.subplots(figsize=(7.2, 3.0), dpi=300)
ax.set_xlim(0, 105)
ax.set_ylim(0, 46.5)
ax.axis("off")


def box(x, y, w, h, text, fill=FILL, size=8.2, weight="normal"):
    ax.add_patch(
        FancyBboxPatch(
            (x, y), w, h,
            boxstyle="round,pad=0.25,rounding_size=0.9",
            linewidth=1.1, edgecolor=EDGE, facecolor=fill,
        )
    )
    ax.text(x + w / 2, y + h / 2, text, ha="center", va="center",
            fontsize=size, color="#111111", fontweight=weight)


def panel(title, layout, rule, cx):
    ax.text(cx, 41.0, title, ha="center", va="center", fontsize=9.6,
            fontweight="bold", color="#111111")
    ax.text(cx, 38.3, layout, ha="center", va="center", fontsize=8.0,
            color="#444444")
    ax.text(cx, 35.7, rule, ha="center", va="center", fontsize=8.0,
            color="#444444", style="italic")


def arrow(x1, y1, x2, y2):
    ax.add_patch(FancyArrowPatch((x1, y1), (x2, y2), arrowstyle="-|>",
                                 mutation_scale=9, linewidth=1.0, color="#555555"))


# ---------------------------------------------------------------- direct-mapped
panel("Direct-Mapped", "4 sets x 1 way", "set = block mod 4", 17.5)
for row in range(4):
    y = 29.6 - row * 6.2
    ax.text(3.2, y + 2.1, f"S{row}", ha="center", va="center", fontsize=8.6)
    box(6.5, y, 20.0, 4.2, "tag | one block")
ax.text(17.5, 5.0, "one fixed line per set", ha="center", fontsize=8.4,
        color="#444444")

# ------------------------------------------------------- 2-way set associative
panel("2-Way Set-Associative", "2 sets x 2 ways", "set = block mod 2", 52.5)
for row, label in enumerate(["S0", "S1"]):
    y = 29.0 - row * 8.6
    ax.text(38.4, y + 2.1, label, ha="center", va="center", fontsize=8.6)
    box(41.6, y, 10.2, 4.2, "W0")
    box(53.2, y, 10.2, 4.2, "W1")
ax.text(52.5, 11.4, "any way inside the set", ha="center", fontsize=8.4,
        color="#444444")
ax.text(52.5, 5.0, "LRU picks the victim", ha="center", fontsize=8.4,
        color="#444444")

# ------------------------------------------------------------ fully associative
panel("Fully Associative", "1 set x 4 ways", "set = block mod 1", 87.0)
ax.text(71.6, 27.0, "S0", ha="center", va="center", fontsize=8.6)
for col in range(4):
    x = 73.5 + col * 6.3
    box(x, 24.9, 5.4, 4.2, f"W{col}", size=7.6)
ax.text(87.0, 19.8, "any line can hold any block", ha="center", fontsize=8.4,
        color="#444444")
ax.text(87.0, 5.0, "one full search, no conflicts", ha="center", fontsize=8.4,
        color="#444444")

# ------------------------------------------------------------------- bottom bar
ax.text(52.5, 44.6, "the same 4 cache blocks, three mapping rules",
        ha="center", va="center", fontsize=9.4, color="#222222")

fig.tight_layout(pad=0.2)
fig.savefig("screenshots/architecture.png", bbox_inches="tight",
            facecolor="white")
print("screenshots/architecture.png")
