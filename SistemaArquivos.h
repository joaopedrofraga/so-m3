#ifndef SISTEMA_ARQUIVOS_H
#define SISTEMA_ARQUIVOS_H

#include <string>

using namespace std;

// para diferenciar arquivos de diretorios na arvore (so p facilitar)
typedef enum { 
    TIPO_ARQUIVO, 
    TIPO_DIRETORIO 
} TipoNo;

typedef struct Arquivo {
    char* nome;
    char* conteudo;
    size_t tamanho;
} Arquivo;

struct Diretorio;
struct NoArvoreB;

// estrutura principal, pode ser arq ou dir (a union economiza espaço)
typedef struct NoArvore {
    char* nome;
    TipoNo tipo;
    union {
        Arquivo* arquivo;
        Diretorio* diretorio;
    } dados;
} NoArvore;

// no da arvore b
typedef struct NoArvoreB {
    NoArvore** chaves;  // ponteiros para os Nos (arq/dir)
    int t;      
    NoArvoreB** C;      // ponteiros para os filhos
    int n;             // numero de chaves atual
    bool folha;
} NoArvoreB;

typedef struct ArvoreB {
    NoArvoreB* raiz;
    int t;
} ArvoreB;

struct Diretorio {
    ArvoreB* arvore;
    Diretorio* pai;
};


// --- FUNCOES DA ARVORE B ---
ArvoreB* arvoreB_criar(int t);
void arvoreB_percorrer(NoArvoreB* no);
NoArvore* arvoreB_buscar(NoArvoreB* no, const char* nome);
void arvoreB_inserir(ArvoreB* arvore, NoArvore* no);
void arvoreB_remover(ArvoreB* arvore, const char* nome);

// --- FUNCOES DO SISTEMA DE ARQUIVOS ---
NoArvore* criar_arquivo_txt(const char* nome, const char* conteudo);
void remover_arquivo_txt(Diretorio* dir_atual, const char* nome);
NoArvore* criar_diretorio(const char* nome, Diretorio* pai);
void remover_diretorio(Diretorio* dir_atual, const char* nome);

void listar_conteudo_diretorio(Diretorio* dir);
void mudar_diretorio(Diretorio** dir_atual, const char* caminho); // ponteiro duplo pra modificar o original

// EXPORTAR O RESULTADO
void exportar_sistema_para_imagem(Diretorio* raiz, const string& nome_arquivo);

#endif