# Monitoramento de Som com Raspberry Pi Pico

Este projeto implementa um sistema de monitoramento de som em tempo real usando o Raspberry Pi Pico. O sistema é capaz de detectar níveis de som e acionar alarmes visuais (via LEDs WS2812B) e sonoros (via buzzer) quando um determinado limiar de som é ultrapassado. O projeto também inclui um display OLED para exibição de informações de monitoramento, como o valor do ADC, voltagem e contador de alarmes.

## Funcionalidades

- **Monitoramento de Som**: O sistema monitora os níveis de som por meio de um microfone conectado ao Raspberry Pi Pico. Quando o som ultrapassa um determinado limite, o alarme é acionado.
- **Contador de Alarmes**: O sistema mantém um contador de alarmes, que pode ser visualizado no display OLED.
- **Modo Silencioso**: O sistema permite que o monitoramento de som continue funcionando sem acionar o buzzer, podendo ser ativado e desativado com o botão A.
- **Controle com Botões**: Dois botões (A e B) são utilizados para controlar o sistema, alternando entre a visualização dos alarmes e o monitoramento de som, além de ativar/desativar o modo silencioso.
- **Exibição de Dados**: O display OLED exibe o valor do ADC, a voltagem e o número de alarmes registrados.

## Hardware

O projeto é baseado no **Raspberry Pi Pico** e utiliza os seguintes componentes:

- **Raspberry Pi Pico**: Microcontrolador responsável pelo processamento.
- **Matriz de LEDs WS2812B**: Usada para criar alarmes visuais.
- **Display OLED SSD1306**: Para exibição dos dados de monitoramento e alarmes.
- **Microfone**: Para capturar os níveis de som.
- **Buzzer**: Acionado quando o som ultrapassa o limite.
- **Botões A e B**: Para controlar as funcionalidades do sistema.

### Esquema de Conexões

- **GPIO5**: Botão A (controles de modo silencioso).
- **GPIO6**: Botão B (alternância entre modos de monitoramento e alarmes).
- **GPIO10**: Conexão com o microfone (via ADC).
- **GPIO21**: Buzzer.
- **GPIO07**: Matriz de LEDs WS2812B.
- **I2C (GPIO4, GPIO5)**: Display OLED SSD1306.

## Software

O software foi desenvolvido em **C** usando o **pico-sdk** para Raspberry Pi Pico. Ele foi programado na **IDE Visual Studio Code**.

## Como Rodar

### Requisitos

- **Visual Studio Code** com a extensão **C/C++ for Visual Studio Code**.
- **pico-sdk**: A biblioteca do Raspberry Pi Pico deve estar instalada.

### Passos para Execução

1. Clone este repositório em seu ambiente local:

    ```bash
    git clone https://github.com/seu-usuario/nome-do-repositorio.git
    ```

2. Instale as dependências necessárias (verifique se o **pico-sdk** está corretamente configurado):

    ```bash
    cd nome-do-repositorio
    ```

3. Compile o código usando o comando de compilação do **CMake**:

    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

4. Conecte o Raspberry Pi Pico ao computador via USB.

5. Carregue o código no Raspberry Pi Pico:

    ```bash
    picotool load build/seu-programa.uf2
    ```

6. O sistema será iniciado automaticamente, e você poderá monitorar os níveis de som, visualizar os alarmes e controlar o sistema com os botões.

## Testes Realizados

Os seguintes testes foram realizados para validar o funcionamento do sistema:

- **Monitoramento de Som**: Simulação de diferentes níveis de som e verificação da resposta do sistema.
- **Alternância de Estados**: Testes com os botões A e B para alternar entre monitoramento de som e visualização de alarmes.
- **Exibição no OLED**: Verificação da atualização correta das informações no display OLED.
- **Funcionamento do Alarme**: Testes de acionamento do buzzer e LEDs quando os limites de som eram ultrapassados.

## Contribuições

Este projeto está aberto a contribuições. Sinta-se à vontade para fazer um fork do repositório, fazer alterações e enviar pull requests.
