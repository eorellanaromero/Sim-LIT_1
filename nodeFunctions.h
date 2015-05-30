/* Project: SimLIT
 * Version: 1.2
 * File: nodeFunctions.h
 * Last modified: 24/09/2014

 * Author: Eric Orellana-Romero
 * Mail: e.orellanaromero@gmail.com
 * 
 * Description: Archivo en el que se incluyen funciones referentes a los procesos realizados por nodos que componen una WSN.
 */

#ifndef __NODE_FUNCTIONS_H_
#define __NODE_FUNCTIONS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOSS_PIXEL 0xFF
#define INIT_PIXEL 0x00
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#ifndef __PIXEL_STRUCT__
#define __PIXEL_STRUCT__

typedef struct
{
    unsigned int mixerHeightPosition;
    unsigned int mixerWidthPosition;
    int          lossPixelFlag;
    unsigned int intensity;
} Pixel;

#endif //__PIXEL_STRUCT__

/* General Functions
 */

bool isValidImage(FILE *imagePtr)
{
    /* Indica si el formato de la imagen es soportado. Actualmente soporta imágenes bmp.
     * Retorna true si es soportado y false si no lo es.
     *
     * El parámetro imagePtr debe referenciar a un fichero que contenga una imagen. Es responsabilidad del usuario la
     * apertura de la imagen.
     */

    int i;
    char id[3];
    long offset = 0x0L; // Identifier position

    for (i = 0; offset < 0x2L; i++, offset++)
    {
         fseek(imagePtr, offset, SEEK_SET); // Ajusta posicion del fichero.
         id[i] = getc(imagePtr);
    }
    id[2] = (char)NULL;

    if(!strcmp(id,"BM")) return true;
    else return false;
}

void getImageDimensions(FILE *imagePtr, unsigned int *height, unsigned int *width)
{
    /* Extrae desde el encabezado de la imagen referenciada por el puntero imagePtr las dimensiones del bitmap de la misma. La
     * apertura de la imagen es responsabilidad del usuario.
     *
     * El resultado es entregado al usuario por referencia mediante los punteros height y width.
     */

    // extracción ancho de la imagen
    long offset = 0x12L; // Width position

    fseek(imagePtr, offset, SEEK_SET); // Ajusta posicion del fichero.
    fread(width, 4, 1, imagePtr); // 4 bytes lenght

    // extracción alto de la imagen
    offset = 0x16L; // Height position

    fseek(imagePtr, offset, SEEK_SET);// Ajusta posición del fichero.
    fread(height, 4, 1, imagePtr);
}

void getImageBpp(FILE *imagePtr, unsigned int *bpp)
{
    /* Extrae desde el encabezado de la imagen referenciada por el puntero imagePtr la codificación utilizada para los píxeles
     * de la misma. La apertura de la imagen es responsabilidad del usuario.
     *
     * El resultado es entregado al usuario por referencia mediante el puntero bpp.
     */

    long offset = 0x1CL;// Bpp position

    fseek(imagePtr, offset, SEEK_SET); // Ajusta posición del fichero.
    fread(bpp, 2, 1, imagePtr);
}

void getOffsetBitmap(FILE *imagePtr, unsigned int *offsetBitmap)
{
   /* Extrae desde el encabezado de la imagen referenciada por el puntero imagePtr el offset, en bytes, necesario para acceder
    * al bitmap de la misma. La apertura de la imagen es responsabilidad del usuario.
    *
    * El resultado es entregado al usuario por referencia mediante el puntero offsetBitmap.
    */

   long offset = 0xAL; // Offsetdata position

   fseek(imagePtr, offset, SEEK_SET); // Ajusta posición del fichero.
   fread(offsetBitmap, 4, 1, imagePtr);
}

void cpyBmpHeader(FILE *orgImage, FILE *dstImage)
{
    /* Copia el encabezado de la imagen apuntada por orgImage en el fichero apuntado por dstImage.
     *
     * La apertura de las imágenes es responsabilidad del usuario.
     */

    unsigned int offset;

    getOffsetBitmap(orgImage, &offset);
    rewind(orgImage);
    for(int i = 0; i < offset; i++)
        if((fputc(fgetc(orgImage), dstImage)) == EOF)
        {
            fprintf(stderr,"simlit: Error to write header image\n");
            exit(EXIT_FAILURE);
        }
}

unsigned int extractBitmap(FILE *image, Pixel *bitmap, unsigned int height, unsigned int width)
{
    /* Extrae el bitmap de la imagen apuntada por image y lo almacena en el vector bitmap.
     * Almacena en el vector bitmap los datos extraidos, la responsabilidad de asignar la memoria necesaria es del usuario.
     * Retorna el número de elementos extraidos.
     */

    unsigned int offset, i;

    getOffsetBitmap(image, &offset);

    fseek(image, offset, SEEK_SET); // Ajusta posición del fichero.
    for(i = 0; i < (height*width); i++)
    {
        /* inicialización bitmap
         */
        bitmap[i].mixerHeightPosition = i/height;
        bitmap[i].mixerWidthPosition = i%height;
        bitmap[i].lossPixelFlag = 1;

        /* asignación de intensidad al pixel
         */
        bitmap[i].intensity = fgetc(image);
    }

    return i;
}

unsigned int writeBitmap(FILE *image, Pixel *bitmap, unsigned int height, unsigned int width)
{
    /* Escribe el bitmap en la imagen apuntada por image.
     * Retorna el número de elementos escritos.
     */

    unsigned int h_mix, w_mix;
    unsigned int offset, i;

    getOffsetBitmap(image, &offset);

    rewind(image);
    fseek(image, offset, SEEK_SET); // Ajusta posición del fichero.
    for(i = 0; i < (height*width); i++)
    {
        h_mix = bitmap[i].mixerHeightPosition;
        w_mix = bitmap[i].mixerWidthPosition;

        if(bitmap[w_mix + h_mix*width].lossPixelFlag == 0)
            fputc(LOSS_PIXEL, image);
        if(bitmap[w_mix + h_mix*width].lossPixelFlag == 1)
            fputc(bitmap[w_mix + h_mix*width].intensity, image);
    }

    return i;
}

Pixel* cpyBitmap(Pixel *original_bitmap, unsigned int height, unsigned int width)
{
    /* Copia la información del vector original_bitmap a cada posición de cpy_bitmap, retorna un puntero al arreglo
     */

    Pixel *cpy_bitmap;

    cpy_bitmap = (Pixel*)malloc((height*width)*sizeof(Pixel));
    for(unsigned int i = 0; i < (height*width); i++)
    {
        cpy_bitmap[i].intensity = original_bitmap[i].intensity;
        cpy_bitmap[i].lossPixelFlag = original_bitmap[i].lossPixelFlag;
        cpy_bitmap[i].mixerHeightPosition = original_bitmap[i].mixerHeightPosition;
        cpy_bitmap[i].mixerWidthPosition = original_bitmap[i].mixerWidthPosition;
    }

    return cpy_bitmap;
}

bool createImage(char *name, FILE *header, Pixel *bitmap, unsigned int height, unsigned int width)
{
    /* Escribe una imagen en un fichero con el nombre apuntado por name.
     * Emplea una imagen previamente creada para extraer y escribir el encabezado (bmp) y los datos almacenados en el bitmap.
     *
     * Retorna false si existen problemas para escribir el fichero o true si crea correctamente.
     */
    FILE *image;

//    if(fopen(name, "r") != NULL) // se verifica existencia del archivo
//    {
//        fprintf(stderr, "simlit: Error, existing image \'%s\', image not overwritten\n", name);
//        exit(EXIT_FAILURE);
//    }

    if((image = fopen(name, "w+")) == NULL) return false;

    cpyBmpHeader(header, image);
    writeBitmap(image, bitmap, height, width);
    fclose(image);

    return true;
}

bool createInterImage(char *storage_folder, char *image_name, char* sub_process_name, FILE *capture_image, Pixel *bitmap, unsigned int height, unsigned int width)
{
    /* Crea imágenes resultantes luego de un subproceso. Incluye en el nombre de la misma el subproceso realizados
     */

    char *name;

    name = (char*)malloc((strlen(storage_folder)+strlen(image_name)+strlen(sub_process_name)+2)*sizeof(char));
    for(int n_index = 0; n_index < (strlen(storage_folder)+strlen(image_name)+strlen(sub_process_name)+2); n_index++) name[n_index] = '\0';
    strcat(name, storage_folder);
    strcat(name, "/");
    for(int n_index = (strlen(storage_folder)+1), index = 0; index < (strlen(image_name)-4); n_index++, index++)
        name[n_index] = image_name[index]; // se excluye la extensión ".bmp"
    strcat(name, sub_process_name);
    strcat(name, ".bmp");

    if(!createImage(name, capture_image, bitmap, height, width))
    {
        fprintf(stderr, "simlit: Error, impossible create image \"%s\"\n", name);
        if(strlen(name) > 256) fprintf(stderr, "simlit: Error, to match characters in name (%d)\n", strlen(name));
        return false;
    }

    free(name);

    return true;
}

bool createInterImage_AfterTransmition(
        char *storage_folder, char *image_name, char* lossfile_name, char* sub_process_name, FILE *capture_image, Pixel *bitmap, unsigned int height, unsigned int width)
{
    char* lossfile_subprocess_name;
    bool status;

    lossfile_subprocess_name = (char *)malloc((strlen(lossfile_name)+strlen(sub_process_name)-3)*sizeof(char));
    for(int index = 0; index < (strlen(lossfile_name)+strlen(sub_process_name)-3); index++) lossfile_subprocess_name[index] = '\0';
    lossfile_subprocess_name[0] = '.';
    for(int index = 1; index < (strlen(lossfile_name)-4); index++) lossfile_subprocess_name[index] = lossfile_name[index-1];
    strcat(lossfile_subprocess_name, sub_process_name);

    status = createInterImage(storage_folder, image_name, lossfile_subprocess_name, capture_image, bitmap, height, width);

    free(lossfile_subprocess_name);
    return status;
}

/* Source Node Functions
 */

FILE* captureImage_sourceNode(char *imageName)
{
    /* Apertura de imagen y extracción de parámetros para simulación.
     *
     * El parámetro imageName contiene el nombre y ubicación del fichero que contiene a la imagen. La asignación de memoria
     * y establecimiento del nombre es responsabilidad del usuario.
     *
     * Retorna un puntero al fichero de la imagen o 0 en caso de error.
     */
    FILE *image;

    if((image = fopen(imageName,"rb")) == NULL) return NULL;
    if(!isValidImage(image)) return NULL;

    return image;
}

/* Interleaving Methods
 */

void torusMixer(unsigned int k, unsigned int n, unsigned int size, unsigned int *indexH, unsigned int *indexW)
{
    /* Genera las posiciones entrelazadas para los píxeles de un bitmap utilizando para ello el algoritmo de mezcla Torus Automorphisms.
     * Retorna mediante los punteros indexH e indexW las posiciones entrelazadas para cada píxel del bitmap.
     */

    unsigned int M[2][2], TorusMatrice[2][2];

    // Initialisation de la matrice et autres variables
    TorusMatrice[0][0] = 1;
    TorusMatrice[0][1] = 1;
    TorusMatrice[1][0] = k;
    TorusMatrice[1][1] = k+1;

    // On boucle
    for(int count=1; count < n; count++)
    {
        M[0][0]=(TorusMatrice[0][0]+TorusMatrice[0][1]*k)%size;
        M[0][1]=(TorusMatrice[0][0]+TorusMatrice[0][1]*(k+1))%size;
        M[1][0]=(TorusMatrice[1][0]+TorusMatrice[1][1]*k)%size;
        M[1][1]=(TorusMatrice[1][0]+TorusMatrice[1][1]*(k+1))%size;

        TorusMatrice[0][0] = M[0][0];
        TorusMatrice[0][1] = M[0][1];
        TorusMatrice[1][0] = M[1][0];
        TorusMatrice[1][1] = M[1][1];
    }

    int index = 0;
    if(n)
    {
        for(int i = 0; i < size; i++)
            for(int j = 0; j < size; j++)
               {
                    int ip = (TorusMatrice[0][0]*i + TorusMatrice[0][1]*j)%size;
                    int jp = (TorusMatrice[1][0]*i + TorusMatrice[1][1]*j)%size;

                    indexH[index] = ip;
                    indexW[index] = jp;

                    index++;
                }
    }
    else
    {
        for(int i = 0; i < size; i++)
            for(int j = 0; j < size; j++)
            {
                indexH[index] = i;
                indexW[index] = j;
                index++;
            }
    }
}

void turnerMixer(unsigned int byteOffset, unsigned int packetOffset, unsigned int height, unsigned int width, unsigned int payload,
                 unsigned int *indexH, unsigned int *indexW)
{
    /* Pi = {I(j), I(j+(1*ByteOffset)), I(j+(2*ByteOffset)), ..., I(j+((m-1)*ByteOffset))}
     *  j = i*PacketOffset
     *
     * Considerar el espacio disponible en un paquete al utilizar bloques, corresponde a (payload*8)/(height_block*width_block*bpp), donde bpp corresponde a
     * la cantidad de bits empleados para representar un píxel de la imagen.
     */

    for(int id = 0; id < (height*width); id++)
    {            
        unsigned int i, j, m, pixelId;
            
        m = id%payload;
        i = id/payload; // i: número del paquete actual
        j = i*packetOffset; 
            
        pixelId = (j + m*byteOffset)%(height*width); // se acotan los resultados a la longitud del bitmap
            
        indexH[id] = pixelId/height;
        indexW[id] = pixelId%height;
    }
}

void dsjalMixer(unsigned int step, unsigned int height, unsigned int width, unsigned int *indexH, unsigned int *indexW)
{
    unsigned int i, j, k;
    unsigned int _h_, _w_; // referencias de coordenadas al alcanzar la máxima longitud de la imagen

    indexH[0] = 0; _h_ = indexH[0];
    indexW[0] = 0; _w_ = indexW[0];

    for(i = 0, k = 0; i < height; i++)
        for(j = 0; j < width; j++, k++)
        {
            indexW[k+1] = (indexW[k]+step)%width;
            indexH[k+1] = indexH[k] + (int)((indexW[k]+step)/width);

            /* si Yi+1 (index_h[k+1]) alcanza el máximo de filas, las próximas coordenadas será la posición siguiente al primer píxel guardado,y
             * así sucesivamente.
             */
            if(indexH[k+1] >= height)
            {
                indexH[k+1] =  _h_ + (int)((_w_)/(width-1));    // x
                _h_ = indexH[k+1];

                indexW[k+1] = (_w_+1)%width; // y
                _w_ = indexW[k+1];
            }
        }

    indexH[0] = 0; // al finalizar indexW se hace 128 (en imágenes de 128x128)
    indexW[0] = 0; // por lo que se sobreescribe con valores iniciales.
}

typedef enum {TorusAutomorphism, TurnerAndPeterson, DSJAL} Mixers;

int interleavingImage(Pixel *bitmap, unsigned int height, unsigned int width, unsigned int height_block, unsigned int width_block,
                      unsigned int payload, float user_bpp, Mixers mixerMethod, int *parameters, int totalParameters)
{
    /* Simula el entrelazamiento del bitmap según el método de entrelazamiento interleavingMethod.
     * Modifica el bitmap bitmap agregando las posiciones entrelazadas de cada píxel, según el método y los parámetros.
     *
     * Retorna 1 si existe error en la cantidad de parámetros del método.
     * Retorna 2 si las dimensiones de la matriz o de los bloques ingresados son erroneas. (dimensiones cuadradas es correcto)
     * Retorna 3 si height_block no es múltiplo de height o width_block no es múltiplo de width.
     * Retorna 4 si height_block es mayor a height o width_block es mayor a width.
     * Retorna 5 si el espacio asignado a los paquetes de datos es insuficiente, válido para los métodos que los consideran.

     * Retorna -1 si el método de mezcla no es soportado.
     * Retorna 0 si finaliza correctamente.
     */

    /* Nota: ya que los métodos de entrelazamiento no consideran la utilización de bloques en la mezcla, se implementa la asignación
     * de las posiciones mezcladas a los píxeles del bitmap tomándolo en cuenta. Esto implica que al utilizar algún método de mezcla
     * se deben ingresar las coordenadas divididas por la de los bloques, de forma de generar posiciones mezcladas para cada bloque
     * en vez de cada píxel.
     */
	bool sequentialMixerFlag = false;

    unsigned int *mixerHeightPosition, *mixerWidthPosition;

    if(height != width) return 2;
    if(height_block != width_block) return 2;
    if((height % height_block) || (width % width_block)) return 3; // bloques múltiplos de sus dimensiones
    if((height_block > height) || (width_block > width)) return 4; // bloques inferiores o iguales a las dimensiones de la imagen

    if(mixerMethod == TorusAutomorphism)
    {
        if(totalParameters != 2) return 1;
        
        mixerHeightPosition = (unsigned int *)malloc((height/height_block)*(width/width_block)*sizeof(unsigned int));
        mixerWidthPosition = (unsigned int *)malloc((height/height_block)*(width/width_block)*sizeof(unsigned int));
        for(int index = 0; index < ((height/height_block)*(width/width_block)); index++)
        {
            mixerHeightPosition[index] = 0;
            mixerWidthPosition[index] = 0;
        }

        unsigned int k = parameters[0];
        unsigned int n = parameters[1];

        torusMixer(k, n, (height/height_block), mixerHeightPosition, mixerWidthPosition);
		
		sequentialMixerFlag = true;
    }

    if(mixerMethod == TurnerAndPeterson)
    {
        if(totalParameters != 2) return 1;
        if((payload*8) < (height_block*width_block*user_bpp)) return 5; // el espacio en un paquete debe contener a lo menos un píxel.

        mixerHeightPosition = (unsigned int *)malloc((height/height_block)*(width/width_block)*sizeof(unsigned int));
        mixerWidthPosition = (unsigned int *)malloc((height/height_block)*(width/width_block)*sizeof(unsigned int));
        for(int index = 0; index < ((height/height_block)*(width/width_block)); index++)
        {
            mixerHeightPosition[index] = 0;
            mixerWidthPosition[index] = 0;
        }

        unsigned int byteOffset = parameters[0];
        unsigned int packetOffset = parameters[1];

        turnerMixer(byteOffset, packetOffset, height/height_block, width/width_block, (payload*8)/(height_block*width_block*user_bpp), mixerHeightPosition,
                    mixerWidthPosition);

		sequentialMixerFlag = true;
    }
    
    if(mixerMethod == DSJAL)
    {
        if(totalParameters != 1) return 1;

        mixerHeightPosition = (unsigned int *)malloc((height/height_block)*(width/width_block)*sizeof(unsigned int));
        mixerWidthPosition = (unsigned int *)malloc((height/height_block)*(width/width_block)*sizeof(unsigned int));
        for(int index = 0; index < ((height/height_block)*(width/width_block)); index++)
        {
            mixerHeightPosition[index] = 0;
            mixerWidthPosition[index] = 0;
        }

        unsigned int step = parameters[0];

        dsjalMixer(step, height/height_block, width/width_block, mixerHeightPosition, mixerWidthPosition);

		sequentialMixerFlag = true;
    }

    if(!sequentialMixerFlag) return -1;

    /* Asignación de posiciones a los píxeles del bitmap, mediante bloques
     */
    unsigned int h_mix, w_mix;
            
    for(int i = 0, posIndex = 0; i < height; i += height_block) /**/
        for(int j = 0; j < width; j += width_block, posIndex++) /* loop para avanzar de bloque en bloque */
            for(int iBlock = max((int)i, 0), itemp = 0; iBlock < min(i+height_block, height); iBlock++, itemp++)  /**/
                for(int jBlock = max((int)j, 0), jtemp = 0; jBlock < min(j+width_block, width); jBlock++, jtemp++)/* loop para avanzar en el bloque */
                {
                    /* A partir de las posiciones entrelazadas de cada bloque se obtienen las posiciones mezcladas de cada píxel
                     * en el interior de cada uno. Las posiciones de cada píxel se generan en base a las dimensiones de la imagen
                     * original (se emplea adapted interleaving).
                     */

                    h_mix = mixerHeightPosition[posIndex]*height_block + itemp;
                    w_mix = mixerWidthPosition[posIndex]*width_block + jtemp;

                    bitmap[w_mix + h_mix*width].mixerHeightPosition = iBlock;
                    bitmap[w_mix + h_mix*width].mixerWidthPosition = jBlock;
                }
        
    /* Liberación de memoria
     */
    free(mixerWidthPosition);
    free(mixerHeightPosition);
    
    return 0;
}

/* Sink Node Functions
 */

typedef enum {SequentialDeinterleaving} DeinterleavingType;

void deinterleavingImage(Pixel *bitmap, unsigned int height, unsigned int width, DeinterleavingType deinterleavingMethod)
{
    /* Simula el reordamiento del bitmap entrelazado, no se implementa el entrelazamiento inverso, se utilizan las posiciones entrelazadas
     * asignadas a cada píxel.
     */

    Pixel tempBitmap[height*width];
    unsigned int h_mix, w_mix;

    /* Copia temporal del bitmap
     */
    for(unsigned int i = 0; i < (height*width); i++)
    {
        tempBitmap[i].intensity = bitmap[i].intensity;
        tempBitmap[i].lossPixelFlag = bitmap[i].lossPixelFlag;
        tempBitmap[i].mixerWidthPosition = bitmap[i].mixerWidthPosition;
        tempBitmap[i].mixerHeightPosition = bitmap[i].mixerHeightPosition;
    }

    /* Desentrelazamiento
     */
    for(unsigned int i = 0; i < (height*width); i++)
    {
        h_mix = tempBitmap[i].mixerHeightPosition;
        w_mix = tempBitmap[i].mixerWidthPosition;

        bitmap[w_mix + h_mix*width].lossPixelFlag = tempBitmap[i].lossPixelFlag;
        bitmap[w_mix + h_mix*width].mixerWidthPosition = tempBitmap[i].mixerWidthPosition;
        bitmap[w_mix + h_mix*width].mixerHeightPosition = tempBitmap[i].mixerHeightPosition;
    }
}

bool isLossPixels(Pixel *bitmap, unsigned int height, unsigned int width)
{
    /* Evalua la existencia de píxeles perdidos en el bitmap.
     * Retorna true si existe al menos uno marcado como perdido, false si todos están marcados como recibidos
     */

    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
            if(!bitmap[j + i*width].lossPixelFlag)
                return true;

    return false;
}

#endif // __NODE_FUNCTIONS_H_
