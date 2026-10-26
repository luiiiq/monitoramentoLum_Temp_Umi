// Projeto: Monitoramento Ambiental - Vinheria Agnello
// Equipe: Calm Tech Team 
// Integrantes:
// Caio Fernando De Deus Gomes
// Amanda Souza de Barros
// Luiza de Freitas Benevides
// Mariana do Nascimento

// Descrição: Sistema de monitoramento ambiental da Vinheria Agnello,
// responsável por acompanhar temperatura, umidade e luminosidade,
// exibindo os dados em um LCD e sinalizando condições do ambiente
// por meio de LEDs e alertas sonoros. Possui botões para navegação pelo menu e consulta
// de informações, relógio em tempo real (RTC), armazenamento de registros
// na EEPROM.

// Rev: 2 -  Segunda versão do projeto de monitoramento ambiental da Vinheria Agnello, com novas funcionalidades.
// Placa: Arduino Uno | Sensor: DHT22(Laboratório 11) | RTC: DS1307 | LCD I2C 16x2

// Inclusão de Biblioteca
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <RTClib.h> // Biblioteca para Relógio em Tempo Real
#include <EEPROM.h>
#include <avr/pgmspace.h> // Economizar a memória RAM do Arduino Uno.
#include <string.h>  // Biblioteca para manipulação de textos e blocos de memória

// Configuração do Display LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Cria o objeto responsável por acessar o relógio em tempo real.
RTC_DS1307 RTC;

const int botaoAlerta = 6;

// Definição dos pinos
int buzzer = 8;
int ledVerde = 13;
int ledAmarelo = 10;
int ledVermelho = 9;
int LDR = A0;

// Botões de configuração
const int botaoMenu = 3;
const int botaoConfirmar = 4;
const int botaoSair = 5;

// Configurações selecionáveis
bool usarFahrenheit = false;
bool idiomaIngles = false;

// Configurações salvas na EEPROM
struct ConfiguracoesSalvas {
  byte assinatura;
  byte fahrenheit;
  byte ingles;
  byte pais;
  int8_t diferencaHoras;
};

const byte ASSINATURA_CONFIG = 0xA7;
const int ENDERECO_CONFIG = 0;

// Variáveis para o Menu
bool menuAberto = false;
int opcaoMenu = 0;
int telaSelecionada = 0;
bool telaIndividualAtiva = false;

// Valor inicial LDR e porcentagem inicial
int valorLDR = 0;
int porcentagemLuz = 0;
int NUM_LEITURAS = 10;
// Calibração automática do LDR
int menorValorLDR = 1023;
int maiorValorLDR = 0;

unsigned long tempoInicioCalibracao = 0;
const unsigned long duracaoCalibracao = 10000; // 10 segundos

bool calibracaoConcluida = false;
// Média das leituras do LDR em 10 segundos
unsigned long tempoInicioMedia = 0;
const unsigned long intervaloMedia = 10000;
unsigned long somaLeituras = 0;
int quantidadeLeituras = 0;

// Variáveis do Buzzer
unsigned long tempoInicioBuzzer = 0;
bool buzzerTocando = false;
bool jaTocouNoAmarelo = false;

#define LOG_OPTION 1 // Opção para ativar a leitura do log
#define SERIAL_OPTION 1 // Opção de comunicação serial: 0 para desligado, 1 para ligado

// País selecionado: 0 = Brasil, 1 = Estados Unidos
int paisSelecionado = 0;

// Diferença em relação ao horário de Brasília.
// Brasil = 0; Nova York = -1 no horário de verão ou -2 no horário padrão.
int diferencaHoras = 0;

// Define o pino e o modelo do sensor de temperatura e umidade.
#define DHTPIN 2
#define DHTTYPE DHT11 // Lab é DHT11

// Cria o objeto que permite realizar leituras do sensor DHT22.
DHT dht(DHTPIN, DHTTYPE);

// Configurações dos registros de log
const int maxRecords = 100;

struct RegistroLog {
  uint32_t timestamp;
  int16_t temperatura;
  int16_t umidade;
};

const int recordSize = sizeof(RegistroLog);

// Os logs começam depois das configurações salvas
const int ENDERECO_INICIO_LOGS = sizeof(ConfiguracoesSalvas);
int currentAddress = ENDERECO_INICIO_LOGS;


//  Logs Verificação
bool modoConsultaLogs = false;

// Guarda o último minuto registrado para controlar a verificação por minuto.
int lastLoggedMinute = -1;

// Idioma
String idiomaUmidade = "";

// Controlar 1° Média
bool primeiraMediaConcluida = false;

// Controlando o início da média
bool telaCalculandoMediaExibida = false;

// Alerta Vermelho
bool alertaVermelhoRegistrado = false;

const byte frame01_0[8] PROGMEM = {
  B11110,
  B11110,
  B11111,
  B11111,
  B11110,
  B11100,
  B11100,
  B11100
};

const byte frame01_1[8] PROGMEM = {
  B11010,
  B11110,
  B11000,
  B10000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte frame01_2[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00001,
  B00011,
  B00010,
  B00110,
  B11111
};

const byte frame01_3[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00011,
  B11111,
  B10000
};

const byte frame01_4[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B11000,
  B10110,
  B00001,
  B11001,
  B00111
};

const byte frame01_5[8] PROGMEM = {
  B11100,
  B11100,
  B11111,
  B11111,
  B11111,
  B11111,
  B11110,
  B11111
};

const byte frame01_6[8] PROGMEM = {
  B00011,
  B01111,
  B11111,
  B11100,
  B10000,
  B00000,
  B00000,
  B00000
};

const byte frame01_7[8] PROGMEM = {
  B11000,
  B00000,
  B10000,
  B11000,
  B00000,
  B00000,
  B00000,
  B00000
};

// ZOOM A: Primeira fase da aproximação da folha à câmara
const byte zoomA0[8] PROGMEM = {
  B11100,
  B11110,
  B11111,
  B11111,
  B11110,
  B11100,
  B11000,
  B10000
};

const byte zoomA1[8] PROGMEM = {
  B11000,
  B11100,
  B11110,
  B11000,
  B10000,
  B00000,
  B00000,
  B00000
};

const byte zoomA2[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00011,
  B00111,
  B00110,
  B01100,
  B11100
};

const byte zoomA3[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00001,
  B00011,
  B00111,
  B11100,
  B10000
};

const byte zoomA4[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B10000,
  B11000,
  B01100,
  B00110,
  B00011
};

const byte zoomA5[8] PROGMEM = {
  B11000,
  B11100,
  B11110,
  B11111,
  B11111,
  B11110,
  B11100,
  B11000
};

const byte zoomA6[8] PROGMEM = {
  B00001,
  B00111,
  B01111,
  B11110,
  B11000,
  B10000,
  B00000,
  B00000
};

// ZOOM B: Transição intermédia de escala
const byte zoomB0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00011,
  B00111,
  B00100
};

const byte zoomB1[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10000,
  B11000,
  B01000
};

const byte zoomB2[8] PROGMEM = {
  B00000,
  B00000,
  B00001,
  B01111,
  B11111,
  B11110,
  B11000,
  B00000
};

const byte zoomB3[8] PROGMEM = {
  B00000,
  B00000,
  B00011,
  B11110,
  B11000,
  B10000,
  B00000,
  B00000
};

const byte zoomB4[8] PROGMEM = {
  B00000,
  B10000,
  B11000,
  B11100,
  B01110,
  B00110,
  B00000,
  B00000
};

const byte zoomB5[8] PROGMEM = {
  B00000,
  B00100,
  B01100,
  B11010,
  B01110,
  B00100,
  B00000,
  B00000
};


// FRAME 02: Estabilização da trajetória da folha
const byte frame02_0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00011,
  B00100
};

const byte frame02_1[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10000,
  B01000
};

const byte frame02_2[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00111,
  B11111,
  B11111,
  B11100,
  B00000
};

const byte frame02_3[8] PROGMEM = {
  B00000,
  B00000,
  B00111,
  B11100,
  B10000,
  B00000,
  B00000,
  B00000
};

const byte frame02_4[8] PROGMEM = {
  B01000,
  B10001,
  B11110,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte frame02_5[8] PROGMEM = {
  B00100,
  B10010,
  B11001,
  B01110,
  B00000,
  B00000,
  B00000,
  B00000
};

// FRAME 03 / 04: Estrutura estática da folha no ponto médio

const byte folhaBase0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00111,
  B11111,
  B11111,
  B11100,
  B00000
};

const byte folhaBase1[8] PROGMEM = {
  B00000,
  B00000,
  B00111,
  B11100,
  B10000,
  B00000,
  B00000,
  B00000
};

const byte frame03_2[8] PROGMEM = {
  B00000,
  B00001,
  B11010,
  B00101,
  B00010,
  B00001,
  B00000,
  B00000
};

const byte frame03_3[8] PROGMEM = {
  B01100,
  B10011,
  B00001,
  B10100,
  B00001,
  B00010,
  B11100,
  B00000
};

const byte frame03_4[8] PROGMEM = {
  B00000,
  B11010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000
};


// Folha Voando - FRAME A: Oscilação (Fase superior)

const byte vooA0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00010,
  B01100,
  B10000
};

const byte vooA1[8] PROGMEM = {
  B00000,
  B00000,
  B01000,
  B11000,
  B01000,
  B01000,
  B10000,
  B10000
};

const byte vooA2[8] PROGMEM = {
  B00000,
  B00001,
  B00010,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte vooA3[8] PROGMEM = {
  B11001,
  B01110,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};


// Folha Voando - FRAME B: Oscilação aerodinâmica (Fase inferior)

const byte vooB0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00100,
  B01000
};

const byte vooB1[8] PROGMEM = {
  B00000,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte vooB2[8] PROGMEM = {
  B11111,
  B00011,
  B11110,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte vooB3[8] PROGMEM = {
  B11000,
  B10000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

// Arrays para inversão
// Guardam temporariamente versões invertidas dos desenhos da folha.

byte invertido0[8];
byte invertido1[8];
byte invertido2[8];
byte invertido3[8];

// Transição Final - ENTRADA 1: Início do fecho da cena 
const byte entrada1_0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00110
};

const byte entrada1_1[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00110,
  B11000
};

// Transição Final - ENTRADA 2: Avanço do preenchimento geométrico
const byte entrada2_0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00010,
  B00100,
  B11000
};

const byte entrada2_1[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00001,
  B00110,
  B01000,
  B10000,
  B00000
};

const byte entrada2_2[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B10000,
  B01000,
  B00100,
  B00010
};


// Transição Final - ENTRADA 3: Quase oclusão do espaço

const byte entrada3_0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00010,
  B00100,
  B01000
};

const byte entrada3_1[8] PROGMEM = {
  B00000,
  B00000,
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000
};

const byte entrada3_2[8] PROGMEM = {
  B00000,
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000
};

const byte entrada3_3[8] PROGMEM = {
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000
};

// Frame Central - Folha mais Próxima: O pico do efeito "Macro/Zoom"
const byte close0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00010,
  B01100,
  B10000
};

const byte close1[8] PROGMEM = {
  B00100,
  B01000,
  B10000,
  B10000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte close2[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00110,
  B01000,
  B10000
};

const byte close3[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11110,
  B00001,
  B00000
};

const byte close4[8] PROGMEM = {
  B00001,
  B00010,
  B00100,
  B11000,
  B00000,
  B00000,
  B11000,
  B00110
};

const byte close5[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B01111,
  B00000,
  B00000,
  B00000
};

const byte close6[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00011,
  B00010,
  B00100,
  B01000
};

const byte close7[8] PROGMEM = {
  B00000,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000,
  B00000
};

// Transição Final - SAÍDA 1: Limpeza da tela por varrimento diagonal
const byte saida1_0[8] PROGMEM = {
  B00000,
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000
};

const byte saida1_1[8] PROGMEM = {
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000
};

const byte saida1_2[8] PROGMEM = {
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte saida1_3[8] PROGMEM = {
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

// Transição Final - SAÍDA 2: Conclusão do corte
const byte saida2_0[8] PROGMEM = {
  B00000,
  B00000,
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000
};

const byte saida2_1[8] PROGMEM = {
  B00000,
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000
};

const byte saida2_2[8] PROGMEM = {
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000
};

// Transição Final - SAÍDA 3
// Descrição: Frame final de corte geométrico antes da revelação do logótipo.
const byte saida3_0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00001,
  B00010,
  B00100,
  B01000,
  B10000
};

const byte saida3_1[8] PROGMEM = {
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000
};

// Logo
char logoLinha1[] = "   CALM TECH    ";
char logoLinha2[] = "      TEAM      ";

// Utilidades da Animação
// Reúnem operações repetidas de limpeza da tela e manipulação dos desenhos,
// deixando as funções de cada cena mais simples de acompanhar


// Limpa as Duas linhas LCD, evitando que partes de uma imagem antiga permaneçam visíveis.
void limpaTela() {
  lcd.setCursor(0, 0);
  lcd.print("                "); // Limpa tela superior

  lcd.setCursor(0, 1);
  lcd.print("                "); // Limpa tela inferior
}

// Carrega desenhos armazenados na memória Flash para o LCD
void criarCharFlash(byte posicao, const byte desenho[8]) {
  byte buffer[8];

  memcpy_P(buffer, desenho, 8);

  lcd.createChar(posicao, buffer);
}

// FRAME 01 -- posiciona cada parte da folha:
void mostrarFrame01() {
  criarCharFlash(0, frame01_0);
  criarCharFlash(1, frame01_1);
  criarCharFlash(2, frame01_2);
  criarCharFlash(3, frame01_3);
  criarCharFlash(4, frame01_4);
  criarCharFlash(5, frame01_5);
  criarCharFlash(6, frame01_6);
  criarCharFlash(7, frame01_7);

  limpaTela();

  lcd.setCursor(0, 0); lcd.write(byte(0));
  lcd.setCursor(1, 0); lcd.write(byte(1));
  lcd.setCursor(2, 0); lcd.write(byte(2));
  lcd.setCursor(3, 0); lcd.write(byte(3));
  lcd.setCursor(4, 0); lcd.write(byte(4));

  lcd.setCursor(0, 1); lcd.write(byte(5));
  lcd.setCursor(1, 1); lcd.write(byte(6));
  lcd.setCursor(2, 1); lcd.write(byte(7));
}

// ZOOM A
void mostrarZoomA() {
  criarCharFlash(0, zoomA0);
  criarCharFlash(1, zoomA1);
  criarCharFlash(2, zoomA2);
  criarCharFlash(3, zoomA3);
  criarCharFlash(4, zoomA4);
  criarCharFlash(5, zoomA5);
  criarCharFlash(6, zoomA6);

  limpaTela();

  lcd.setCursor(0, 0); lcd.write(byte(0));
  lcd.setCursor(1, 0); lcd.write(byte(1));
  lcd.setCursor(2, 0); lcd.write(byte(2));
  lcd.setCursor(3, 0); lcd.write(byte(3));
  lcd.setCursor(4, 0); lcd.write(byte(4));

  lcd.setCursor(0, 1); lcd.write(byte(5));
  lcd.setCursor(1, 1); lcd.write(byte(6));
}

// ZOOM B
void mostrarZoomB() {
  criarCharFlash(0, zoomB0);
  criarCharFlash(1, zoomB1);
  criarCharFlash(2, zoomB2);
  criarCharFlash(3, zoomB3);
  criarCharFlash(4, zoomB4);
  criarCharFlash(5, zoomB5);

  limpaTela();

  lcd.setCursor(2, 0); lcd.write(byte(0));
  lcd.setCursor(3, 0); lcd.write(byte(1));

  lcd.setCursor(0, 1); lcd.write(byte(2));
  lcd.setCursor(1, 1); lcd.write(byte(3));
  lcd.setCursor(2, 1); lcd.write(byte(4));
  lcd.setCursor(3, 1); lcd.write(byte(5));
}

// FRAME 02
void mostrarFrame02() {
  criarCharFlash(0, frame02_0);
  criarCharFlash(1, frame02_1);
  criarCharFlash(2, frame02_2);
  criarCharFlash(3, frame02_3);
  criarCharFlash(4, frame02_4);
  criarCharFlash(5, frame02_5);

  limpaTela();

  lcd.setCursor(2, 0); lcd.write(byte(0));
  lcd.setCursor(3, 0); lcd.write(byte(1));

  lcd.setCursor(0, 1); lcd.write(byte(2));
  lcd.setCursor(1, 1); lcd.write(byte(3));
  lcd.setCursor(2, 1); lcd.write(byte(4));
  lcd.setCursor(3, 1); lcd.write(byte(5));
}

// FRAME 03
void mostrarFrame03() {
  criarCharFlash(0, folhaBase0);
  criarCharFlash(1, folhaBase1);
  criarCharFlash(2, frame03_2);
  criarCharFlash(3, frame03_3);
  criarCharFlash(4, frame03_4);

  limpaTela();

  lcd.setCursor(0, 1); lcd.write(byte(0));
  lcd.setCursor(1, 1); lcd.write(byte(1));
  lcd.setCursor(2, 1); lcd.write(byte(2));
  lcd.setCursor(3, 1); lcd.write(byte(3));
  lcd.setCursor(4, 1); lcd.write(byte(4));
}

// Desenha uma parte do Logo
// Mostra somente a quantidade de colunas solicitada, permitindo revelar
// o texto gradualmente durante a animação de saída da folha.
void desenhaLogoAte(int quantidadeColunas) {
  if (quantidadeColunas < 0) {
    quantidadeColunas = 0;
  }

  if (quantidadeColunas > 16) {
    quantidadeColunas = 16;
  }

  for (int coluna = 0; coluna < quantidadeColunas; coluna++) {

    lcd.setCursor(coluna, 0);
    lcd.print(logoLinha1[coluna]);

    lcd.setCursor(coluna, 1);
    lcd.print(logoLinha2[coluna]);
  }
}

// SAÍDA 1
void mostrarSaida1() {
  criarCharFlash(0, saida1_0);
  criarCharFlash(1, saida1_1);
  criarCharFlash(2, saida1_2);
  criarCharFlash(3, saida1_3);

  limpaTela();

  desenhaLogoAte(5);

  lcd.setCursor(8, 1);
  lcd.write(byte(0));

  lcd.setCursor(9, 1);
  lcd.write(byte(1));

  lcd.setCursor(10, 0);
  lcd.write(byte(2));

  lcd.setCursor(11, 0);
  lcd.write(byte(3));
}

// SAÍDA 2
void mostrarSaida2() {
  criarCharFlash(0, saida2_0);
  criarCharFlash(1, saida2_1);
  criarCharFlash(2, saida2_2);

  limpaTela();

  desenhaLogoAte(10);

  lcd.setCursor(11, 0);
  lcd.write(byte(0));

  lcd.setCursor(12, 0);
  lcd.write(byte(1));

  lcd.setCursor(13, 0);
  lcd.write(byte(2));
}

// SAÍDA 3
void mostrarSaida3() {
  criarCharFlash(0, saida3_0);
  criarCharFlash(1, saida3_1);

  limpaTela();

  desenhaLogoAte(14);

  lcd.setCursor(14, 0);
  lcd.write(byte(0));

  lcd.setCursor(15, 0);
  lcd.write(byte(1));
}

void animacaoIntroducao() {
  // Folha aparece e se transforma rapidamente
  mostrarFrame01();
  delay(100);

  mostrarZoomA();
  delay(100);

  mostrarZoomB();
  delay(100);

  mostrarFrame02();
  delay(100);

  mostrarFrame03();
  delay(100);

  // A folha dá lugar à logo, que aparece gradualmente
  mostrarSaida1();
  delay(60);

  mostrarSaida2();
  delay(60);

  mostrarSaida3();
  delay(60);

  // Logo completa
  limpaTela();

  lcd.setCursor(3, 0);
  lcd.print("CALM TECH");

  lcd.setCursor(6, 1);
  lcd.print("TEAM");

  delay(600);
};

// Salva as configurações
void salvarConfiguracoes() {
  ConfiguracoesSalvas config;

  config.assinatura = ASSINATURA_CONFIG;
  config.fahrenheit = usarFahrenheit;
  config.ingles = idiomaIngles;
  config.pais = paisSelecionado;
  config.diferencaHoras = diferencaHoras;

  EEPROM.put(ENDERECO_CONFIG, config);
};

// Carrega as configurações
bool carregarConfiguracoes() {
  ConfiguracoesSalvas config;
  EEPROM.get(ENDERECO_CONFIG, config);

  if (config.assinatura != ASSINATURA_CONFIG) {
    return false;
  }

  usarFahrenheit = config.fahrenheit;
  idiomaIngles = config.ingles;
  paisSelecionado = config.pais;
  diferencaHoras = config.diferencaHoras;

  return true;
}

// Função Fuso
void mostrarOpcaoFusoUTC(int pais) {
  lcd.clear();

  lcd.setCursor(0, 0);

  if (pais == 0) {
    lcd.print("Brasil: Brasilia");
    lcd.setCursor(0, 1);
    lcd.print("UTC-03:00");
  } else {
    lcd.print("EUA: Nova York");
    lcd.setCursor(0, 1);
    lcd.print("UTC-05:00");
  };
};

// Configurações sobre as Medidas
void selecionarConfiguracoes() {
  int opcaoTemperatura = 0;
  int opcaoIdioma = 0;
  // Escolha da temperatura
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperatura:");
  lcd.setCursor(0, 1);
  lcd.print("> Celsius");
  while (true) {
    if (digitalRead(botaoMenu) == LOW) {
      delay(200);
      opcaoTemperatura = !opcaoTemperatura;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
    if (opcaoTemperatura == 0) {
      lcd.print("> Celsius");
    } else {
      lcd.print("> Fahrenheit");
    };
    while (digitalRead(botaoMenu) == LOW) {
      delay(10);
    };
  };
    if (digitalRead(botaoConfirmar) == LOW) {
      delay(200);
      usarFahrenheit = (opcaoTemperatura == 1);
      while (digitalRead(botaoConfirmar) == LOW) {
        delay(10);
      }
      break;
    };
  };

  // Escolha do idioma
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Idioma:");
  lcd.setCursor(0, 1);
  lcd.print("> Portugues");
  while (true) {
    if (digitalRead(botaoMenu) == LOW) {
      delay(200);
      opcaoIdioma = !opcaoIdioma;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      if (opcaoIdioma == 0) {
        lcd.print("> Portugues");
      } else {
        lcd.print("> English");
      };
      while (digitalRead(botaoMenu) == LOW) {
        delay(10);
      };
  };
    if (digitalRead(botaoConfirmar) == LOW) {
      delay(200);
      idiomaIngles = (opcaoIdioma == 1);
      while (digitalRead(botaoConfirmar) == LOW) {
        delay(10);
      };
      break;
    };
  };

  // Escolha do fuso horário UTC
  // Escolha do país e do fuso horário
  int opcaoPais = paisSelecionado;

  mostrarOpcaoFusoUTC(opcaoPais);

while (true) {
  // Botão Menu: alterna entre Brasil e Estados Unidos
  if (digitalRead(botaoMenu) == LOW) {
    delay(200);

    opcaoPais = !opcaoPais;

    mostrarOpcaoFusoUTC(opcaoPais);

    while (digitalRead(botaoMenu) == LOW) {
      delay(10);
    };
  };

  // Botão Confirmar: salva a escolha
  if (digitalRead(botaoConfirmar) == LOW) {
    delay(200);

    paisSelecionado = opcaoPais;

    if (paisSelecionado == 0) {
      diferencaHoras = 0;  // Brasil: mantém o horário de Brasília
    } else {
      diferencaHoras = -1; // Nova York: horário de verão
    };

    while (digitalRead(botaoConfirmar) == LOW) {
      delay(10);
    };

    break;
  };
};

  if (idiomaIngles) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Settings");
    lcd.setCursor(0, 1);
    lcd.print("saved!");
    delay(1500);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Configuracoes");
    lcd.setCursor(0, 1);
    lcd.print("salvas!");
    delay(1500);
  };
  // Salva as escolhas na EEPROM
  salvarConfiguracoes();
};

// Reinciar Setup
bool perguntarReiniciarSetup() {
  int opcao = 0; // 0 = Sim, 1 = Nao

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Refazer setup?");

  while (true) {
    lcd.setCursor(0, 1);

    if (opcao == 0) {
      lcd.print(">Sim     Nao   ");
    } else {
      lcd.print(" Sim    >Nao   ");
    }

    // Botao Menu: alterna entre Sim e Nao
    if (digitalRead(botaoMenu) == LOW) {
      delay(200);
      opcao = !opcao;

      while (digitalRead(botaoMenu) == LOW) {
        delay(10);
      }
    }

    // Botao Confirmar: confirma a escolha
    if (digitalRead(botaoConfirmar) == LOW) {
      delay(200);

      while (digitalRead(botaoConfirmar) == LOW) {
        delay(10);
      }

      return opcao == 0;
    };
  };
};

// Converte o horário de Brasília para o país selecionado.
DateTime aplicarFusoUTC(DateTime horarioBrasil) {
  return horarioBrasil + TimeSpan(0, diferencaHoras, 0, 0);
};

// Ajusta a Data
void ajustarData() {
  DateTime atual = aplicarFusoUTC(RTC.now());

  int dia = atual.day();
  int mes = atual.month();
  int ano = atual.year();
  int campo = 0;

  while (campo < 3) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(idiomaIngles ? "Set date" : "Ajustar data");

    lcd.setCursor(0, 1);

    if (campo == 0) {
      lcd.print(idiomaIngles ? "Day: " : "Dia: ");
      lcd.print(dia);
    } else if (campo == 1) {
      lcd.print(idiomaIngles ? "Month: " : "Mes: ");
      lcd.print(mes);
    } else {
      lcd.print(idiomaIngles ? "Year: " : "Ano: ");
      lcd.print(ano);
    }

    if (digitalRead(botaoMenu) == LOW) {
      delay(200);

      if (campo == 0) {
        dia = (dia % 31) + 1;
      } else if (campo == 1) {
        mes = (mes % 12) + 1;
      } else {
        ano++;
        if (ano > 2099) ano = 2026;
      }

      while (digitalRead(botaoMenu) == LOW) {
        delay(10);
      }
    }

    if (digitalRead(botaoConfirmar) == LOW) {
      delay(200);
      campo++;

      while (digitalRead(botaoConfirmar) == LOW) {
        delay(10);
      }
    }
  }

  // Corrige o dia para meses com menos de 31 dias.
  int diasMes;

  if (mes == 2) {
    diasMes = ((ano % 4 == 0 && ano % 100 != 0) ||
               ano % 400 == 0) ? 29 : 28;
  } else if (mes == 4 || mes == 6 ||
             mes == 9 || mes == 11) {
    diasMes = 30;
  } else {
    diasMes = 31;
  }

  if (dia > diasMes) dia = diasMes;

  // Converte o horário local para o horário-base do RTC.
  DateTime novaData(ano, mes, dia,
                    atual.hour(), atual.minute(),
                    atual.second());

  RTC.adjust(novaData - TimeSpan(0, diferencaHoras, 0, 0));

  lcd.clear();
  lcd.print(idiomaIngles ? "Date adjusted!" : "Data ajustada!");
  delay(1200);
}

// Ajusta a Hora
void ajustarHora() {
  DateTime atual = aplicarFusoUTC(RTC.now());

  int hora = atual.hour();
  int minuto = atual.minute();
  int campo = 0;

  while (campo < 2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(idiomaIngles ? "Set time" : "Ajustar hora");

    lcd.setCursor(0, 1);

    if (campo == 0) {
      lcd.print(idiomaIngles ? "Hour: " : "Hora: ");
      if (hora < 10) lcd.print("0");
      lcd.print(hora);
    } else {
      lcd.print(idiomaIngles ? "Min: " : "Min: ");
      if (minuto < 10) lcd.print("0");
      lcd.print(minuto);
    }

    if (digitalRead(botaoMenu) == LOW) {
      delay(200);

      if (campo == 0) {
        hora = (hora + 1) % 24;
      } else {
        minuto = (minuto + 1) % 60;
      }

      while (digitalRead(botaoMenu) == LOW) {
        delay(10);
      }
    }

    if (digitalRead(botaoConfirmar) == LOW) {
      delay(200);
      campo++;

      while (digitalRead(botaoConfirmar) == LOW) {
        delay(10);
      }
    }
  }

  DateTime novaHora(atual.year(), atual.month(),
                    atual.day(), hora, minuto, 0);

  RTC.adjust(novaHora - TimeSpan(0, diferencaHoras, 0, 0));

  lcd.clear();
  lcd.print(idiomaIngles ? "Time adjusted!" : "Hora ajustada!");
  delay(1200);
};

// Converte a temperatura somente para exibicao.
float converterTemperatura(float temperaturaC) {
  if (usarFahrenheit) {
    return (temperaturaC * 9.0 / 5.0) + 32.0;
  }
    return temperaturaC;
};

// Retorna a unidade selecionada.
const char* unidadeTemperatura() {
  if (usarFahrenheit) {
    return " F";
  };
  return " C";
};

// Calibração automática do sensor LDR
void calibrarLDR() {
  int leitura = analogRead(LDR);

  // Identifica o menor valor registrado
  if (leitura < menorValorLDR) {
    menorValorLDR = leitura;
  };

  // Identifica o maior valor registrado
  if (leitura > maiorValorLDR) {
    maiorValorLDR = leitura;
  };

  // Finaliza a calibração após 10 segundos
  if (millis() - tempoInicioCalibracao >= duracaoCalibracao) {
    calibracaoConcluida = true;
  };
};

// Desliga Leds e Buzzer
void apagaLeds() {
  digitalWrite(ledVerde,LOW); // Desliga LED Verde
  digitalWrite(ledAmarelo,LOW); // Desliga LED Amarelo
  digitalWrite(ledVermelho,LOW); // Desliga LED Vermelho
  noTone(buzzer); // Desliga buzzer
  buzzerTocando = false; // buzzer volta pra false
};

// Exibe o nome do fator analisado, seu valor e sua unidade no LCD.
void chamadaFator(String fator, float valor, String simbolo) {
  lcd.setCursor(0, 0);
  lcd.print("                "); // Apaga a primeira linha
  lcd.setCursor(0, 0);  
  lcd.print(fator);
  lcd.print(": ");
  lcd.print(valor);
  lcd.print(simbolo);
  lcd.setCursor(0, 1);
  lcd.print("                "); // Apaga a segunda linha
  lcd.setCursor(0, 1);
  lcd.print(F("Status: "));
};

// Valores Temperaturas
void temperaturaAnalise(float valorTemp) {
  if (idiomaIngles) {
    if(valorTemp < 10) {
      lcd.print(F("LOW"));
    } else if (valorTemp >= 10 && valorTemp <= 16) {
      lcd.print(F("OK"));
    } else {
      lcd.print(F("HIGH"));
  };
  } else {
    if(valorTemp < 10) {
      lcd.print(F("BAIXA"));
    } else if (valorTemp >= 10 && valorTemp <= 16) {
        lcd.print(F("OK"));
    } else {
        lcd.print(F("ALTA"));
    };
  };
};

// Valores Umidade
void umidadeAnalise(float valorUmidade) {
  if (idiomaIngles) {
    if(valorUmidade < 57) {
    lcd.print(F("LOW"));
  } else if (valorUmidade >= 57 && valorUmidade <= 76) {
    lcd.print(F("OK"));
  } else {
    lcd.print(F("HIGH"));
  };
  } else {
    if(valorUmidade < 57) {
    lcd.print(F("BAIXA"));
  } else if (valorUmidade >= 57 && valorUmidade <= 76) {
    lcd.print(F("OK"));
  } else {
      lcd.print(F("ALTA"));
    };
  };
};

// Valores Porcentagem
void analisePorcentagem(float valorPorcento) {
  if (valorPorcento <= 25) {
    lcd.setCursor(0, 0);
    lcd.print(idiomaIngles ? "Very DARK" : "Ambiente muito"); 
    lcd.setCursor(0, 1);
    lcd.print(idiomaIngles ? "Ideal" : "ESCURO: ideal");
  } else if (valorPorcento > 80) {
    lcd.setCursor(0, 0);
    lcd.print(idiomaIngles ? "Very BRIGHT" : "Ambiente muito"); 
    lcd.setCursor(0, 1);
    lcd.print(idiomaIngles ? "ERROR" : "CLARO");
  } else {
    lcd.setCursor(0, 0);
    lcd.print(idiomaIngles ? "Dimly lit" : "Ambiente a meia"); 
    lcd.setCursor(0, 1);
    lcd.print(idiomaIngles ? "environment" : "luz");
    delay(2000);
  };
};

// Mostrar o Menu de opções
void mostrarOpcaoMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(idiomaIngles ? "View" : "Visualizar:");
  lcd.setCursor(0, 1);
  switch (opcaoMenu) {
  case 0:
    lcd.print(idiomaIngles ? "> Temperature" : "> Temperatura");
    telaSelecionada = 0;
    break;
  case 1:
    lcd.print(idiomaIngles ? "> Humidity" : "> Umidade");
    telaSelecionada = 1;
    break;
  case 2:
    lcd.print(idiomaIngles ? "> Brightness" : "> Luminosidade");
    telaSelecionada = 2;
    break;
  case 3:
    lcd.print(idiomaIngles ? "> Date" : "> Data");
    telaSelecionada = 3;
    break;
  case 4:
    lcd.print(idiomaIngles ? "> Time" : "> Hora");
    telaSelecionada = 4;
    break;
  case 5:
    lcd.print(idiomaIngles ? "> Set date" : "> Ajustar data");
    telaSelecionada = 5;
    break;
  case 6:
    lcd.print(idiomaIngles ? "> Set time" : "> Ajustar hora");
    telaSelecionada = 6;
    break;
  };
};

// Auxiliar: para abrir este menu
void abrirMenu() {
  menuAberto = true;
  opcaoMenu = 0;
  mostrarOpcaoMenu();
};

// Tratar os buttons e a escolha
void processarMenu() {
  if (modoConsultaLogs) {
    if (digitalRead(botaoSair) == LOW) {
      delay(200);
      modoConsultaLogs = false;
      menuAberto = false;
      telaIndividualAtiva = false;
      lcd.clear();
      lcd.print(idiomaIngles ? "Monitoring" : "Monitoramento");
      lcd.setCursor(0, 1);
      lcd.print(idiomaIngles ? "resumed..." : "retomado...");
      delay(1000);
      lcd.clear();
      while (digitalRead(botaoSair) == LOW) {
        delay(10);
      };
    };
    return;
  }
  if (!menuAberto) {
    if (digitalRead(botaoMenu) == LOW) {
      delay(200);
      abrirMenu();
      while (digitalRead(botaoMenu) == LOW) {
        delay(10);
      };
    };
    return;
  };
  if (digitalRead(botaoMenu) == LOW) {
    delay(200);
    opcaoMenu = (opcaoMenu + 1) % 7;
    mostrarOpcaoMenu();
    while (digitalRead(botaoMenu) == LOW) {
      delay(10);
    };
  };
  if (digitalRead(botaoConfirmar) == LOW) {
    delay(200);
    telaSelecionada = opcaoMenu;
    telaIndividualAtiva = true;
    menuAberto = false;
    while (digitalRead(botaoConfirmar) == LOW) {
      delay(10);
    };
    return;
  };
  if (digitalRead(botaoSair) == LOW) {
    delay(200);
    menuAberto = false;
    telaIndividualAtiva = false;
    lcd.clear();
    lcd.print(idiomaIngles ? "Monitoring" : "Monitoramento");
    lcd.setCursor(0, 1);
    lcd.print(idiomaIngles ? "resumed..." : "retomado...");
    delay(1000);
    lcd.clear();
    while (digitalRead(botaoSair) == LOW) {
      delay(10);
    };
  };
};

//  Salvar os Logs
void salvarLog(DateTime horario, float temperatura, float umidade) {
  if (isnan(temperatura) || isnan(umidade)) {
    return;
  };
  RegistroLog registro;
  registro.timestamp = horario.unixtime();
  registro.temperatura = (int16_t)(temperatura * 100);
  registro.umidade = (int16_t)(umidade * 100);
  EEPROM.put(currentAddress, registro);
  currentAddress += recordSize;
  if (currentAddress + recordSize > EEPROM.length()) {
    currentAddress = ENDERECO_INICIO_LOGS;
  };
};

// Mostrar a opção selecionada, mudando de tela
void mostrarTelaSelecionada(float temperatura,float umidade,int luminosidade, DateTime horario) {
  lcd.clear();
  switch (telaSelecionada) {
    case 0:
      lcd.print(idiomaIngles ? "Temperature:" : "Temperatura:");
      lcd.setCursor(0, 1);
      lcd.print(converterTemperatura(temperatura), 1);
      lcd.print(unidadeTemperatura());
      break;
    case 1:
      lcd.print(idiomaIngles ? "Humidity" : "Umidade:");
      lcd.setCursor(0, 1);
      lcd.print(umidade, 1);
      lcd.print("%");
      break;
    case 2:
      lcd.print(idiomaIngles ? "Brightness" : "Luminosidade:");
      lcd.setCursor(0, 1);
      lcd.print(luminosidade);
      lcd.print("%");
      break;
    case 3:
      lcd.print(idiomaIngles ? "Date:" : "Data:");
      lcd.setCursor(0, 1);
      if (idiomaIngles) {
        if (horario.month() < 10) lcd.print(F("0"));
          lcd.print(horario.month());
          lcd.print("/");
        if (horario.day() < 10) lcd.print(F("0"));
          lcd.print(horario.day());
          lcd.print("/");
      } else {
        if (horario.day() < 10) lcd.print(F("0"));
          lcd.print(horario.day());
          lcd.print("/");
          if (horario.month() < 10) lcd.print(F("0"));
            lcd.print(horario.month());
            lcd.print("/");
      }
      lcd.print(horario.year());
      break;
    case 4:
      lcd.print(idiomaIngles ? "Time:" : "Hora:");
      lcd.setCursor(0, 1);
      if (horario.hour() < 10) lcd.print(F("0"));
        lcd.print(horario.hour());
        lcd.print(":");
      if (horario.minute() < 10) lcd.print(F("0"));
        lcd.print(horario.minute());
        lcd.print(":");
      if (horario.second() < 10) lcd.print(F("0"));
        lcd.print(horario.second());
        break;
    case 5:
      ajustarData();
      break;
    case 6:
      ajustarHora();
      break;
  };
};

void setup() {
  // Configuração das portas
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(LDR, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(botaoMenu, INPUT_PULLUP);
  pinMode(botaoConfirmar, INPUT_PULLUP);
  pinMode(botaoSair, INPUT_PULLUP);

  // Inicia todos os LEDs apagados
  apagaLeds();

  // Inicializa o sensor DHT22.
  dht.begin();

  // Inicia a comunicação serial com velocidade de 9600 bits por segundo.
  Serial.begin(9600);

  // Inicializa o display LCD e ativa sua iluminação.
  lcd.init();
  lcd.backlight();

  // Inicializa o RTC
  if (!RTC.begin()) {
    Serial.println("Erro ao iniciar o RTC!");
    lcd.clear();
    lcd.print(F("Erro no RTC!"));
    while (true) {
      delay(100);
    };
  };

  // Ajusta o relógio
  if (!RTC.isrunning()) {
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  };

  // Executa a animação de entrada 
  animacaoIntroducao();

  // Inicia a calibração automática do LDR
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrando LDR");
  lcd.setCursor(0, 1);
  lcd.print("Aguarde 10 seg");

  tempoInicioCalibracao = millis();

  while (!calibracaoConcluida) {
    calibrarLDR();
    delay(10);
  };

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibracao OK");
  delay(1000);

  // Recupera as configuracoes salvas na EEPROM
  if (!carregarConfiguracoes()) {
    // Primeira inicializacao: configura obrigatoriamente
    selecionarConfiguracoes();
  } else {
    // Ja existem configuracoes: pergunta se deseja refazer
    if (perguntarReiniciarSetup()) {
      selecionarConfiguracoes();
    }
    // Se escolher Nao, continua com as configuracoes salvas
  };

  // Mudança na variável em razão di idioma
  if (idiomaIngles) {
    idiomaUmidade = "Humidity";
  }
  else {
  idiomaUmidade = "Umidade";
  };

  // Iniciando Monitoramento animação
  lcd.clear();
  if (idiomaIngles) {
    lcd.setCursor(2, 0);
    lcd.print(F("Initiating"));
    lcd.setCursor(2, 1);
    lcd.print(F("monitoring"));
    delay(2000);
    // Sejam Bem-vindos(a)
    lcd.clear();
    lcd.setCursor(5, 1);
    lcd.print(F("Welcome!"));
    delay(600);
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print(F("Welcome!"));
    delay(600);
    // Faz aparecer embaixo
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print(F("Welcome!"));
    delay(1000);
  }
  else {
    lcd.setCursor(2, 0);
    lcd.print(F("Iniciando"));
    lcd.setCursor(2, 1);
    lcd.print(F("monitoramento"));
    delay(2000);
    // Sejam Bem-vindos(a)
    lcd.clear();
    lcd.setCursor(5, 1);
    lcd.print(F("Sejam"));
    delay(600);
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print(F("Sejam"));
    delay(600);
    // Faz "Bem-vindos(a)" aparecer embaixo
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print(F("Sejam"));
    lcd.setCursor(1, 1);
    lcd.print(F("Bem-vindos(a)"));
    delay(1000);
    // Depois "Bem-vindos(a)" sobe
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print(F("Bem-vindos(a)"));
  };

  delay(1200);
  lcd.clear();
  delay(500);

  // Obtém a data atual do RTC
  // Exibe a data conforme o idioma selecionado
  DateTime now = aplicarFusoUTC(RTC.now());

  lcd.clear();
  lcd.setCursor(0, 0);
  if (idiomaIngles) {
    lcd.print(F("Today's date:"));
  } else {
    lcd.print(F("Data de hoje:"));
  };
  lcd.setCursor(0, 1);
  if (idiomaIngles) {
    // Formato americano: MM/DD/YYYY
    if (now.month() < 10) lcd.print(F("0"));
    lcd.print(now.month());
    lcd.print("/");
    if (now.day() < 10) lcd.print(F("0"));
    lcd.print(now.day());
    lcd.print("/");
  } else {
    // Formato brasileiro: DD/MM/AAAA
    if (now.day() < 10) lcd.print(F("0"));
    lcd.print(now.day());
    lcd.print("/");
    if (now.month() < 10) lcd.print(F("0"));
    lcd.print(now.month());
    lcd.print("/");
    };
    lcd.print(now.year());
    delay(3000);
    lcd.clear();
    delay(500);
};


void enviarDados(String estado,int porcentagem,float temperatura,float umidade,DateTime horario) {
  Serial.print("{\"estado\":\"");
  Serial.print(estado);

  Serial.print("\",\"idioma\":\"");
  Serial.print(idiomaIngles ? "en" : "pt");

  Serial.print("\",\"unidade\":\"");
  Serial.print(usarFahrenheit ? "fahrenheit" : "celsius");

  Serial.print("\",\"porcentagem\":");
  Serial.print(porcentagem);

  Serial.print(",\"temperatura\":");
  Serial.print(temperatura, 2);

  Serial.print(",\"umidade\":");
  Serial.print(umidade, 2);

  Serial.print(",\"data\":\"");
  if (idiomaIngles) {
    // Formato americano: MM/DD/YYYY
    if (horario.month() < 10) Serial.print(F("0"));
      Serial.print(horario.month());
      Serial.print("/");
      if (horario.day() < 10) Serial.print(F("0"));
      Serial.print(horario.day());
      Serial.print("/");
  } else {
    // Formato brasileiro: DD/MM/AAAA
      if (horario.day() < 10) Serial.print(F("0"));
      Serial.print(horario.day());
      Serial.print("/");
      if (horario.month() < 10) Serial.print(F("0"));
      Serial.print(horario.month());
      Serial.print("/");
    };

    Serial.print(horario.year());

  // Envia a hora no formato HH:MM:SS
  Serial.print("\",\"hora\":\"");
  if (horario.hour() < 10) Serial.print("0");
  Serial.print(horario.hour());
  Serial.print(":");
  if (horario.minute() < 10) Serial.print("0");
  Serial.print(horario.minute());
  Serial.print(":");
  if (horario.second() < 10) Serial.print("0");
  Serial.print(horario.second());
  Serial.println("\"}");
};

void registrarEstadoVermelho(
  DateTime horario,
  int porcentagem,
  float temperatura,
  float umidade
) {
  Serial.println();

  if (idiomaIngles) {
    Serial.println(F("ALERT: RED STATE"));

    Serial.print(F("Date: "));
    if (horario.month() < 10) Serial.print("0");
    Serial.print(horario.month());
    Serial.print("/");
    if (horario.day() < 10) Serial.print("0");
    Serial.print(horario.day());
    Serial.print("/");
    Serial.println(horario.year());

    Serial.println(F("State: RED"));

    Serial.print(F("Time: "));
  } else {
    Serial.println(F("ALERTA: ESTADO VERMELHO"));

    Serial.print(F("Data: "));
    if (horario.day() < 10) Serial.print("0");
    Serial.print(horario.day());
    Serial.print("/");
    if (horario.month() < 10) Serial.print("0");
    Serial.print(horario.month());
    Serial.print("/");
    Serial.println(horario.year());

    Serial.println(F("Estado: VERMELHO"));

    Serial.print(F("Hora: "));
  }

  if (horario.hour() < 10) Serial.print("0");
  Serial.print(horario.hour());
  Serial.print(":");
  if (horario.minute() < 10) Serial.print("0");
  Serial.print(horario.minute());
  Serial.print(":");
  if (horario.second() < 10) Serial.print("0");
  Serial.println(horario.second());

  Serial.print(idiomaIngles ? F("Reason: ") : F("Motivo: "));

  bool primeiroMotivo = true;

  if (porcentagem > 80) {
    Serial.print(idiomaIngles ? F("Brightness") : F("Luminosidade"));
    primeiroMotivo = false;
  }

  if (temperatura > 16) {
    if (!primeiroMotivo) Serial.print(F(" + "));
    Serial.print(idiomaIngles ? F("Temperature") : F("Temperatura"));
    primeiroMotivo = false;
  }

  if (umidade > 76) {
    if (!primeiroMotivo) Serial.print(F(" + "));
    Serial.print(idiomaIngles ? F("Humidity") : F("Umidade"));
  }

  Serial.println();
  Serial.println(F("------------------------"));
};

void loop() {
  // Controlando o menu
  processarMenu();

  // Desliga o buzzer se estiver navegando pelo menu
  if (menuAberto) {
    noTone(buzzer);
    buzzerTocando = false;
    return;
  };

  // Inicia o intervalo da média
  if (tempoInicioMedia == 0) {
    tempoInicioMedia = millis();
  }

  // Faz uma leitura do LDR por ciclo
  valorLDR = analogRead(LDR);
  somaLeituras += valorLDR;
  quantidadeLeituras++;


  // Calcula a média ao completar 10 segundos
  if (millis() - tempoInicioMedia >= intervaloMedia) {
    if (quantidadeLeituras > 0) {
      valorLDR = somaLeituras / quantidadeLeituras;

      // Indica que a primeira média foi calculada
      primeiraMediaConcluida = true;
    };

    somaLeituras = 0;
    quantidadeLeituras = 0;
    tempoInicioMedia = millis();
  };

  // Aguarda a primeira média antes de iniciar o monitoramento
  if (!primeiraMediaConcluida) {

    // Limpa a tela somente na primeira exibição
    if (!telaCalculandoMediaExibida) {
      lcd.clear();
      telaCalculandoMediaExibida = true;
    }

    lcd.setCursor(0, 0);

    if (idiomaIngles) {
      lcd.print(F("Calculating mean"));
      lcd.setCursor(0, 1);
      lcd.print(F("Please wait..."));
    } else {
      lcd.print(F("Calculando media"));
      lcd.setCursor(0, 1);
      lcd.print(F("Aguarde..."));
    }

    apagaLeds();
    return;
  };

  // map() transforma a faixa calibrada da leitura do LDR em uma escala
  // aproximada de porcentagem para apresentação no LCD.
  // Converte a leitura do LDR usando os valores obtidos na calibração
  if (maiorValorLDR > menorValorLDR) {
    porcentagemLuz = map(
      valorLDR,
      menorValorLDR,
      maiorValorLDR,
      0,
      100
    );
  } else {
    porcentagemLuz = map(valorLDR, 0, 1023, 100, 0);
  };

  porcentagemLuz = constrain(porcentagemLuz, 0, 100);

  DateTime horarioUTC = RTC.now();
  DateTime adjustedTime = aplicarFusoUTC(horarioUTC);

  // Realiza a leitura da umidade do ar e da temperatura
  float umidade = dht.readHumidity(); 
  float temperatura = dht.readTemperature();

  // Trata caso possível falha
  if (isnan(temperatura) || isnan(umidade)) {
    apagaLeds();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Erro no sensor"));
    lcd.setCursor(0, 1);
    lcd.print(F("Verifique DHT22"));
    Serial.println("Erro na leitura do DHT22!");
    delay(1000);
    return;
  };
  if (telaIndividualAtiva) {
    mostrarTelaSelecionada(
    temperatura,
    umidade,
    porcentagemLuz,
    adjustedTime
  );
  delay(3000);
  telaIndividualAtiva = false;
  lcd.clear();
  };

  // Verifica se o minuto atual é diferente do minuto do último registro
  if (adjustedTime.minute() != lastLoggedMinute) {
    lastLoggedMinute = adjustedTime.minute(); // Atualiza o minuto armazenado.
    digitalWrite(LED_BUILTIN, HIGH); // Acende o LED interno do Arduino.
    delay(1000); // espera por 1 segundo
    digitalWrite(LED_BUILTIN, LOW);//Apaga o LED interno.
    delay(1000);
    // Verifica se houve erro na leitura da temperatura ou da umidade.
    if (isnan(temperatura) || isnan(umidade)) {
      Serial.println("Failed to read from DHT22 sensor!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Sensor Error!"));
      delay(2000);
      return;
    };
    salvarLog(
      horarioUTC,
      temperatura,
      umidade
    );
  };

  delay(100);
  lcd.clear();
  // Led Verde: Temperatura entre 10°C - 16°C e Umidade entre 57% - 76% e Luminosidade entre 0% - 25%
  if (porcentagemLuz <= 25 && temperatura >= 10 && temperatura <= 16 && umidade >= 57 && umidade <= 76) {
    apagaLeds(); 
    digitalWrite(ledVerde, HIGH);
    alertaVermelhoRegistrado = false;

    enviarDados("VERDE", porcentagemLuz, temperatura, umidade, adjustedTime);

    jaTocouNoAmarelo = false;
    chamadaFator("Temp.", converterTemperatura(temperatura), unidadeTemperatura());
    lcd.print(F("OK"));
    delay(1000);
    lcd.clear();
    chamadaFator(idiomaUmidade, umidade, "%");
    lcd.print(F("OK"));
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(idiomaIngles ? "Very DARK" : "Ambiente muito"); 
    lcd.setCursor(0, 1);
    lcd.print(idiomaIngles ? "Ideal" : "ESCURO: ideal");
    delay(1000);
  }
  // Led Vermelho: Temperatura maior que 16°C ou Umidade maior que 76% ou Luminosidade maior que 80%
  else if(porcentagemLuz > 80 || temperatura > 16 || umidade > 76) {
    apagaLeds();
    if (!alertaVermelhoRegistrado) {
      registrarEstadoVermelho(
        adjustedTime,
        porcentagemLuz,
        temperatura,
        umidade
      );
      alertaVermelhoRegistrado = true;
    };
    // Buzzer tocando em intervalos para avisar ERRO
    if (!buzzerTocando) {
      tone(buzzer, 1000);
      delay(800);
    };
    digitalWrite(ledVermelho, HIGH);

    enviarDados("VERMELHO", porcentagemLuz, temperatura, umidade, adjustedTime);

    jaTocouNoAmarelo = false;
    chamadaFator("Temp.", converterTemperatura(temperatura), unidadeTemperatura());
    temperaturaAnalise(temperatura);
    delay(1000);
    lcd.clear();
    chamadaFator(idiomaUmidade, umidade, "%");
    umidadeAnalise(umidade);
    delay(1000);
    lcd.clear();
    analisePorcentagem(porcentagemLuz);
    delay(1000);
  }
  // Led Amarelo: Temperatura menor 10°C ou Umidade menor que 57% ou Luminosidade maior que  25% e menor que 80%
  else {
    apagaLeds();
    alertaVermelhoRegistrado = false;
    // Aciona o buzzer novamente enquanto o ambiente
    // permanecer na faixa de alerta.
    // O sinal toca por 3 segundos e aguarda
    // antes de iniciar um novo ciclo.
    if (!buzzerTocando &&
      millis()  >= 9000) {
      tone(buzzer, 1000);
      tempoInicioBuzzer = millis();
      buzzerTocando = true;
    };
    digitalWrite(ledAmarelo, HIGH);

    enviarDados("AMARELO", porcentagemLuz, temperatura, umidade, adjustedTime);

    chamadaFator("Temp.", converterTemperatura(temperatura), unidadeTemperatura());
    temperaturaAnalise(temperatura);
    delay(1000);
    lcd.clear();
    chamadaFator(idiomaUmidade, umidade, "%");
    umidadeAnalise(umidade);
    delay(1000);
    lcd.clear();
    analisePorcentagem(porcentagemLuz);
    delay(1000);
    }
    // Desliga Buzzer depois de 3 segundos  e atualiza
    // o estado buzzerTocando para permitir novos acionamentos.
    if (buzzerTocando) {
      if (
      millis() - tempoInicioBuzzer
      >= 3000
      ) {
      noTone(buzzer);
      buzzerTocando =
      false;
      };
      };
      delay(20);
};
