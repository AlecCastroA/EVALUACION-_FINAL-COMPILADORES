#pragma once
#include <string>
using namespace std;

// Tipos de token del mini compilador
enum class TipoToken
{
    PALABRA_RESERVADA,
    IDENTIFICADOR,
    LITERAL_NUMERICO,
    LITERAL_REAL,
    LITERAL_CADENA,
    OPERADOR_ASIGNACION,
    OPERADOR_RELACIONAL,
    OPERADOR_ARITMETICO,
    //-------
    PARENTESIS_IZQUIERDO,
    PARENTESIS_DERECHO,
    COMA,
    PUNTO_COMA,
    //-------
    DESCONOCIDO
};

struct Token
{
    TipoToken tipo;
    string lexema;
    int linea;
    int columna;
};

struct ErrorLexico
{
    string lexema;
    int linea;
    int columna;
    string mensaje;
};
