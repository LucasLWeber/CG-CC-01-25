# Registro de Entregas - Atividades Vivenciais

## Vivencial 1 - 17/05/2025

Alunos: Eduardo Colissi, Lucas Weber e Rodrigo Fischer

Nesta entrega foi implementado o uso da função `LoadSimpleObj`, responsável por carregar modelos `.obj` simples na aplicação. Para isso, foi criado um arquivo `.h` contendo a definição da função, que foi incluída no arquivo principal `Vivencial1.cpp`.

O programa realiza o carregamento de **dois objetos 3D distintos** por meio de dois VAOs. Cada objeto pode ser **controlado individualmente** (posição e rotação), de acordo com as teclas mapeadas. O **objeto 1 é selecionado por padrão**, mas o usuário pode alternar entre os dois utilizando as teclas `1` e `2`.

A movimentação da câmera continua funcional e atua globalmente na cena.

## 🎮 Controles por Teclado

| Tecla         | Função                                             | Objeto Afetado               |
|---------------|----------------------------------------------------|------------------------------|
| `1`           | Seleciona o objeto 1                               | Global (altera contexto)     |
| `2`           | Seleciona o objeto 2                               | Global (altera contexto)     |
| `←` (Left)    | Move o objeto selecionado para a esquerda (X–)     | Objeto selecionado           |
| `→` (Right)   | Move o objeto selecionado para a direita (X+)      | Objeto selecionado           |
| `↑` (Up)      | Move o objeto para cima (Y+)                       | Objeto selecionado           |
| `↓` (Down)    | Move o objeto para baixo (Y–)                      | Objeto selecionado           |
| `I`           | Aproxima o objeto da câmera (Z+)                   | Objeto selecionado           |
| `J`           | Afasta o objeto da câmera (Z–)                     | Objeto selecionado           |
| `X`           | Rotaciona o objeto em torno do eixo X              | Objeto selecionado           |
| `Y`           | Rotaciona o objeto em torno do eixo Y              | Objeto selecionado           |
| `Z`           | Rotaciona o objeto em torno do eixo Z              | Objeto selecionado           |
| `W`           | Move a câmera para cima (no eixo Y)                | Câmera (global)              |
| `S`           | Move a câmera para baixo (no eixo Y)               | Câmera (global)              |
| `A`           | Move a câmera para a esquerda (no eixo X)          | Câmera (global)              |
| `D`           | Move a câmera para a direita (no eixo X)           | Câmera (global)              |
| `ESC`         | Encerra a aplicação                                | Global                       |


## 📌 Observações

- A função `key_callback()` gerencia os comandos de teclado e realiza o controle individual dos objetos com base no valor da variável `selectedObject`.
- As variáveis `rotate1X`, `rotate1Y`, `rotate1Z`, `rotate2X`, `rotate2Y`, `rotate2Z`, bem como `pos1X`, `pos1Y`, `pos1Z`, `pos2X`, etc., são responsáveis por armazenar o estado de rotação e posição de cada objeto.
- A câmera continua funcionando globalmente e pode ser movida independentemente da seleção dos objetos.

---

## Vivencial 2 - 31/05/2025

Alunos: Lucas Weber e Rodrigo Fischer

Este projeto é um renderizador 3D desenvolvido em C++ com OpenGL 4.x, capaz de carregar modelos .obj, aplicar iluminação básica e manipular interativamente a posição da luz e os tipos de iluminação via teclado.

## 🚀 Funcionalidades Implementadas

Suporte a arquivos .obj com:
- Vértices (v)
- Coordenadas de textura (vt)
- Normais (vn)
- Faces triangulares (f v/vt/vn)

Dados carregados:
- Vértices 3D (vec3)
- UVs 2D (vec2)
- Normais 3D (vec3)

## 🎨 Pipeline Gráfico
Shaders implementados:
- Vertex Shader: processa posições, normais e UVs.
- Fragment Shader: realiza cálculo de iluminação e cor final por fragmento.
- Suporte à renderização com glDrawArrays.

## 💡 Iluminação
Iluminação baseada em três fontes:
- Luz-chave (key light)
- Luz de preenchimento (fill light)
- Luz de fundo (back light)
- Controle interativo via teclado para ativar/desativar cada luz.


## 🎮 Controles por Teclado

| Tecla             | Ação                                  |
| ----------------- | ------------------------------------- |
| `ESC`             | Fecha a janela                        |
| `← (Left Arrow)`  | Move a luz para a esquerda (`x--`)    |
| `→ (Right Arrow)` | Move a luz para a direita (`x++`)     |
| `↑ (Up Arrow)`    | Move a luz para cima (`y++`)          |
| `↓ (Down Arrow)`  | Move a luz para baixo (`y--`)         |
| `Page Up`         | Aproxima a luz da cena (`z++`)        |
| `Page Down`       | Afasta a luz da cena (`z--`)          |
| `1`               | Ativa/desativa a luz-chave            |
| `2`               | Ativa/desativa a luz de preenchimento |
| `3`               | Ativa/desativa a luz de fundo         |

---