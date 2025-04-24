#ifndef FUNCIONES_H
#define FUNCIONES_H

bool cargarBMP(const char* ruta, unsigned char* header, unsigned char*& data, int& dataSize);
bool guardarBMP(const char* ruta, unsigned char* header, unsigned char* data, int dataSize);

void aplicarXOR(unsigned char* img1, unsigned char* img2, unsigned char* salida, int size);
void aplicarRotacion(unsigned char* imagen, unsigned char* resultado, int size, int bits, bool derecha);
void aplicarDesplazamiento(unsigned char* imagen, unsigned char* resultado, int size, int bits, bool derecha);

bool verificarEnmascaramiento(unsigned char* imagen, unsigned char* mascara, const char* archivoTxt, int desplazamiento, int tamMascara);
bool probarTransformaciones(unsigned char* base, unsigned char* mascara, unsigned char* im, int size, int tamMascara, const char* archivoTxt, int desplazamiento, int* operaciones, int* parametros, int numOps);
void guardarTransformacion(const char* nombreArchivo, int* secuencia, int largo);
void probarYGuardarTransformaciones(
    unsigned char* base,
    unsigned char* mascara,
    unsigned char* im,
    int size,
    int tamMascara,
    const char* archivoTxt,
    int desplazamiento
    );
void reconstruirImagen(
    unsigned char* imagenFinal,
    unsigned char* im,
    int size,
    const char* archivoOrden,
    const char* archivoSalida,
    const char* nombreHeader
    );
bool archivoExiste(const char* nombre);

#endif // FUNCIONES_H
