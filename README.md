# 🌬️ Sentinela IAQ
### Sistema Inteligente de Segurança Respiratória e Ocupação

> Monitoramento em tempo real de qualidade do ar, temperatura, umidade e presença humana — com feedback local e transmissão para a nuvem.

---

## 🩺 O Problema

Ambientes fechados como escritórios, laboratórios e salas de aula dependem quase exclusivamente de sistemas de ar-condicionado que **recirculam o ar sem renová-lo**. Esse modelo cria uma combinação silenciosa e perigosa:

| Fator | Consequência |
|---|---|
| Ausência de renovação de ar | Acúmulo de CO₂ e contaminantes |
| Alta densidade de ocupação | Proliferação acelerada de vírus e bactérias |
| Umidade desregulada | Ambiente ideal para agentes patogênicos aéreos |
| Controle apenas de temperatura | Saúde e conforto ocupacional ignorados |

Esse conjunto de condições caracteriza a chamada **Síndrome dos Edifícios Doentes** — condição amplamente documentada que afeta a saúde, o conforto e o desempenho humano.

> 📄 **Referência científica:**
> SCHIRMER, Waldir Nagel. *A poluição do ar em ambientes internos e a síndrome dos edifícios doentes*. Disponível em: [SciELO](https://www.scielo.br). O estudo documenta como edifícios projetados sem troca efetiva de ar acumulam contaminantes biológicos e químicos, com impacto direto na saúde dos ocupantes.

Medir apenas a temperatura **não é mais suficiente**. É indispensável monitorar a qualidade do ar e a taxa de ocupação em tempo real.

---

## 💡 A Solução: Sentinela IAQ

O **Sentinela IAQ** é um dispositivo embarcado baseado em ESP32 que atua em três camadas complementares:

### 🔲 1. Processamento na Borda (Edge Computing)
Coleta contínua e local de:
- Temperatura e umidade relativa do ar
- Concentração de gases nocivos (CO₂ via MQ-135)
- Detecção de presença humana no ambiente

### 🔔 2. Feedback Imediato para os Ocupantes
- **Display LCD 16×2** exibe as leituras em tempo real
- **Alertas sonoros (buzzer)** indicam o momento exato em que o ar atinge níveis críticos e ação é necessária (ex.: abrir janelas)
- Autonomia total: os ocupantes não precisam de aplicativo ou acesso à internet para serem alertados

### ☁️ 3. Computação em Nuvem
- Transmissão das leituras via **Wi-Fi** para a plataforma **ThingSpeak**
- Gestores de infraestrutura visualizam **gráficos históricos**, identificam padrões de risco e tomam decisões baseadas em dados consolidados

---

## 🛠️ Componentes do Projeto

### Hardware

| Componente | Pino (ESP32) | Função |
|---|---|---|
| ESP32 | — | Microcontrolador principal com Wi-Fi integrado |
| DHT22 | GPIO 15 | Sensor de temperatura e umidade de alta precisão |
| MQ-135 | GPIO 34 (ADC) | Sensor de qualidade do ar / concentração de gases (CO₂) |
| Sensor Ultrassônico HC-SR04 | TRIG: GPIO 19 / ECHO: GPIO 18 | Detecção e contagem de presença na porta |

### Software e Plataformas

| Ferramenta | Uso |
|---|---|
| Arduino IDE + C++ | Programação do firmware embarcado |
| Wokwi Simulator | Simulação e prototipagem virtual do circuito |
| `WiFi.h` + `WebServer.h` | Servidor web local no ESP32 |
| `DHT.h` (Adafruit) | Leitura do sensor de temperatura e umidade |

---

## 📐 Diagrama do Circuito

![Diagrama do circuito Sentinela IAQ](./circuit_diagram.png)

> *Montagem completa com ESP32, DHT22, PIR, MQ-135, LCD I2C e buzzer.*

---

## 🏗️ Arquitetura do Sistema

```
┌─────────────────────────────────────────────────────┐
│                    BORDA (ESP32)                     │
│                                                      │
│  DHT22 (GPIO 15) ──► Temperatura / Umidade          │
│  MQ-135 (GPIO 34) ──► Concentração CO₂ (ppm)       │
│  HC-SR04 (GPIO 18/19) ──► Contagem de Ocupantes    │
│                     │                                │
│          Processamento + Score de Segurança          │
│                     │                                │
│              WebServer (porta 80)                    │
└──────────────────── │ ───────────────────────────────┘
                      │ Wi-Fi (HTTP)
                      ▼
┌─────────────────────────────────────────────────────┐
│          DASHBOARD WEB (Navegador / Celular)         │
│                                                      │
│  • Temperatura, Umidade, CO₂ em tempo real          │
│  • Lotação da sala (contador / capacidade)          │
│  • Índice de Segurança (Score 0–100)                │
│  • Botões: Entrada / Saída / Reset                  │
└─────────────────────────────────────────────────────┘
```

---

## 🚀 Como Executar

### Pré-requisitos
- [Arduino IDE](https://www.arduino.cc/en/software) com suporte ao ESP32 instalado
- Bibliotecas instaladas (via **Gerenciador de Bibliotecas** da Arduino IDE):
  - `DHT sensor library` — Adafruit
  - `Adafruit Unified Sensor`
- Bibliotecas nativas do ESP32 (já incluídas):
  - `WiFi.h`
  - `WebServer.h`

### Configuração
1. Clone este repositório:
   ```bash
   git clone https://github.com/seu-usuario/sentinela-iaq.git
   cd sentinela-iaq
   ```

2. Abra o arquivo `sentinela_iaq.ino` no Arduino IDE.

3. Altere as credenciais Wi-Fi no início do código:
   ```cpp
   const char* ssid     = "nome_da_sua_rede";
   const char* password = "sua_senha_wifi";
   ```

4. Selecione a placa **ESP32 Dev Module** em *Ferramentas > Placa* e faça o upload.

5. Abra o **Monitor Serial** (115200 baud) e aguarde a mensagem:
   ```
   ENDEREÇO IP PARA DIGITAR NO CELULAR: 192.168.x.x
   ```

6. Digite o IP no navegador do celular (na mesma rede Wi-Fi) para acessar o dashboard.

### Simulação (Wokwi)
O projeto pode ser simulado diretamente no [Wokwi](https://wokwi.com/) sem hardware físico. Importe os arquivos `diagram.json` e o código-fonte na plataforma.

---

## 📊 Lógica do Score de Segurança

O firmware calcula um **índice de 0 a 100** em tempo real, com penalidades progressivas:

| Condição | Penalidade no Score |
|---|---|
| CO₂ > 800 ppm | −0,05 ponto por ppm acima de 800 |
| Umidade < 40% ou > 60% | −15 pontos |
| 1 pessoa na sala | −5 pontos |
| Sala cheia (≥ 5 pessoas) | −40 pontos (penalidade máxima) |

| Score | Status |
|---|---|
| 80 – 100 | 🟢 Seguro |
| 50 – 79 | 🟡 Atenção |
| 0 – 49 | 🔴 Crítico |

### Contagem de Ocupantes (Lógica de Catraca)
O sensor HC-SR04 vigia a porta a cada **50 ms**. Quando detecta um objeto a ≤ 5 cm:
- Incrementa o contador de pessoas
- Ativa um **cooldown de 1,5 segundos** para evitar contagem dupla
- Bloqueia entrada se a capacidade máxima (5 pessoas) for atingida

---

## 💻 Código-Fonte

O firmware completo está em [`src/sentinela_iaq.ino`](./src/sentinela_iaq.ino).

### Visão Geral do Firmware

```cpp
// --- Mapeamento de Pinos ---
#define DHTPIN   15   // DHT22 — temperatura e umidade
#define TRIG_PIN 19   // HC-SR04 — disparo ultrassônico
#define ECHO_PIN 18   // HC-SR04 — recepção do eco
#define MQ_PIN   34   // MQ-135 — leitura analógica de gases

// --- Configurações ---
#define DISTANCIA_PRESENCA 5  // cm — limiar de detecção na porta
const int CAPACIDADE_MAXIMA = 5;

// Loop principal: duas tarefas paralelas sem uso de delay()
// • A cada 50ms  → HC-SR04 vigia a porta (contagem de ocupantes)
// • A cada 2000ms → DHT22 + MQ-135 atualizam temperatura, umidade e CO₂
//                   e recalculam o Score de Segurança
```

### Rotas do Servidor Web (HTTP)

| Rota | Método | Ação |
|---|---|---|
| `/` | GET | Exibe o dashboard completo |
| `/entrada` | GET | Incrementa o contador de pessoas |
| `/saida` | GET | Decrementa o contador de pessoas |
| `/reset` | GET | Zera o contador e reinicia o estado |

---

## 📁 Estrutura do Repositório

```
sentinela-iaq/
├── src/
│   └── sentinela_iaq.ino      # Código principal do firmware
├── diagrams/
│   └── circuit_diagram.png    # Diagrama do circuito (Fritzing/Wokwi)
├── wokwi/
│   └── diagram.json           # Arquivo de simulação Wokwi
└── README.md
```

---

## 📚 Referências

- SCHIRMER, W. N. *A poluição do ar em ambientes internos e a síndrome dos edifícios doentes*. SciELO Brasil.
- [ESP32 Technical Reference — Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [DHT Sensor Library — Adafruit](https://github.com/adafruit/DHT-sensor-library)
- [Wokwi Simulator](https://wokwi.com/)

---

## 📄 Licença

Este projeto está licenciado sob a [MIT License](LICENSE).

---

<div align="center">
  <sub>Desenvolvido como projeto de IoT para monitoramento de ambientes internos.</sub>
</div>
