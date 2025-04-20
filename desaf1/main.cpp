#include <iostream>
#include <fstream>

using namespace std;

const int HEADER_SIZE = 54;

// === Cargar imagen BMP de 24 bits ===
bool cargarBMP(const char* ruta, unsigned char* header, unsigned char*& data, int& dataSize) {
    ifstream file(ruta, ios::binary);
    if (!file) return false;

    file.read((char*)header, HEADER_SIZE);

    if (header[0] != 'B' || header[1] != 'M') {
        cout << "Archivo no es un BMP válido." << endl;
            return false;
    }

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

// === Guardar imagen BMP ===
bool guardarBMP(const char* ruta, unsigned char* header, unsigned char* data, int dataSize) {
    ofstream file(ruta, ios::binary);
    if (!file) return false;

    file.write((char*)header, HEADER_SIZE);
    file.write((char*)data, dataSize);
    file.close();
    return true;
}

// === Aplicar XOR entre dos imágenes ===
void aplicarXOR(unsigned char* img1, unsigned char* img2, unsigned char* salida, int size) {
    for (int i = 0; i < size; i++) {
        salida[i] = img1[i] ^ img2[i];
    }
}

// === Verificar enmascaramiento con archivo .txt ===
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

    if (sDesdeArchivo != desplazamiento) {
        cout << "El desplazamiento del archivo no coincide." << endl;
        return false;
    }

    for (int k = 0; k < tamMascara; k += 3) {
        int r, g, b;
        if (!(file >> r >> g >> b)) {
            cout << "Error leyendo valores RGB en archivo." << endl;
            return false;
        }

        int r_calc = imagen[k + desplazamiento + 0] + mascara[k + 0];
        int g_calc = imagen[k + desplazamiento + 1] + mascara[k + 1];
        int b_calc = imagen[k + desplazamiento + 2] + mascara[k + 2];

        if (r != r_calc || g != g_calc || b != b_calc) {
            return false;
        }
    }

    return true;
}

int main() {
    unsigned char headerID[HEADER_SIZE], headerIM[HEADER_SIZE], headerM[HEADER_SIZE];
    unsigned char *dataID = nullptr, *dataIM = nullptr, *dataXOR = nullptr, *mascara = nullptr;
    int sizeID = 0, sizeIM = 0, sizeMascara = 0;

    // Cargar imágenes
    if (!cargarBMP("ID.bmp", headerID, dataID, sizeID)) {
        cout << "Error al cargar ID.bmp" << endl;
        return 1;
    }

    if (!cargarBMP("IM.bmp", headerIM, dataIM, sizeIM)) {
        cout << "Error al cargar IM.bmp" << endl;
        delete[] dataID;
        return 1;
    }

    if (!cargarBMP("M.bmp", headerM, mascara, sizeMascara)) {
        cout << "Error al cargar M.bmp" << endl;
        delete[] dataID;
        delete[] dataIM;
        return 1;
    }

    if (sizeID != sizeIM) {
        cout << "Las imágenes ID y IM no tienen el mismo tamaño." << endl;
            delete[] dataID;
        delete[] dataIM;
        delete[] mascara;
        return 1;
    }

    // Aplicar XOR
    dataXOR = new unsigned char[sizeID];
    aplicarXOR(dataID, dataIM, dataXOR, sizeID);
    guardarBMP("resultado.bmp", headerID, dataXOR, sizeID);

    // Verificar enmascaramiento con el archivo .txt
    int desplazamiento = 100; // puedes cambiarlo o extraerlo del archivo si prefieres
    if (verificarEnmascaramiento(dataXOR, mascara, "M1.txt", desplazamiento, sizeMascara)) {
        cout << "✅ Enmascaramiento coincide con M1.txt" << endl;
    } else {
        cout << "❌ Enmascaramiento NO coincide con M1.txt" << endl;
    }

    // Limpiar memoria
    delete[] dataID;
    delete[] dataIM;
    delete[] dataXOR;
    delete[] mascara;

    return 0;
}
