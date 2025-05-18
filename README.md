# controleTanqueLCD

Este projeto foi desenvolvido utilizando uma CPU[Figura 1] baseada no microcontrolador ATmega328P, projetada e simulada no ambiente SimulIDE.

Para executar o projeto, é necessário utilizar o SimulIDE, um simulador gratuito voltado para circuitos eletrônicos e sistemas embarcados. Abra o arquivo CPU-Universal-Atmega328p.simu no SimulIDE, clique no microcontrolador com o botão direito do mouse clique em "carregar firmware" e selecione o arquivo .hex

Você pode baixar o simulador gratuitamente no site oficial: https://simulide.com/p/downloads/

Proposta do codigo - Simular uma rotina de um conjunto de um tanque,
valvula, sensor de nivel, LCD e botao, cogumelo para emergencia. O sensor
é simulado pelo potenciometro, nomeado setpoint, a valvula o led, nomeado saida
a bomba o led, nomeado pwm.
        Funcionamento do código - A quantidade de liquido é amostrado no LCD
a bomba alimenta um tanque de capacidade de 300L quando o tanque chega na metade
do armazenamento a valvula é ligada para diminuir o armazenamento usado, a bomba
fica ligada até o tanque atingir 250L, para evitar risco de transbordamento.
Usa-se delay na valvula e no tanque para não ter o problema de fecha e abre de
maneira compulsoria e desgastar o equipamento.
         Foi implementado também uma interrupção externa no caso de acidentes,
simulamos um botao cogumelo, com o BT0, quando pressionado a rotina é desligada,
quando pressionado novamente a rotina é restabelecida.

Figura 1

![Sem título-1](https://github.com/user-attachments/assets/629cb857-7775-4090-8aab-da4c55ab8d61)
