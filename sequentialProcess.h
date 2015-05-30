/* Project: SimLIT
 * Version: 1.2
 * File: sequentialProcess.h
 * Last modified: 24/09/2014

 * Author: Eric Orellana-Romero
 * Mail: e.orellanaromero@gmail.com
 * 
 * Description: Simula los procesos asociados la transferencia de imágenes sobre WVSN sin considerar entrelazamiento del bitmap. Se calculan metricas
 *              de comparación entre la imágen recostruida y la original, se registran estos datos agregandolos a ficheros de resultados.
 */

#ifndef __SEQUENTIALPROCESS_H_
#define __SEQUENTIALPROCESS_H_

/* NAME FILES
 */
#define RECEIVED_SEQ_ID ".received.sequential"
#define CONCEALMENT_SEQ_ID ".concealment.sequential"

/* Include files
 */
#include <time.h>
#include <stdio.h>

#include "functions.h"
#include "nodeFunctions.h"
#include "channelFunctions.h"
#include "errorConcealmentFunctions.h"
#include "evaluationMetricsFunctions.h"

void sequentialProcess(int argc, char *argv[])
{
    /* Flag user options vector
     * Las banderas asociadas a cada flag se encuentran definidas en el archivo funtions.h
     */
    bool userOptions[TOTAL_OPTIONS]; // true: user modify, false: default

    /* User input options
     */
    setUserOptions(argc, argv, userOptions);

    /* Set defaults parameters
     */
    setDefaultParameters(userOptions);

    /* Capture image, source node
     */
    FILE *capture_image;
    unsigned int width, height;

    /* Obtención ruta y nombre de imagen capturada
     */
    char *capture_image_path;

    capture_image_path = (char *)malloc((strlen(work_path)+strlen(image_name)+2)*sizeof(char));
    for(int index = 0; index < (strlen(work_path)+strlen(image_name)+2); index++) capture_image_path[index] = '\0';
    strcat(capture_image_path, work_path);
    strcat(capture_image_path, "/");
    strcat(capture_image_path, image_name);

    if((capture_image = captureImage_sourceNode(capture_image_path)) == NULL)
    {
        fprintf(stderr, "simlit: Error, source node do not capture image \"%s\" in folder \"%s\"\n", image_name, work_path);
        exit(EXIT_FAILURE);
    }

    getImageDimensions(capture_image, &height, &width);

    /* Extracción del bitmap de la imagen
     */
    Pixel *bitmap;

    bitmap = (Pixel *)malloc((height*width)*sizeof(Pixel));
    extractBitmap(capture_image, bitmap, height, width);

    /* Simulación de transmisión mediante WVSN
     */
    char *lossfiles_path;

    lossfiles_path = (char*)malloc((strlen(lossfiles_folder)+strlen(loss_file)+2)*sizeof(char));
    for(int n_index = 0; n_index < (strlen(lossfiles_folder)+strlen(loss_file)+2); n_index++) lossfiles_path[n_index] = '\0';
    strcat(lossfiles_path, lossfiles_folder);
    strcat(lossfiles_path, "/");
    strcat(lossfiles_path, loss_file);

	bool statusTransmition;
	statusTransmition = transmitImageOnWVSN(bitmap, lossfiles_path, height, width, height_block, width_block, user_bpp, size_package, SequentialTransmition);
	
    if(!statusTransmition)
    {
        fprintf(stderr, "simlit: Error to transmit image, invalid \'%s\' pattern loss file\n", lossfiles_path);
        exit(EXIT_FAILURE);
    }

    if(userOptions[15])
    {
        /* Creación de imagen secuencial recibida
         */
        if(!createInterImage_AfterTransmition(storage_folder, image_name, loss_file, (char *)RECEIVED_SEQ_ID, capture_image, bitmap, height, width))
            exit(EXIT_FAILURE);
    }

    /* Aplicación del algoritmo de ocultamiento de errores
     */
    Pixel *concealment_bitmap;

    concealment_bitmap = errorConcealment(bitmap, height, width, Avg8Connected);

    if(userOptions[16])
    {
        /* Creación de imagen secuencial reconstruida
         */
        if(!createInterImage_AfterTransmition(storage_folder, image_name, loss_file, (char *)CONCEALMENT_SEQ_ID, capture_image, concealment_bitmap, height, width))
            exit(EXIT_FAILURE);
    }

    /* Aplicación métrica de comparación
     */
    FILE *summary_text_file, *summary_data_file;
    double psnr_mix, *wrnpr_mix;
    time_t time_pt = time(NULL);
    struct tm *time_now = localtime(&time_pt);

    psnr_mix = evaluateBitmapS(bitmap, concealment_bitmap, height, width, PSNR);
    wrnpr_mix = (double *)malloc(u_wrnpr*sizeof(double));
    for(int u = 1; u <= u_wrnpr; u++)
        evaluateBitmap(bitmap, height, width, height_block, width_block, &u, 1, WRNPR, &wrnpr_mix[u-1]); 

    /* Generación de ficheros de informe
     */
    char *summary_text_path, *summary_data_path;

    summary_text_path = (char*)malloc((strlen(storage_folder)+strlen(summary_text_name)+2)*sizeof(char));
    for(int n_index = 0; n_index < (strlen(storage_folder)+strlen(summary_text_name)+2); n_index++) summary_text_path[n_index] = '\0';
    strcat(summary_text_path, storage_folder);
    strcat(summary_text_path, "/");
    strcat(summary_text_path, summary_text_name);

    summary_data_path = (char*)malloc((strlen(storage_folder)+strlen(summary_data_name)+2)*sizeof(char));
    for(int n_index = 0; n_index < (strlen(storage_folder)+strlen(summary_data_name)+2); n_index++) summary_data_path[n_index] = '\0';
    strcat(summary_data_path, storage_folder);
    strcat(summary_data_path, "/");
    strcat(summary_data_path, summary_data_name);

    /* fichero texto
     */
    if((summary_text_file = fopen(summary_text_path, "a+")) == NULL)
    {
        fprintf(stderr, "simlit: Error, impossible open or create \"%s\" summary text file\n", summary_text_path);
        exit(EXIT_FAILURE);
    }
    fprintf(summary_text_file, "%0.2d/%0.2d/%0.4d %0.2d:%0.2d:%0.2d %s %s (LossRate: %0.2f%) Seq.Metrics PSNR: %0.3f dB", time_now->tm_mday,
            time_now->tm_mon+1, time_now->tm_year+1900, time_now->tm_hour, time_now->tm_min, time_now->tm_sec, image_name, loss_file,
            getLossRate(lossfiles_path)*100, psnr_mix);
    for(int u = 1; u <= u_wrnpr; u++) fprintf(summary_text_file, " WRNPR_U%d: %0.3f", u, wrnpr_mix[u-1]);
    fprintf(summary_text_file, "\n");

    fclose(summary_text_file);

    /* fichero de datos
     */
    if((summary_data_file = fopen(summary_data_path, "a+")) == NULL)
    {
        fprintf(stderr, "simlit: Error, impossible open or create \"%s\" summary data file\n", summary_data_path);
        exit(EXIT_FAILURE);
    }
    fprintf(summary_data_file, "%0.2d %0.2d %0.4d %0.2d %0.2d %0.2d %0.2f %0.3f", time_now->tm_mday, time_now->tm_mon+1, time_now->tm_year+1900,
            time_now->tm_hour, time_now->tm_min, time_now->tm_sec, getLossRate(lossfiles_path)*100, psnr_mix);
    for(unsigned int u = 1; u <= u_wrnpr; u++) fprintf(summary_data_file, " %0.3f", wrnpr_mix[u-1]);
    fprintf(summary_data_file, "\n");

    free(lossfiles_path);
	free(wrnpr_mix);
    fclose(summary_data_file);

    /* Liberación de memoria
     */
    fclose(capture_image);
    free(bitmap);
    free(concealment_bitmap);
    free(capture_image_path);
    free(summary_text_path);
    free(summary_data_path);

    /* Liberación de memoria global
     */
    freeUsedMemory();
}

#endif //__SEQUENTIALPROCESS_H_
