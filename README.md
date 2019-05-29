# Como executar com docker:

1. Crie o docker e entre nele com o comando:
    ``` sh
    docker run --rm -itv $(pwd):/code -w /code gcc
    ```
1. No docker execute os comandos para inicializar o **Chat**: 
    * Para compilar:
        ``` sh
        gcc -o chat -lrt main.c
        ```
    * Para executar:
        ``` sh
        ./chat
        ```
