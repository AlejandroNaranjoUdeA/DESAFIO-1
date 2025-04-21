#include <iostream>
#include <fstream>

using namespace std;

const int HEADER_SIZE = 54;

// === ROTACIÓN Y DESPLAZAMIENTO DE BITS ===
unsigned char rotarDerecha(unsigned char valor, int bits) {
    return (valor >> bits) | (valor << (8 - bits));
}

unsigned char rotarIzquierda(unsigned char valor, int bits) {
    return (valor << bits) | (valor >> (8 - bits));
}

unsigned char desplazarDerecha(unsigned char valor, int bits) {
    return valor >> bits;
}

unsigned char desplazarIzquierda(unsigned char valor, int bits) {
    return valor << bits;
}

void aplicarRotacion(unsigned char* imagen, unsigned char* resultado, int size, int bits, bool derecha) {
    for (int i = 0; i < size; i++) {
        resultado[i] = derecha ? rotarDerecha(imagen[i], bits) : rotarIzquierda(imagen[i], bits);
    }
}

void aplicarDesplazamiento(unsigned char* imagen, unsigned char* resultado, int size, int bits, bool derecha) {
    for (int i = 0; i < size; i++) {
        resultado[i] = derecha ? desplazarDerecha(imagen[i], bits) : desplazarIzquierda(imagen[i], bits);
    }
}

// === BMP ===
bool cargarBMP(const char* ruta, unsigned char* header, unsigned char*& data, int& dataSize) {
    ifstream file(ruta, ios::binary);
    if (!file) return false;

    file.read((char*)header, HEADER_SIZE);

    if (header[0] != 'B' || header[1] != 'M') return false;

    int offset = *(int*)&header[10];
    dataSize = *(int*)&header[34];
    if (dataSize == 0) {
        int width = *(int*)&header[18];
        int height = *(int*)&header[22];
        int rowSize = ((24 * width + 31) / 32) * 4;
        dataSize = rowSize * abs(height);
    }

    data = new unsigned char[dataSize];
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

// === XOR ===
void aplicarXOR(unsigned char* img1, unsigned char* img2, unsigned char* salida, int size) {
    for (int i = 0; i < size; i++) {
        salida[i] = img1[i] ^ img2[i];
    }
}

// === ENMASCARAMIENTO ===
bool verificarEnmascaramiento(
    unsigned char* imagen,
    unsigned char* mascara,
    const char* archivoTxt,
    int desplazamiento,
    int tamMascara
    ) {
    ifstream file(archivoTxt);
    if (!file) {
        cout << "No se pudo abrir el archivo " << archivoTxt << endl;
        return false;
    }

    int sDesdeArchivo;
    file >> sDesdeArchivo;

    if (sDesdeArchivo != desplazamiento) return false;

    for (int k = 0; k < tamMascara; k += 3) {
        int r, g, b;
        if (!(file >> r >> g >> b)) return false;

        int r_calc = imagen[k + desplazamiento + 0] + mascara[k + 0];
        int g_calc = imagen[k + desplazamiento + 1] + mascara[k + 1];
        int b_calc = imagen[k + desplazamiento + 2] + mascara[k + 2];

        if (r != r_calc || g != g_calc || b != b_calc) return false;
    }

    return true;
}

int main() {
    unsigned char headerID[HEADER_SIZE], headerIM[HEADER_SIZE], headerM[HEADER_SIZE];
    unsigned char *dataID = nullptr, *dataIM = nullptr, *dataXOR = nullptr, *mascara = nullptr;
    int sizeID = 0, sizeIM = 0, sizeMascara = 0;

    if (!cargarBMP("ID.bmp", headerID, dataID, sizeID)) return 1;
    if (!cargarBMP("IM.bmp", headerIM, dataIM, sizeIM)) return 1;
    if (!cargarBMP("M.bmp", headerM, mascara, sizeMascara)) return 1;
    if (sizeID != sizeIM) return 1;

    // XOR
    dataXOR = new unsigned char[sizeID];
    aplicarXOR(dataID, dataIM, dataXOR, sizeID);
    guardarBMP("resultado_xor.bmp", headerID, dataXOR, sizeID);

    // ROTACIÓN derecha 3 bits
    unsigned char* rotada = new unsigned char[sizeID];
    aplicarRotacion(dataID, rotada, sizeID, 3, true);
    guardarBMP("rotada.bmp", headerID, rotada, sizeID);

    // DESPLAZAMIENTO izquierda 2 bits
    unsigned char* desplazada = new unsigned char[sizeID];
    aplicarDesplazamiento(dataID, desplazada, sizeID, 2, false);
    guardarBMP("desplazada.bmp", headerID, desplazada, sizeID);

    // Verificación
    int desplazamiento = 100;
    if (verificarEnmascaramiento(dataXOR, mascara, "M1.txt", desplazamiento, sizeMascara)) {
        cout << "✅ Enmascaramiento coincide con M1.txt" << endl;
    } else {
        cout << "❌ Enmascaramiento NO coincide con M1.txt" << endl;
    }

    delete[] dataID;
    delete[] dataIM;
    delete[] dataXOR;
    delete[] rotada;
    delete[] desplazada;
    delete[] mascara;

    return 0;
}
