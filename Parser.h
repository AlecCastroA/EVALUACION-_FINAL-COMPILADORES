#pragma once
#include <vector>
#include "Token.h"
using namespace std;

struct ErrorSintactico
{
    string tokenEsperado;
    string tokenEncontrado;
    int linea;
    int columna;
    string mensaje;
};

// el parser guarda aqui cada sentencia que reconoce
struct Instruccion
{
    enum class Tipo { DECLARACION, ASIGNACION } tipo;
    string nombre;         // variable declarada o asignada
    string tipoDato;       // solo en declaraciones: int, float o string
    vector<Token> valor;   // declaracion: un token. asignacion: toda la expresion
    int linea;
    int columna;
};

class Parser
{
private:
    vector<Token> tokens;
    int posicion;

    Token actual();
    bool esFinal();
    Token avanzar();
    bool coincide(TipoToken tipoEsperado, string lexemaEsperado = "");
    void registrarError(string tokenEsperado, string mensaje);

    void parsePrograma();
    void parseSentencia();
    void parseDeclaracion();
    void parseAsignacion();
    void parseExpresion();
    void parseTermino();
    void parseFactor();

    vector<ErrorSintactico> errores;
    vector<Instruccion> instrucciones;

public:
    Parser(vector<Token> _tokens);
    bool analizar();
    vector<ErrorSintactico> obtenerErrores();
    vector<Instruccion> obtenerInstrucciones();
};