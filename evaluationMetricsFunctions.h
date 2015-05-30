/* Project: SimLIT
 * Version: 1.2
 * File: evaluationMetricsFunctions.h
 * Last modified: 24/09/2014

 * Author: Eric Orellana-Romero
 * Mail: e.orellanaromero@gmail.com
 * 
 * Description: Archivo en el que se incluyen métricas de evaluación de calidad de imágenes, se emplean para cuantificar la calidad de la imagen recostruida,
 *              luego del proceso de transferencia, y la original.
 */

#ifndef __EVALUATIONMETRICSFUNCTIONS_H_
#define __EVALUATIONMETRICSFUNCTIONS_H_

#include <math.h>
#include "nodeFunctions.h"

int maxIntensity(Pixel *bitmap, unsigned int height, unsigned int width)
{
    /* Calcula el valor máximo de intensidad en el bitmap ingresado
     */
    int max;

    max = bitmap[0].intensity;
    for(unsigned int i = 1; i < (height*width); i++)
        if(bitmap[i].intensity > max)
            max = bitmap[i].intensity;

    return max;
}

double calculatePSNR(Pixel *original_bitmap, Pixel *modify_bitmap, unsigned int height, unsigned int width)
{
    /* Función que calcula la métrica PSNR entre dos bitmaps
     */
    double mse, psnr;

    mse = 0;
    for(unsigned int i = 0; i < (height*width); i++)
        mse += pow((double)original_bitmap[i].intensity - (double)modify_bitmap[i].intensity, 2);

    mse = mse/((double)(height*width));
    psnr = 10.0 * log10(pow((double)maxIntensity(original_bitmap, height, width), 2) / mse);

    return psnr;
}

double calculateWRNPR(Pixel *bitmap, unsigned int height, unsigned int width, unsigned int height_block, unsigned int width_block, unsigned int u)
{
    int well_received, lpr, lpc, loss_pixels;
    double wrnpr;

	wrnpr = 0.0; loss_pixels = 0;
	
    for(int r = 0; r < height; r += height_block)
		for(int c = 0; c < width; c += width_block)
		{
            if(!bitmap[c + r*width].lossPixelFlag)
            {
		        well_received = 0; loss_pixels++;
		        for(int i = max(r - u*height_block, 0); i <= min(r + u*height_block, height - height_block); i += height_block)
			        for(int j = max(c - u*width_block, 0); j <= min(c + u*width_block, width - width_block); j += width_block)
				        if(!((i == r) && (j == c)))
				            if(bitmap[j + i*width].lossPixelFlag)
				                well_received++;
		                
   				lpr = (min(r + u*height_block, height - height_block) - max(r - u*height_block, 0))/height_block + 1;
				lpc = (min(c + u*width_block, width - width_block) - max(c - u*width_block,0))/width_block + 1;

				wrnpr += (double)well_received/(double)(lpr*lpc - 1);
		    }
		}

    if(!u) return -1; // u es igual a 0, valor no válido
	if(!loss_pixels) return -2; // no existen píxeles perdidos
	return wrnpr/(double)loss_pixels;
}

typedef enum {PSNR, WRNPR} EvaluationBitmapMetric;

double evaluateBitmapS(Pixel *original_bitmap, Pixel *modify_bitmap, unsigned int height, unsigned int width, EvaluationBitmapMetric method)
{
    /* Función que ejecuta métricas de calidad a bitmaps
     */
    double metric = 0;

    if(method == PSNR)
        metric = calculatePSNR(original_bitmap, modify_bitmap, height, width);
     
    return metric;
}

bool evaluateBitmap(Pixel *bitmap, unsigned int height, unsigned int width, unsigned int height_block, unsigned int width_block, int *parameters, 
                    unsigned int totalParameters, EvaluationBitmapMetric method, double *metric)
{
    /* Función que ejecuta métricas de calidad a bitmaps
     */

    if(method == WRNPR)
    {
        if(totalParameters != 1) return false;
        *metric = calculateWRNPR(bitmap, height, width, height_block, width_block, *parameters);
    }

    return true;
}

#endif // __EVALUATIONMETRICSFUNCTIONS_H_
