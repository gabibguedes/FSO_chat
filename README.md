# FSO - Chat
__Alunos:__

* Gabriela Barrozo Guedes - 16/0121612
* Vitor Leal dos Santos -  16/0148375

## Como Executar
1. Para compilar o programa execute o comando:
    ``` sh
    make
    ```
1. Para rodar o chat execute:
    ``` sh
    make run
    ```
#### Para executar com docker:

1. Crie o docker e entre nele com o comando:
    ``` sh
    docker run --rm -itv $(pwd):/code -w /code gcc
    ```
2. Siga os comandos para execução dentro do container.

## Funcionamento do programa
Ao ser executado o programa funciona da seguinte forma:

1. __Criação de usuário:__ O usuário do programa escolhe um nome para ser utilizado no chat. Esse nome deve conter até 10 caracteres, ser diferente de "all" e não estar sendo utilizado por outro usuário.

1. Após a criação de usuário, é possivel utilizar o sistema de chat através de comandos para:

    * __Exibir mensagem de ajuda:__ Ao iniciar o programa, ou ao digitar o comando `help`, é exibido ao usuário um texto de ajuda para mostrar os comandos que podem ser utilizados.

    * __Listagem de usuários:__ Para ver a lista de usuários logados, o usuário deve digitar o comando `listar`.

    * __Enviar mensagem:__ Para enviar uma mensagem, o usuário deve digitar o comando `enviar`. Aparecerá na tela instruções de como a mensagem deve ser escrita. O usuário deve digitar a mensagem no formato `PARA:MENSAGEM`.

    * __Enviar broadcast:__ O usuário pode enviar um broadcast utilizando do comando `enviar`. Para a mensagem ser enviada a todos os usuários, o nome de destino deve ser `all`. Seguindo o formato: `all:MENSAGEM`.

    * __Criar sala:__ O usuário pode criar uma sala com o comando `criar_sala`.

    * __Enviar mensagem para uma sala:__ Para enviar mensagem para uma sala, basta utilizar o comando `enviar_sala` e em seguida digitar a mensagem no formato: `SALA_DE_DESTINO:MENSAGEM`.

    * __Entrar em uma nova sala:__ Para entrar em uma sala o usuário deve ir na opção de enviar mensagem a uma sala e mandar a mensagem `JOIN` para a sala que deseja participar.

    * __Listar salas:__ Para listar as salas existentes o usuário deve utilizar o comando `listar_salas`.

1. As mensagens são recebidas e mostradas na tela durante o funcionamento do programa.

1. Para parar a execução, o usuário deve utilizar o comando `sair`.

## Implementação do 1o Checkpoint

__Requisitos Feitos:__
- [X] Criação da fila de mensagens de acordo com o nome de usuário `/chat-USUARIO`
- [X] Verificação se o nome de usuário está livre
- [X] Permissão de leitura e escrita para o dono da fila e de somente escrita para os outros usuários
- [X] Uso do protocolo de mensagem `DE:PARA:MENSAGEM` 
- [X] Uso de thread para lidar com o recebimento de mensagens
- [X] Limite de caracteres aplicados
- [X] Ctrl+C bloqueado. O usuário só pode sair ao escrever `sair`
- [X] Arquivo de fila removido ao final do programa
- [X] Mensagem de `UNKNOWNUSER` ao tentar enviar mensagens para filas inexistentes
- [X] Comando para listagem de usuários disponiveis
- [X] Impedir usuários com o nome `all`
- [X] Quando o destinatário de uma mensagem for `all` a mensagem deve ser enviada para todos os usuários disponíveis

__Requisitos Faltantes:__
- [ ] Caso a mensagem não consiga ser enviada, o programa deve fazer 3 tentativas de envio da mensagem. Caso a mensagem não seja entregue após as 3 tentativas deve ser apresentada a mensagem `ERRO DE:PARA:MSG`

## Implementação do 2o Checkpoint

__Requisitos Feitos:__
- [X] Criar sala de chat
- [X] Comando JOIN para entrar na sala
- [X] Enviar mensagens para a sala
- [X] Verificação se as mensagens recebidas são de membros da sala
- [X] Destruir a sala

__Requisitos Faltantes:__
- [ ] Assinatura de mensagem
- [ ] Sair da sala de chat


## Problemas conhecidos
O programa apresenta um erro de segmentação caso o usuário escreva a mensagem a ser enviada no formato incorreto (sem os dois pontos `:`).

<!-- 
- sair da sala
- assinatura
- arrumar o script
- verificar se ta no padrão(ex.: itens "se for membro" e "destroy") 
-->

