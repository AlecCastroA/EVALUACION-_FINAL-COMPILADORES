#pragma once
#include <vector>
#include <string>
#include "Parser.h"
using namespace std;

struct Terceto
{
    string destino;
    string izquierdo;
    string operador; // vacio si es una asignacion simple, sin operacion
    string derecho;  // vacio si es una asignacion simple
};

class GeneradorCodigo
{
private:
    int contadorTemp;
    vector<Token> aPostfijo(vector<Token> &infijo);
    int precedencia(string op);

public:
    GeneradorCodigo();
    vector<Terceto> generarTercetos(string destinoFinal, vector<Token> expresionInfija);
    vector<Terceto> optimizar(vector<Terceto> &tercetos);
    vector<Token> convertirAPostfijo(vector<Token> expresion);
};