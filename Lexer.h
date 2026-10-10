#pragma once
#include <vector>
#include "Token.h"
using namespace std;

class Lexer
{
private:
    string codigo;
    int posicion;

    bool esFinal();
    char actual();
    char siguiente();
    char avanzar();
    void ignorarEspacios();
    void ignorarComentario();

    Token reconocerNumero();
    // ----------
    Token reconocerIdentificador();
    Token reconocerCadena();
    Token reconocerOperador();
    bool esPalabraReservada(string palabra);

    //-------error lexico
    int linea;
    int columna;
    vector<ErrorLexico> errores;
    void registrarError(string, int, int, string);

public:
    Lexer(string codigoFuente);
    vector<Token> analizar();
    vector<ErrorLexico> obtenerErrores();
};
