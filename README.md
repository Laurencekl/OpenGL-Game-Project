# OpenGL Game Project

Projeto de estudos da disciplina de Computacao Grafica, desenvolvido em C++ com OpenGL.

O repositorio acompanha a evolucao das atividades da disciplina e servira como base para um jogo 3D simples no futuro.

## Cena atual

- Carro esportivo em modelo 3D GLB, estacionado junto a calcada.
- Rua com textura de asfalto e faixas desenhadas em OpenGL.
- Calcadas elevadas, meio-fio e gramados criados com geometria e cores.
- Casa em modelo GLB, apoiada no gramado e com caminho ate a calcada.
- Seis postes em modelo OBJ, tres de cada lado da rua, com luzes locais.
- Um cone de transito em modelo GLB.
- Fusca estacionado no gramado ao lado da casa; carro esportivo na calcada oposta.
- Neymar em pose estatica segurando um guarda-chuva preto e Pikachu ao seu lado.
- Nuvens em movimento e chuva animadas em funcao do tempo (Aula 13).
- Camera controlada por teclado (`W`, `A`, `S`, `D`) e mouse.
- Transformacoes de translacao, rotacao e escala.
- Display Lists para reutilizar os comandos de desenho dos modelos.

## Controles de iluminacao e animacao

- `F`: liga ou desliga as luzes dos seis postes.
- `B`: alterna entre iluminacao ambiente normal e modo claro.
- `C`: liga ou desliga a chuva e alterna a cor do ceu.
- `P`: pausa ou continua o movimento das nuvens e da chuva.
- Espaco: restaura a camera e as transformacoes do carro esportivo.

Os postes permanecem fixos. O modo claro ajusta a iluminacao ambiente global; nao e uma lanterna ou uma luz movel.

## Animacao da Aula 13

A funcao `animate()` usa `glfwGetTime()` para calcular o tempo decorrido entre quadros (`delta_time`). As posicoes sao atualizadas por velocidade multiplicada por esse intervalo, seguindo a estrutura do exemplo da disciplina.

As nuvens se deslocam horizontalmente e retornam ao outro lado ao sair da area. A chuva e formada por linhas desenhadas em OpenGL: as gotas descem e recebem novas posicoes no alto quando chegam ao chao. Desligar a chuva suspende seu desenho e movimento; pausar a animacao interrompe tambem o movimento das nuvens.

## Texturas e geometria

A imagem `textures/aerial_asphalt_01_diff_1k.jpg` e aplicada a superficie da rua por coordenadas UV. A imagem permanece inteira e se repete sobre o asfalto. As calcadas e os gramados ficam ao lado da pista, em superficies separadas.

As calcadas usam quadrilateros cinza elevados, com laterais para o meio-fio e pequenas juntas entre as placas. Os gramados usam quadrilateros com variacoes de verde. As faixas da pista e o caminho da casa tambem sao geometria colorida, sem imagens de textura. O carro esportivo, a casa e o Neymar usam texturas dos arquivos GLB; os demais modelos usam cores de materiais. O Pikachu baixado nao incluia o material referenciado, por isso recebeu materiais de cores aproximadas.

## Tecnologias

- C++17
- OpenGL
- GLFW
- GLEW
- Assimp
- stb_image

## Estrutura

- `main.cpp`: inicializacao, controles, camera e desenho da cena.
- `glb_model.h`: carregamento e desenho dos modelos 3D.
- `models/`: modelos da cena e arquivo `ASSET_CREDITS.txt` com os creditos dos novos modelos.
- `textures/`: imagem usada no plano da rua.

## Observacao

Este projeto tem finalidade educacional e esta sendo desenvolvido gradualmente durante as aulas de Computacao Grafica.
