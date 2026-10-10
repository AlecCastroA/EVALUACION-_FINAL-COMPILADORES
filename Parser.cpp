#include "Parser.h"
#include <iostream>

Parser::Parser(vector<Token> _tokens)
{
    tokens = _tokens;
    posicion = 0;
}

bool Parser::esFinal()
{
    return posicion >= (int)tokens.size();
}

Token Parser::actual()
{
    if (esFinal())
    {
        // cuando se acaban los tokens devuelve  un "$" falso para no salirse del vector
        return {TipoToken::DESCONOCIDO, "$", -1, -1};
    }
    return tokens[posicion];
}

Token Parser::avanzar()
{
    Token t = tokens[posicion];
    posicion++;
    return t;
}

void Parser::registrarError(string tokenEsperado, string mensaje)
{
    Token t = actual();
    ErrorSintactico error;
    error.tokenEsperado = tokenEsperado;
    error.tokenEncontrado = t.lexema;
    error.linea = t.linea;
    error.columna = t.columna;
    error.mensaje = mensaje;
    errores.push_back(error);
}

bool Parser::coincide(TipoToken tipoEsperado, string lexemaEsperado)
{
    if (esFinal())
    {
        registrarError(lexemaEsperado.empty() ? "token" : lexemaEsperado,
                        "Fin de entrada inesperado");
        return false;
    }

    Token t = actual();
    bool tipoOk = (t.tipo == tipoEsperado);
    bool lexemaOk = lexemaEsperado.empty() || (t.lexema == lexemaEsperado);

    if (tipoOk && lexemaOk)
    {
        avanzar();
        return true;
    }

    registrarError(lexemaEsperado.empty() ? "token de otro tipo" : lexemaEsperado,
                    "Se esperaba \"" + (lexemaEsperado.empty() ? string("otro tipo de token") : lexemaEsperado) + "\" y se encontro \"" + t.lexema + "\"");
    return false;
}

void Parser::parsePrograma()
{
    while (!esFinal() && errores.empty())
    {
        parseSentencia();
    }
}

void Parser::parseSentencia()
{
    if (esFinal())
    {
        registrarError("una sentencia", "Se esperaba una sentencia y se llego al final de la entrada");
        return;
    }

    Token t = actual();

    if (t.tipo == TipoToken::PALABRA_RESERVADA &&
        (t.lexema == "int" || t.lexema == "float" || t.lexema == "string"))
    {
        parseDeclaracion();
    }
    else if (t.tipo == TipoToken::IDENTIFICADOR)
    {
        parseAsignacion();
    }
    else
    {
        registrarError("un tipo (int/float/string) o un identificador",
                        "Se esperaba el inicio de una sentencia y se encontro \"" + t.lexema + "\"");
        avanzar();
    }
}

// <Declaracion> -> <Tipo> ID = <Valor> ;
void Parser::parseDeclaracion()
{
    Token tipoToken = actual();
    if (!coincide(TipoToken::PALABRA_RESERVADA)) return;

    Token idToken = actual();
    if (!coincide(TipoToken::IDENTIFICADOR)) return;
    if (!coincide(TipoToken::OPERADOR_ASIGNACION, "=")) return;

    if (esFinal())
    {
        registrarError("un valor (numero, real, string o identificador)", "Fin de entrada inesperado");
        return;
    }

    // el valor puede ser numero, real, string o id
    Token v = actual();
    if (v.tipo == TipoToken::LITERAL_NUMERICO || v.tipo == TipoToken::LITERAL_REAL ||
        v.tipo == TipoToken::LITERAL_CADENA || v.tipo == TipoToken::IDENTIFICADOR)
    {
        avanzar();
    }
    else
    {
        registrarError("un valor (numero, real, string o identificador)",
                        "Se esperaba un valor y se encontro \"" + v.lexema + "\"");
        return;
    }

    if (!coincide(TipoToken::PUNTO_COMA, ";")) return;

    // se guarda la declaracion para que el semantico la revise despues
    Instruccion instr;
    instr.tipo = Instruccion::Tipo::DECLARACION;
    instr.nombre = idToken.lexema;
    instr.tipoDato = tipoToken.lexema;
    instr.valor = { v };
    instr.linea = idToken.linea;
    instr.columna = idToken.columna;
    instrucciones.push_back(instr);
}

// <Asignacion> -> ID = <Expresion> ;
void Parser::parseAsignacion()
{
    Token idToken = actual();
    if (!coincide(TipoToken::IDENTIFICADOR)) return;
    if (!coincide(TipoToken::OPERADOR_ASIGNACION, "=")) return;

    int inicioExpr = posicion;
    parseExpresion();
    if (!errores.empty()) return;
    int finExpr = posicion;

    if (!coincide(TipoToken::PUNTO_COMA, ";")) return;

    // guarda los tokens de la expresion para armar los tercetos mas adelante
    Instruccion instr;
    instr.tipo = Instruccion::Tipo::ASIGNACION;
    instr.nombre = idToken.lexema;
    instr.valor = vector<Token>(tokens.begin() + inicioExpr, tokens.begin() + finExpr);
    instr.linea = idToken.linea;
    instr.columna = idToken.columna;
    instrucciones.push_back(instr);
}

// <Expresion> -> <Termino> ( (+|-) <Termino> )*
// primero se resuelve * y / (termino) y despues + y -
void Parser::parseExpresion()
{
    parseTermino();
    while (!esFinal() && errores.empty() &&
           actual().tipo == TipoToken::OPERADOR_ARITMETICO &&
           (actual().lexema == "+" || actual().lexema == "-"))
    {
        avanzar();
        parseTermino();
        if (!errores.empty()) return;
    }
}

// <Termino> -> <Factor> ( (*|/) <Factor> )*
void Parser::parseTermino()
{
    parseFactor();
    while (!esFinal() && errores.empty() &&
           actual().tipo == TipoToken::OPERADOR_ARITMETICO &&
           (actual().lexema == "*" || actual().lexema == "/"))
    {
        avanzar();
        parseFactor();
        if (!errores.empty()) return;
    }
}

// <Factor> -> ( <Expresion> ) | ID | NUM | REAL
void Parser::parseFactor()
{
    if (esFinal())
    {
        registrarError("un factor (identificador, numero, real o '(')", "Fin de entrada inesperado");
        return;
    }

    Token t = actual();

    if (t.tipo == TipoToken::PARENTESIS_IZQUIERDO)
    {
        avanzar();
        parseExpresion();
        if (!errores.empty()) return;
        coincide(TipoToken::PARENTESIS_DERECHO, ")");
    }
    else if (t.tipo == TipoToken::IDENTIFICADOR || t.tipo == TipoToken::LITERAL_NUMERICO ||
             t.tipo == TipoToken::LITERAL_REAL)
    {
        avanzar();
    }
    else
    {
        registrarError("un factor (identificador, numero, real o '(')",
                        "Se esperaba un factor y se encontro \"" + t.lexema + "\"");
    }
}

bool Parser::analizar()
{
    parsePrograma();
    return errores.empty();
}

vector<ErrorSintactico> Parser::obtenerErrores()
{
    return errores;
}

vector<Instruccion> Parser::obtenerInstrucciones()
{
    return instrucciones;
}