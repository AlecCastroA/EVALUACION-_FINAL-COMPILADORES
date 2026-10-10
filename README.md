# Mini compilador - Evaluación Final
# COMPILADORES NRC: 38503
# Castro Amudio Alejandro Gabriel

Mini compilador en C++ que procesa un código fuente sencillo y ejecuta
estas fases: análisis léxico, análisis sintáctico, análisis semántico,
generación de código intermedio (tercetos) y optimización.

## Lenguaje que reconoce

- Declaraciones: `int edad = 20;` `float nota = 15.5;` `string ciudad = "Arequipa";`
- Asignaciones con expresiones: `resultado = (a + b) * c;`
- Los strings deben ir entre comillas dobles.

## Archivos

- `main.cpp`: programa principal, lee `codigo.txt` y muestra el resultado de cada fase.
- `Token.h`: tipos de token y estructura del error léxico.
- `Lexer.h` / `Lexer.cpp`: análisis léxico.
- `Parser.h` / `Parser.cpp`: análisis sintáctico.
- `AnalizadorSemantico.h` / `AnalizadorSemantico.cpp`: tabla de símbolos y validaciones.
- `GeneradorCodigo.h` / `GeneradorCodigo.cpp`: notación postfija, tercetos y optimización.
- `codigo.txt`: archivo de entrada con código de ejemplo.

## Requisitos

Un compilador de C++ compatible con C++11 o superior (por ejemplo g++,
clang++ o el compilador de Visual Studio).

## Compilación

Desde la carpeta del proyecto, por ejemplo con g++:

    g++ main.cpp Lexer.cpp Parser.cpp AnalizadorSemantico.cpp GeneradorCodigo.cpp -o programa

Si usas otro compilador, compila de la misma forma los cinco archivos
.cpp (main, Lexer, Parser, AnalizadorSemantico y GeneradorCodigo).

## Ejecución

1. Escribe el código a analizar en `codigo.txt` (puede tener varias líneas).
2. Ejecuta el programa desde la misma carpeta donde está `codigo.txt`:

       ./programa

El programa no pide datos por teclado: siempre lee `codigo.txt`.

## Ejemplo

`codigo.txt`:

    int a = 10;
    int b = 5;
    int c = 2;
    int resultado = 0;
    resultado = (a + b) * c;

Resultado al final de la ejecución:

    Tabla de simbolos:
    a -> int
    b -> int
    c -> int
    resultado -> int

    Notacion postfija: a b + c *
    t1 = a + b
    t2 = t1 * c
    resultado = t2