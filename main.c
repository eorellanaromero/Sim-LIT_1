/* Project: SimLIT
 * Version: 1.2
 * File: main.c
 * Last modified: 24/09/2014

 * Author: Eric Orellana-Romero
 * Mail: e.orellanaromero@gmail.com
 * 
 * Description: Framework para la simulación de transferencia de imágenes sobre WVSN.
 */

#include <dirent.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>

#include "sequentialProcess.h"
#include "interleavingProcess.h"

#define SEQUENTIAL_OPT "-sequential-process"
#define INTERLEAVING_OPT "-interleaving-process"
#define C_SEP '-'
#define N_SEP 3

int main(int argc, char *argv[])
{
	timeval t_ini, t_fin;
	gettimeofday(&t_ini, NULL);

    /* falta modificar las imagenes creadas con el nombre del archivo de pérdidas para qyue no se sobreescriban y arreglar el problema de lectura de
     * archivos de pérdidas.
     */
    int argc_func, i;
    char** argv_func;
    bool process_opt[2]; // process_opt[0]: proceso secuencial, process_opt[1]: proceso entrelazado

    i = 1; argc_func = 1;
    process_opt[0] = false; process_opt[1] = false;

    /* Asignación nombre de aplicación a vector de opciones de programa
     */
    argv_func = (char **)malloc(sizeof(char *));
    argv_func[0] = (char *)malloc(strlen(argv[0])*sizeof(char));
    strcat(argv_func[0], argv[0]);

    while(i < argc)
    {
        if(i < argc)
            if(!strcmp(argv[i], SEQUENTIAL_OPT))
            {
                process_opt[0] = true;
                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], INTERLEAVING_OPT))
            {
                process_opt[1] = true;
                i++;
            }

        /* Almacenamiento de opciones de programa en nueva variable para asignarla a los procesos correspondientes
         */
        if(i < argc)
            if(strcmp(argv[i], SEQUENTIAL_OPT) && strcmp(argv[i], INTERLEAVING_OPT))
            {
                argv_func = (char **)realloc(argv_func,(argc_func+1)*sizeof(char *));
                argv_func[argc_func] = (char *)malloc((strlen(argv[i])+1)*sizeof(char));

                for(int index = 0; index < (strlen(argv[i])+1); index++) argv_func[argc_func][index] = '\0';
                strcat(argv_func[argc_func],argv[i]);

                i++; argc_func++;
            }
    }

    if(!process_opt[0] && !process_opt[1])
    {
        fprintf(stderr, "simlit: Error, no selected process type (\'%s\' or/and \'%s\')\n", SEQUENTIAL_OPT, INTERLEAVING_OPT);
        exit(EXIT_FAILURE);
    }

    /* Adquisición de banderas de opciones de programa
     */
    bool userOptions[TOTAL_OPTIONS]; // true: user modify, false: default

    setUserOptions(argc_func, argv_func, userOptions);
    setDefaultParameters(userOptions);

    /* Lectura de carpeta de trabajo
     */
    DIR *dir;
    struct dirent *ent;
    char **images_files;
    int n_images_files;

    n_images_files = 0;
    images_files = (char **)malloc(sizeof(char *));

    /* Obtención archivos de imágenes en carpeta
     */
    if(userOptions[2])
    {
        n_images_files = 1;
        images_files[0] = (char *)malloc(strlen(image_name+1)*sizeof(char));
        for(int index = 0; index < (strlen(image_name)+1); index++) images_files[0][index] = '\0';
        strcat(images_files[0], image_name);
    }
    else
    {
        if((dir = opendir(work_path)) == NULL)
        {
            fprintf(stderr, "simlit: Error, impossible open \'%s\' work folder\n", work_path);
            exit(EXIT_FAILURE);
        }
        while((ent = readdir(dir)) != NULL)
        {
            if(ent->d_type == DT_REG)
            {
                char *images_files_path;

                images_files_path = (char *)malloc((strlen(work_path)+strlen(ent->d_name)+2)*sizeof(char));
                for(int index = 0; index < (strlen(work_path)+strlen(ent->d_name)+2); index++) images_files_path[index] = '\0';
                strcat(images_files_path, work_path);
                strcat(images_files_path, "/");
                strcat(images_files_path, ent->d_name);

                FILE *image;

                if((image = fopen(images_files_path, "r")) != NULL)
                {
                    if(isValidImage(image))
                    {
                        n_images_files++;
                        images_files = (char **)realloc(images_files, n_images_files*sizeof(char *));
                        images_files[n_images_files-1] = (char *)malloc((strlen(ent->d_name)+1)*sizeof(char));
                        for(int index = 0; index < (strlen(ent->d_name)+1); index++) images_files[n_images_files-1][index] = '\0';
                        strcat(images_files[n_images_files-1], ent->d_name);
                    }

                    fclose(image);
                }

                free(images_files_path);
            }
        }

        free(ent);
        closedir(dir);

        if(!n_images_files)
        {
            fprintf(stderr, "simlit: Error, no assignable image inside \'%s\' work folder\n", work_path);
            exit(EXIT_FAILURE);
        }
    }

    /* Obtención archivos con patrones de pérdidas en carpeta (extensión .loss)
     */
    char **patternloss_files;
    int n_patternloss_files;

    n_patternloss_files = 0;
    patternloss_files = (char **)malloc(sizeof(char *));

    if(userOptions[7])
    {
        n_patternloss_files = 1;

        patternloss_files[0] = (char *)malloc(strlen(loss_file)*sizeof(char));
        for(int index = 0; index < strlen(loss_file); index++) patternloss_files[0][index] = '\0';
        strcat(patternloss_files[0], loss_file);
    }
    else
    {
        if((dir = opendir(lossfiles_folder)) == NULL)
        {
            fprintf(stderr, "simlit: Error, impossible open \'%s\' loss files folder\n", lossfiles_folder);
            exit(EXIT_FAILURE);
        }
        while((ent = readdir(dir)) != NULL)
        {
            if(ent->d_type == DT_REG)
            {
                char *temp_lossfile, extension[6];

                temp_lossfile = (char *)malloc((strlen(ent->d_name)+1)*sizeof(char));
                for(int index = 0; index < (strlen(ent->d_name)+1); index++) temp_lossfile[index] = '\0';
                strcat(temp_lossfile, ent->d_name);

                for(int index = 0; index < 6; index++) extension[index] = '\0';
                for(int index = (strlen(temp_lossfile)-5), index_ext = 0; index < strlen(temp_lossfile); index++, index_ext++)
                    extension[index_ext] = temp_lossfile[index];

                if(!strcmp(extension, ".loss"))
                {
                    n_patternloss_files++;
                    patternloss_files = (char **)realloc(patternloss_files, n_patternloss_files*sizeof(char *));

                    patternloss_files[n_patternloss_files-1] = (char *)malloc(strlen(temp_lossfile)*sizeof(char));
                    for(int index = 0; index < strlen(temp_lossfile); index++) patternloss_files[n_patternloss_files-1][index] = '\0';
                    strcat(patternloss_files[n_patternloss_files-1], temp_lossfile);
                }

                free(temp_lossfile);
            }
        }

        free(ent);
        closedir(dir);

        if(!n_patternloss_files)
        {
            fprintf(stderr, "simlit: Error, no assignable pattern lossfile inside \'%s\' lossfiles folder\n", lossfiles_folder);
            exit(EXIT_FAILURE);
        }
    }

    /* Encabezado archivo resumen de resultados
     */
    FILE *text_summary_file, *data_summary_file;
    bool txt_existenci, data_existenci;
    char *summary_text_path, *summary_data_path;

    /* Obtención ruta arhivos de resumen
     */
    summary_text_path = (char *)malloc((strlen(storage_folder)+strlen(summary_text_name)+2)*sizeof(char));
    for(int n_index = 0; n_index < (strlen(storage_folder)+strlen(summary_text_name)+2); n_index++) summary_text_path[n_index] = '\0';
    strcat(summary_text_path, storage_folder);
    strcat(summary_text_path, "/");
    strcat(summary_text_path, summary_text_name);

    summary_data_path = (char *)malloc((strlen(storage_folder)+strlen(summary_data_name)+2)*sizeof(char));
    for(int n_index = 0; n_index < (strlen(storage_folder)+strlen(summary_data_name)+2); n_index++) summary_data_path[n_index] = '\0';
    strcat(summary_data_path, storage_folder);
    strcat(summary_data_path, "/");
    strcat(summary_data_path, summary_data_name);

    /* Determinación de existencia previa de los ficheros
     */
    if((text_summary_file = fopen(summary_text_path, "r")) == NULL) txt_existenci = false;
    else { txt_existenci = true; fclose(text_summary_file); }

    if((data_summary_file = fopen(summary_data_path, "r")) == NULL) data_existenci = false;
    else { data_existenci = true; fclose(data_summary_file); }

    /* Apertura para actualización de los ficheros
     */
    if((text_summary_file = fopen(summary_text_path, "a+")) == NULL)
    {
        fprintf(stderr, "simlit: Error, impossible open or create \"%s\" summary text file\n", summary_text_path);
        exit(EXIT_FAILURE);
    }

    if((data_summary_file = fopen(summary_data_path, "a+")) == NULL)
    {
        fprintf(stderr, "simlit: Error, impossible open or create \"%s\" summary data file\n", summary_data_path);
        exit(EXIT_FAILURE);
    }

    /* Escritura encabezado
     */
    if(txt_existenci) fprintf(text_summary_file, "\n");
    if(data_existenci) fprintf(data_summary_file, "\n");

    /* Obtención de parámetros desde imagen en la carpeta de trabajo
     */
    FILE *image;
    char *capture_image_path;
    unsigned int height, width, image_bpp;

    capture_image_path = (char *)malloc((strlen(work_path)+strlen(images_files[0])+2)*sizeof(char));
    for(int index = 0; index < (strlen(work_path)+strlen(images_files[0])+2); index++) capture_image_path[index] = '\0';
    strcat(capture_image_path, work_path);
    strcat(capture_image_path, "/");
    strcat(capture_image_path, images_files[0]);

    if((image = fopen(capture_image_path, "rb")) == NULL)
    {
        fprintf(stderr, "simlit: Error, impossible open \"%s\" capture image\n", capture_image_path);
        exit(EXIT_FAILURE);
    }

    getImageDimensions(image, &height, &width);
    getImageBpp(image, &image_bpp);

    free(capture_image_path);
    fclose(image);

    /* Escritura encabezado archivo resumen de resultados (text)
     */
    fprintf(text_summary_file, "Work Path: %s\n", work_path);
    fprintf(text_summary_file, "Storage Results Folder: %s\n", storage_folder);
    fprintf(text_summary_file, "Loss Files Folder: %s\n", lossfiles_folder);
    fprintf(text_summary_file, "Image Dimensions (height x width): %d x %d pixels\n", height, width);
    fprintf(text_summary_file, "Block Dimensions (height x width): %d x %d pixels\n", height_block, width_block);
    fprintf(text_summary_file, "Size Package: %d bytes\n", size_package);
    fprintf(text_summary_file, "Image Bpp: %d bits\n", image_bpp);
    fprintf(text_summary_file, "User Bpp: %0.2f bits\n", user_bpp);
    fprintf(text_summary_file, "Interleaving Method: ");
    if(!process_opt[1]) fprintf(text_summary_file, "no\n");
    if(userOptions[8])  fprintf(text_summary_file, "Torus Automorphisms (Parameters: k = %d, n = %d)\n", k, n);
    if(userOptions[9])  fprintf(text_summary_file, "Turner and Peterson (Parameters: byteOffset = %d, packetOffset = %d)\n", byteOffset,
                                packetOffset);
    if(userOptions[10]) fprintf(text_summary_file, "DSJ-AL (Parameter: step = %d)\n", step);
    fprintf(text_summary_file, "Text Summary File: %s\n", summary_text_name);
    fprintf(text_summary_file, "Data Summary File: %s\n", summary_data_name);
    fprintf(text_summary_file, "\n");


    fclose(text_summary_file);
    fclose(data_summary_file);

    /* Ejecución de el o los procesos seleccionados para cada imagen en la carpeta de trabajo
     */
    for(int index_files = 0; index_files < n_images_files; index_files++)
    {
        if(userOptions[2])
        {
            for(i = 0; strcmp(argv_func[i], IMAGE_NAME); i++);
            i++;

            free(argv_func[i]);
            argv_func[i] = (char *)malloc((strlen(images_files[index_files])+1)*sizeof(char));
            for(int index = 0; index < (strlen(images_files[index_files])+1); index++) argv_func[i][index] = '\0';
            strcat(argv_func[i], images_files[index_files]);
        }
        else
        {
            userOptions[2] = true;

            argc_func++;
            argv_func = (char **)realloc(argv_func, argc_func*sizeof(char *));
            argv_func[argc_func-1] = (char *)malloc((strlen(IMAGE_NAME)+1)*sizeof(char));
            for(int index = 0; index < (strlen(IMAGE_NAME)+1); index++) argv_func[argc_func-1][index] = '\0';
            strcat(argv_func[argc_func-1], (char *)IMAGE_NAME);

            argc_func++;
            argv_func = (char **)realloc(argv_func, argc_func*sizeof(char *));
            argv_func[argc_func-1] = (char *)malloc((strlen(images_files[index_files])+1)*sizeof(char));
            for(int index = 0; index < (strlen(images_files[index_files])+1); index++) argv_func[argc_func-1][index] = '\0';
            strcat(argv_func[argc_func-1], images_files[index_files]);
        }

        /* Ejecución por cada archivo de pérdidas
         */
        for(int index_lossfiles = 0; index_lossfiles < n_patternloss_files; index_lossfiles++)
        {
            if(userOptions[7])
            {
                for(i = 0; strcmp(argv_func[i], LOSS_FILE); i++);
                i++;

                free(argv_func[i]);
                argv_func[i] = (char *)malloc(strlen(patternloss_files[index_lossfiles])*sizeof(char));
                for(int index = 0; index < strlen(patternloss_files[index_lossfiles]); index++) argv_func[i][index] = '\0';
                strcat(argv_func[i], patternloss_files[index_lossfiles]);
            }
            else
            {
                userOptions[7] = true;

                argc_func++;
                argv_func = (char **)realloc(argv_func, argc_func*sizeof(char *));
                argv_func[argc_func-1] = (char *)malloc((strlen(LOSS_FILE)+1)*sizeof(char));
                for(int index = 0; index < (strlen(LOSS_FILE)+1); index++) argv_func[argc_func-1][index] = '\0';
                strcat(argv_func[argc_func-1], (char *)LOSS_FILE);

                argc_func++;
                argv_func = (char **)realloc(argv_func, argc_func*sizeof(char *));
                argv_func[argc_func-1] = (char *)malloc(strlen(patternloss_files[index_lossfiles])*sizeof(char));
                for(int index = 0; index < strlen(patternloss_files[index_lossfiles]); index++) argv_func[argc_func-1][index] = '\0';
                strcat(argv_func[argc_func-1], patternloss_files[index_lossfiles]);
            }

            if(process_opt[0])
            	sequentialProcess(argc_func, argv_func);

            if(process_opt[1])
                interleavingProcess(argc_func, argv_func);
        }

        /* Escritura separador de resultados
         */
        if(index_files < (n_images_files-1))
        {
            if((text_summary_file = fopen(summary_text_path, "a+")) == NULL)
            {
                fprintf(stderr, "simlit: Error, impossible open or create \"%s\" summary text file\n", summary_text_path);
                exit(EXIT_FAILURE);
            }
            for(int index = 0; index < N_SEP; index++) fprintf(text_summary_file, "%c", C_SEP);
            fprintf(text_summary_file, "\n");
            fclose(text_summary_file);
        }
    }

    /* Liberación de memoria local
     */
    for(i = 0; i < argc_func; i++)
        free(argv_func[i]);
    free(argv_func);

    for(i = 0; i < n_images_files; i++)
        free(images_files[i]);
    free(images_files);

    for(i = 0; i < n_patternloss_files; i++)
        free(patternloss_files[i]);
    free(patternloss_files);

	/* Obtención tiempo de procesamiento
	 */
	gettimeofday(&t_fin, NULL);
	if((text_summary_file = fopen(summary_text_path, "a+")) == NULL)
    {
    	fprintf(stderr, "simlit: Error, impossible open or create \"%s\" summary text file\n", summary_text_path);
        exit(EXIT_FAILURE);
    }	
	int minutes = (t_fin.tv_sec - t_ini.tv_sec)/60;
	int seconds = (t_fin.tv_sec - t_ini.tv_sec)%60;
	fprintf(text_summary_file, "\n(*) Processing time: %0.2d minutes, %0.2d seconds and %0.3d milliseconds\n", minutes, seconds, 
			abs(t_fin.tv_usec - t_ini.tv_usec)/1000);
	fclose(text_summary_file);

	/* Liberación de memoria local
     */
    free(summary_text_path);
    free(summary_data_path);
}
