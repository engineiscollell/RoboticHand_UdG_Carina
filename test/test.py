


# He de mirar si els codis que hem fet funcionarian en la realitat

# per fer aixo els hauria d'aplicar a les dades i veure que passa



# Que es el quality type?


import pandas as pd
import matplotlib.pyplot as plt


mostres_rest_1 = [
    "test/data/rest_1/gravacio_rest_1_m1.csv",
    "test/data/rest_1/gravacio_rest_1_m2.csv",
    "test/data/rest_1/gravacio_rest_1_m3.csv",
    "test/data/rest_1/gravacio_rest_1_m4.csv",
    "test/data/rest_1/gravacio_rest_1_m5.csv",
    "test/data/rest_1/gravacio_rest_1_m6.csv",
]


mostres_rest_2 = [
    "test/data/rest_2/gravacio_rest_2_m1.csv",
    "test/data/rest_2/gravacio_rest_2_m2.csv",
]

# Carregar les dades
df = pd.read_csv("test/data/tripode_lenta/gravacio_tripode_contraccio_lenta_1_m5.csv", usecols=["time_ms", "emg_value"])


# Que hem digui el numero de linies, de quan es el interval de temps de les dades (mirar la diferencia entre el primer i l'ultim)

# Numero de files
num_files = len(df)

# Temps inicial i final
t_inici = df["time_ms"].iloc[0]
t_final = df["time_ms"].iloc[-1]

# Interval total
interval_ms = t_final - t_inici
interval_s = interval_ms / 1000

# diferencia mitjana entre mostres
dt = df["time_ms"].diff().mean()

fs = 1000 / dt

print("Numero de files:", num_files)
print("Temps inicial (ms):", t_inici)
print("Temps final (ms):", t_final)
print("Durada total:", interval_ms, "ms (", interval_s, "s )")
print("Interval mitjà entre mostres:", dt, "ms")
print("Freqüència de mostreig aproximada:", fs, "Hz")


# Aplicar el codi que s'utilitzaria havera que detectaria

"""
BASIC
"""

llindar = 600
lockoutTime = 500  # ms


ultimTrigger = df["time_ms"].iloc[0] - lockoutTime - 1
emgAnterior = False
estatMoviment = False  # False = repòs, True = moviment

# Per guardar els esdeveniments de canvi d'estat
events = []

for _, row in df.iterrows():
    ara = row["time_ms"]
    v = row["emg_value"]

    emgActual = v > llindar
    trigger = False

    # Detectar flanc de pujada amb lockout
    if emgActual and not emgAnterior and ((ara - ultimTrigger) > lockoutTime):
        ultimTrigger = ara
        estatMoviment = not estatMoviment
        trigger = True

        events.append({
            "time_ms": ara,
            "emg_value": v,
            "nou_estat": "MOVIMENT" if estatMoviment else "REPOS"
        })

    emgAnterior = emgActual


# ========= RESULTATS =========
print("\n--- RESULTATS SIMULACIÓ ---")
print("Nombre de triggers detectats:", len(events))

if len(events) == 0:
    print("No s'ha detectat cap canvi d'estat.")
else:
    print("Canvis d'estat detectats:")
    for e in events:
        print(f"  t={e['time_ms']} ms | emg={e['emg_value']} | nou estat={e['nou_estat']}")


# Convertir events a DataFrame per pintar-los fàcilment
df_events = pd.DataFrame(events)

# ---------- GRÀFIC ----------
plt.figure(figsize=(14, 6))

# Senyal EMG
plt.plot(
    df["time_ms"],
    df["emg_value"],
    color="royalblue",
    linewidth=1.5,
    label="EMG"
)

# Zona per sobre del llindar
plt.fill_between(
    df["time_ms"],
    df["emg_value"],
    llindar,
    where=df["emg_value"] > llindar,
    color="orange",
    alpha=0.35,
    label="Per sobre del llindar"
)

# Llindar
plt.axhline(
    llindar,
    color="red",
    linestyle="--",
    linewidth=1.5,
    label=f"Llindar = {llindar}"
)

# Triggers detectats
if not df_events.empty:
    plt.scatter(
        df_events["time_ms"],
        df_events["emg_value"],
        color="limegreen",
        edgecolors="black",
        s=90,
        zorder=5,
        label="Triggers"
    )

    # Línies verticals als triggers
    for _, e in df_events.iterrows():
        plt.axvline(
            e["time_ms"],
            color="green",
            linestyle=":",
            alpha=0.8
        )

plt.xlabel("Temps (ms)")
plt.ylabel("Valor EMG")
plt.title("Senyal EMG i triggers detectats")
plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig("emg_plot.png", dpi=300)
print("Graf guardat com emg_plot.png")