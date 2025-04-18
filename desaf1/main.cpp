#include <iostream>
#include <fstream>

using namespace std;

const int HEADER_SIZE = 54;

bool cargarBMP(const char* ruta, unsigned char* header, unsigned char*& data, int& dataSize) {
    ifstream file(ruta, ios::binary);
    if (!file) return false;

    // Leer encabezado (54 bytes)
    file.read((char*)header, HEADER_SIZE);

    // Validar que sea BMP (primeros dos bytes deben ser 'B' y 'M')
    if (header[0] != 'B' || header[1] != 'M') {
        cout << "Archivo no es un BMP válido." << endl;
            return false;
    }

    // Leer ancho, alto y tamaño de imagen desde el header
    int offset = *(int*)&header[10];
    dataSize = *(int*)&header[34];
    if (dataSize == 0) { // A veces el tamaño no está en el header
        int width = *(int*)&header[18];
        int height = *(int*)&header[22];
        int rowSize = ((24 * width + 31) / 32) * 4; // filas alineadas a múltiplos de 4 bytes
        dataSize = rowSize * abs(height);
    }

    data = new unsigned char[dataSize];

    // Saltar al comienzo de los datos y leerlos
    file.seekg(offset, ios::beg);
    file.read((char*)data, dataSize);
    file.close();
    return true;
}

bool guardarBMP(const char* ruta, unsigned char* header, unsigned char* data, int dataSize) {
    ofstream file(ruta, ios::binary);
    if (!file) return false;

    file.write((char*)header, HEADER_SIZE);
    file.write((char*)data, dataSize);
    file.close();
    return true;
}

void aplicarXOR(unsigned char* img1, unsigned char* img2, unsigned char* salida, int size) {
    for (int i = 0; i < size; i++) {
        salida[i] = img1[i] ^ img2[i];
    }
}

int main() {
    unsigned char header1[HEADER_SIZE];
    unsigned char header2[HEADER_SIZE];
    unsigned char* img1 = nullptr;
    unsigned char* img2 = nullptr;

    int size1 = 0, size2 = 0;

    if (!cargarBMP("ID.bmp", header1, img1, size1)) {
        cout << "Error al cargar ID.bmp" << endl;
        return 1;
    }

    if (!cargarBMP("IM.bmp", header2, img2, size2)) {
        cout << "Error al cargar IM.bmp" << endl;
        delete[] img1;
        return 1;
    }

    if (size1 != size2) {
        cout << "Las imágenes no tienen el mismo tamaño." << endl;
            delete[] img1;
        delete[] img2;
        return 1;
    }

    unsigned char* resultado = new unsigned char[size1];
    aplicarXOR(img1, img2, resultado, size1);

    if (!guardarBMP("resultado.bmp", header1, resultado, size1)) {
        cout << "Error al guardar la imagen resultante." << endl;
    } else {
        cout << "Imagen XOR guardada como resultado.bmp" << endl;
    }

    delete[] img1;
    delete[] img2;
    delete[] resultado;
    return 0;
}
