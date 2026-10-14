# OpenGL Game Project

Projeto de estudos da disciplina de Computacao Grafica, desenvolvido em C++ com OpenGL.

O repositorio acompanha a evolucao das atividades da disciplina e servira como base para um jogo 3D simples no futuro.

## Cena atual

- Carro esportivo em modelo 3D GLB.
- Rua criada com um plano 3D e textura de asfalto.
- Dois postes em modelo OBJ.
- Um cone de transito em modelo GLB.
- Camera controlada por teclado (`W`, `A`, `S`, `D`) e mouse.
- Transformacoes de translacao, rotacao e escala.
- Display Lists para reutilizar os comandos de desenho dos modelos.

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
- `models/`: modelos do carro, postes e cone.
- `textures/`: imagem usada no plano da rua.

## Observacao

Este projeto tem finalidade educacional e esta sendo desenvolvido gradualmente durante as aulas de Computacao Grafica.
