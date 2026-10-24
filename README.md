# Monitoramento Ambiental — Vinheria Agnello
O Monitoramento Ambiental da Vinheria Agnello é um sistema desenvolvido com Arduino Uno para acompanhar as condições ambientais do local de armazenamento de vinhos, considerando temperatura, umidade do ar e luminosidade.

O sistema realiza leituras dos sensores, apresenta informações em um display LCD 16x2 e sinaliza as condições identificadas por meio de LEDs e alertas sonoros. Também disponibiliza um menu interativo para consultar informações, configurar preferências e ajustar data e hora.

Além disso, utiliza um relógio de tempo real (RTC) para registrar referências temporais e a memória EEPROM para preservar configurações e armazenar registros de temperatura e umidade.

O projeto também possui comunicação serial, permitindo transmitir os dados ambientais em formato JSON para integração com uma interface externa.

## Equipe
**Calm Tech Team**

- Caio Fernando De Deus Gomes
- Amanda Souza de Barros
- Luiza de Freitas Benevides
- Mariana do Nascimento

## Objetivo do projeto
• Monitorar continuamente a temperatura, a umidade e a luminosidade do ambiente.
• Identificar condições ambientais adequadas, de atenção e críticas.
• Facilitar a visualização dos dados por meio de um display LCD.
• Sinalizar situações de alerta com LEDs e buzzer.
• Registrar a data e a hora das ocorrências do estado vermelho no Monitor Serial.
• Identificar os fatores responsáveis por cada ocorrência do estado vermelho.
• Armazenar configurações e registros ambientais na EEPROM.
• Oferecer opções de idioma, unidade de temperatura, país e fuso horário.
• Permitir a consulta de dados e a comunicação com uma interface externa.

## Funcionalidades
- **Monitoramento de temperatura:** realiza leituras por meio do sensor DHT22.
- **Monitoramento de umidade:** acompanha a umidade relativa do ar.
- **Monitoramento de luminosidade:** utiliza um sensor LDR com calibração automática inicial.
- **Indicação por LEDs:** utiliza LEDs verde, amarelo e vermelho para sinalizar diferentes condições ambientais.
- **Alertas sonoros:** utiliza um buzzer para chamar a atenção quando necessário.
- **Display LCD:** apresenta as medições, os estados ambientais e as opções de navegação.
- **Menu interativo:** permite consultar individualmente temperatura, umidade, luminosidade, data e hora.
- **Configurações:** oferece seleção de idioma (português ou inglês), unidade de temperatura (Celsius ou Fahrenheit) e localidade de horário (Brasília ou Nova York).
- **Relógio em tempo real:** utiliza o módulo RTC DS1307 para acompanhar data e hora.
- **Registro de dados:** armazena registros de temperatura e umidade na EEPROM do Arduino.
- **Animação de inicialização:** apresenta uma animação personalizada no LCD com a identidade visual da equipe.

## Componentes utilizados
| Componente | Função |
|---|---|
| Arduino Uno | Controla o sistema e processa as leituras |
| Sensor DHT22 | Mede temperatura e umidade do ar |
| Sensor LDR | Detecta variações de luminosidade |
| Display LCD I2C 16x2 | Exibe informações e menus |
| Módulo RTC DS1307 | Mantém a data e a hora |
| LED verde | Indica a condição ambiental definida como ideal |
| LED amarelo | Indica uma condição de atenção |
| LED vermelho | Indica uma condição de alerta |
| Buzzer | Emite alertas sonoros |
| Botões | Permitem navegar pelo menu, confirmar opções e sair das telas |
| Resistores | Limitam a corrente elétrica, ajudando a proteger os LEDs contra danos |
| Protoboard | Permite montar e testar o circuito |
| Jumpers de Conexão | Conectam os componentes entre si e aos pinos do Arduino, permitindo a passagem de sinais elétricos e alimentação |
| Memória EEPROM | Armazena configurações e registros ambientais

## Mapeamento dos pinos
| Componente | Pino do Arduino |
|---|---|
| Sensor DHT22 | Digital 2 |
| Botão Menu | Digital 3 |
| Botão Confirmar | Digital 4 |
| Botão Sair | Digital 5 |
| Buzzer | Digital 8 |
| LED vermelho | Digital 9 |
| LED amarelo | Digital 10 |
| LED verde | Digital 13 |
| Sensor LDR | Analógico A0 |
| LCD I2C e RTC DS1307 | Comunicação I2C |

**Observação:** o LCD e o módulo RTC utilizam o barramento I2C. No Arduino Uno, as conexões padrão são SDA no pino A4 e SCL no pino A5. Além disso, é usado DHT11 no laboratória de aula.

## Critérios de monitoramento
O sistema utiliza faixas de referência definidas no código para classificar as condições ambientais.

| Indicador | Faixa definida no código |
|---|---|
| Temperatura | De 10 °C a 16 °C |
| Umidade relativa | De 57% a 76% |
| Luminosidade | De 0% a 25% |

A classificação considera a combinação dos valores medidos:

- **LED verde:** temperatura entre 10 °C e 16 °C, umidade entre 57% e 76% e luminosidade de até 25%.
- **LED vermelho:** luminosidade acima de 80%, temperatura acima de 16 °C ou umidade acima de 76%.
- **LED amarelo:** demais combinações que não se enquadram nas condições anteriores.

Essas faixas representam os critérios programados para o projeto acadêmico e não devem ser interpretadas como limites técnicos universais para o armazenamento de vinhos.

## Armazenamento de dados
Os registros são armazenados na EEPROM do Arduino e incluem:

- Data e hora do registro, representadas por um timestamp.
- Temperatura medida.
- Umidade relativa do ar.

A memória EEPROM permite preservar os registros mesmo após o desligamento do Arduino. O armazenamento desses dados possibilita consultar o histórico das condições ambientais e identificar momentos em que a temperatura ou a umidade atingiram valores críticos ou ficaram fora das faixas estabelecidas no código.

**Nota:** a estrutura de armazenamento está implementada no código. A consulta e a exibição dos registros históricos devem ser verificadas conforme as funcionalidades efetivamente habilitadas na versão final.

## Configurações de idioma e horário
O sistema permite selecionar:

- **Idioma:** português ou inglês.
- **Temperatura:** Celsius ou Fahrenheit.
- **Localidade de horário:** Brasília, Brasil, ou Nova York, Estados Unidos.

O relógio RTC mantém a referência de data e hora utilizada pelo sistema. A opção de localidade aplica um ajuste de horas ao horário obtido pelo módulo.

**Importante:** o ajuste de Nova York está definido como uma diferença fixa no código. Como o horário de verão dos Estados Unidos varia ao longo do ano, o sistema não realiza automaticamente essa mudança sazonal.

## Calibração e média da luminosidade
O LDR é calibrado automaticamente durante os primeiros 10 segundos. Nesse intervalo, o programa identifica os menores e maiores valores analógicos observados.

Depois da inicialização, o sistema calcula a média das leituras do LDR em intervalos de 10 segundos. O monitoramento aguarda a conclusão da primeira média antes de começar a classificar o ambiente.

Durante essa espera, o LCD apresenta a mensagem de cálculo da média.

A leitura é convertida para uma escala aproximada de 0% a 100%, utilizando os valores mínimo e máximo encontrados na calibração. Se não houver variação entre esses valores, o programa utiliza uma conversão alternativa baseada na faixa analógica de 0 a 1023.

A porcentagem é limitada ao intervalo de 0% a 100%.

## Registro de alertas no Monitor Serial
Quando o sistema entra no estado vermelho, ele registra:
  - Data da ocorrência.
  - Hora da ocorrência.
  - Estado identificado.

Exemplo de saída:

ALERTA: ESTADO VERMELHO
Data: 24/10/2026
Estado: VERMELHO
Hora: 13:25:10
Motivo: Temperatura + Umidade
------------------------

## Dependências e Bibliotecas (Software)
- `Wire.h` — comunicação I2C.
- `LiquidCrystal_I2C.h` — controle do display LCD.
- `DHT.h` — leitura do sensor DHT22.
- `RTClib.h` — comunicação com o módulo RTC.
- `EEPROM.h` — armazenamento de registros na memória EEPROM.
- `avr/pgmspace.h` — utilização da memória Flash para armazenar os desenhos da animação.
- `string.h` — operações com blocos de memória e textos.
- **Configuração do Ambiente (PlatformIO)**
  Este projeto utiliza o **PlatformIO** no VS Code para o desenvolvimento do firmware do Arduino. Para que o código compile corretamente sem erros de importação, preste atenção aos seguintes pontos:
  O PlatformIO é muito rígido com a organização dos arquivos. O arquivo principal de código **`main.ino`** deve ficar solto obrigatoriamente dentro da pasta raiz **`src/`**, e **NÃO** dentro de subpastas (como `src/arduino/main.cpp`).
Se o seu arquivo estiver no lugar errado, basta arrastá-lo diretamente para a raiz da pasta `src/` pelo explorador do VS Code.
  Se o VS Code exibir uma linha vermelha de erro embaixo de uma biblioteca, siga estes passos para resolver:
  1. Abra o arquivo **`platformio.ini`** na raiz do projeto.
  2. Adicione a dependência **`lib_deps`** logo abaixo das configurações da sua placa:
```ini
[env:uno]
platform = atmelavr
board = uno
framework = arduino
lib_deps = 
	marcoschwartz/LiquidCrystal_I2C@^1.1.4
	adafruit/DHT sensor library
	adafruit/RTClib@^2.1.4
```
  3. Salve o arquivo (`Ctrl + S`) para que o PlatformIO baixe os arquivos automaticamente.
  4. Se a linha vermelha continuar aparecendo, force a atualização do VS Code:
   * Pressione **`Ctrl + Shift + P`**
   * Digite: `PlatformIO: Rebuild IntelliSense Index`
   * Pressione **`Enter`**

As bibliotecas externas precisam estar instaladas no ambiente de desenvolvimento para que o código seja compilado corretamente.

## Linguagem Utilizada
<img
  align="left" 
  alt="c++" 
  title="c++"
  width="30px" 
  style="padding-right: 10px;" 
  src="https://cdn.jsdelivr.net/gh/devicons/devicon@latest/icons/cplusplus/cplusplus-original.svg" 
/>
<img
  align="left" 
  alt="JavaScript" 
  title="JavaScript"
  width="30px" 
  style="padding-right: 10px;" 
  src="https://cdn.jsdelivr.net/gh/devicons/devicon@latest/icons/javascript/javascript-plain.svg" 
/>
<br>


## Como executar o projeto
1. Monte o circuito com o Arduino Uno, os sensores, o display, os LEDs, o buzzer e os botões.
2. Conecte o Arduino ao computador.
3. Abra o projeto no Arduino IDE ou em um ambiente compatível.
4. Instale as bibliotecas necessárias.
5. Certifique-se de que a extensão PlatformIO IDE esteja instalada.
6. Selecione a placa Arduino Uno e a porta correspondente.
6. Clique em Carregar (Upload) para gravar e compilar o código no microcontrolador.
7. Aguarde a animação de inicialização e a calibração inicial do LDR.
8. Configure o idioma, a unidade de temperatura e a localidade do horário utilizando os botões.
9. Utilize os botões do circuito para acessar e navegar pelo menu:
  **Botão Menu** (pino digital 3): pressione por alguns segundos até o menu aparecer no LCD.
  **Botão Confirmar** (pino digital 4): seleciona ou confirma a opção escolhida.
  **Botão Sair** (pino digital 5): sai do menu ou retorna à tela anterior, conforme o comportamento programado.
10. Acompanhe as medições e os indicadores apresentados no display LCD.

## Tecnologias e conceitos aplicados
- Sistemas embarcados com Arduino.
- Programação em C++ para microcontroladores.
- Leitura e processamento de sensores.
- Comunicação I2C.
- Relógio em tempo real (RTC).
- Armazenamento não volátil com EEPROM.
- Controle de LEDs e buzzer.
- Navegação por menus com botões.
- Conversão de unidades e tratamento de dados.
- Uso de memória Flash para otimizar o armazenamento de animações.

## Versão do projeto
**Revisão 2** — segunda versão do sistema de monitoramento ambiental da Vinheria Agnello, com funcionalidades adicionais de configuração, monitoramento e apresentação de informações.

## Contexto acadêmico
Projeto desenvolvido no contexto acadêmico da FIAP, pela matéria de Edge Computing e Computer Systems, com foco na aplicação prática de conceitos de programação, eletrônica e sistemas embarcados.

## Simulação no WOKWI
Caso queira testar a lógica antes de montar o circuito físico:
1. Acesse o [projeto no WOKWI](https://wokwi.com/projects/477688493467889665).
2. Certifique-se que as bibliotecas citadas acima estejam instaladas.
3. Clique no botão verde de iniciar a simulação (Start the Simulation).
4. Aguarde a animação de inicialização no LCD e a apresentação do logotipo da equipe CALM TECH TEAM.
5. Observe as informações exibidas no LCD, incluindo as medições dos sensores e as opções disponíveis no menu.
6. Altere os valores de luminosidade, temperatura e umidade na simulação para observar como os LEDs indicadores reagem às diferentes condições ambientais.
7. Utilize os botões do circuito para navegar pelo menu e testar as opções implementadas, sendo necessário no Botão do Menu (Digital 3) pressionar por alguns segundos até aparecer o Menu, o Botão de Confirmar (Digital 4) escolhe a opção e o Botão de Sair (Digital 5) sai do Menu.

*. Imagem: [Monitoramento Ambiental](./src/assets/img/imgWokwi.png)

---
