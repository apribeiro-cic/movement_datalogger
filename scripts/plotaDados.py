# IMPORTANTE: Necessário instalar as bibliotecas numpy e matplotlib
# Para instalar, execute:
# pip install numpy matplotlib
# Necessário ter o pip instalado e configurado no seu ambiente Python.

# É possível executar este script no Google Colab, onde as bibliotecas já estão instaladas.
# Nesse caso, basta carregar o arquivo CSV no ambiente do Colab, alterando o caminho do arquivo (csv_path).

import numpy as np
import matplotlib.pyplot as plt
import os

# Caminho do arquivo
csv_path = '../dados/imu_data.csv'

# Verifica se o arquivo existe
if not os.path.exists(csv_path):
    print(f"[ERRO] Arquivo '{csv_path}' não encontrado.")
    exit(1)

# Carrega os dados ignorando o cabeçalho
data = np.loadtxt(csv_path, delimiter=',', skiprows=1)

# Colunas: tempo, ax, ay, az, gx, gy, gz
tempo = data[:, 0]

# Aceleração
ax = data[:, 1]
ay = data[:, 2]
az = data[:, 3]

# Giroscópio
gx = data[:, 4]
gy = data[:, 5]
gz = data[:, 6]

# ----- Gráfico de Aceleração -----
plt.figure(figsize=(10, 5))
plt.plot(tempo, ax, 'r-', label='Ax')
plt.plot(tempo, ay, 'g-', label='Ay')
plt.plot(tempo, az, 'b-', label='Az')
plt.title("Aceleração vs Tempo")
plt.xlabel("Tempo (s)")
plt.ylabel("Aceleração (m/s²)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()

# ----- Gráfico de Giroscópio -----
plt.figure(figsize=(10, 5))
plt.plot(tempo, gx, 'r--', label='Gx')
plt.plot(tempo, gy, 'g--', label='Gy')
plt.plot(tempo, gz, 'b--', label='Gz')
plt.title("Velocidade Angular vs Tempo")
plt.xlabel("Tempo (s)")
plt.ylabel("Velocidade Angular (°/s)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
