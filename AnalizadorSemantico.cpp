#include "AnalizadorSemantico.h"

AnalizadorSemantico::AnalizadorSemantico(vector<Instruccion> _instrucciones)
{
    instrucciones = _instrucciones;
}

void AnalizadorSemantico::registrarError(string mensaje, int linea, int columna)
{
    ErrorSemantico error;
    error.mensaje = mensaje;
    error.linea = linea;
    error.columna = columna;
    errores.push_back(error);
}

bool AnalizadorSemantico::esCompatible(string tipoDeclarado, string tipoValor)
{
    if (tipoDeclarado == tipoValor) return true;
    // un int cabe en un float, al reves no
    if (tipoDeclarado == "float" && tipoValor == "int") return true;
    return false;
}

// saca el tipo de una expresion (int o float) revisando cada variable que usa
// si algo esta mal devuelve "" y el error ya queda registrado
string AnalizadorSemantico::tipoDeExpresion(vector<Token> &expr, int linea, int columna)
{
    bool esFloat = false;

    for (Token &t : expr)
    {
        if (t.tipo == TipoToken::PARENTESIS_IZQUIERDO || t.tipo == TipoToken::PARENTESIS_DERECHO ||
            t.tipo == TipoToken::OPERADOR_ARITMETICO)
        {
            continue;
        }
        else if (t.tipo == TipoToken::IDENTIFICADOR)
        {
            if (tablaSimbolos.find(t.lexema) == tablaSimbolos.end())
            {
                registrarError("Identificador no declarado: \"" + t.lexema + "\"", linea, columna);
                return "";
            }
            string tipo = tablaSimbolos[t.lexema];
            if (tipo == "string")
            {
                registrarError("La variable \"" + t.lexema + "\" es de tipo string y no puede usarse en una expresion aritmetica", linea, columna);
                return "";
            }
            if (tipo == "float") esFloat = true;
        }
        else if (t.tipo == TipoToken::LITERAL_REAL)
        {
            esFloat = true;
        }
        // un LITERAL_NUMERICO no cambia nada, sigue siendo int
    }

    return esFloat ? "float" : "int";
}

bool AnalizadorSemantico::analizar()
{
    for (Instruccion &instr : instrucciones)
    {
        if (instr.tipo == Instruccion::Tipo::DECLARACION)
        {
            // primero se revisa que la variable no exista ya en la tabla
            if (tablaSimbolos.find(instr.nombre) != tablaSimbolos.end())
            {
                registrarError("Variable \"" + instr.nombre + "\" declarada dos veces", instr.linea, instr.columna);
                continue;
            }

            Token v = instr.valor[0];
            string tipoValor;

            // si el valor es otra variable, tiene que estar declarada antes
            if (v.tipo == TipoToken::IDENTIFICADOR)
            {
                if (tablaSimbolos.find(v.lexema) == tablaSimbolos.end())
                {
                    registrarError("Identificador no declarado: \"" + v.lexema + "\"", instr.linea, instr.columna);
                    continue;
                }
                tipoValor = tablaSimbolos[v.lexema];
            }
            else if (v.tipo == TipoToken::LITERAL_NUMERICO) tipoValor = "int";
            else if (v.tipo == TipoToken::LITERAL_REAL) tipoValor = "float";
            // lo que va entre comillas siempre es string
            else if (v.tipo == TipoToken::LITERAL_CADENA) tipoValor = "string";

            if (!esCompatible(instr.tipoDato, tipoValor))
            {
                registrarError("Tipo incompatible: no se puede asignar un valor de tipo \"" + tipoValor +
                                "\" a una variable declarada como \"" + instr.tipoDato + "\"", instr.linea, instr.columna);
                continue;
            }

            // solo se registra en la tabla si la declaracion salio bien
            tablaSimbolos[instr.nombre] = instr.tipoDato;
        }
        else // ASIGNACION
        {
            if (tablaSimbolos.find(instr.nombre) == tablaSimbolos.end())
            {
                registrarError("Variable \"" + instr.nombre + "\" no ha sido declarada", instr.linea, instr.columna);
                continue;
            }

            string tipoDestino = tablaSimbolos[instr.nombre];
            string tipoExpr = tipoDeExpresion(instr.valor, instr.linea, instr.columna);
            if (tipoExpr.empty()) continue; // el error ya se registro arriba

            if (!esCompatible(tipoDestino, tipoExpr))
            {
                registrarError("Tipo incompatible: no se puede asignar una expresion de tipo \"" + tipoExpr +
                                "\" a la variable \"" + instr.nombre + "\" (tipo \"" + tipoDestino + "\")", instr.linea, instr.columna);
            }
        }
    }

    return errores.empty();
}

map<string, string> AnalizadorSemantico::obtenerTablaSimbolos()
{
    return tablaSimbolos;
}

vector<ErrorSemantico> AnalizadorSemantico::obtenerErrores()
{
    return errores;
}