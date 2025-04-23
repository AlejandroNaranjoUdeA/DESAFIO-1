
#include <iostream>
#include <fstream>
#include <cstring>

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

void aplicarXOR(unsigned char* img1, unsigned char* img2, unsigned char* salida, int size) {
    for (int i = 0; i < size; i++) {
        salida[i] = img1[i] ^ img2[i];
    }
}

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

bool verificarEnmascaramiento(
    unsigned char* imagen,
    unsigned char* mascara,
    const char* archivoTxt,
    int desplazamiento,
    int tamMascara
    ) {
    ifstream file(archivoTxt);
    if (!file) return false;

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

bool probarTransformaciones(
    unsigned char* base,
    unsigned char* mascara,
    unsigned char* im,
    int size,
    int tamMascara,
    const char* archivoTxt,
    int desplazamiento,
    int* operaciones,
    int* parametros,
    int numOps
    ) {
    unsigned char* temp1 = new unsigned char[size];
    memcpy(temp1, base, size);

    unsigned char* temp2 = new unsigned char[size];

    for (int i = 0; i < numOps; i++) {
        int tipo = operaciones[i];
        int param = parametros[i];

        if (tipo == 1) {
            aplicarXOR(temp1, im, temp2, size);
        } else if (tipo == 2) {
            aplicarRotacion(temp1, temp2, size, param, true);
        } else if (tipo == 3) {
            aplicarRotacion(temp1, temp2, size, param, false);
        } else {
            delete[] temp1;
            delete[] temp2;
            return false;
        }

        memcpy(temp1, temp2, size);

        if (i < numOps - 1) {
            if (!verificarEnmascaramiento(temp1, mascara, archivoTxt, desplazamiento, tamMascara)) {
                delete[] temp1;
                delete[] temp2;
                return false;
            }
        }
    }

    bool ok = verificarEnmascaramiento(temp1, mascara, archivoTxt, desplazamiento, tamMascara);
    delete[] temp1;
    delete[] temp2;
    return ok;
}

void probarTodasCombinaciones(
    unsigned char* base,
    unsigned char* mascara,
    unsigned char* im,
    int size,
    int tamMascara,
    const char* archivoTxt,
    int desplazamiento
    ) {
    int ops[3], params[3];
    int tipos[] = {1, 2, 3}; // 1=XOR, 2=ROTDER, 3=ROTI
    int n = 3;

    for (int i = 0; i < n; i++) {
        ops[0] = tipos[i];
        params[0] = (tipos[i] == 1) ? 0 : 3;

        if (probarTransformaciones(base, mascara, im, size, tamMascara, archivoTxt, desplazamiento, ops, params, 1)) {
            cout << "✔️ Coincide con: " << ops[0] << endl;
        }

        for (int j = 0; j < n; j++) {
            ops[1] = tipos[j];
            params[1] = (tipos[j] == 1) ? 0 : 3;

            if (probarTransformaciones(base, mascara, im, size, tamMascara, archivoTxt, desplazamiento, ops, params, 2)) {
                cout << "✔️ Coincide con: " << ops[0] << " -> " << ops[1] << endl;
            }

            for (int k = 0; k < n; k++) {
                ops[2] = tipos[k];
                params[2] = (tipos[k] == 1) ? 0 : 3;

                if (probarTransformaciones(base, mascara, im, size, tamMascara, archivoTxt, desplazamiento, ops, params, 3)) {
                    cout << "✔️ Coincide con: " << ops[0] << " -> " << ops[1] << " -> " << ops[2] << endl;
                }
            }
        }
    }
}

int main() {
    unsigned char headerID[HEADER_SIZE], headerIM[HEADER_SIZE], headerM[HEADER_SIZE];
    unsigned char *dataID = nullptr, *dataIM = nullptr, *mascara = nullptr;
    int sizeID = 0, sizeIM = 0, sizeMascara = 0;

    if (!cargarBMP("ID.bmp", headerID, dataID, sizeID)) return 1;
    if (!cargarBMP("IM.bmp", headerIM, dataIM, sizeIM)) return 1;
    if (!cargarBMP("M.bmp", headerM, mascara, sizeMascara)) return 1;
    if (sizeID != sizeIM) return 1;

    probarTodasCombinaciones(dataID, mascara, dataIM, sizeID, sizeMascara, "M1.txt", 100);

    delete[] dataID;
    delete[] dataIM;
    delete[] mascara;
    return 0;
}
