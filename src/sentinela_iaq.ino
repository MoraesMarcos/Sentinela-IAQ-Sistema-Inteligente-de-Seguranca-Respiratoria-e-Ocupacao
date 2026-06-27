#include <WiFi.h>

#include <WebServer.h>

#include "DHT.h"



// --- Mapeamento de Pinos ---

#define DHTPIN 15

#define DHTTYPE DHT22

#define TRIG_PIN 19

#define ECHO_PIN 18

#define MQ_PIN 34


// --- Configurações do Sistema ---

#define DISTANCIA_PRESENCA 5 // Distância máxima (em cm) para contar uma entrada

const int CAPACIDADE_MAXIMA = 5; // Limite de pessoas na sala



// --- Credenciais da Rede Wi-Fi ---

const char* ssid = "";

const char* password = "";



WebServer server(80);

DHT dht(DHTPIN, DHTTYPE);



// Variáveis globais de sensores

float temperatura = 0.0;

float umidade = 0.0;

int ppmCO2 = 400;

float distanciaCM = 0.0;

float scoreSeguranca = 100.0;



// Variáveis para a Lógica de Catraca (Contagem)

int contadorPessoas = 0;

bool portaObstruida = false;



// --- Variáveis de Tempo (Multitarefa sem delay) ---

unsigned long tempoSensores = 0;

unsigned long tempoUltrassom = 0;

unsigned long cooldownPorta = 0; // Impede que o sensor conte a mesma pessoa várias vezes

const unsigned long intervaloSensores = 2000; // Sensores de ar lidos a cada 2 segundos

const unsigned long intervaloUltrassom = 50;  // Porta vigiada a cada 50 milissegundos (rápido!)



// --- Rota Principal (Página HTML) ---

void handleRoot() {

  String html = "<!DOCTYPE html><html lang='pt-br'><head>";

  html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";

  html += "<title>Sentinela IAQ</title>";

 

  // Fontes e Ícones Profissionais

  html += "<link href='https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;600;700&display=swap' rel='stylesheet'>";

  html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css'>";

 

  // Estilos CSS

  html += "<style>";

  html += "body { font-family: 'Poppins', sans-serif; background: #e0e5ec; color: #2d3436; margin: 0; padding: 20px; display: flex; justify-content: center; align-items: center; min-height: 100vh; }";

  html += ".dashboard { background: #ffffff; border-radius: 20px; box-shadow: 0 10px 30px rgba(0,0,0,0.1); padding: 30px; width: 100%; max-width: 450px; }";

  html += ".header { text-align: center; margin-bottom: 25px; border-bottom: 2px solid #f1f2f6; padding-bottom: 15px; }";

  html += ".header h1 { color: #0984e3; font-size: 26px; margin: 0; display: flex; align-items: center; justify-content: center; gap: 10px; }";

  html += ".header p { color: #636e72; font-size: 14px; margin: 5px 0 0 0; }";

 

  html += ".grid { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; margin-bottom: 20px; }";

  html += ".card { background: #f8f9fa; border-radius: 12px; padding: 15px; display: flex; flex-direction: column; align-items: center; text-align: center; border: 1px solid #dfe6e9; }";

  html += ".card i { font-size: 24px; color: #74b9ff; margin-bottom: 10px; }";

  html += ".card .title { font-size: 12px; color: #b2bec3; font-weight: 600; text-transform: uppercase; letter-spacing: 1px; }";

  html += ".card .value { font-size: 20px; font-weight: 700; color: #2d3436; margin-top: 5px; }";

 

  String corCO2 = (ppmCO2 > 800) ? "#d63031" : "#2d3436";

 

  // Lógica de Cores da Sala

  bool salaCheia = (contadorPessoas >= CAPACIDADE_MAXIMA);

  String textoOcupacao = salaCheia ? "BLOQUEADA" : "LIBERADA";

  String bgOcupacao = salaCheia ? "background: #ff7675; color: white; border: none;" : "background: #55efc4; color: #2d3436; border: none;";



  html += ".score-box { background: #f1f2f6; border-radius: 15px; padding: 20px; text-align: center; margin-bottom: 20px; }";

  html += ".score-title { font-size: 14px; color: #636e72; font-weight: 600; margin-bottom: 5px; }";

  String corScore = (scoreSeguranca >= 80) ? "#00b894" : ((scoreSeguranca >= 50) ? "#fdcb6e" : "#d63031");

  html += ".score-value { font-size: 38px; font-weight: 700; color: " + corScore + "; }";

 

  html += ".btn { display: block; width: 100%; padding: 12px; background: #0984e3; color: white; border: none; border-radius: 10px; font-size: 16px; font-weight: 600; text-align: center; text-decoration: none; cursor: pointer; transition: 0.3s; }";

  html += ".btn:hover { background: #74b9ff; }";

  html += "</style>";

 

  // Recarrega o app a cada 3 segundos

  html += "<meta http-equiv='refresh' content='3'>";

  html += "</head><body>";

 

  html += "<div class='dashboard'>";

  html += "<div class='header'><h1><i class='fa-solid fa-shield-virus'></i> Sentinela IAQ</h1><p>Monitoramento de Ar e Ocupação</p></div>";

 

  // Card de Ocupação

  html += "<div class='card' style='width: auto; margin-bottom: 20px; " + bgOcupacao + "'>";

  html += "<div><i class='fa-solid fa-users' style='color: inherit;'></i></div>";

  html += "<div class='title' style='color: inherit; opacity: 0.8;'>Lotação da Sala (" + String(contadorPessoas) + "/" + String(CAPACIDADE_MAXIMA) + ")</div>";

  html += "<div class='value' style='color: inherit; font-size: 24px;'>" + textoOcupacao + "</div>";

  if(salaCheia) {

    html += "<div style='font-size: 12px; margin-top: 5px; font-weight: bold;'>MÁXIMO DE " + String(CAPACIDADE_MAXIMA) + " PESSOAS ATINGIDO</div>";

  }

  html += "</div>";



  // Cards de Sensores

  html += "<div class='grid'>";

  html += "<div class='card'><i class='fa-solid fa-temperature-half'></i><span class='title'>Temp</span><span class='value'>" + String(temperatura, 1) + " °C</span></div>";

  html += "<div class='card'><i class='fa-solid fa-droplet'></i><span class='title'>Umidade</span><span class='value'>" + String(umidade, 1) + " %</span></div>";

  html += "<div class='card' style='grid-column: span 2;'><i class='fa-solid fa-wind' style='color: " + corCO2 + "'></i><span class='title'>Nível de CO2</span><span class='value' style='color: " + corCO2 + "'>" + String(ppmCO2) + " ppm</span></div>";

  html += "</div>";

 

  // Card de Score Final

  html += "<div class='score-box'><div class='score-title'>ÍNDICE DE SEGURANÇA (SCORE)</div><div class='score-value'>" + String(scoreSeguranca, 1) + "</div></div>";

 

  // Botões de Controle

  html += "<div style='display: flex; gap: 10px;'>";

  html += "<a href='/entrada' class='btn' style='background: #00b894;'><i class='fa-solid fa-arrow-right-to-bracket'></i> Entrar</a>";

  html += "<a href='/saida' class='btn' style='background: #e17055;'><i class='fa-solid fa-arrow-right-from-bracket'></i> Sair</a>";

  html += "</div>";

  html += "<br><a href='/reset' class='btn' style='background: #636e72;'><i class='fa-solid fa-rotate-right'></i> Zerar Tudo</a>";

 

  html += "</div></body></html>";

  server.send(200, "text/html", html);

}



// --- Funções de Ação ---

void handleSaida() {

  if (contadorPessoas > 0) {

    contadorPessoas--;

  }

  server.sendHeader("Location", "/");

  server.send(303);

}



void handleEntrada() {

  if (contadorPessoas < CAPACIDADE_MAXIMA) {

    contadorPessoas++;

  }

  server.sendHeader("Location", "/");

  server.send(303);

}



void handleReset() {

  contadorPessoas = 0;

  portaObstruida = false;

  server.sendHeader("Location", "/");

  server.send(303);

}



// --- SETUP ---

void setup() {

  Serial.begin(115200);

  delay(500);

 

  pinMode(TRIG_PIN, OUTPUT);

  pinMode(ECHO_PIN, INPUT);

  pinMode(MQ_PIN, INPUT);

 

  dht.begin();

 

  Serial.println("\n--- INICIANDO SERVIDOR WEB SENTINELA IAQ ---");

  WiFi.begin(ssid, password);

 

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");

  }

 

  Serial.println("\nWi-Fi Conectado!");

  Serial.print("ENDEREÇO IP PARA DIGITAR NO CELULAR: ");

  Serial.println(WiFi.localIP());

 

  // Configuração das rotas

  server.on("/", handleRoot);

  server.on("/entrada", handleEntrada);

  server.on("/saida", handleSaida);

  server.on("/reset", handleReset);

 

  server.begin();

}



// --- LOOP PRINCIPAL ---

void loop() {

  server.handleClient();

 

  // ---------------------------------------------------------

  // 1. TAREFA RÁPIDA: VIGILÂNCIA DA PORTA (A CADA 50ms)

  // ---------------------------------------------------------

  if (millis() - tempoUltrassom >= intervaloUltrassom) {

    digitalWrite(TRIG_PIN, LOW);

    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);

    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);

   

    // Timeout de 30ms para não travar o ESP32

    long duracao = pulseIn(ECHO_PIN, HIGH, 30000);

    distanciaCM = duracao * 0.034 / 2;

   

    // Verifica se a leitura é válida e a pessoa está a até 5 cm de distância

    if (distanciaCM > 0 && distanciaCM <= DISTANCIA_PRESENCA) {

     

      // Só conta se a porta não estava obstruída E se já passou 1.5s desde a última entrada

      if (!portaObstruida && (millis() - cooldownPorta > 1500)) {

        portaObstruida = true;

        cooldownPorta = millis(); // Ativa a trava de segurança (1.5 segundos)

       

        if (contadorPessoas < CAPACIDADE_MAXIMA) {

          contadorPessoas++;

          Serial.println("Entrada detectada! Ocupantes: " + String(contadorPessoas));

        } else {

          Serial.println("Entrada bloqueada! Capacidade máxima atingida.");

        }

      }

     

    } else if (distanciaCM > DISTANCIA_PRESENCA) {

      // Pessoa afastou ou passou da porta

      portaObstruida = false;

    }

   

    tempoUltrassom = millis();

  }



  // ---------------------------------------------------------

  // 2. TAREFA LENTA: LEITURA DE AR E SCORE (A CADA 2 SEGUNDOS)

  // ---------------------------------------------------------

  if (millis() - tempoSensores >= intervaloSensores) {

    umidade = dht.readHumidity();

    temperatura = dht.readTemperature();

    int valorGas = analogRead(MQ_PIN);

   

    // --- Cálculo do Score ---

    ppmCO2 = map(valorGas, 0, 4095, 400, 2000);

    scoreSeguranca = 100.0;



    if (ppmCO2 > 800) scoreSeguranca -= (ppmCO2 - 800) * 0.05;

    if (umidade < 40 || umidade > 60) scoreSeguranca -= 15;

   

    // Penalidades de Lotação

    if (contadorPessoas >= CAPACIDADE_MAXIMA) {

      scoreSeguranca -= 40; // Penalidade máxima por aglomeração

    } else {

      scoreSeguranca -= (contadorPessoas * 5); // Perde 5 pontos por pessoa

    }

   

    if (scoreSeguranca < 0) scoreSeguranca = 0;

   

    tempoSensores = millis();

  }

}
