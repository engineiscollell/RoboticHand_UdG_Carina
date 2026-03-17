"""
Simulació en Python del codi basic_threshold_dinamic_v2.ino
============================================================
Replica fidelment la lògica de l'Arduino:
  - Lectura ràpida (cada 2ms) per detecció de triggers
  - Histograma lent (cada 20ms) per estadístiques
  - Cold start de 60s (no es permeten triggers fins que la finestra estigui plena)
  - Llindar adaptatiu: Q3 + kIQR * IQR + offset (recalculat cada 1s)
  - Detecció flanc pujada + lockout 500ms
"""

import pandas as pd
import matplotlib.pyplot as plt
import math

# ========================== CARREGAR DADES ==========================
df = pd.read_csv("test/data/emg_simulat_5min_v2.csv", usecols=["time_ms", "emg_value"])

num_files = len(df)
t_inici = df["time_ms"].iloc[0]
t_final = df["time_ms"].iloc[-1]
interval_ms = t_final - t_inici
interval_s = interval_ms / 1000
dt = df["time_ms"].diff().mean()
fs = 1000 / dt

print(f"Numero de files: {num_files}")
print(f"Temps inicial (ms): {t_inici}")
print(f"Temps final (ms): {t_final}")
print(f"Durada total: {interval_ms} ms ({interval_s} s)")
print(f"Interval mitjà entre mostres: {dt:.2f} ms")
print(f"Freqüència de mostreig aproximada: {fs:.1f} Hz")

# ======================== PARÀMETRES ========================
# Toggle + lockout
lockoutTime = 500  # ms

# Temporitzadors separats
readPeriodMs = 2        # 500 Hz - lectura + detecció
histPeriodMs = 20       # 50 Hz  - histograma
thrUpdatePeriodMs = 1000  # cada 1s - recàlcul llindar

# Cold start
coldStartMs = 60000     # 60s - no triggers fins que la finestra estigui plena

# Histograma sliding window
windowMs = 60000
WIN = windowMs // histPeriodMs  # 3000 mostres

BINS = 128
V_MIN = 0
V_MAX = 2048

# Llindar adaptatiu
kIQR = 5
offsetThr = 50.0
thresholdV = 1000.0  # valor inicial
IQR_MIN = 20


# =================== FUNCIONS HISTOGRAMA ===================

def v_to_bin(v):
    if v <= V_MIN:
        return 0
    if v >= V_MAX:
        return BINS - 1
    r = (v - V_MIN) / (V_MAX - V_MIN)
    b = int(r * (BINS - 1) + 0.5)
    return max(0, min(BINS - 1, b))


def bin_to_v(b):
    step = (V_MAX - V_MIN) / BINS
    return V_MIN + (b + 0.5) * step


def percentile_from_hist(hist, count, p):
    if count <= 0:
        return 0.0
    target = round(p * (count - 1))
    cum = 0
    for b in range(BINS):
        cum += hist[b]
        if cum > target:
            return bin_to_v(b)
    return bin_to_v(BINS - 1)


# TODO fer variable iqr min
def recomputa_llindar(hist, count):
    q1 = percentile_from_hist(hist, count, 0.25)
    q3 = percentile_from_hist(hist, count, 0.75)
    iqr = q3 - q1
    print(iqr)
    if iqr < IQR_MIN:
        iqr = IQR_MIN
    return q3 + kIQR * iqr + offsetThr, q1, q3, iqr


# ======================== SIMULACIÓ ========================

# Estat histograma
hist = [0] * BINS
ringBin = [0] * WIN
ringIdx = 0
ringPle = False

# Estat detecció
startTime = t_inici
ultimTrigger = t_inici - lockoutTime - 1
emgAnterior = False
estatMoviment = False
lastV = 0

# Temporitzadors
lastRead = t_inici - readPeriodMs - 1
lastHist = t_inici - histPeriodMs - 1
lastThrUpdate = t_inici - thrUpdatePeriodMs - 1

# Resultats
events = []
threshold_history = []

for _, row in df.iterrows():
    ara = row["time_ms"]
    v = row["emg_value"]

    # ---- RÀPID (cada 2ms): lectura + detecció ----
    if (ara - lastRead) >= readPeriodMs:
        lastRead = ara
        lastV = v  # simula analogRead

        histReady = (ara - startTime) >= coldStartMs
        emgActual = lastV > thresholdV

        if histReady and emgActual and not emgAnterior and ((ara - ultimTrigger) > lockoutTime):
            ultimTrigger = ara
            estatMoviment = not estatMoviment

            events.append({
                "time_ms": ara,
                "emg_value": lastV,
                "nou_estat": "MOVIMENT" if estatMoviment else "REPOS",
                "threshold": thresholdV
            })

        emgAnterior = emgActual

    # ---- LENT (cada 20ms): actualitzar histograma ----
    if (ara - lastHist) >= histPeriodMs:
        lastHist = ara

        b = v_to_bin(lastV)

        if ringPle:
            old = ringBin[ringIdx]
            if hist[old] > 0:
                hist[old] -= 1

        ringBin[ringIdx] = b
        hist[b] += 1

        ringIdx += 1
        if ringIdx >= WIN:
            ringIdx = 0
            ringPle = True

    # ---- RECÀLCUL LLINDAR (cada 1s) ----
    if (ara - lastThrUpdate) >= thrUpdatePeriodMs:
        lastThrUpdate = ara
        count = WIN if ringPle else ringIdx
        thresholdV, q1, q3, iqr = recomputa_llindar(hist, count)

        threshold_history.append({
            "time_ms": ara,
            "threshold": thresholdV,
            "q1": q1,
            "q3": q3,
            "iqr": iqr
        })


# ======================== RESULTATS ========================
print("\n--- RESULTATS SIMULACIÓ (LLINDAR DINÀMIC v2) ---")
print(f"Cold start: {coldStartMs / 1000:.0f}s (no triggers durant aquest temps)")
print(f"Nombre de triggers detectats: {len(events)}")

if len(events) == 0:
    print("No s'ha detectat cap canvi d'estat.")
else:
    print("Canvis d'estat detectats:")
    for e in events:
        print(f"  t={e['time_ms']} ms | emg={e['emg_value']} | thr={e['threshold']:.1f} | estat={e['nou_estat']}")

print(f"\nLlindar final: {thresholdV:.1f}")
if threshold_history:
    thrs = [t["threshold"] for t in threshold_history]
    print(f"Llindar mínim: {min(thrs):.1f} | màxim: {max(thrs):.1f}")


# ======================== GRÀFICS ========================
df_events = pd.DataFrame(events)
df_thr = pd.DataFrame(threshold_history)

fig, axes = plt.subplots(2, 1, figsize=(16, 10), sharex=True,
                         gridspec_kw={"height_ratios": [3, 1]})

# ---------- GRÀFIC 1: Senyal EMG + llindar dinàmic + triggers ----------
ax1 = axes[0]

ax1.plot(df["time_ms"], df["emg_value"],
         color="royalblue", linewidth=0.8, alpha=0.9, label="EMG")

# Zona de cold start (fons gris)
ax1.axvspan(t_inici, t_inici + coldStartMs,
            color="gray", alpha=0.15, label=f"Cold start ({coldStartMs/1000:.0f}s)")

# Llindar dinàmic
if not df_thr.empty:
    ax1.plot(df_thr["time_ms"], df_thr["threshold"],
             color="red", linewidth=2, linestyle="-",
             label="Llindar dinàmic", zorder=4)

# Triggers
if not df_events.empty:
    ax1.scatter(df_events["time_ms"], df_events["emg_value"],
                color="limegreen", edgecolors="black", s=90, zorder=5,
                label="Triggers")
    for _, e in df_events.iterrows():
        ax1.axvline(e["time_ms"], color="green", linestyle=":", alpha=0.6)

ax1.set_ylabel("Valor EMG")
ax1.set_title("Senyal EMG amb llindar dinàmic adaptatiu (v2 - lectura separada)")
ax1.legend(loc="upper right")
ax1.grid(True, alpha=0.3)

# ---------- GRÀFIC 2: Evolució del llindar i estadístiques ----------
ax2 = axes[1]

# Zona de cold start
ax2.axvspan(t_inici, t_inici + coldStartMs,
            color="gray", alpha=0.15)

if not df_thr.empty:
    ax2.plot(df_thr["time_ms"], df_thr["threshold"],
             color="red", linewidth=1.5, label="Threshold")
    ax2.plot(df_thr["time_ms"], df_thr["q3"],
             color="orange", linewidth=1, linestyle="--", label="Q3")
    ax2.plot(df_thr["time_ms"], df_thr["q1"],
             color="purple", linewidth=1, linestyle="--", label="Q1")
    ax2.fill_between(df_thr["time_ms"], df_thr["q1"], df_thr["q3"],
                     color="orange", alpha=0.15, label="IQR (Q1–Q3)")

ax2.set_xlabel("Temps (ms)")
ax2.set_ylabel("Valor")
ax2.set_title("Evolució del llindar i estadístiques (Q1, Q3, IQR)")
ax2.legend(loc="upper right")
ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig("emg_plot_dinamic_v2_20.png", dpi=300)
print("\nGraf guardat com emg_plot_dinamic_v2.png")
