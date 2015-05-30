/* Project: SimLIT
 * Version: 1.2
 * File: functions.h
 * Last modified: 24/09/2014

 * Author: Eric Orellana-Romero
 * Mail: e.orellanaromero@gmail.com
 * 
 * Description: Archivo en el que se incluyen funciones generales.
 */

#ifndef USER_OPTIONS_H
#define USER_OPTIONS_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int width_block, height_block, size_package, k, n, byteOffset, packetOffset, step, u_wrnpr;
float user_bpp;
char *work_path, *storage_folder, *image_name, *lossfiles_folder, *loss_file, *summary_data_name, *summary_text_name;

/* Opciones del programa
 */

#define TOTAL_OPTIONS 22                // actualizar valor al agregar nueva opción
#define WORK_DIR "-images-folder"       // index 0
#define STOR_FOL "-results-folder"      // index 1
#define IMAGE_NAME "-image-name"        // index 2
#define WIDTH_BLOCK "-width-block"      // index 3
#define HEIGHT_BLOCK "-height-block"    // index 4
#define SIZE_PACK "-size-package"       // index 5
#define USER_BPP "-bpp"                 // index 6
#define LOSS_FILE "-loss-file"          // index 7
#define TORUS_MIX "-torus-mixer"        // index 8
#define TURNER_MIX "-turner-mixer"      // index 9
#define DSJAL_MIX "-dsjal-mixer"        // index 10
#define INT_FILE_IMG "-interleaving-image"                // index 11
#define REC_FILE_IMG "-received-interleaving-image"       // index 12
#define DEINT_FILE_IMG "-deinterleaving-image"            // index 13
#define CONC_FILE_IMG "-concealment-interleaving-image"   // index 14
#define REC_SEQ_FILE_IMG "-received-sequential-image"     // index 15
#define CONC_SEQ_FILE_IMG "-concealment-sequential-image" // index 16
#define MOD_SUMMARY_TXT "-summary-text"                   // index 17
#define MOD_SUMMARY_DATA "-summary-data"                  // index 18
#define LOSS_FILES_FOL "-loss-files-folder"               // index 19
//#define PARALLEL_PROCESS "-cuda-processing"		          // index 20
#define SET_WRNPR_U "-set-u-parameter-wrnpr"              // index 21

/* Funciones
 */

bool char2uint(char *nChar, unsigned int *ui_number)
{
/* Convierte el tipo de  variable  de un  vector char (que almacena un número) a
 * una variable int. El número debe ser entero positivo.
 */

    int  i, k;
	unsigned int n_uInteger;
    char *n_Character;

    n_Character = (char *)malloc((strlen(nChar)+1)*sizeof(char)); // una posición más para carácter nulo.

    for(i = 0; i < strlen(nChar); i++) n_Character[i] = nChar[i];
    n_Character[strlen(nChar)] = '\0';

    for(i = (strlen(n_Character)-1), k = 0, n_uInteger = 0; i >= 0; i--, k++)
    {
        if((n_Character[i] < '0') || (n_Character[i] > '9'))
            return false;
        n_uInteger += ((int)n_Character[i]-48)*pow(10, k);
    }

	*ui_number = n_uInteger;
    free(n_Character);
    return true;
}

bool char2ufloat(char *nChar, float *float_number)
{
/* Convierte el tipo de variable  de  un  vector char (que almacena un número) a
 * una variable float.
 */

    int i = 0, k = 0;
    char *n_Character;
    float n_uFloat = 0.0;
    bool flag = false;

    n_Character = (char *)malloc((strlen(nChar)+1)*sizeof(char)); // una posición más para carácter nulo

    for(i = 0; i < strlen(nChar); i++) n_Character[i] = nChar[i];
    n_Character[strlen(nChar)] = '\0';

    for(i = (strlen(n_Character)-1), k = 0, n_uFloat = 0.0; i >= 0; i--, k++)
    {
        if(((n_Character[i] < '0') || (n_Character[i] > '9')) && ((n_Character[i] != '.') && (n_Character[i] != ',')))
            return false;

        if((n_Character[i] >= '0') && (n_Character[i] <= '9'))
            n_uFloat += (((float)n_Character[i]-(float)48)*(float)pow(10, k));

        if((n_Character[i] == '.') || n_Character[i] == ',')
        {
            if(flag) return false;

            n_uFloat /= (float)pow(10, k);
            k = -1;
            flag = true;
        }
    }

	*float_number = n_uFloat;
    free(n_Character);
    return true;
}

void err_arg(char *arg)
{
    fprintf(stderr, "simlit: Error, invalid argument for option %s\n", arg);
    exit(EXIT_FAILURE);
}

void setUserOptions(int argc, char **argv, bool *userOptions)
{
/* Analiza las opciones ingresadas por el usuario y asigna las variables correpondientes.
 * Actualiza el array de flags userOptions.
 */

    // vector flag initialising
    for(int x = 0; x < TOTAL_OPTIONS; x++) userOptions[x] = false;

    unsigned int i = 1, i_temp;
    while(i < argc)
    {
        /* Argumentos de programa
         */
        i_temp = i;

        if(i < argc)
            if(!strcmp(argv[i], WORK_DIR))
            {
                userOptions[0] = true;
                i++;
                if(i >= argc) err_arg((char *)WORK_DIR);
                if(argv[i][0] == '-') err_arg((char *)WORK_DIR);

                work_path = (char *)malloc((strlen(argv[i])+1)*sizeof(char));
                for(int index = 0; index < (strlen(argv[i])+1); index++) work_path[index] = '\0';
                strcpy(work_path, argv[i]);

                // if(!verExt(ruta,(char*)".bmp")) err_imgnfound(argv[0],ruta);

                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], STOR_FOL))
            {
                userOptions[1] = true;
                i++;
                if(i >= argc) err_arg((char *)STOR_FOL);
                if(argv[i][0] == '-') err_arg((char *)STOR_FOL);

                storage_folder = (char *)malloc((strlen(argv[i])+1)*sizeof(char));
                for(int index = 0; index < (strlen(argv[i])+1); index++) storage_folder[index] = '\0';
                strcpy(storage_folder, argv[i]);

                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], IMAGE_NAME))
            {
                userOptions[2] = true;
                i++;
                if(i >= argc) err_arg((char *)IMAGE_NAME);
                if(argv[i][0] == '-') err_arg((char *)IMAGE_NAME);

                image_name = (char *)malloc((strlen(argv[i])+1)*sizeof(char));
                for(int index = 0; index < (strlen(argv[i])+1); index++) image_name[index] = '\0';
                strcpy(image_name, argv[i]);

                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], HEIGHT_BLOCK))
            {
                userOptions[4] = true;
                i++;
                if(i >= argc) err_arg((char *)HEIGHT_BLOCK);
                if(!char2uint(argv[i], &height_block)) err_arg((char *)HEIGHT_BLOCK);

                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], WIDTH_BLOCK))
            {
                userOptions[3] = true;
                i++;
                if(i >= argc) err_arg((char *)WIDTH_BLOCK);
                if(!char2uint(argv[i], &width_block)) err_arg((char *)WIDTH_BLOCK);

                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], SIZE_PACK))
            {
                userOptions[5] = true;
                i++;
                if(i >= argc) err_arg((char *)SIZE_PACK);
                if(!char2uint(argv[i], &size_package)) err_arg((char *)SIZE_PACK);

                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], USER_BPP))
            {
                userOptions[6] = true;
                i++;
                if(i >= argc) err_arg((char *)USER_BPP);
                if(!char2ufloat(argv[i], &user_bpp)) err_arg((char *)USER_BPP);

                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], LOSS_FILE))
            {
                userOptions[7] = true;
                i++;
                if(i >= argc) err_arg((char *)LOSS_FILE);
                if(argv[i][0] == '-') err_arg((char *)LOSS_FILE);

                loss_file = (char *)malloc((strlen(argv[i])+1)*sizeof(char));
                for(int index = 0; index < (strlen(argv[i])+1); index++) loss_file[index] = '\0';
                strcpy(loss_file, argv[i]);

                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], TORUS_MIX))
            {
                userOptions[8] = true;
                if(userOptions[9] || userOptions[10])
                {
                    fprintf(stderr, "simlit: Error, impossible to make two simultaneous mixing methods\n");
                    exit(EXIT_FAILURE);
                }

                i++;
                if(i >= argc) err_arg((char *)TORUS_MIX);
                if(!char2uint(argv[i], &k)) err_arg((char *)TORUS_MIX);

                i++;
                if(i >= argc) err_arg((char *)TORUS_MIX);
                if(!char2uint(argv[i], &n)) err_arg((char *)TORUS_MIX);

                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], TURNER_MIX))
            {
                userOptions[9] = true;
                if(userOptions[10] || userOptions[11])
                {
                    fprintf(stderr, "simlit: Error, impossible to make two simultaneous mixing methods\n");
                    exit(EXIT_FAILURE);
                }

                i++;
                if(i >= argc) err_arg((char *)TURNER_MIX);
                if(!char2uint(argv[i], &byteOffset)) err_arg((char *)TURNER_MIX);

                i++;
                if(i >= argc) err_arg((char *)TURNER_MIX);
                if(!char2uint(argv[i], &packetOffset)) err_arg((char *)TURNER_MIX);

                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], DSJAL_MIX))
            {
                userOptions[10] = true;
                if(userOptions[9] || userOptions[11])
                {
                    fprintf(stderr, "simlit: Error, impossible to make two simultaneous mixing methods\n");
                    exit(EXIT_FAILURE);
                }

                i++;
                if(i >= argc) err_arg((char *)DSJAL_MIX);
                if(!char2uint(argv[i], &step)) err_arg((char *)DSJAL_MIX);

                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], INT_FILE_IMG)) // create interleaving image
            {
                userOptions[11] = true;
                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], REC_FILE_IMG)) // create received image
            {
                userOptions[12] = true;
                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], DEINT_FILE_IMG)) // create deinterleaving image
            {
                userOptions[13] = true;
                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], CONC_FILE_IMG)) // create concealment image
            {
                userOptions[14] = true;
                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], REC_SEQ_FILE_IMG)) // create received sequential image
            {
                userOptions[15] = true;
                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], CONC_SEQ_FILE_IMG)) // create concealment sequential image
            {
                userOptions[16] = true;
                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], MOD_SUMMARY_TXT)) // modify summary text file
            {
                userOptions[17] = true;
                i++;

                if(i >= argc) err_arg((char *)MOD_SUMMARY_TXT);
                if(argv[i][0] == '-') err_arg((char *)MOD_SUMMARY_TXT);

                summary_text_name = (char *)malloc((strlen(argv[i])+1)*sizeof(char));
                for(int index = 0; index < (strlen(argv[i])+1); index++) summary_text_name[index] = '\0';
                strcpy(summary_text_name, argv[i]);

                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], MOD_SUMMARY_DATA)) // modify summary data file
            {
                userOptions[18] = true;
                i++;

                if(i >= argc) err_arg((char *)MOD_SUMMARY_DATA);
                if(argv[i][0] == '-') err_arg((char *)MOD_SUMMARY_DATA);

                summary_data_name = (char *)malloc((strlen(argv[i])+1)*sizeof(char));
                for(int index = 0; index < (strlen(argv[i])+1); index++) summary_data_name[index] = '\0';
                strcpy(summary_data_name, argv[i]);

                i++;
            }

        if(i < argc)
            if(!strcmp(argv[i], LOSS_FILES_FOL))
            {
                userOptions[19] = true;

                i++;
                if(i >= argc) err_arg((char *)LOSS_FILES_FOL);
                if(argv[i][0] == '-') err_arg((char *)LOSS_FILES_FOL);

                lossfiles_folder = (char *)malloc((strlen(argv[i])+1)*sizeof(char));
                for(int index = 0; index < (strlen(argv[i])+1); index++) lossfiles_folder[index] = '\0';
                strcpy(lossfiles_folder, argv[i]);

                i++;
            }            
        if(i < argc)
            if(!strcmp(argv[i], SET_WRNPR_U))
            {
                userOptions[21] = true;
                i++;
                
                if(i >= argc) err_arg((char *)SET_WRNPR_U);
                if(!char2uint(argv[i], &u_wrnpr)) err_arg((char *)SET_WRNPR_U);
                
                i++;
            }

        if(i == i_temp)
        {
            fprintf(stderr, "simlit: Error, \'%s\' is not valid parameter\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }
}

/* Default parameters
 */
#define DEF_DIR_WORK "."
#define DEF_STOR_FOL DEF_DIR_WORK
#define DEF_IMAGE_NAME "image.bmp"
#define DEF_WIDTH_BLOCK 1
#define DEF_HEIGHT_BLOCK 1
#define DEF_SIZE_PACK 27
#define DEF_USER_BPP 8
#define DEF_LOSS_FILE "patternLossfile.loss"
#define DEF_SUMMARY_TEXT_NAME "simulation-report.text"
#define DEF_SUMMARY_DATA_NAME "simulation-report.data"
#define DEF_LOSS_FILES_FOL "LossFiles"
#define DEF_WRNPR_U 1

/* Default parameters interleaving methods
 */
#define DEF_K 28
#define DEF_N 114
#define DEF_BYTEOFFSET 11727
#define DEF_PACKETOFFSET 5333
#define DEF_STEP 92

void setDefaultParameters(bool *userOptions)
{
    /* Asigna valores por defecto a las variables globales que no han sido modificadas por el usuario
     */
    if(!userOptions[0]) // work directory
    {
        work_path = (char *)malloc((strlen(DEF_DIR_WORK)+1)*sizeof(char));
        for(int index = 0; index < (strlen(DEF_DIR_WORK)+1); index++) work_path[index] = '\0';
        strcpy(work_path, (char *)DEF_DIR_WORK);
    }
    if(!userOptions[1]) // storage folder
    {
        storage_folder = (char *)malloc((strlen(DEF_STOR_FOL)+1)*sizeof(char));
        for(int index = 0; index < (strlen(DEF_STOR_FOL)+1); index++) storage_folder[index] = '\0';
        strcpy(storage_folder, (char *)DEF_STOR_FOL);
    }
    if(!userOptions[2]) // image name
    {
        image_name = (char *)malloc((strlen(DEF_IMAGE_NAME)+1)*sizeof(char));
        for(int index = 0; index < (strlen(DEF_IMAGE_NAME)+1); index++) image_name[index] = '\0';
        strcpy(image_name, (char *)DEF_IMAGE_NAME);
    }
    if(!userOptions[3]) // width block
        width_block = DEF_WIDTH_BLOCK;
    if(!userOptions[4]) // height block
        height_block = DEF_HEIGHT_BLOCK;
    if(!userOptions[5]) // size package
        size_package = DEF_SIZE_PACK;
    if(!userOptions[6]) // user bpp
        user_bpp = DEF_USER_BPP;
    if(!userOptions[7]) // loss file
    {
        loss_file = (char *)malloc((strlen(DEF_LOSS_FILE)+1)*sizeof(char));
        for(int index = 0; index < (strlen(DEF_LOSS_FILE)+1); index++) loss_file[index] = '\0';
        strcpy(loss_file, (char *)DEF_LOSS_FILE);
    }
    if(!userOptions[8]) // torus mixer
    {
        k = DEF_K;
        n = DEF_N;
    }
    if(!userOptions[9]) // turner mixer
    {
        byteOffset = DEF_BYTEOFFSET;
        packetOffset = DEF_PACKETOFFSET;
    }
    if(!userOptions[10]) // dsjal mixer
        step = DEF_STEP;
    if(!userOptions[17]) // summary text name file
    {
        summary_text_name = (char *)malloc((strlen(DEF_SUMMARY_TEXT_NAME)+1)*sizeof(char));
        for(int index = 0; index < (strlen(DEF_SUMMARY_TEXT_NAME)+1); index++) summary_text_name[index] = '\0';
        strcpy(summary_text_name, DEF_SUMMARY_TEXT_NAME);
    }
    if(!userOptions[18]) // summary data name file
    {
        summary_data_name = (char *)malloc((strlen(DEF_SUMMARY_DATA_NAME)+1)*sizeof(char));
        for(int index = 0; index < (strlen(DEF_SUMMARY_DATA_NAME)+1); index++) summary_data_name[index] = '\0';
        strcpy(summary_data_name, DEF_SUMMARY_DATA_NAME);
    }
    if(!userOptions[19]) // loss files folder
    {
        lossfiles_folder = (char *)malloc((strlen(DEF_LOSS_FILES_FOL)+1)*sizeof(char));
        for(int index = 0; index < (strlen(DEF_LOSS_FILES_FOL)+1); index++) lossfiles_folder[index] = '\0';
        strcpy(lossfiles_folder, DEF_LOSS_FILES_FOL);
    }
    if(!userOptions[21]) // wrnpr "u" parameter
        u_wrnpr = DEF_WRNPR_U;
}

void freeUsedMemory(void)
{
    free(work_path);
    free(loss_file);
    free(image_name);
    free(storage_folder);
    free(lossfiles_folder);
    free(summary_data_name);
    free(summary_text_name);
}

#endif // USER_OPTIONS_H
