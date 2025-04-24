#include <iostream>
#include "funciones.h"
#include <fstream>

int main() {
    unsigned char headerID[54], headerIM[54], headerM[54];
    unsigned char *dataID = nullptr, *dataIM = nullptr, *mascara = nullptr;
    int sizeID = 0, sizeIM = 0, sizeMascara = 0;

    if (!cargarBMP("ID.bmp", headerID, dataID, sizeID)) return 1;
    if (!cargarBMP("IM.bmp", headerIM, dataIM, sizeIM)) return 1;
    if (!cargarBMP("M.bmp", headerM, mascara, sizeMascara)) return 1;
    if (sizeID != sizeIM) return 1;

    for (int i = 1; i <= 10; i++) {
        char nombreArchivo[20];
        sprintf(nombreArchivo, "M%d.txt", i);
        if (!archivoExiste(nombreArchivo)) break;

        std::cout << "\n🔎 Probando archivo: " << nombreArchivo << std::endl;
        std::ifstream file(nombreArchivo);
        int desplazamiento = -1;
        file >> desplazamiento;
        file.close();

        if (desplazamiento >= 0) {
            probarTodasCombinaciones(dataID, mascara, dataIM, sizeID, sizeMascara, nombreArchivo, desplazamiento);
        } else {
            std::cout << "No se pudo leer desplazamiento de " << nombreArchivo << std::endl;
        }
    }

    delete[] dataID;
    delete[] dataIM;
    delete[] mascara;
    return 0;
}
