#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t mutex;
pthread_cond_t cond_escr, cond_leit, cond_leit2, cond_escr2;

//Globais
int leit=0,escr=0, esperando_escrita = 0, esperando_leitura = 0, esperando_leitura2 = 0, esperando_escrita2 = 0, existe_escrita = 0, existe_leitura = 0;
int n_leituras = 0;
int n_escritas = 0;

int decisao_leit = 0; // Sinalizador que pode assumir valor 0 ou 1, se 0 leitoras não entram na fila ou se for 1 entram na fila  
int decisao_esc = 0;  // Mesmo caso do de cima, mas para escritores

int var_com = -1; //Variavel compartilhada entre as threads escritoras e leitoras. Se -1, nenhuma escrita foi realizada. 

FILE *pont_arquivo; // Arquivo de log

//Espaco para testes quantidades de escritas e leituras. Não aplicado na logica do programa
int total_esc = 0, cont_esc = 0;
int total_leituras = 0, cont_leit =0;
///////////////////////////////////////////////////////

void EntraLeitura(int tid) {
    pthread_mutex_lock(&mutex);
    //Essa parte  só serve no inicio e seta uma condição para a primeira leitora
    existe_leitura++;
    if ( existe_leitura  == 1 && existe_escrita == 0){
        decisao_leit = 0;
    }
    ///////////////////////////////////////////////////////////////////////////////

    if ( decisao_leit == 1) {
        esperando_leitura++;
        printf("L%d: Bloqueio! Aguardo na fila 1!\n", tid);
        pthread_cond_wait(&cond_leit, &mutex);
        esperando_leitura--;
    }
    
    while ( escr > 0 ) {
        esperando_leitura2++;
        printf("L%d: Bloqueio! Aguardo na fila 2\n", tid);
        pthread_cond_wait(&cond_leit2, &mutex);
        esperando_leitura2--;
    }
    leit++;
    //Para que testar a corretude da logica pelo o log, preciso marcar quando a primeira leitora de uma nova remessa entra.
    if ( leit == 1) {
        printf("L%d: Entrei! Sou a primeira.\n", tid);
        char palavra[30] = "tl.entra_leitora_primeira()\n"; //Escreve no arquivo do log: entra_leitora_primeira()
        fprintf(pont_arquivo, "%s", palavra);
    } else {
        printf("L%d: Entrei!\n", tid);
    }
    char palavra[1000] = "tl.entra_leitora()\n";//Escreve no arquivo do log: entra_leitora()
    fprintf(pont_arquivo, "%s", palavra);

    pthread_mutex_unlock(&mutex);
}

void SaiLeitura(int tid) {
    pthread_mutex_lock(&mutex);
    leit--;
    char palavra[25] = "tl.sai_leitora()\n";//Escreve no arquivo do log: sai_leitora()
    fprintf(pont_arquivo, "%s", palavra);
    if(leit==0){
        if ( esperando_escrita2!=0 ) {
            printf("L%d: Terminei! Sou a ultima e acordei escritora da fila 2\n", tid);
            pthread_cond_signal(&cond_escr2);
            decisao_esc = 1; // Gero fila 1 de escritores
            decisao_leit = 1; // Gero fila leitores
        } else if ( esperando_leitura!= 0) {
            printf("L%d: Terminei! Sou a ultima e acordei mais leitoras da fila 1\n", tid);
            pthread_cond_broadcast(&cond_leit);
        } else if ( esperando_escrita!= 0 ) {
            printf("L%d: Terminei! Sou a ultima e acordei escritoras da fila 1\n", tid);
            decisao_leit = 1; // Gero fila de leitores
            pthread_cond_broadcast(&cond_escr);
        } else {
            printf("L%d: Terminei! Sou a ultima.\n", tid);
            decisao_leit = 0; // Bloqueio geracao de fila leitores
            decisao_esc = 0;  // Se for a ultima leitura libero as escritoras
        }
        char palavra[30] = "tl.sai_leitora_ultima()\n"; //Escreve no arquivo do log: sai_leitora_ultima()
        fprintf(pont_arquivo, "%s", palavra);
    } else {
        printf("L%d: Terminei!\n", tid);
        if ( esperando_escrita != 0){
            printf("L%d: Há escritoras na fila 1, bloqueio novas leitoras!\n", tid);
            decisao_leit = 1;
        }
    }
    
    //Variavel para contabilizar leituras realizadas, não está aplicada na logica do programa
    cont_leit++;
    printf("Leituras até agora: %d  Total: %d\n", cont_leit, total_leituras);
    
    pthread_mutex_unlock(&mutex);
}

void *leitor ( void *arg ) {
    int tid = *( int* )arg; // Id da thread
    int i = 0; // Zera o contador de vezes do while
    int var_local;
    char nome_arq[7]; 
    char id[2];
    char lido[2];
    FILE *arq_saida_leitora;

    sprintf(id, "%i", tid); // Converte o int tid no char e salva em id
    sprintf( nome_arq, "%s.txt", id); // Concatena o char id com a terminacao do arquivo e salva em nome_arq
    
    arq_saida_leitora = fopen(nome_arq, "w"); // Abre a arquivo das leitoras
    if (arq_saida_leitora == NULL){
      printf("ERRO! O arquivo da leitora %i não foi aberto!\n", tid);
    }

    while( i < n_leituras ) {
        EntraLeitura( tid );
        var_local = var_com; // Le a variavel compartilhada
        sprintf(lido, "%i", var_local); // Converte o int var_local no char e salva em lido
        fprintf(arq_saida_leitora, "%s\n", lido); // Escreve no arquivo exclusivo da thread
        printf("L%d: Li o valor %d.\n", tid, var_local); 
        SaiLeitura( tid );
        i++;
    }

    fclose(arq_saida_leitora);
    pthread_exit(NULL);
}

void EntraEscrita (int id) {
    pthread_mutex_lock(&mutex);
    //Essa parte  só serve no inicio e seta uma condição para a primeira escritora
    existe_escrita++;
    if ( existe_escrita == 1 && existe_leitura == 0 ) {
        decisao_esc = 0;
    }
    ///////////////////////////////////////////////////////////////////////////////
    if( decisao_esc == 1) {
        esperando_escrita++;
        printf("E%d: Bloqueio! Aguardo na fila 1.\n", id);
        pthread_cond_wait(&cond_escr, &mutex);
        esperando_escrita--;
    }

    while ( escr>0 ||  leit>0 ) { 
        esperando_escrita2++;
        printf("E%d: Bloqueio! Aguardo na fila 2.\n", id);
        pthread_cond_wait(&cond_escr2, &mutex);
        esperando_escrita2--;
    }

    escr++;
    printf("E%d: Entrei!\n", id);
    char palavra[25] = "tl.entra_escrita()\n"; //Escreve no arquivo do log: entra_escrita()
    fprintf(pont_arquivo, "%s", palavra);
    
    pthread_mutex_unlock(&mutex);
}

void SaiEscrita (int id) {
    pthread_mutex_lock(&mutex);
    escr--;
    
    char palavra[25] = "tl.sai_escrita()\n"; // Escreve no arquivo do log: sai_escrita()
    fprintf(pont_arquivo, "%s", palavra);
    
    if(esperando_escrita2 != 0){
        decisao_leit = 1; // Gera fila de leitoras
        decisao_esc = 1; // Gera fila de escritoras
        pthread_cond_signal(&cond_escr2); // Ainda tenho escritoras na fila 2 para terminarem
        printf("E%d: Terminei! Acordei uma escritora fila 2.\n", id);
    }else if (esperando_leitura2 != 0 ) {
        printf("E%d: Terminei! Acordei leitoras da fila 2.\n", id);
        decisao_esc = 1; // Gera fila 1 das escritoras
        pthread_cond_broadcast(&cond_leit2);
    } else if ( esperando_escrita!=0 ) {
        decisao_leit = 1; // Gera fila leitoras
        printf("E%d: Termenei! Acordei escritoras da fila 1.\n", id);
        pthread_cond_broadcast(&cond_escr); // Escritoras passam da primeira fila e novas escriras ficaram nessa fila até essas passarem pela fila 2
    } else {
        printf("E%d: Termenei!\n", id);
        // Se ultima escrita e não tem fila na escrita nem na leitora, permite qualquer uma ganhar a condição de corrida
        decisao_esc = 0;
        decisao_leit = 0;
        if (esperando_leitura!= 0){
            printf("E%d: Ainda há leitoras na fila 1, vou acordalas!\n", id);
            pthread_cond_broadcast(&cond_leit);
        }
    }
    
    //Variavel para contabilizar escritas realizadas, não está aplicada na logica do programa
    cont_esc++;
    printf("Escritas até agora: %d de total: %d\n", cont_esc, total_esc);

    pthread_mutex_unlock(&mutex);
}

void *escritor ( void *arg ) {
    int tid = *(int*)arg;
    int i=0;
    while( i < n_escritas ) {
        EntraEscrita(tid);
        var_com = tid; // Escreve a id da thread escritora na varivel compartilhada por todas ( escritoras e leitoras ).
        printf("E%d: Escrevi o valor %d\n", tid, tid);
        SaiEscrita(tid);
        i++;
    }
    pthread_exit(NULL);
}

int main( int argc, char *argv[]){

    // Valida e recebe os valores de entrada
    if( argc < 6 ) {
      printf( "Use: %s <threads escritoras> <threads leitoras> <numero de leituras> <numero de escritas> <nome_do_arquivo.py>\n", argv[0]);
      exit( EXIT_FAILURE );
    }
    // Abre o arquivo de log
    pont_arquivo = fopen(argv[5], "w");
    if (pont_arquivo == NULL){
      printf("ERRO! O arquivo não foi aberto!\n");
    }else{
     printf("O arquivo foi aberto com sucesso!\n");
    }
    // Coloca no primeira linha do arquivo log o import necessario para executar as funcoes printadas do log
    char palavra[25] = "import teste_log as tl\n"; 
    fprintf(pont_arquivo, "%s", palavra);
    // Quantidades de execucoes que cada thread ira fazer
    n_leituras = atoi(argv[3]); 
    n_escritas = atoi(argv[4]);
    // Verifica se quantidade minima de threads escritoras e leitoras
    if( atoi( argv[1] )<2 && atoi( argv[2] )<2 ) {
        printf("Escritoras e/ou leitoras < 2 !\n");
        exit(-1);
    }

    int M = atoi( argv[1] );// Quantidades de threads de escritas
    int N = atoi( argv[2] );// Quantidades de threads de leitura

    // Espaco para testes quantidades de escritas e leituras
    total_esc = M * n_escritas;
    total_leituras = N * n_leituras;
    printf("Total esc: %d\nTotal leitura: %d\n", total_esc, total_leituras);
    ///////////////////////////////////////////////////////

    pthread_t tid[M];
    pthread_t tid_1[N];

    int *t, i;

    //--inicilaiza o mutex (lock de exclusao mutua)
    pthread_mutex_init( &mutex, NULL );
    pthread_cond_init( &cond_leit, NULL );
    pthread_cond_init( &cond_leit2, NULL );
    pthread_cond_init( &cond_escr, NULL );
    pthread_cond_init( &cond_escr2, NULL );

    // Cria as threads. Como inicializo as escritoras primeiro, elas tendem a ganhar a cpu primeiro sempre
    for( i =0; i<M; i++ ){
        t = malloc( sizeof( int ) ); if( t==NULL ) return -1;
        *t = i;
        if ( pthread_create( &tid[i], NULL, escritor, (void *)t )) {
            printf( "--ERRO: pthread_create()\n"); exit( -1 );
        }
    }
    for( i =0; i<N; i++ ){
        t = malloc( sizeof( int ) ); if( t==NULL ) return -1;
        *t = i;
        if (pthread_create( &tid_1[i], NULL, leitor, ( void * )t)) {
            printf( "--ERRO: pthread_create()\n" ); exit( -1 );
        }
    }

    // Espera todas as threads terminarem
    for ( i=0; i<M; i++ ) {
        if ( pthread_join( tid[i], NULL) ) {
            printf( "--ERRO: pthread_join() \n" ); exit( -1 );
        }
    }
    for ( i=0; i<N; i++ ) {
        if ( pthread_join( tid_1[i], NULL ) ) {
            printf( "--ERRO: pthread_join() \n" ); exit( -1 );
        }
    }

    //Escreve no final do arquivo de log a funcao que faz uma ultima verificacao no logica de leitor e escritor
    char palavra_final[25] = "tl.teste_final()\n";
    fprintf(pont_arquivo, "%s", palavra_final);
    
    fclose(pont_arquivo);

    pthread_mutex_destroy( &mutex );
    pthread_cond_destroy( &cond_leit );
    pthread_cond_destroy( &cond_leit2 );
    pthread_cond_destroy( &cond_escr );
    pthread_cond_destroy( &cond_escr2 );

    pthread_exit(NULL);

}
