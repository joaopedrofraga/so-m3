#include "SistemaArquivos.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>

using namespace std;

NoArvore* criar_arquivo_txt(const char* nome, const char* conteudo) {
    NoArvore* no = new NoArvore;
    no->nome = new char[strlen(nome) + 1];
    strncpy(no->nome, nome, strlen(nome) + 1);
    no->tipo = TIPO_ARQUIVO;

    no->dados.arquivo = new Arquivo;
    no->dados.arquivo->nome = no->nome;
    
    size_t tam_conteudo = strlen(conteudo);
    no->dados.arquivo->conteudo = new char[tam_conteudo + 1];
    strncpy(no->dados.arquivo->conteudo, conteudo, tam_conteudo + 1);
    no->dados.arquivo->tamanho = tam_conteudo;
    
    return no;
}

NoArvore* criar_diretorio(const char* nome, Diretorio* pai) {
    NoArvore* no = new NoArvore;
    no->nome = new char[strlen(nome) + 1];
    strcpy(no->nome, nome); // FIXME: usar strncpy aqui tbm
    no->tipo = TIPO_DIRETORIO;

    no->dados.diretorio = new Diretorio;
    no->dados.diretorio->arvore = arvoreB_criar(3); // t=3 como padrão
    no->dados.diretorio->pai = pai;
    
    return no;
}

void remover_arquivo_txt(Diretorio* dir_atual, const char* nome) {
    NoArvore* no_arquivo = arvoreB_buscar(dir_atual->arvore->raiz, nome);
    if (!no_arquivo || no_arquivo->tipo != TIPO_ARQUIVO) {
        cout << "Erro: Arquivo '" << nome << "' nao existe." << endl;
        return;
    }
    
    arvoreB_remover(dir_atual->arvore, nome);
    
    // AQUI É ONDE EU FALEI NA CALL QUE LIBERA A MEMORIA!!
    delete[] no_arquivo->dados.arquivo->conteudo;
    delete no_arquivo->dados.arquivo;
    delete[] no_arquivo->nome;
    delete no_arquivo; // Cuidado com double free aqui
    
    cout << "Arquivo '" << nome << "' foi removido." << endl;
}


// FUNCAO DE REMOVER O DIRETORIO (ESSA TA FUNCIONANDO)
void remover_diretorio(Diretorio* dir_atual, const char* nome) {
    NoArvore* no_dir = arvoreB_buscar(dir_atual->arvore->raiz, nome);
    if (!no_dir || no_dir->tipo != TIPO_DIRETORIO) {
        cout << "Erro: Diretorio '" << nome << "' nao encontrado." << endl;
        return;
    }

    // um diretorio so pode ser removido se estiver vazio.
    if (no_dir->dados.diretorio->arvore->raiz->n > 0) {
        cout << "Erro: O diretorio '" << nome << "' nao esta vazio." << endl;
        return;
    }
    
    arvoreB_remover(dir_atual->arvore, nome);
    
    delete no_dir->dados.diretorio->arvore;
    delete no_dir->dados.diretorio;
    delete[] no_dir->nome;
    delete no_dir;

    cout << "Diretorio '" << nome << "' removido." << endl;
}

void listar_conteudo_diretorio(Diretorio* dir) {
    cout << "Conteudo:" << endl;
    if (!dir->arvore->raiz || dir->arvore->raiz->n == 0) {
        // se n encontrar nada ele percorre a arvore
        cout << "  (diretorio vazio)" << endl;
    } else {
        arvoreB_percorrer(dir->arvore->raiz);
    }
}

void mudar_diretorio(Diretorio** dir_atual, const char* caminho) {
    if (strcmp(caminho, ".") == 0) {
        return; // ir para '.' não faz nada
    }

    // '..' é um caso especial
    if (strcmp(caminho, "..") == 0) {
        if ((*dir_atual)->pai != nullptr) {
            *dir_atual = (*dir_atual)->pai;
        }
        return;
    }

    NoArvore* no_alvo = arvoreB_buscar((*dir_atual)->arvore->raiz, caminho);
    if (no_alvo && no_alvo->tipo == TIPO_DIRETORIO) {
        *dir_atual = no_alvo->dados.diretorio;
    } else {
        cout << "Erro: Diretorio '" << caminho << "' nao existe neste local." << endl;
    }
}


// funcoes auxiliares para gerar o fs.img
void exportar_recursivo(ofstream& saida, Diretorio* dir, int profundidade);
void percorrer_e_exportar(ofstream& saida, NoArvoreB* no, int profundidade);

void percorrer_e_exportar(ofstream& saida, NoArvoreB* no, int profundidade) {
    int i;
    for (i = 0; i < no->n; i++) {
        if (no->folha == false) {
            percorrer_e_exportar(saida, no->C[i], profundidade);
        }

        for (int d = 0; d < profundidade; ++d) saida << "  ";
        saida << "|-- " << no->chaves[i]->nome;

        if (no->chaves[i]->tipo == TIPO_DIRETORIO) {
            saida << "/" << endl;
            exportar_recursivo(saida, no->chaves[i]->dados.diretorio, profundidade + 1);
        } else {
            saida << endl;
        }
    }
    if (no->folha == false) {
        percorrer_e_exportar(saida, no->C[i], profundidade);
    }
}

void exportar_recursivo(ofstream& saida, Diretorio* dir, int profundidade) {
    if (dir && dir->arvore && dir->arvore->raiz) {
        percorrer_e_exportar(saida, dir->arvore->raiz, profundidade);
    }
}

void exportar_sistema_para_imagem(Diretorio* raiz, const string& nome_arquivo) {
    ofstream arquivo_saida(nome_arquivo.c_str());
    if (!arquivo_saida.is_open()) {
        cerr << "PANIC: Nao consegui criar o " << nome_arquivo << endl;
        return;
    }
    arquivo_saida << "/" << endl;
    exportar_recursivo(arquivo_saida, raiz, 0);
    arquivo_saida.close();
}

void remover_de_folha(NoArvoreB* no, int idx);
void remover_de_nao_folha(NoArvoreB* no, int idx);
NoArvore* get_pred(NoArvoreB* no, int idx);
NoArvore* get_succ(NoArvoreB* no, int idx);
void preencher_filho(NoArvoreB* no, int idx);
void pegar_do_anterior(NoArvoreB* no, int idx);
void pegar_do_proximo(NoArvoreB* no, int idx);
void fundir_filhos(NoArvoreB* no, int idx);
void remover_interno(NoArvoreB* no, const char* nome);
int achar_chave(NoArvoreB* no, const char* nome);

ArvoreB* arvoreB_criar(int t) {
    ArvoreB* arvore = new ArvoreB;
    arvore->t = t;
    NoArvoreB* raiz = new NoArvoreB;
    raiz->t = t;
    raiz->folha = true;
    raiz->n = 0;
    raiz->chaves = new NoArvore*[2*t - 1];
    raiz->C = new NoArvoreB*[2*t];
    arvore->raiz = raiz;
    return arvore;
}

void arvoreB_percorrer(NoArvoreB* no) {
    int i;
    for (i = 0; i < no->n; i++) {
        if (!no->folha) arvoreB_percorrer(no->C[i]);
        cout << "  " << no->chaves[i]->nome;
        if (no->chaves[i]->tipo == TIPO_DIRETORIO) cout << "/";
        cout << endl;
    }
    if (!no->folha) arvoreB_percorrer(no->C[i]);
}

NoArvore* arvoreB_buscar(NoArvoreB* no, const char* nome) {
    if (!no) return nullptr;
    int i = 0;
    while (i < no->n && strcmp(nome, no->chaves[i]->nome) > 0) i++;
    if (i < no->n && strcmp(nome, no->chaves[i]->nome) == 0) return no->chaves[i];
    if (no->folha) return nullptr;
    return arvoreB_buscar(no->C[i], nome);
}

void arvoreB_dividir_filho(NoArvoreB* x, int i, NoArvoreB* y) {
    NoArvoreB* z = new NoArvoreB;
    z->t = y->t; z->folha = y->folha; z->n = y->t - 1;
    z->chaves = new NoArvore*[2*z->t - 1];
    z->C = new NoArvoreB*[2*z->t];
    for(int j = 0; j < y->t - 1; j++) z->chaves[j] = y->chaves[j + y->t];
    if (!y->folha) { for (int j=0; j < y->t; j++) z->C[j] = y->C[j + y->t]; }
    y->n = y->t - 1;
    for (int j = x->n; j >= i + 1; j--) x->C[j + 1] = x->C[j];
    x->C[i+1] = z;
    for (int j=x->n-1; j >= i; j--) x->chaves[j + 1] = x->chaves[j];
    x->chaves[i] = y->chaves[y->t - 1];
    x->n = x->n + 1;
}

void arvoreB_inserir_nao_cheio(NoArvoreB* x, NoArvore* k) {
    int i = x->n - 1;
    if (x->folha) {
        while (i>=0 && strcmp(x->chaves[i]->nome, k->nome) > 0) {
            x->chaves[i+1] = x->chaves[i];
            i--;
        }
        x->chaves[i+1] = k;
        x->n = x->n+1;
    } else {
        while (i >= 0 && strcmp(x->chaves[i]->nome, k->nome) > 0) i--;
        if (x->C[i + 1]->n == 2*x->t - 1) {
            arvoreB_dividir_filho(x, i + 1, x->C[i+1]);
            if (strcmp(k->nome, x->chaves[i+1]->nome) > 0) i++;
        }
        arvoreB_inserir_nao_cheio(x->C[i+1], k);
    }
}

void arvoreB_inserir(ArvoreB* arvore, NoArvore* no) {
    NoArvoreB* r = arvore->raiz;
    if (r->n == 2*arvore->t-1) {
        NoArvoreB* s = new NoArvoreB;
        s->t = arvore->t; s->folha = false; s->n = 0;
        s->C = new NoArvoreB*[2*arvore->t];
        s->chaves = new NoArvore*[2*arvore->t - 1];
        arvore->raiz = s;
        s->C[0] = r;
        arvoreB_dividir_filho(s, 0, r);
        arvoreB_inserir_nao_cheio(s, no);
    } else {
        arvoreB_inserir_nao_cheio(r, no);
    }
}

int achar_chave(NoArvoreB* no, const char* nome) {
    int idx=0;
    while(idx < no->n && strcmp(no->chaves[idx]->nome, nome) < 0) ++idx;
    return idx;
}

void arvoreB_remover(ArvoreB* arvore, const char* nome) {
    if (!arvore || !arvore->raiz) return;
    remover_interno(arvore->raiz, nome);
    if (arvore->raiz->n==0 && !arvore->raiz->folha) {
        NoArvoreB* temp = arvore->raiz;
        arvore->raiz = arvore->raiz->C[0];
        delete[] temp->chaves; delete[] temp->C; delete temp;
    }
}

void remover_interno(NoArvoreB* no, const char* nome) {
    int idx = achar_chave(no, nome);
    if (idx < no->n && strcmp(no->chaves[idx]->nome, nome) == 0) {
        if(no->folha) remover_de_folha(no, idx);
        else remover_de_nao_folha(no, idx);
    } else {
        if (no->folha) return;
        bool flag = (idx == no->n);
        if (no->C[idx]->n < no->t) preencher_filho(no, idx);
        if (flag && idx > no->n) remover_interno(no->C[idx-1], nome);
        else remover_interno(no->C[idx], nome);
    }
}

void remover_de_folha(NoArvoreB* no, int idx) {
    for (int i=idx+1; i < no->n; ++i) no->chaves[i-1] = no->chaves[i];
    no->n--;
}

void remover_de_nao_folha(NoArvoreB* no, int idx) {
    NoArvore* k = no->chaves[idx];
    if (no->C[idx]->n >= no->t) {
        NoArvore* pred = get_pred(no, idx);
        no->chaves[idx] = pred;
        remover_interno(no->C[idx], pred->nome);
    } else if (no->C[idx+1]->n >= no->t) {
        NoArvore* succ = get_succ(no, idx);
        no->chaves[idx] = succ;
        remover_interno(no->C[idx+1], succ->nome);
    } else {
        fundir_filhos(no, idx);
        remover_interno(no->C[idx], k->nome);
    }
}

NoArvore* get_pred(NoArvoreB* no, int idx) {
    NoArvoreB* cur = no->C[idx];
    while (!cur->folha) cur = cur->C[cur->n];
    return cur->chaves[cur->n-1];
}

NoArvore* get_succ(NoArvoreB* no, int idx) {
    NoArvoreB* cur = no->C[idx+1];
    while(!cur->folha) cur = cur->C[0];
    return cur->chaves[0];
}

void preencher_filho(NoArvoreB* no, int idx) {
    if (idx!=0 && no->C[idx-1]->n >= no->t) pegar_do_anterior(no, idx);
    else if (idx!=no->n && no->C[idx+1]->n >= no->t) pegar_do_proximo(no, idx);
    else {
        if(idx != no->n) fundir_filhos(no, idx);
        else fundir_filhos(no, idx-1);
    }
}

void pegar_do_anterior(NoArvoreB* no, int idx) {
    NoArvoreB* child=no->C[idx]; NoArvoreB* sibling=no->C[idx-1];
    for (int i=child->n-1; i>=0; --i) child->chaves[i+1] = child->chaves[i];
    if(!child->folha){for(int i=child->n; i>=0; --i) child->C[i+1] = child->C[i];}
    child->chaves[0] = no->chaves[idx-1];
    if(!child->folha) child->C[0] = sibling->C[sibling->n];
    no->chaves[idx-1] = sibling->chaves[sibling->n-1];
    child->n++; sibling->n--;
}

void pegar_do_proximo(NoArvoreB* no, int idx) {
    NoArvoreB* child=no->C[idx]; NoArvoreB* sibling=no->C[idx+1];
    child->chaves[child->n] = no->chaves[idx];
    if(!(child->folha)) child->C[child->n + 1] = sibling->C[0];
    no->chaves[idx] = sibling->chaves[0];
    for(int i=1; i<sibling->n; ++i) sibling->chaves[i-1] = sibling->chaves[i];
    if(!sibling->folha){for(int i=1; i<=sibling->n; ++i) sibling->C[i-1] = sibling->C[i];}
    child->n++; sibling->n--;
}

void fundir_filhos(NoArvoreB* no, int idx) {
    // printf("Fundindo no %s no indice %d\n", no->chaves[idx]->nome, idx);
    NoArvoreB* child=no->C[idx]; NoArvoreB* sibling=no->C[idx+1];
    child->chaves[no->t-1] = no->chaves[idx];
    for(int i=0; i<sibling->n; ++i) child->chaves[i+no->t] = sibling->chaves[i];
    if(!child->folha){for(int i=0; i<=sibling->n; ++i) child->C[i+no->t] = sibling->C[i];}
    for(int i=idx+1; i<no->n; ++i) no->chaves[i-1] = no->chaves[i];
    for(int i=idx+2; i<=no->n; ++i) no->C[i-1] = no->C[i];
    child->n += sibling->n+1;
    no->n--;
    delete sibling; // irmao foi mesclado, pode deletar
}