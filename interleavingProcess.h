/* Project: SimLIT
 * Version: 1.2
 * File: interleavingProcess.h
 * Last modified: 24/09/2014

 * Author: Eric Orellana-Romero
 * Mail: e.orellanaromero@gmail.com
 * 
 * Description: Simula los procesos asociados la transferencia de imágenes sobre WVSN empleando alguna técnica de entrelazamiento. Se considera el cálculo de 
 *              metricas de comparación entre la imágen recostruida y la original, se registra estos datos agregandolos a ficheros de resultados.
 */

#ifndef __INTERLEAVINGPROCESS_H_
#define __INTERLEAVINGPROCESS_H_

/* NAME FILES
 */
#define INTERLEAVING_ID ".interleaving"
#define RECEIVED_ID ".received.interleaving"
#define DEINTERLEAVING_ID ".deinterleaving"
#define CONCEALMENT_ID ".concealment.interleaving"

/* Include files
 */
#include <stdio.h>
#include <time.h>

#include "functions.h"
#include "nodeFunctions.h"
#include "channelFunctions.h"
#include "errorConcealmentFunctions.h"
#include "evaluationMetricsFunctions.h"

void interleavingProcess(int argc, char *argv[])
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
        fprintf(stderr, "simlit: Error, source node do not capture image \"%s\"\n", image_name);
        exit(EXIT_FAILURE);
    }

    getImageDimensions(capture_image, &height, &width);

    /* Extracción del bitmap de la imagen
     */
    Pixel *bitmap;

    bitmap = (Pixel *)malloc((height*width)*sizeof(Pixel));
    extractBitmap(capture_image, bitmap, height, width);

    /* Entrelazamiento del bitmap
     */
    int status = -30;
    if(userOptions[8]) // torus mixer
    {
        int parameters[2];

        parameters[0] = k; parameters[1] = n;
        status = interleavingImage(bitmap, height, width, height_block, width_block, size_package, user_bpp, TorusAutomorphism, parameters, 2);
    }

    if(userOptions[9]) // turner mixer
    {
        int parameters[2];

        parameters[0] = byteOffset; parameters[1] = packetOffset;
        status = interleavingImage(bitmap, height, width, height_block, width_block, size_package, user_bpp, TurnerAndPeterson, parameters, 2);
    }

    if(userOptions[10]) // dsjal mixer
    {
        int parameters;

        parameters = step;
        status = interleavingImage(bitmap, height, width, height_block, width_block, size_package, user_bpp, DSJAL, &parameters, 1);
    }
	
	/* Detección de errores durante el procesamiento del entrelazado
	 */
	if(status == -1)
	{
		fprintf(stderr, "simlit: Error, no supported interleaving method\n");
        exit(EXIT_FAILURE);
	}
	if(status == 1)
    {
    	fprintf(stderr, "simlit: Error, invalid amount of input interleaving parameters\n");
        exit(EXIT_FAILURE);
    }
    if(status == 2)
    {
    	fprintf(stderr, "simlit: Error, input bitmap or block interleaving non square\n");
        exit(EXIT_FAILURE);
    }
    if(status == 3)
    {
    	fprintf(stderr, "simlit: Error, block size is not multiple of input image dimensions\n");
        exit(EXIT_FAILURE);
    }
    if(status == 4)
    {
    	fprintf(stderr, "simlit: Error, block size is bigger that input image dimensions\n");
        exit(EXIT_FAILURE);
    }
    if(status == 5)
    {
      	fprintf(stderr, "simlit: Error, length of data packets is insufficient\n");
        exit(EXIT_FAILURE);
    }
    if(status == -30)
    {
        fprintf(stderr, "simlit: Error, no selected interleaving method\n");
        exit(EXIT_FAILURE);
    }
    
	/* Creación de imagen entrelazada
     */
	if(userOptions[11])
    {
        if(!createInterImage(storage_folder, image_name, (char *)INTERLEAVING_ID, capture_image, bitmap, height, width))
            exit(EXIT_FAILURE);
    }

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

    if(userOptions[12])
    {
        /* Creación de imagen entrelazada recibida
         */
        if(!createInterImage_AfterTransmition(storage_folder, image_name, loss_file, (char *)RECEIVED_ID, capture_image, bitmap, height, width))
            exit(EXIT_FAILURE);
    }

    /* Desentrelazamiento del bitmap, nodo destino
     */
    deinterleavingImage(bitmap, height, width, SequentialDeinterleaving);

    if(userOptions[13])
    {
        /* Creación de imagen desentrelazada
         */
        if(!createInterImage_AfterTransmition(storage_folder, image_name, loss_file, (char *)DEINTERLEAVING_ID, capture_image, bitmap, height, width))
            exit(EXIT_FAILURE);
    }

    /* Aplicación del algoritmo de ocultamiento de errores
     */
    Pixel *concealment_bitmap;

    concealment_bitmap = errorConcealment(bitmap, height, width, Avg8Connected);

    if(userOptions[14])
    {
        /* Creación de imagen reconstruida
         */
        if(!createInterImage_AfterTransmition(storage_folder, image_name, loss_file, (char *)CONCEALMENT_ID, capture_image, concealment_bitmap, height, width))
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
    fprintf(summary_text_file, "%0.2d/%0.2d/%0.4d %0.2d:%0.2d:%0.2d %s %s (LossRate: %0.2f%) Int.Metrics PSNR: %0.3f dB", time_now->tm_mday,
            time_now->tm_mon+1, time_now->tm_year+1900, time_now->tm_hour, time_now->tm_min, time_now->tm_sec, image_name, loss_file,
            getLossRate(lossfiles_path)*100, psnr_mix);
    for(unsigned int u = 1; u <= u_wrnpr; u++) fprintf(summary_text_file, " WRNPR_U%d: %0.3f", u, wrnpr_mix[u-1]);
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
    for(int u = 1; u <= u_wrnpr; u++) fprintf(summary_data_file, " %0.3f", wrnpr_mix[u-1]);
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

#endif //__INTERLEAVINGPROCESS_H_
