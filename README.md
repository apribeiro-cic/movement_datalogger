# Datalogger de Movimento com Feedback Visual 📈

Este projeto foi desenvolvido para a placa **BitDogLab**, baseada no microcontrolador **Raspberry Pi Pico W**, e tem como objetivo capturar dados de movimento em tempo real utilizando o sensor **MPU6050**. Os dados de aceleração e giroscópio são salvos automaticamente em um cartão microSD no formato `.csv`, permitindo a posterior análise gráfica através de linguagens como Python.

---


## 📌 Sobre o Projeto

O Datalogger de Movimento foi desenvolvido como projeto prático da 2ª fase da residência tecnológica **EmbarcaTech**.

A proposta é integrar sensores de movimento com armazenamento em cartão SD usando SPI, além de exibir dados em tempo real localmente com display OLED e alertas visuais com LED RGB. O projeto ainda inclui controle de início e parada da gravação por botão físico com debounce e feedback.

---

## 🧠 Como funciona

O sistema funciona como um gravador de dados portátil, que registra acelerações e rotações angulares em 3 eixos. Seu funcionamento geral é:

### 🧪 Coleta de Dados

- **MPU6050 (I²C):** captura dados de aceleração (X, Y, Z) e giroscópio (X, Y, Z).
- As leituras são feitas e salvas em um arquivo `.csv` no cartão SD.
- Cada linha do arquivo contém os valores separados por vírgula:  
  `timestamp, accX, accY, accZ, gyroX, gyroY, gyroZ`

### 💾 Armazenamento

- **Cartão microSD (SPI):** todos os dados coletados de forma contínua (depois de pressionar o botão B com o cartão montado) e são armazenados no cartão SD.
- O nome do arquivo é fixo (`imu_data.csv`) e é sobrescrito a cada execução.

### 📊 Visualização Local

- **OLED SSD1306:** exibe status do sistema.
- **LED RGB:** exibe de forma interativa o estado atual do sistema.

### 🎮 Controle por Botão

| Controle  | Função                                               |
|-----------|------------------------------------------------------|
| Botão A   | Monta e desmonta o cartão SD.                        |
| Botão B   | Inicia ou pausa a gravação dos dados (toggle).       |
| Botão J   | Entra no modo bootsel ou faz leitura (se sd montado) |

---

## 📁 Utilização

Atendendo aos requisitos de organização da 2ª fase da residência, o arquivo CMakeLists.txt está configurado para facilitar a importação do projeto no Visual Studio Code. 

Segue as instruções:

1. Na barra lateral, clique em **Raspberry Pi Pico Project** e depois em **Import Project**.

   ![image](https://github.com/user-attachments/assets/4b1ed8c7-6730-4bfe-ae1f-8a26017d1140)

2. Selecione o diretório do projeto e clique em **Import** (utilizando a versão **2.1.1** do Pico SDK).

   ![image](https://github.com/user-attachments/assets/be706372-b918-4ade-847e-12706af0cc99)

4. Agora, basta **compilar** e **rodar** o projeto, com a placa **BitDogLab** conectada.

---
