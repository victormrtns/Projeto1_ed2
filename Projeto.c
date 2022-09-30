#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// -------------------- funcoes ------------------------------
int inserir(FILE *out, FILE *in);
void remover(FILE *remove, FILE *out);
void compactar(FILE *in);

// ------------------- struct --------------------------------
typedef struct {
    char isbn[14];
    char titulo[50];
    char autor[50];
    char ano[5];
}Registro;

struct lista{
	int conteudo; 
	struct lista *prox;
}; 
typedef struct lista Lista;


int main(){
	
    FILE *insere;			// ponteiro p/ arq insere.bin
    FILE *remove;			// ponteiro p/ arq remove.bin
    FILE *out;				// ponteiro p/ arq main criado para manipular os dados
    int op;  
    
    // se nao conseguir abrir o arq para leitura/escrita cria ele pela 1 vez (w+b) senao abre para manipulacao
	if( (out = fopen("main.bin","r+b")) == NULL ){
		
		out = fopen("main.bin","w+b");
		
		// adiciona header inicializando com -1 no arq main
        int num = -1;
		fwrite(&num,sizeof(int),1,out);      
    }
    
    if( (insere = fopen("insere.bin","r+b")) == NULL){			// abre arq insere.bin para leitura dos dados a serem inseridos
       
	    printf("Erro na abertura do arquivo");
        return 0;
    }

	if( (remove = fopen("remove.bin","r+b")) == NULL){			// abre arq insere.bin para leitura dos dados a serem inseridos
       
	    printf("Erro na abertura do arquivo");
        return 0;
    }


    do{
    	printf(" MENU \n 1 - Insercao \n 2 - Remocao \n 3 - Compactacao \n 4 - Carrega Arquivos \n Opcao: ");
    	scanf("%d",&op);
    	
        switch(op){
            case 1:{
	            inserir(out, insere);
	            break;
			}
            case 2:{
	            remover(remove, out);
	            break;
			}
            case 3:{
				break;
			}          
            case 0:{
				break;
			}
            default:{ 
				printf("opcao invalida\n");
				break;
			}
		}
		
    }while( op!=0 );

    fclose(insere);
    fclose(remove);
    fclose(out);   
    return 1;
}

int inserir(FILE *out, FILE *in){
	
	rewind(in);
	rewind(out);
	
    Registro reg;				// recebe os campos do registro lido no arq insere
    char registro[125];			// recebe string formatada
    
	int tam_reg;				// calcula tam da string formatada a ser inserida no main
	int tam_reg_remove;			// recupera tam do registro removido para verificar se consegue inserir ali

	int prox_offset;
	
    int contador = 0;			// conta quantos registros ja foram lidos
    char aux[14];				// recebe isbn e depois modifica com / para marcar qual ja foi lido no arq insere.bin
	
	// cria/aloca lista
	Lista *lista;
	lista = malloc(sizeof(Lista));
	lista->prox = NULL;
	
	// recupera header
    int header;
    fread(&header, sizeof(int), 1, out);

	// enquanto consegue ler o arq insere.bin recupera os dados
	while( fread(&reg, sizeof(reg), 1, in) ){					// recebe um registro lido
	
        sprintf(aux, "%s", reg.isbn);							// recebe o isbn
		//if(aux[0] == '/'){} //foi lido
        //else if(aux[0]== '@'){} //foi apagado
        
        if( aux[0] != '/'){							// quando nao tiver / registro no arq insere ainda nao foi inserido, entao le
        
	        sprintf(registro, "%s#%s#%s#%s", reg.isbn, reg.titulo, reg.autor, reg.ano);		// registro recebe a string formatada
	        tam_reg = strlen(registro);														// tam_reg guarda o tamanho da string inteira
	        tam_reg++;
			
			// se header for -1 nenhum arq foi removido, insere no final do arquivo, caso contrario verifica lista ligada
			if(header == -1){
				
				fseek(out, 0, SEEK_END);								// posiciona no final do arq para escrever
				fwrite(&tam_reg, sizeof(int), 1, out);					// escreve tam no inicio do isbn	
	        	fwrite(registro, sizeof(char), tam_reg, out);			// escreve no arq main a string formatada com o tam no inicio
	        	
			} else {
			
				// posiciona ponteiro no arquivo para pos do valor no header, le o tamanho e verifica se da para inserir
				prox_offset = header;
				
				// insere na lista
				Lista *nova;
				nova = malloc (sizeof (Lista));
				nova->conteudo = prox_offset;
				nova->prox = lista->prox;
				lista->prox = nova;
				
				rewind(out);				// reposiciona comeco do arq pq leu header entao ta 4 bytes do header q leu a mais
				
				do{
					fseek(out, prox_offset-sizeof(char), SEEK_SET);			// reposiciona para o reg na posicao prox_offset
					fread(&tam_reg_remove, sizeof(int), 1, out);			// le tam registro
					
					// se nao consegue inserir le depois do * para ver proxima pos na lista
					if(tam_reg > tam_reg_remove){

						fseek(out, sizeof(char), SEEK_CUR);			// pula leitura do asterisco e le prox valor 
						fread(&prox_offset, sizeof(int), 1, out);
						
						// insere prox offset na lista
						Lista *nova;
						nova = malloc (sizeof (Lista));
						nova->conteudo = prox_offset;
						nova->prox = lista->prox;
						lista->prox = nova;
					}
					//Se nao for maior sai do while (break)
					if((tam_reg <= tam_reg_remove) || (prox_offset == -1)){
						break;
					}
				
				}while( (tam_reg > tam_reg_remove) || (prox_offset != -1) );
				
				// se sair do while pq chegou no -1 insere no final do arquivo
				if(prox_offset == -1){
	
					fseek(out, 0, SEEK_END);								// posiciona no final do arq para escrever
					fwrite(&tam_reg, sizeof(int), 1, out);					// escreve tam no inicio do isbn	
		        	fwrite(registro, sizeof(char), tam_reg, out);			// escreve no arq main a string formatada com o tam no inicio
					
				}else{				// se nao, saiu pq eh menor ou igual e da pra inserir, entao insere nessa posicao
					
					int offset_anterior = prox_offset;
					
					fseek(out, sizeof(char), SEEK_CUR);			// pula leitura do asterisco e le prox valor p/ adicionar na lista o prox q apontava
					fread(&prox_offset, sizeof(int), 1, out);
					
					// insere prox offset na lista
					Lista *nova;
					nova = malloc (sizeof (Lista));
					nova->conteudo = prox_offset;
					nova->prox = lista->prox;
					lista->prox = nova;
					
					// reposiciona de volta para o offset anterior para inserir no lugar do registro removido que achou
					fseek(out, (offset_anterior-sizeof(char)) + sizeof(int) + sizeof(char), SEEK_SET);
					
					// reposiciona para ler depois * (vai ser o novo header apos insercao no lugar do removido)
					//fseek(out, sizeof(char), SEEK_CUR);
					fread(&header, sizeof(int), 1, out);		// le header
					
					// posiciona para atualizar header no inicio
					fseek(out, 0, SEEK_SET);
					fwrite(&header, sizeof(int), 1, out);
					
					// reposiciona de volta para subescrever a partir do * com registro a inserir
					fseek(out, (offset_anterior-sizeof(char)) + sizeof(int), SEEK_SET);
					fwrite(registro, sizeof(char), tam_reg, out);
			
				}
					
			}
			
	// ------------------------------------------------------------------------------------------------------------------------ //
	        
	        // mofifica arq insere com / no inicio do isbn para marcar como j? lido
			aux[0] = '/';
	        fseek(in, contador*sizeof(reg), SEEK_SET);				// posiciona 
			fwrite(&aux, sizeof(reg.isbn), 1, in);					// escreve 
				
	        break;
	    }
	    contador++;		// contador para saber quantos registros ja foram lidos no insere
    
    }	// fim while
    return 1;
}

void remover(FILE *remove, FILE *out){
	
	rewind(remove);
	rewind(out);
	
	int header;
	int byteoffset;
	int tam_registro;
	char aux;
	
	char isbn[14] = "";
	char isbn_main[14] = "";
	
	int validador;
	int i=0;
	int contador = 0;
	
	// enquanto conseguir ler arq remove, procura isbn que ainda nao foi usado
	while(fread(&isbn, 1, 13, remove) == 13){
		
		//if(isbn[0] == '/'){} //foi removido
		fseek(remove, 1, SEEK_CUR);
		
		if(isbn[0] != '/'){
			
			// recupera header
			fread(&header, sizeof(int), 1, out);
			
			//recupera isbn + tam registro 
			do{
				fread(&tam_registro, sizeof(int), 1, out);		// le um inteiro (tam_reg)
				fread(&aux,sizeof(char),1,out);
				
				while(aux == '*'){

					if(aux == '*'){
						fseek(out,sizeof(int),SEEK_CUR);
						fseek(out,sizeof(tam_registro),SEEK_CUR);
		
						fread(&tam_registro, sizeof(int), 1, out);		// le um inteiro (tam_reg)
						fread(&aux,sizeof(char),1,out);
					}
				}
				fseek(out,-1,SEEK_CUR);
				fread(isbn_main, 13, 1, out);					// le isbn
				strcat(isbn_main, "\0");	
				
				//Validar se as strings sao iguais
				validador = strcmp(isbn_main, isbn);				
				fseek(out, tam_registro-13, SEEK_CUR);
				
			}while(validador != 0);

			fseek(out, 0-tam_registro, SEEK_CUR);							// reposiciona para inicio reg

			byteoffset = ftell(out) - sizeof(int)+1;						// calcula certo por conta do espaço entre os registros

			fwrite("*",sizeof(char),1,out);									// escreve * depois do tam
			fwrite(&header,sizeof(int),1,out);								// escreve header

			int tam_resto = tam_registro - sizeof(char)-sizeof(int);

			// escreve \0 no resto do reg
			for(i=0;i<tam_resto;i++){
				fwrite("\0",sizeof(char),1,out);
			}
			rewind(out);													// reposiciona inicio arq
			fwrite(&byteoffset,sizeof(int),1,out); 							// atualiza header
		
			isbn[0] = '/';
			fseek(remove, contador*sizeof(isbn), SEEK_SET);
			fwrite(&isbn, sizeof(isbn), 1, remove);
			
			break;
		
		}
		contador++;
	
	}

}
