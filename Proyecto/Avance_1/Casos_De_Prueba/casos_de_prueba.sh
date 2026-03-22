#!/bin/bash
echo "Casos de prueba válidos:"
echo -e  "------------------------------------Prueba válida #1\n"
../simulacion_Cliente_Tenedor_Servidor/src/simulacion < test1.txt
echo -e  "\n------------------------------------Prueba válida #2\n"
../simulacion_Cliente_Tenedor_Servidor/src/simulacion < test2.txt
echo -e  "\n------------------------------------Prueba válida #3\n"
../simulacion_Cliente_Tenedor_Servidor/src/simulacion < test3.txt

echo -e "\nCasos de prueba inválidos:"
echo -e  "------------------------------------Prueba inválida #1\n"
../simulacion_Cliente_Tenedor_Servidor/src/simulacion < test4.txt
echo -e  "\n------------------------------------Prueba inválida #2\n"
../simulacion_Cliente_Tenedor_Servidor/src/simulacion < test5.txt
echo -e  "\n------------------------------------Prueba inválida #3\n"
../simulacion_Cliente_Tenedor_Servidor/src/simulacion < test6.txt
echo -e  "\n------------------------------------Prueba inválida #4\n"
../simulacion_Cliente_Tenedor_Servidor/src/simulacion < test7.txt
echo -e  "\n------------------------------------Prueba inválida #5\n"
../simulacion_Cliente_Tenedor_Servidor/src/simulacion < test8.txt

echo -e "\nCasos de prueba al límite:"
echo -e  "\n------------------------------------Prueba límite #1\n"
../simulacion_Cliente_Tenedor_Servidor/src/simulacion < test9.txt

echo -e "\nFin de casos de prueba"
