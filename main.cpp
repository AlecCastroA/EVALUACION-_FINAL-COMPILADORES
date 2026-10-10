#include <iostream>
#include <vector>
#include <fstream>
#include "Lexer.h"
#include "Parser.h"
#include "AnalizadorSemantico.h"
#include "GeneradorCodigo.h"
using namespace std;

string nombreToken(TipoToken tipo)
{
    switch (tipo)
    {
    case TipoToken::PALABRA_RESERVADA:
        return "PALABRA_RESERVADA";
    case TipoToken::IDENTIFICADOR:
        return "IDENTIFICADOR";
    case TipoToken::LITERAL_NUMERICO:
        return "LITERAL_NUMERICO";
    case TipoToken::LITERAL_CADENA:
        return "LITERAL_CADENA";
    case TipoToken::OPERADOR_ASIGNACION:
        return "OPERADOR_ASIGNACION";
    case TipoToken::OPERADOR_RELACIONAL:
        return "OPERADOR_RELACIONAL";
    case TipoToken::PARENTESIS_IZQUIERDO:
        return "PARENTESIS_IZQUIERDO";
    case TipoToken::PARENTESIS_DERECHO:
        return "PARENTESIS_DERECHO";
    case TipoToken::COMA:
        return "COMA";
    case TipoToken::DESCONOCIDO:
        return "DESCONOCIDO";
    // estos los agregamos para el EF
    case TipoToken::LITERAL_REAL:
        return "LITERAL_REAL";
    case TipoToken::OPERADOR_ARITMETICO:
        return "OPERADOR_ARITMETICO";
    case TipoToken::PUNTO_COMA:
        return "PUNTO_COMA";
    }
    return "";
}

int main()
{
    // ahora lee de un archivo en vez de pedir el codigo por consola
    string rutaArchivo = "codigo.txt";
    ifstream archivo(rutaArchivo);

    if (!archivo.is_open())
    {
        cout << "No se pudo abrir el archivo: " << rutaArchivo << endl;
        return 1;
    }

    string codigo((istreambuf_iterator<char>(archivo)), istreambuf_iterator<char>());
    archivo.close();

    cout << "Codigo fuente leido:\n" << codigo << "\n" << endl;

    Lexer lexer(codigo);
    vector<Token> tokens = lexer.analizar();

    cout << "--------TOKENS ENCONTRADOS---------\n";
    for (Token token : tokens)
    {
        cout << "<" << nombreToken(token.tipo) << ", " << token.lexema << ">" << endl;
    }

    vector<ErrorLexico> errores = lexer.obtenerErrores();
    cout << "\n-------- ERRORES LEXICOS --------\n";
    if (errores.empty())
    {
        cout << "No se encontraron errores lexicos." << endl;
    }
    else
    {
        for (ErrorLexico error : errores)
        {
            cout << "Lexema: " << error.lexema << endl;
            cout << "Linea: " << error.linea << endl;
            cout << "Columna: " << error.columna << endl;
            cout << "Mensaje: " << error.mensaje << endl;
            cout << "----------------------" << endl;
        }
    }

    // si hay errores lexicos no seguimos al parser, el lexer igual termina de leer todo
    if (errores.empty())
    {
        Parser parser(tokens);
        bool aceptada = parser.analizar();

        cout << "\n-------- ANALISIS SINTACTICO --------\n";
        if (aceptada)
        {
            cout << "Cadena ACEPTADA." << endl;
        }
        else
        {
            cout << "Cadena RECHAZADA." << endl;
            for (ErrorSintactico e : parser.obtenerErrores())
            {
                cout << "Linea: " << e.linea << ", Columna: " << e.columna << endl;
                cout << "Encontrado: \"" << e.tokenEncontrado << "\"" << endl;
                cout << "Mensaje: " << e.mensaje << endl;
                cout << "----------------------" << endl;
            }
        }

        if (aceptada)
        {
            AnalizadorSemantico semantico(parser.obtenerInstrucciones());
            bool valido = semantico.analizar();

            cout << "\n-------- ANALISIS SEMANTICO --------\n";
            if (valido)
            {
                cout << "Sin errores semanticos." << endl;
                cout << "\nTabla de simbolos:" << endl;
                for (auto &par : semantico.obtenerTablaSimbolos())
                {
                    cout << par.first << " -> " << par.second << endl;
                }

                cout << "\n-------- CODIGO INTERMEDIO (antes de optimizar) --------\n";
                GeneradorCodigo generador;
                vector<Terceto> todosTercetos;

                for (Instruccion instr : parser.obtenerInstrucciones())
                {
                    if (instr.tipo == Instruccion::Tipo::ASIGNACION)
                    {
                        // agregado despues para mostrar la postfija
                        cout << "Notacion postfija: ";
                        for (Token tk : generador.convertirAPostfijo(instr.valor))
                        {
                            cout << tk.lexema << " ";
                        }
                        cout << endl;

                        vector<Terceto> tercetos = generador.generarTercetos(instr.nombre, instr.valor);
                        for (Terceto t : tercetos)
                        {
                            todosTercetos.push_back(t);
                            if (t.operador.empty())
                                cout << t.destino << " = " << t.izquierdo << endl;
                            else
                                cout << t.destino << " = " << t.izquierdo << " " << t.operador << " " << t.derecho << endl;
                        }
                    }
                }

                if (!todosTercetos.empty())
                {
                    cout << "\n-------- CODIGO INTERMEDIO (despues de optimizar) --------\n";
                    vector<Terceto> optimizados = generador.optimizar(todosTercetos);
                    for (Terceto t : optimizados)
                    {
                        if (t.operador.empty())
                            cout << t.destino << " = " << t.izquierdo << endl;
                        else
                            cout << t.destino << " = " << t.izquierdo << " " << t.operador << " " << t.derecho << endl;
                    }
                }
            }
            else
            {
                cout << "Se encontraron errores semanticos:" << endl;
                for (ErrorSemantico e : semantico.obtenerErrores())
                {
                    cout << "Linea: " << e.linea << ", Columna: " << e.columna << endl;
                    cout << "Mensaje: " << e.mensaje << endl;
                    cout << "----------------------" << endl;
                }
            }
        }
    }

    return 0;
}