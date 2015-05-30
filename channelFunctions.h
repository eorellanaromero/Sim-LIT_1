/* Project: SimLIT
 * Version: 1.2
 * File: channelFunction.h
 * Last modified: 24/09/2014

 * Author: Eric Orellana-Romero
 * Mail: e.orellanaromero@gmail.com
 * 
 * Description: Archivo en el que se incluyen las funciones relacionadas específicamente a la transferencia de imágenes, es decir,
 *              simulación de pérdidas de paquetes.
 */

#ifndef __CHANNEL_FUNCTIONS_H_
#define __CHANNEL_FUNCTIONS_H_

#include "nodeFunctions.h"

/* unsigned int width_block, height_block, size_package, k, n, byteOffset, packetOffset, step;
 * double user_bpp;
 * char *work_path, *store_folder, *image_name, *loss_file;
 */

unsigned int calculateTotalPackages(
        unsigned int width, unsigned int height, unsigned int width_block, unsigned int height_block, unsigned int sizePackage, double usrBpp)
{
    /* sizePackage es el espacio en un paquete [bytes].
     *
     * bpPackage es la cantidad de bloques por paquete.
     *
     * width_block y height_block es la dimension en pixeles de un bloque. Un bloque
     * es de (width_block x height_block) pixels.
     * pixeles.
     *
     * usrBpp indica codificación que se da a los pixels (ej 2 bpp).
     */

    // se necesita en transfer bpPackages bloques por paquete y totalPackage.

    unsigned int bpPackage;
    float temp;

    /* bloques por paquete
     *                       (espacio total del paquete)[bits]
     * bloques por paquete = ----------------------------------
     *                             (peso de un bloque)
     */

    temp = (8.0*(float)sizePackage)/(usrBpp*(float)width_block*(float)height_block);
    bpPackage = (int)temp;// bloques por paquete

    // no se pueden enviar menos de un bloque por paquete, el espacio es insuficiente para almacenar un bloque.
    if(bpPackage == 0)
    {
        fprintf(stderr,"simlit: Error, insufficient length of package\n");
        exit(EXIT_FAILURE);
    }

    // número de paquetes perdidos

    /* cálculo del total de paquetes a generar */
    // cantidad máxima de bloques en el bitmap.
    unsigned int totalBlocks = (width*height)/(width_block*height_block);
    unsigned int totalPackage; // cantidad de paquetes generados

    // si un paquete tiene una capacidad mayor a los bloques del bitmap.
    if(bpPackage > totalBlocks)
        totalPackage = 1;// basta con crear un paquete.

    else
        /* Si la cantidad de bloques por paquete no es múltiplo del  total de bloques el
         * total de paquetes será el cuociente entre el total de  bloques y  la cantidad
         * de bloques por paquete (parte entera) + uno, para  almacenar los bloques fal-
         * tantes.
         */
        if((totalBlocks%bpPackage) !=  0)
            totalPackage = (unsigned int)(totalBlocks/bpPackage) + 1;

        else
            /* si la cantidad de bloques por paquete es múltiplo del total de bloques el to-
             * tal de paquetes será el cuociente entre estos dos.
             */
            totalPackage = (unsigned int)(totalBlocks/bpPackage);

    return totalPackage;
}

int* getLossIndex(char *nameLossFile, int *totalPackages)
{
    FILE *loss_file_ptr;
    int *indexLossVector;
    int id;
    unsigned int boolIndex;

    if((loss_file_ptr = fopen(nameLossFile, "r")) == NULL)
    {
        fprintf(stderr, "simlit: Error, impossible open loss file \'%s\'\n", nameLossFile);
        exit(EXIT_FAILURE);
    }

    /* Los archivos de pérdidas se componen de varias líneas que indican el número secuencial del paquete, un espacio en blanco, y el
     * indicador de pérdidas para ese paquete. True indica que el paquete es recibido, false significa que es perdido.
     *
     * Retorna el número de elementos asignados al vector (por referencia) y el vector de índices.
     * Si existe algún error retorna 0.
     */

    indexLossVector = (int *)malloc(sizeof(int));

    id = 1;
    rewind(loss_file_ptr);
    while(!feof(loss_file_ptr))
    {
        fscanf(loss_file_ptr, "%d %d\n", &id, &boolIndex);
        indexLossVector[id-1] = (int)boolIndex;

        indexLossVector = (int *)realloc(indexLossVector, (id+1)*sizeof(int));
    }
    *totalPackages = id;

    fclose(loss_file_ptr);
    return indexLossVector;
}

double getLossRate(char *lossfile_name)
{
    FILE *lossfile;
    int n_loss, sec, index_loss;
    double avg;

    if((lossfile = fopen(lossfile_name, "r")) == NULL)
    {
        fprintf(stderr, "simlit: Error, impossible open loss file \'%s\'\n", lossfile_name);
        exit(EXIT_FAILURE);
    }

    n_loss = 0;
    rewind(lossfile);
    while(!feof(lossfile))
    {
        fscanf(lossfile, "%d %d\n", &sec, &index_loss);
        if(!index_loss) n_loss++;
    }

    avg = (double)n_loss/(double)sec;

    fclose(lossfile);
    return avg;
}

typedef enum {SequentialTransmition} TransmitionType;

bool transmitImageOnWVSN(
        Pixel *bitmap, char *nameLossFile, unsigned int height, unsigned int width, unsigned int height_block, unsigned int width_block,
        float bpp, unsigned int payload, TransmitionType transmitionMethod)
{
    /* Simula la paquetización y transmisión de los píxeles del bitmap. Considera las posiciones mezcladas de cada píxel.
     * Según esto modifica el indicador de pérdidas para cada Píxel.
     * Retorna true si concluye correctamente o false si existe algún problema. Dichos problemas pueden deberse a no poder abrir el fichero,
     * o a la cantidad de paquetes considerados en los ficheros.
     */

    unsigned int totalPackages;
    int totalPackages_file;
    int *indexLoss;

    totalPackages = calculateTotalPackages(width, height, width_block, height_block, payload, bpp);
    indexLoss = getLossIndex(nameLossFile, &totalPackages_file);

    if(totalPackages != totalPackages_file) return false;

    /* Asignación de índice de pérdidas a los píxeles del bitmap
     */	
    unsigned int h_mix, w_mix, nblock = 0;
    int blocksPerPackage = (payload*8)/(height_block*width_block*bpp);
  
  	for(int i = 0, i_loss = 0; i < height; i += height_block) /**/
       	for(int j = 0; j < width; j += width_block) /* loop para avanzar de bloque en bloque */
       	{
           	for(int iBlock = max((int)i, 0); iBlock < min(i+height_block, height); iBlock++)  /**/
           	    for(int jBlock = max((int)j, 0); jBlock < min(j+width_block, width); jBlock++)/* loop para avanzar en el bloque */
           	    {
           	        /* asignación de posiciones entrelazadas, se transmite el bitmap mezclado
           	         */
           	        h_mix = bitmap[jBlock + iBlock*width].mixerHeightPosition;
           	        w_mix = bitmap[jBlock + iBlock*width].mixerWidthPosition;

           	        bitmap[w_mix + h_mix*width].lossPixelFlag = indexLoss[i_loss];
           	    }
           	nblock++; // se ha procesado un bloque más

           	if(!(nblock%blocksPerPackage)) i_loss++;
       	}

    free(indexLoss);
    return true;
}

#endif // __CHANNEL_FUNCTIONS_H_
