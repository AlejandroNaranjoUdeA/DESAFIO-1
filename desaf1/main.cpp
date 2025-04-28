#include <iostream>
#include "funciones.h"
#include <fstream>

int main() {
    unsigned char headerID[54], headerIM[54], headerM[54];
    unsigned char *dataID = nullptr, *dataIM = nullptr, *mascara = nullptr;
    int sizeID = 0, sizeIM = 0, sizeMascara = 0;

    std::cout << "\n Cargando imagenes..." << std::endl;

        if (!cargarBMP("I_D.bmp", headerID, dataID, sizeID)) {
        std::cout << "Error al cargar I_D.bmp" << std::endl;
        return 1;
    }
    if (!cargarBMP("I_M.bmp", headerIM, dataIM, sizeIM)) {
        std::cout << "Error al cargar I_M.bmp" << std::endl;
        return 1;
    }
    if (!cargarBMP("M.bmp", headerM, mascara, sizeMascara)) {
        std::cout << "Error al cargar M.bmp" << std::endl;
        return 1;
    }
    if (sizeID != sizeIM) {
        std::cout << "Error: Tamano de I_D.bmp y I_M.bmp no coinciden." << std::endl;
            return 1;
    }

    // Vaciar el archivo de orden previo (por si existe)
    std::ofstream limpiar("orden_transformaciones.txt");
    limpiar.close();

    std::cout << "\n Buscando transformaciones posibles..." << std::endl;

        for (int i = 1; i <= 10; i++) {
        char nombreArchivo[20];
        sprintf(nombreArchivo, "M%d.txt", i);
        if (!archivoExiste(nombreArchivo)) break;

        std::cout << "\n Analizando archivo: " << nombreArchivo << std::endl;
                std::ifstream file(nombreArchivo);
        int desplazamiento = -1;
        file >> desplazamiento;
        file.close();

        if (desplazamiento >= 0) {
            probarYGuardarTransformaciones(dataID, mascara, dataIM, sizeID, sizeMascara, nombreArchivo, desplazamiento);
        } else {
            std::cout << "No se pudo leer desplazamiento de " << nombreArchivo << std::endl;
        }
    }

    // Reconstruir imagen original aplicando transformaciones inversas
    std::cout << "\n Reconstruyendo imagen original..." << std::endl;
    reconstruirImagen(dataID, dataIM, sizeID, "orden_transformaciones.txt", "I_O_reconstruida.bmp", "I_D.bmp");

    // Comparar la imagen reconstruida con la original
    std::cout << "\n Comparando imagen original con reconstruida..." << std::endl;
    compararImagenes("I_O.bmp", "I_O_reconstruida.bmp");

    // Liberar memoria
    delete[] dataID;
    delete[] dataIM;
    delete[] mascara;

    std::cout << "\n Programa finalizado correctamente." << std::endl;
        return 0;
}
