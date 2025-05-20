# Estação de Alerta de Enchente com FreeRTOS

Um sistema embarcado para monitoramento de enchentes utilizando a plataforma RP2040 (BitDog Lab) com FreeRTOS, que monitora **nível de água** e **volume de chuva** simulados por um joystick. O projeto alterna automaticamente entre **Modo Normal** e **Modo Alerta**, oferecendo sinalizações visuais (LEDs RGB, matriz de LEDs e display OLED) e **alertas sonoros diferenciados** para acessibilidade. O botão do joystick ativa o modo BOOTSEL para atualizações.  

## Demonstração em Vídeo
[![Link para Vídeo de Demonstração](https://via.placeholder.com/400x300.png/000000/FFFFFF?text=V%C3%ADdeo+Demonstra%C3%A7%C3%A3o)](https://exemplo.com)

## Funcionalidades Principais

- **Modo Normal**  
  - Exibição contínua de níveis em display OLED  
  - LED verde fixo  
  - Matriz LED desligada  
  - Silencioso  

- **Modo Alerta** (Ativação automática)  
  - LED vermelho piscante (100ms)  
  - Matriz LED com animação de nuvem/raio intermitente  
  - Buzzer com padrões distintos:  
    - 1000Hz para nível d'água alto  
    - 2000Hz para volume de chuva excessivo  
  - Moldura piscante no display com mensagem de alerta  

- **Modo BOOTSEL**  
  - Ativação via botão do joystick  


## Componentes Utilizados

| Componente                 | GPIO/Pino     | Função                                                      |
|----------------------------|---------------|-------------------------------------------------------------|
| Display OLED SSD1306       | 14 (SDA), 15 (SCL) | Exibição de dados e status do sistema           |
| LED RGB Verde              | 11            | Indicação de modo normal                     |
| LED RGB Vermelho           | 13            | Alerta de emergência                         |
| Matriz de LEDs WS2812B     | Configurável¹ | Representação gráfica de alertas            |
| Buzzer                     | 21 (A), 10 (B)| Alertas sonoros diferenciados               |
| Joystick (Eixos X/Y)       | 26 (Y), 27 (X)| Simulação de sensores (nível água/chuva)     |
| Botão Joystick             | 22            | Ativação modo DFU                           |

## Organização do Código

- **`main.c`**  
  - Inicialização de hardware e criação de tarefas FreeRTOS  
  - Handlers para modo BOOTSELL

- **Tarefas FreeRTOS**  
  - `vJoystickTask` - Leitura analógica e envio para filas  
  - `vLedRGBTask` - Controle do LED RGB  
  - `vMatrizLedTask` - Animação da matriz LED  
  - `vDisplayOledTask` - Atualização do display gráfico  
  - `vBuzzerTask` - Gerenciamento de alertas sonoros  

- **Estruturas de Dados**  
  - `DadosSensor` - Armazena valores normalizados (0-100%)  




## ⚙️ Instalação e Uso

1. **Pré-requisitos**
   - Clonar o repositório:
     ```bash
     git clone https://github.com/JotaPablo/EstacaoMonitoramento.git
     cd EstacaoMonitoramento
     ```
   - Instalar o **Visual Studio Code** com as extensões:
     - **C/C++**
     - **Raspberry Pi Pico SDK**
     - **Compilador ARM GCC**
     - **CMake Tools**

2. **Ajuste do caminho do FreeRTOS**
   - Abra o arquivo `CMakeLists.txt` na raiz do projeto e ajuste a variável `FREERTOS_KERNEL_PATH` para o diretório onde você instalou o FreeRTOS Kernel.  
     Exemplo:
     ```cmake
     set(FREERTOS_KERNEL_PATH "C:/FreeRTOS-Kernel")
     ```
     → Substitua `"C:/FreeRTOS-Kernel"` pelo caminho correto na sua máquina.

3. **Compilação**
   - Compile o projeto manualmente via terminal:
     ```bash
     mkdir build
     cd build
     cmake ..
     make
     ```
   - Ou utilize a opção **Build** da extensão da Raspberry Pi Pico no VS Code.

4. **Execução**
   - Conecte o Raspberry Pi Pico ao computador mantendo o botão **BOOTSEL** pressionado.
   - Copie o arquivo `.uf2` gerado na pasta `build` para o dispositivo `RPI-RP2`.