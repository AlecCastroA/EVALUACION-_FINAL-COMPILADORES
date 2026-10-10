#include "GeneradorCodigo.h"
#include <stack>
#include <map>
#include <sstream>

GeneradorCodigo::GeneradorCodigo()
{
    contadorTemp = 1;
}

int GeneradorCodigo::precedencia(string op)
{
    if (op == "*" || op == "/") return 2;
    if (op == "+" || op == "-") return 1;
    return 0;
}

// Algoritmo shunting-yard: convierte la expresion infija a notacion postfija
vector<Token> GeneradorCodigo::aPostfijo(vector<Token> &infijo)
{
    vector<Token> salida;
    stack<Token> operadores;

    for (Token t : infijo)
    {
        if (t.tipo == TipoToken::IDENTIFICADOR || t.tipo == TipoToken::LITERAL_NUMERICO ||
            t.tipo == TipoToken::LITERAL_REAL)
        {
            salida.push_back(t);
        }
        else if (t.tipo == TipoToken::PARENTESIS_IZQUIERDO)
        {
            operadores.push(t);
        }
        else if (t.tipo == TipoToken::PARENTESIS_DERECHO)
        {
            while (!operadores.empty() && operadores.top().tipo != TipoToken::PARENTESIS_IZQUIERDO)
            {
                salida.push_back(operadores.top());
                operadores.pop();
            }
            if (!operadores.empty()) operadores.pop(); // descarta el '('
        }
        else if (t.tipo == TipoToken::OPERADOR_ARITMETICO)
        {
            while (!operadores.empty() && operadores.top().tipo == TipoToken::OPERADOR_ARITMETICO &&
                   precedencia(operadores.top().lexema) >= precedencia(t.lexema))
            {
                salida.push_back(operadores.top());
                operadores.pop();
            }
            operadores.push(t);
        }
    }

    while (!operadores.empty())
    {
        salida.push_back(operadores.top());
        operadores.pop();
    }

    return salida;
}

// Genera tercetos recorriendo la notacion postfija con una pila de operandos
vector<Terceto> GeneradorCodigo::generarTercetos(string destinoFinal, vector<Token> expresionInfija)
{
    vector<Terceto> tercetos;
    vector<Token> postfijo = aPostfijo(expresionInfija);
    stack<string> operandos;

    for (Token t : postfijo)
    {
        if (t.tipo == TipoToken::OPERADOR_ARITMETICO)
        {
            string derecho = operandos.top(); operandos.pop();
            string izquierdo = operandos.top(); operandos.pop();

            string nombreTemp = "t" + to_string(contadorTemp++);
            Terceto terceto;
            terceto.destino = nombreTemp;
            terceto.izquierdo = izquierdo;
            terceto.operador = t.lexema;
            terceto.derecho = derecho;
            tercetos.push_back(terceto);

            operandos.push(nombreTemp);
        }
        else
        {
            operandos.push(t.lexema);
        }
    }

    Terceto final;
    final.destino = destinoFinal;
    final.izquierdo = operandos.top();
    final.operador = "";
    final.derecho = "";
    tercetos.push_back(final);

    return tercetos;
}

static bool esNumero(string s, double &valor)
{
    try
    {
        size_t pos;
        valor = stod(s, &pos);
        return pos == s.size();
    }
    catch (...)
    {
        return false;
    }
}

static string formatearNumero(double valor)
{
    if (valor == (long long)valor)
    {
        return to_string((long long)valor);
    }
    ostringstream oss;
    oss << valor;
    return oss.str();
}

// Optimizacion por plegado de constantes (constant folding):
// si ambos operandos de un terceto son valores numericos conocidos,
// se calcula el resultado en "tiempo de compilacion" y se elimina esa linea.
vector<Terceto> GeneradorCodigo::optimizar(vector<Terceto> &tercetos)
{
    vector<Terceto> optimizados;
    map<string, double> constantes;

    for (Terceto t : tercetos)
    {
        if (!t.operador.empty())
        {
            double valIzq, valDer;
            bool izqEsConstante = constantes.count(t.izquierdo)
                ? (valIzq = constantes[t.izquierdo], true)
                : esNumero(t.izquierdo, valIzq);
            bool derEsConstante = constantes.count(t.derecho)
                ? (valDer = constantes[t.derecho], true)
                : esNumero(t.derecho, valDer);

            if (izqEsConstante && derEsConstante)
            {
                double resultado = 0;
                if (t.operador == "+") resultado = valIzq + valDer;
                else if (t.operador == "-") resultado = valIzq - valDer;
                else if (t.operador == "*") resultado = valIzq * valDer;
                else if (t.operador == "/") resultado = valIzq / valDer;

                constantes[t.destino] = resultado; // se "dobla", no se emite esta linea
            }
            else
            {
                Terceto nuevo = t;
                if (constantes.count(t.izquierdo)) nuevo.izquierdo = formatearNumero(constantes[t.izquierdo]);
                if (constantes.count(t.derecho)) nuevo.derecho = formatearNumero(constantes[t.derecho]);
                optimizados.push_back(nuevo);
            }
        }
        else
        {
            Terceto nuevo = t;
            if (constantes.count(t.izquierdo)) nuevo.izquierdo = formatearNumero(constantes[t.izquierdo]);
            optimizados.push_back(nuevo);
        }
    }

    return optimizados;
}

vector<Token> GeneradorCodigo::convertirAPostfijo(vector<Token> expresion)
{
    return aPostfijo(expresion);
}