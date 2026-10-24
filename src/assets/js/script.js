const botaoConectar = document.querySelector("#conectar");
const imgs = document.querySelectorAll(".painelIntro img");
const imgsLeds = document.querySelectorAll(".leds img");
const painel = document.querySelector(".painelPrincipal");
const porcentagem = document.querySelector(".porcentagem");
const statusSpan = document.querySelector(".status span");
const led = document.querySelector(".corLed");
const temperatura = document.querySelector(".temperatura span");
const unidade = document.querySelector(".temperatura p");
const umidade = document.querySelector(".umidade span");
const data = document.querySelector(".data span");
const hora = document.querySelector(".horas span");
const fechar = document.querySelector(".fechar");
const condicaoParagrafo = document.querySelector(".condicaoParagrafo");
const body = document.body;

fechar.addEventListener("click", () => {
    condicaoParagrafo.classList.toggle("abrirCondicao")
});

botaoConectar.addEventListener("click", async () => {
    try {
        // Conecta ao Arduino
        const porta = await navigator.serial.requestPort();

        // Abre a conexão
        await porta.open({ baudRate: 9600 });

        // Transforma os dados recebidos em texto
        const decoder = new TextDecoderStream();

        porta.readable.pipeTo(decoder.writable);

        // Permite ler os dados recebidos
        const leitor = decoder.readable.getReader();

        let dadosRecebidos = "";

        while (true) {
            const { value, done } = await leitor.read();

            if (done) break;

            dadosRecebidos += value;

            // Separa os dados por linha
            const linhas = dadosRecebidos.split("\n");
            dadosRecebidos = linhas.pop();

            // Lê cada linha recebida
            for (const linha of linhas) {
                try {
                    // Converte o texto em dados JavaScript
                    const dados = JSON.parse(linha.trim());

                    // A partir daqui, continua a atualização normal dos sensores
                    const ingles = dados.idioma === "en";

                    document.body.classList.toggle("en", ingles);
                    document.body.classList.toggle("pt", !ingles);

                    document.querySelectorAll("[data-pt][data-en]").forEach((elemento) => {
                        elemento.textContent = ingles
                            ? elemento.dataset.en
                            : elemento.dataset.pt;
                    });

                    porcentagem.textContent = `${dados.porcentagem} %`;
                 
                    // Atualiza a temperatura
                    let valorTemperatura = dados.temperatura;

                    if (dados.unidade === "fahrenheit") {
                        valorTemperatura = (valorTemperatura * 9 / 5) + 32;
                        temperatura.textContent = `${valorTemperatura.toFixed(1)} F`;
                    } else {
                        temperatura.textContent = `${valorTemperatura.toFixed(1)} °C`;
                    };

                    // Atualiza a umidade
                    umidade.textContent = `${dados.umidade.toFixed(1)} %`;

                    // Atualiza a data e a hora
                    data.textContent = dados.data;
                    hora.textContent = dados.hora;

                    // Idioma
                    if (dados.idioma === "en") {
                        document.body.classList.add("en");
                        document.body.classList.remove("pt");
                    } else {
                        document.body.classList.add("pt");
                        document.body.classList.remove("en");
                    };

                    if (dados.estado == "VERDE") {
                        painel.classList.add("verde");
                        painel.classList.remove("amarelo");
                        painel.classList.remove("vermelho");
                        statusSpan.textContent = "OK";
                        if (dados.idioma === "en") {
                            led.textContent = "GREEN";
                        } else {
                            led.textContent = "VERDE";
                        };

                        imgs.forEach((img) => {
                            if (img.id == "verde") {
                                img.classList.add("ativo");
                            }
                            else {
                                img.classList.remove("ativo");
                            }
                        });
                        imgsLeds.forEach((img) => {
                            if (img.id == "verde") {
                                img.classList.add("ligado");
                            }
                            else {
                                img.classList.remove("ligado");
                            }
                        });
                    }
                    else if (dados.estado == "AMARELO") {
                        painel.classList.remove("verde");
                        painel.classList.add("amarelo");
                        painel.classList.remove("vermelho");
                        if (dados.idioma === "en") {
                            statusSpan.textContent = "Alert";
                            led.textContent = "YELLOW";
                        } else {
                            statusSpan.textContent = "AlertA";
                            led.textContent = "AMARELO";
                        };

                        imgs.forEach((img) => {
                            if (img.id == "amarelo") {
                                img.classList.add("ativo");
                            }
                            else {
                                img.classList.remove("ativo");
                            }
                        });
                        imgsLeds.forEach((img) => {
                            if (img.id == "amarelo") {
                                img.classList.add("ligado");
                            }
                            else {
                                img.classList.remove("ligado");
                            }
                        });

                    } else {
                        painel.classList.remove("verde");
                        painel.classList.remove("amarelo");
                        painel.classList.add("vermelho");
                        statusSpan.textContent = "ERROR!";
                        if (dados.idioma === "en") {
                            led.textContent = "RED";
                        } else {
                            led.textContent = "VERMELHO";
                        };
                        imgs.forEach((img) => {
                            if (img.id == "vermelho") {
                                img.classList.add("ativo");
                            }
                            else {
                                img.classList.remove("ativo");
                            }
                        });
                        imgsLeds.forEach((img) => {
                            if (img.id == "vermelho") {
                                img.classList.add("ligado");
                            }
                            else {
                                img.classList.remove("ligado");
                            }
                        });
                    }
                } catch (erro) {
                    console.log("Linha ignorada:", linha);
                }
            }
        }

    } catch (erro) {
        // Mostra uma mensagem caso ocorra um erro
        console.error("Erro ao conectar:", erro);
    }
});