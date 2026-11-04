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
- Camera controlada por teclado (`W`, `A`, `S`, `D`) e mouse.
- Transformacoes de translacao, rotacao e escala.
- Display Lists para reutilizar os comandos de desenho dos modelos.

## Controles de iluminacao

- `F`: liga ou desliga as luzes dos seis postes.
- `L` + setas: move a primeira luz nos eixos X e Y.
- `L` + Page Up/Page Down: move a primeira luz no eixo Z.
- Espaco: restaura a camera, as transformacoes do carro e a posicao da primeira luz.

## Texturas e geometria

A imagem `textures/aerial_asphalt_01_diff_1k.jpg` e aplicada a superficie da rua por coordenadas UV. A imagem permanece inteira e se repete sobre o asfalto. As calcadas e os gramados ficam ao lado da pista, em superficies separadas.

As calcadas usam quadrilateros cinza elevados, com laterais para o meio-fio e pequenas juntas entre as placas. Os gramados usam quadrilateros com variacoes de verde. As faixas da pista e o caminho da casa tambem sao geometria colorida, sem imagens de textura. O carro e a casa usam texturas dos arquivos GLB; os postes e o cone usam cores de materiais.

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
- `models/`: modelos do carro, casa, postes e cone.
- `textures/`: imagem usada no plano da rua.

## Observacao

Este projeto tem finalidade educacional e esta sendo desenvolvido gradualmente durante as aulas de Computacao Grafica.
