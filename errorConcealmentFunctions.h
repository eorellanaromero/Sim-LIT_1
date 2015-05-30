/* Project: SimLIT
 * Version: 1.2
 * File: errorConcealmentFunctions.h
 * Last modified: 24/09/2014

 * Author: Eric Orellana-Romero
 * Mail: e.orellanaromero@gmail.com
 * 
 * Description: Archivo en el que se incluyen funciones de ocultamiento de errores para imágenes con pérdidas, luego de ser transmitidas sobre WVSN.
 */

#ifndef __ERRORCONCEALMENTFUNCTIONS_H_
#define __ERRORCONCEALMENTFUNCTIONS_H_

#include "nodeFunctions.h"

typedef enum {Avg8Connected} ConcealmentMethod;

Pixel* Avg8Con(Pixel *bitmap, unsigned int height, unsigned int width)
{
    /* Reconstruye los píxeles perdidos mediante los bien recibidos. Considera como valor a asignar al píxel perdido el promedio de la intensidad
     * de los 8 vecinos en torno a él.
     *
     * La aplicación de este método es válida para una imagen desentrelazada.
     *
     * La función retorna el bitmap generado en base a la reconstrucción de píxeles faltantes.
     */

    int add, avg, count;
    unsigned int h_mix, w_mix;
    Pixel *concealment_bitmap;

    concealment_bitmap = (Pixel*)malloc((height*width)*sizeof(Pixel));

    /* Inicialización del bitmap reconstruido
     */
    for(int index = 0; index < (height*width); index++)
        concealment_bitmap[index].intensity = 0;

    /* Asignación de valores al bitmap reconstruido
     */
    for(int index = 0; index < (height*width); index++)
    {
        h_mix = bitmap[index].mixerHeightPosition;
        w_mix = bitmap[index].mixerWidthPosition;

        concealment_bitmap[index].intensity = bitmap[w_mix + h_mix*width].intensity;
        concealment_bitmap[index].lossPixelFlag = bitmap[w_mix + h_mix*width].lossPixelFlag;
        concealment_bitmap[index].mixerHeightPosition = index/height;
        concealment_bitmap[index].mixerWidthPosition = index%height;
    }

    do {
        for(int i = 0; i < height; i++)    /**/
            for(int j = 0; j < width; j++) /** Píxel evaluado */
            {
                add = 0; count = 0;
                if(!concealment_bitmap[j + i*width].lossPixelFlag) // si el píxel no es recibido
                {
                    /* cálculo del promedio de las intensidades de los vecinos (ocho conectados) bien recibidos
                     */
                    for(int iBlock = max((int)i-1,0); iBlock <= min(i+1,height-1) ; iBlock++)   /**/
                        for(int jBlock = max((int)j-1,0); jBlock <= min(j+1,width-1); jBlock++) /** 8 conectados */
                            if((i != iBlock) || (j != jBlock))
                                if(concealment_bitmap[jBlock + iBlock*width].lossPixelFlag) // si el píxel vecino es recibido
                                {
                                    add += concealment_bitmap[jBlock + iBlock*width].intensity;
                                    count++;
                                }

                    if(count) // al menos un píxel es recibido
                    {
                        avg = (int)(add/count);

                        concealment_bitmap[j + i*width].intensity = avg; // sobreescritura de la intesidad del píxel perdido con el valor calculado
                        concealment_bitmap[j + i*width].lossPixelFlag = 1; // sobreescritura del flag de pérdida, se marca como recibido
                    }
                }
            }
    } while(isLossPixels(concealment_bitmap, height, width)); /* se repite el ciclo hasta que no existan píxeles perdidos, hasta que todos sean
                                                               * reparados
                                                               */
    return concealment_bitmap;
}

Pixel* errorConcealment(Pixel *bitmap, unsigned int height, unsigned int width, ConcealmentMethod method)
{
    /* Función que reconstruye un bitmap en base a alguna método de ocultamiento de errores
     */
    Pixel *concealment_bitmap;

    if(method == Avg8Connected)
        concealment_bitmap = Avg8Con(bitmap, height, width);

    return concealment_bitmap;
}

#endif // __ERRORCONCEALMENTFUNCTIONS_H_
