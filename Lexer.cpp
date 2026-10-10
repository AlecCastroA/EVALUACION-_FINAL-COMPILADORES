#include "Lexer.h"
#include <cctype>
#include <set>

Lexer::Lexer(string _codigo)
{
    codigo = _codigo;
    posicion = 0;
    linea = 1;
    columna = 1;
}

bool Lexer::esFinal()
{
    return posicion >= codigo.length();
}

char Lexer::actual()
{
    return codigo[posicion];
}

char Lexer::siguiente()
{
    if (posicion + 1 >= codigo.length())
        return '\0';
    return codigo[posicion + 1];
}

char Lexer::avanzar()
{
    char caracter = codigo[posicion];
    posicion++;

    // controla salto de linea
    if (caracter == '\n')
    {
        linea++;
        columna = 1;
    }
    else
    {
        columna++;
    }
    return caracter;
}

void Lexer::ignorarEspacios()
{
    while (!esFinal() && isspace(actual()))
    {
        avanzar();
    }
}

void Lexer::ignorarComentario()
{
    // "//" hasta el fin de linea; el comentario no genera token
    while (!esFinal() && actual() != '\n')
    {
        avanzar();
    }
}

Token Lexer::reconocerNumero()
{
    int lineaInicio = linea;
    int columnaInicio = columna;
    string lexema = "";

    while (!esFinal() && isdigit(actual()))
    {
        lexema += avanzar();
    }

    // esto es para los reales
    bool esReal = false;
    if (!esFinal() && actual() == '.' && isdigit(siguiente()))
    {
        esReal = true;
        lexema += avanzar();
        while (!esFinal() && isdigit(actual()))
        {
            lexema += avanzar();
        }
    }

    if (!esFinal() && (isalpha(actual()) || actual() == '_'))
    {
        while (!esFinal() && (isalnum(actual()) || actual() == '_'))
        {
            lexema += avanzar();
        }
        registrarError(lexema, lineaInicio, columnaInicio,
                        "Identificador invalido, no puede iniciar con un digito");
        return {TipoToken::DESCONOCIDO, lexema, lineaInicio, columnaInicio};
    }

    // 0.5 si es valido, por eso los ceros a la izquierda solo se revisan en enteros
    if (lexema.size() > 1 && lexema[0] == '0' && !esReal)
    {
        registrarError(lexema, lineaInicio, columnaInicio,
                        "Numero invalido, no se permiten ceros a la izquierda");
        return {TipoToken::DESCONOCIDO, lexema, lineaInicio, columnaInicio};
    }

    return {esReal ? TipoToken::LITERAL_REAL : TipoToken::LITERAL_NUMERICO,
            lexema, lineaInicio, columnaInicio};
}

Token Lexer::reconocerIdentificador()
{
    int lineaInicio = linea;
    int columnaInicio = columna;
    string lexema = "";

    while (!esFinal() && (isalnum(actual()) || actual() == '_'))
    {
        lexema += avanzar();
    }

    if (esPalabraReservada(lexema))
    {
        return {TipoToken::PALABRA_RESERVADA, lexema, lineaInicio, columnaInicio};
    }

    return {TipoToken::IDENTIFICADOR, lexema, lineaInicio, columnaInicio};
}

Token Lexer::reconocerCadena()
{
    int lineaInicio = linea;
    int columnaInicio = columna;
    string lexema = "";

    avanzar(); // consume la comilla de apertura

    while (!esFinal() && actual() != '"' && actual() != '\n')
    {
        lexema += avanzar();
    }

    if (esFinal() || actual() == '\n')
    {
        registrarError(lexema, lineaInicio, columnaInicio, "Cadena sin cerrar");
        return {TipoToken::DESCONOCIDO, lexema, lineaInicio, columnaInicio};
    }

    avanzar(); // consume la comilla de cierre
    return {TipoToken::LITERAL_CADENA, lexema, lineaInicio, columnaInicio};
}

Token Lexer::reconocerOperador()
{
    int lineaInicio = linea;
    int columnaInicio = columna;
    string lexema = "";
    lexema += avanzar();

    // ">=" y "<=" tienen prioridad sobre el simbolo simple
    if ((lexema == ">" || lexema == "<") && actual() == '=')
    {
        lexema += avanzar();
        return {TipoToken::OPERADOR_RELACIONAL, lexema, lineaInicio, columnaInicio};
    }

    if (lexema == "=")
    {
        return {TipoToken::OPERADOR_ASIGNACION, lexema, lineaInicio, columnaInicio};
    }

    registrarError(lexema, lineaInicio, columnaInicio, "Operador no reconocido");
    return {TipoToken::DESCONOCIDO, lexema, lineaInicio, columnaInicio};
}

bool Lexer::esPalabraReservada(string palabra)
{
    static const set<string> reservadas = {"int", "float", "string"};
    return reservadas.count(palabra) > 0;
}

// error lexico
vector<ErrorLexico> Lexer::obtenerErrores()
{
    return errores;
}

void Lexer::registrarError(string lexema, int lineaError, int columnaError, string mensaje)
{
    ErrorLexico error;
    error.lexema = lexema;
    error.linea = lineaError;
    error.columna = columnaError;
    error.mensaje = mensaje;
    errores.push_back(error);
}

vector<Token> Lexer::analizar()
{
    vector<Token> tokens;

    while (!esFinal())
    {
        ignorarEspacios();
        if (esFinal())
        {
            break;
        }

        char c = actual();

        // Comentario "//": se descarta, no genera token
        if (c == '/' && siguiente() == '/')
        {
            ignorarComentario();
            continue;
        }

        // Identificador o palabra reservada
        if (isalpha(c))
        {
            tokens.push_back(reconocerIdentificador());
        }
        // Literal numerico
        else if (isdigit(c))
        {
            tokens.push_back(reconocerNumero());
        }
        // Literal de cadena
        else if (c == '"')
        {
            tokens.push_back(reconocerCadena());
        }
        // Operadores de asignacion y relacionales
        else if (c == '=' || c == '>' || c == '<')
        {
            tokens.push_back(reconocerOperador());
        }
        // parentesis y coma
        else if (c == '(' || c == ')' || c == ',')
        {
            int lineaInicio = linea;
            int columnaInicio = columna;
            char simbolo = avanzar();
            TipoToken tipo = (simbolo == '(') ? TipoToken::PARENTESIS_IZQUIERDO
                            : (simbolo == ')') ? TipoToken::PARENTESIS_DERECHO
                            : TipoToken::COMA;
            tokens.push_back({tipo, string(1, simbolo), lineaInicio, columnaInicio});
        }
        // punto y coma
        else if (c == ';')
        {
            int lineaInicio = linea;
            int columnaInicio = columna;
            char simbolo = avanzar();
            tokens.push_back({TipoToken::PUNTO_COMA, string(1, simbolo), lineaInicio, columnaInicio});
        }
        // + - * / 
        else if (c == '+' || c == '-' || c == '*' || c == '/')
        {
            int lineaInicio = linea;
            int columnaInicio = columna;
            char simbolo = avanzar();
            tokens.push_back({TipoToken::OPERADOR_ARITMETICO, string(1, simbolo), lineaInicio, columnaInicio});
        }
        else
        {
            int lineaError = linea;
            int columnaError = columna;
            string lexema = "";
            lexema += avanzar();
            registrarError(lexema, lineaError, columnaError, "Simbolo no reconocido");
            tokens.push_back({TipoToken::DESCONOCIDO, lexema, lineaError, columnaError});
        }
    }

    return tokens;
}