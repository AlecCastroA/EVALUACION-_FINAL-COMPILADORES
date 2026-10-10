#pragma once
#include <vector>
#include <map>
#include <string>
#include "Parser.h"
using namespace std;

struct ErrorSemantico
{
    string mensaje;
    int linea;
    int columna;
};

// trabaja con las instrucciones que guarda el parser
class AnalizadorSemantico
{
private:
    vector<Instruccion> instrucciones;
    map<string, string> tablaSimbolos; // nombre -> tipo
    vector<ErrorSemantico> errores;

    void registrarError(string mensaje, int linea, int columna);
    bool esCompatible(string tipoDeclarado, string tipoValor);
    string tipoDeExpresion(vector<Token> &expr, int linea, int columna);

public:
    AnalizadorSemantico(vector<Instruccion> _instrucciones);
    bool analizar();
    map<string, string> obtenerTablaSimbolos();
    vector<ErrorSemantico> obtenerErrores();
};