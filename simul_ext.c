#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre);
int Eliminar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main()
{
	 char *comando[LONGITUD_COMANDO];
	 char *orden[LONGITUD_COMANDO];
	 char *argumento1[LONGITUD_COMANDO];
	 char *argumento2[LONGITUD_COMANDO];
	 
	 int i,j;
	 unsigned long int m;
     EXT_SIMPLE_SUPERBLOCK ext_superblock;
     EXT_BYTE_MAPS ext_bytemaps;
     EXT_BLQ_INODOS ext_blq_inodos;
     EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
     EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
     int entradadir;
     int grabardatos;
     FILE *fent;
     
     // Lectura del fichero completo de una sola vez
     //...
     
     fent = fopen("particion.bin","r+b");
     if(fent==NULL){
     	printf("\nFichero no encontrado");
     	exit(-1);
	 }
     fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);
     printf("");
     
     
     memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
     memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
     memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
     memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
     memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
     // Buce de tratamiento de comandos
     for (;;){
		 do {
		 printf (">> ");
		 fflush(stdin);
		 fgets(comando, LONGITUD_COMANDO, stdin);
		 
		 } while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);
		 		 
	// orden para la informacion	 		 
		if(strcmp(orden,"info")==0){
			printf("Iformacion\n");
			printf("Bloques particion = %d\n",MAX_BLOQUES_PARTICION);
			printf("Bloques libres = %d\n",ext_superblock.s_free_blocks_count);
			printf("Primer bloque de datos = %d\n",PRIM_BLOQUE_DATOS);
			printf("Bloque %d Bytes\n",SIZE_BLOQUE);
			printf("inodos particion = %d\n",MAX_INODOS);
			printf("inodos libres = %d\n",ext_superblock.s_free_inodes_count);
			continue;
		}
		
	// orden para el directorio	
		if(strcmp(orden,"dir")==0){
			printf("Directorio\n");
			Directorio(directorio, &ext_blq_inodos);
			//,ext_blq_inodos
			continue;
		}
	// orden para cambiar el nombre del archivo
		if(strcmp(orden,"renombrar")==0){
			printf("Cambiando nombre\n");
			if(Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2)==0){
				printf("EL fichero no existe\n");
			}else if(Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2)==1){
				printf("BIEN\n");
			}else if(Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2)==2){
				printf("El nombre ya existe\n");
			}
			continue;
			
		}
	// orden para imprimir y ver el contenido del archivo	
		if(strcmp(orden,"imprimir")==0){
			printf("Contenido\n");
			Imprimir(directorio, &ext_blq_inodos, datosfich, argumento1);
			continue;
		}
		
	// orden para eliminar un archivo
		if(strcmp(orden,"eliminar")==0){
			printf("Eliminando.txt\n");
			Eliminar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent);
			continue;
		}
	// orden para salir del simulador
        if (strcmp(orden,"salir")==0){
        	
        	Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
			GrabarByteMaps(&ext_bytemaps, fent);
			GrabarSuperBloque(&ext_superblock, fent);
			GrabarDatos(memdatos, fent);
            printf("\nSaliendo\n");
            fclose(fent);
            return 0;
        }
     }
}

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2){
	// Ahora procedemos a Limpiar las variables
	strcpy(orden,"");
	strcpy(argumento1,"");
	strcpy(argumento2,"");	
	
	char aux[strlen(strcomando)-1];
	int contador=0;
	strncpy(aux,strcomando,strlen(strcomando)-1);
	aux[strlen(strcomando)-1]='\0';
	
	
	//Asignamos la orden que queremos usar y los argumentos necesarios
	char *token=strtok(aux, " ");		
		
	while(token!=NULL){		
		if(contador==0){
			strcpy(orden,token);			
		}else if(contador==1){
			strcpy(argumento1,token);			
		}else if(contador==2){
			strcpy(argumento2,token);
		}
		token=strtok(NULL, " ");
		contador++;
	}
	
	//Comprobamos que las ordenes funcionen de forma correcta
	if(strcmp(orden,"info")==0){
		return 0;	//Retorna 0 si la funcion existe
	}else if(strcmp(orden,"dir")==0){
		return 0;
	}else if(strcmp(orden,"renombrar")==0){
		if((strlen(argumento1)>0) && (strlen(argumento2)>0) ){
			return 0;
		}else{
			printf("Argumentos incorrectos\nComando correcto \"renombrar nombre_origen.txt nombre_final.txt\"\n");
			return 1;
		}
		
	}else if(strcmp(orden,"imprimir")==0){
		if(strlen(argumento1)>0){
			if(strlen(argumento2)>0){
				printf("Argumentos incorrectos\nComando correcto \"imprimir archivo.txt\"\n");
				return 1;
			}
			return 0;
		}else{printf("Argumentos incorrectos\nComando correcto \"eliminar archivo.txt\"\n");}
	}else if(strcmp(orden,"eliminar")==0){
		if(strlen(argumento1)>0){
			if(strlen(argumento2)>0){
				printf("Argumentos incorrectos\nComando correcto \"eliminar archivo.txt\"\n");
				return 1;
			}
			return 0;
		}else{printf("Argumentos incorrectos\nComando correcto \"eliminar archivo.txt\"\n");}
	}else if(strcmp(orden,"salir")==0){
		return 0;
	}else{
		printf("ERROR: Comando incorrecto utiliza [dir,info,imprimir,renombrar,eliminar,salir]\n");
		return 1;	
	}
}

//aplicamos las funciones de cada orden 
	//Imprimir
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre){
	int i, j;
	for(i = 1; i < MAX_FICHEROS; i++){
		if((directorio+i)->dir_inodo != NULL_INODO){
			
			if(strcmp(nombre,(directorio+i)->dir_nfich) == 0){
				for(j = 0; inodos->blq_inodos[(directorio+i)->dir_inodo].i_nbloque[j] != NULL_INODO; j++){//
                  char aux[SIZE_BLOQUE];
                  memcpy(aux,memdatos[inodos->blq_inodos[(directorio+i)->dir_inodo].i_nbloque[j]].dato,SIZE_BLOQUE);
				  aux[SIZE_BLOQUE]='\0';
				  printf("%s", aux);
                }
				printf("\n");
			}	
		}
	}
	return 0;
}
	//Directorio
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){
    int i,j;
    for(i = 1; i < MAX_FICHEROS; i++){
        if((directorio+i)->dir_inodo != NULL_INODO){
            printf("%s\t tamano: %d\t inodo: %d\t bloques: ", (directorio+i)->dir_nfich, inodos->blq_inodos[(directorio+i)->dir_inodo].size_fichero, (directorio+i)->dir_inodo);
                for(j = 0; inodos->blq_inodos[(directorio+i)->dir_inodo].i_nbloque[j] != NULL_INODO; j++){
                    printf("%d ", inodos->blq_inodos[(directorio+i)->dir_inodo].i_nbloque[j]);
                }
                printf("\n");
        }
    }
}

	//Renombrar
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){
	int i, j;
	for(i = 1; i < MAX_FICHEROS; i++){
		if((directorio+i)->dir_inodo != NULL_INODO){
			
			if(strcmp(nombreantiguo,(directorio+i)->dir_nfich) == 0){//Si el archivo existe
			
				//printf("\nNOMBRE inicio BIEN\n");
				for(j = 1; j < MAX_FICHEROS; j++){
					if((directorio+j)->dir_inodo != NULL_INODO){
						
						if(strcmp(nombrenuevo,(directorio+j)->dir_nfich) == 0){
							//printf("\nNOMBRE FINAL MAL\n");
							return 2;
							
						}else{
							//printf("\nNOMBRE FINAL MAL\n");
							continue;
						}
					}
				}
				//printf("\nNOMBRE FINAL BIEn\n");
				strcpy((directorio+i)->dir_nfich, nombrenuevo);
				return 1;
				
				
			}
	
		}//Si es NULL realiza un return 
	}
	return 0;

		
}
	// Eliminar
int Eliminar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich){
	int aux;
	int i;
	for(i = 1; i < MAX_FICHEROS; i++){
		if((directorio+i)->dir_inodo != NULL_INODO){
			if(strcmp(nombre,(directorio+i)->dir_nfich) == 0){
				aux =  i;//inicializamos una variable auxiliar para guardar la posicion donde se encuentra el fichero y mas adelante poder BORRAR el directorio donde se encuentra el fichero
				int j;
				for(j = 0; inodos->blq_inodos[(directorio+i)->dir_inodo].i_nbloque[j] != NULL_INODO; j++){//
                  //printf("\n[%d]\n",inodos->blq_inodos[(directorio+i)->dir_inodo].size_fichero);
                
                    ext_bytemaps->bmap_bloques[inodos->blq_inodos[(directorio+i)->dir_inodo].i_nbloque[j]]=0;//Borra bloques del bytemap de bloques
					ext_bytemaps->bmap_inodos[(directorio+i)->dir_inodo]=0;//Borra el bytemap de inodos
					
					inodos->blq_inodos[(directorio+i)->dir_inodo].i_nbloque[j]=NULL_BLOQUE;//borramos los datos del bloque
					inodos->blq_inodos[(directorio+i)->dir_inodo].size_fichero = 0;//borramos el tamaño del fichero
					
					ext_superblock->s_free_blocks_count++;//liberamos los bloques que se han borrado
					ext_superblock->s_free_inodes_count++;//liberamos los inodos que se han borrado
					
                }
                int i;
				for(i = aux; i < MAX_FICHEROS-1; i++){//Borramos  los datos del fichero
					directorio[i] = directorio[i + 1];
				}
			}
		}
	}
}



void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich){
	fseek(fich, SIZE_BLOQUE*1, SEEK_SET);		
	fwrite((EXT_DATOS *)ext_bytemaps, SIZE_BLOQUE, 1, fich);

}
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich){
	fseek(fich, SIZE_BLOQUE*4, SEEK_SET);		
	fwrite((EXT_DATOS *)memdatos, MAX_BLOQUES_DATOS*SIZE_BLOQUE, 1, fich);	
}
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich){
	fseek(fich, SIZE_BLOQUE*2, SEEK_SET);	
	fwrite((EXT_DATOS *)inodos, SIZE_BLOQUE, 1, fich);		
	fseek(fich, SIZE_BLOQUE*3, SEEK_SET);
	
	fwrite((EXT_DATOS *)directorio, SIZE_BLOQUE, 1, fich);
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich){
	fseek(fich, SIZE_BLOQUE*0, SEEK_SET );
	fwrite((EXT_DATOS *)ext_superblock, SIZE_BLOQUE, 1,fich);		
}



