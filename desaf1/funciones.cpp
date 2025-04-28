#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include "funciones.h"

using namespace std;

//implementacion de funciones:

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

// === CARGA Y GUARDADO DE BMP ===
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

// === ENMASCARAMIENTO Y VERIFICACIÓN ===
bool verificarEnmascaramiento(unsigned char* imagen, unsigned char* mascara, const char* archivoTxt, int desplazamiento, int tamMascara) {
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

// === SECUENCIA DE TRANSFORMACIONES ===
bool probarTransformaciones(unsigned char* base, unsigned char* mascara, unsigned char* im, int size, int tamMascara, const char* archivoTxt, int desplazamiento, int* operaciones, int* parametros, int numOps) {
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

void guardarTransformacion(const char* nombreArchivo, int* secuencia, int largo) {
    FILE* f = fopen("orden_transformaciones.txt", "a");
    if (!f) return;

    fprintf(f, "%s:", nombreArchivo);
    for (int i = 0; i < largo; i++) {
        fprintf(f, " %d", secuencia[i]);
    }
    fprintf(f, "\n");
    fclose(f);
}

void probarYGuardarTransformaciones(
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

    for (int i = 0; i < 3; i++) {
        ops[0] = tipos[i];
        params[0] = (tipos[i] == 1) ? 0 : 3;

        if (probarTransformaciones(base, mascara, im, size, tamMascara, archivoTxt, desplazamiento, ops, params, 1)) {
            int secuencia[1] = { ops[0] };
            guardarTransformacion(archivoTxt, secuencia, 1);
        }

        for (int j = 0; j < 3; j++) {
            ops[1] = tipos[j];
            params[1] = (tipos[j] == 1) ? 0 : 3;

            if (probarTransformaciones(base, mascara, im, size, tamMascara, archivoTxt, desplazamiento, ops, params, 2)) {
                int secuencia[2] = { ops[0], ops[1] };
                guardarTransformacion(archivoTxt, secuencia, 2);
            }

            for (int k = 0; k < 3; k++) {
                ops[2] = tipos[k];
                params[2] = (tipos[k] == 1) ? 0 : 3;

                if (probarTransformaciones(base, mascara, im, size, tamMascara, archivoTxt, desplazamiento, ops, params, 3)) {
                    int secuencia[3] = { ops[0], ops[1], ops[2] };
                    guardarTransformacion(archivoTxt, secuencia, 3);
                }
            }
        }
    }
}

//Aplica una secuencia de transformaciones inversas y guarda resultado
void reconstruirImagen(
    unsigned char* imagenFinal,
    unsigned char* im,
    int size,
    const char* archivoOrden,
    const char* archivoSalida,
    const char* nombreHeader
    ) {
    unsigned char header[54];
    ifstream hfile(nombreHeader, ios::binary);
    hfile.read((char*)header, 54);
    hfile.close();

    unsigned char* actual = new unsigned char[size];
    memcpy(actual, imagenFinal, size);
    unsigned char* temp = new unsigned char[size];

    ifstream orden(archivoOrden);
    char nombre[20];
    int pasos[10];
    int total;

    while (orden >> nombre) {
        total = 0;
        char c;
        while (orden.get(c) && c != '\n') {
            if (isdigit(c)) {
                orden.putback(c);
                orden >> pasos[total++];
            }
        }

        // Aplicar en orden inverso
        for (int i = total - 1; i >= 0; i--) {
            int op = pasos[i];

            if (op == 1) {
                aplicarXOR(actual, im, temp, size); // XOR es su propio inverso
            } else if (op == 2) {
                aplicarRotacion(actual, temp, size, 3, false); // inversa: rotar izq
            } else if (op == 3) {
                aplicarRotacion(actual, temp, size, 3, true);  // inversa: rotar der
            }
            memcpy(actual, temp, size);
        }
    }

    guardarBMP(archivoSalida, header, actual, size);
    delete[] actual;
    delete[] temp;
}

void compararImagenes(
    const char* originalPath,
    const char* reconstruidaPath
    ) {
    unsigned char headerO[54], headerR[54];
    unsigned char *dataO = nullptr, *dataR = nullptr;
    int sizeO = 0, sizeR = 0;

    if (!cargarBMP(originalPath, headerO, dataO, sizeO)) {
        std::cout << "No se pudo cargar la imagen original." << std::endl;
        return;
    }
    if (!cargarBMP(reconstruidaPath, headerR, dataR, sizeR)) {
        std::cout << "No se pudo cargar la imagen reconstruida." << std::endl;
        delete[] dataO;
        return;
    }

    if (sizeO != sizeR) {
        cout << "Las imagenes no tienen el mismo tamano." << std::endl;
        delete[] dataO;
        delete[] dataR;
        return;
    }

    int diferentes = 0;
    for (int i = 0; i < sizeO; i++) {
        if (dataO[i] != dataR[i]) {
            diferentes++;
        }
    }

    std::cout << "Comparacion completa : " << std::endl;
    std::cout << "Tamano total de bytes: " << sizeO << std::endl;
    std::cout << "Bytes diferentes     : " << diferentes << std::endl;
    std::cout << "Coincidencia         : " << 100.0 * (sizeO - diferentes) / sizeO << " %" << std::endl;

    delete[] dataO;
    delete[] dataR;
}

bool archivoExiste(const char* nombre) {
    ifstream file(nombre);
    return file.good();
}
