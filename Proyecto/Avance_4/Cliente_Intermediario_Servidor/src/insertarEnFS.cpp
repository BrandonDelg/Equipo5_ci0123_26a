#include "FileSystem.hpp"
#include <iostream>
#include <string>
#include <limits>

void mostrarFiguras(FileSystem& fs) {
   auto figuras = fs.getFiguras();

   std::cout << "\nFiguras actuales:\n";

   if (figuras.empty()) {
      std::cout << "No hay figuras registradas.\n";
      return;
   }

   for (const auto& figura : figuras) {
      std::cout << "- " << figura << std::endl;
   }
}

int main () {
   FileSystem fs("filesystem.dat");
   fs.crearFigura("Perro",
      "1\nlego 1x2:4\nlego 2x2:2\nlego 3x2:4\nlego 3x3:2\nlego 3x2:4\nlego 3x3:2\n"
      "2\nlego 3x2:4\nlego 3x3:2\n");

   fs.crearFigura("Gato",
      "1\nlego 5x2:4\n"
      "2\nlego 6x2:4\n");

   fs.crearFigura("Ballena",
      "1\n"
      "lego 3x2:4\n"
      "lego 2x2:2\n"
      "lego 1x5:3\n"
      "2\n"
      "lego 1x2:4\n"
      "lego 4x2:2\n"
      "lego 3x4:3\n"
   );

   fs.crearFigura("Oveja",
      "1\n"
      "lego 2x2:2\n"
      "lego 1x5:3\n"
      "lego 1x2:4\n"
      "2\n"
      "lego 1x2:4\n"
      "lego 4x2:2\n"
      "lego 3x4:3\n"
   );

   fs.crearFigura("Carro",
      "1\n"
      "Medium Stone Gray Bar 1:2\n"
      "Black Bracket 1x1 with 1x1 Plate Down:2\n"
      "Medium Stone Gray Bracket 1x2 with 12 Up:1\n"
      "Dark Red Brick 1x2 with Bottom Tube:1\n"
      "Red Brick 1x2 with Grille:2\n"
      "Transparent Brick 1x2 without Bottom Tube:2\n"
      "Red Brick 1x4:2\n"
      "Transparent Diamond:2\n"
      "Black Mudguard Plate 2x4 with Arches with Hole:2\n"
      "Red Panel 1x2x1 with Rounded Corners:1\n"
      "Pearl Gold Plate 1x1 Round:3\n"
      "Medium Stone Gray Plate 1x1 with Clip (Thick Ring):2\n"
      "Pearl Gold Plate 1x1 with Horizontal Clip (Thick Open O Clip):1\n"
      "Red Plate 1x2:1\n"
      "Red Plate 1x2 with 1 Stud (with Groove and Bottom Stud Holder):1\n"
      "Medium Stone Gray Plate 1x2 with Horizontal Clips (Open O Clips):2\n"
      "2\n"
      "Medium Stone Gray Plate 1x6:2\n"
      "Red Plate 1x8:2\n"
      "Black Plate 2x2 Corner:2\n"
      "Medium Stone Gray Plate 2x2 with Two Wheel Holder Pins:2\n"
      "Black Plate 2x4:1\n"
      "Black Slope 1x2 Curved:2\n"
      "Black Slope 1x3 Curved:2\n"
      "Reddish Brown Suitcase:1\n"
      "Transparent Red Tile 1x1 Round:2\n"
      "Pearl Gold Tile 1x1 Round with Crown Coin:1\n"
      "Medium Stone Gray Tile 1x2 (with Bottom Groove):1\n"
      "Black Tile 2x2 with Studs on Edge:4\n"
      "Dark Red Tile 2x4:1\n"
      "Red Tile 2x4:1\n"
      "Black Tire 14x6:4\n"
      "Flat Silver Wheel Rim 11x6 with Hub Cap:4\n"
   );

   bool seguir = true;

   while (seguir) {
      std::cout << "\n1) Insertar/Agregar piezas a figura\n";
      std::cout << "2) Borrar figura\n";
      std::cout << "3) Mostrar lista de figuras\n";
      std::cout << "4) Salir\n";
      std::cout << "Opcion: ";

      int op;
      std::cin >> op;
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

     if (op == 1) {
         std::string nombre;
         std::string mitad;
         std::string pieza;

         std::cout << "Nombre de la figura: ";
         std::getline(std::cin, nombre);

         std::cout << "Mitad de la figura (1 o 2): ";
         std::getline(std::cin, mitad);

         if (mitad != "1" && mitad != "2") {
            std::cout << "Mitad invalida. Debe ser 1 o 2.\n";
            continue;
         }

         std::string piezasNuevas;

         std::cout << "Ingrese piezas en formato pieza:cantidad.\n";
         std::cout << "Ejemplo: cabeza:2\n";
         std::cout << "Escriba FIN para terminar.\n";

         while (true) {
            std::cout << "Pieza: ";
            std::getline(std::cin, pieza);

            if (pieza == "FIN" || pieza == "fin") {
               break;
            }

            if (!pieza.empty()) {
               piezasNuevas += pieza + "\n";
            }
         }

         fs.agregarPiezas(nombre, std::stoi(mitad), piezasNuevas);

         std::cout << "Figura guardada correctamente.\n";
         mostrarFiguras(fs);
      }  else if (op == 2) {
         std::string nombre;

         std::cout << "Nombre de la figura a borrar: ";
         std::getline(std::cin, nombre);

         fs.borrarFigura(nombre);

         std::cout << "Figura borrada exitosamente.\n";
         mostrarFiguras(fs);

      } else if (op == 3) {
         mostrarFiguras(fs);

      } else if (op == 4) {
         seguir = false;
         std::cout << "Saliendo...\n";

      } else {
         std::cout << "Opcion incorrecta.\n";
      }
   }

   return 0;
}