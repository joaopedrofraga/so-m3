#include "SistemaArquivos.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm> // reverse

using namespace std;

void imprimir_ajuda() {
    cout << "--- Simulador de FS (M3 SO) ---" << endl;
    cout << "Comandos:" << endl;
    cout << "  ls              - Lista conteudo do diretorio." << endl;
    cout << "  cd <dir>        - Muda de diretorio. '..' para voltar." << endl;
    cout << "  mkdir <dir>     - Cria um diretorio." << endl;
    cout << "  write <arq.txt> - Cria/sobrescreve um arquivo." << endl;
    cout << "  cat <arq.txt>   - Mostra conteudo de um arquivo." << endl;
    cout << "  rm <arq.txt>    - Remove um arquivo." << endl;
    cout << "  rmdir <dir>     - Remove um diretorio (se vazio)." << endl;
    cout << "  help            - Mostra esta ajuda." << endl;
    cout << "  exit            - Sai e salva em 'fs.img'." << endl;
    cout << "---------------------------------" << endl;
}

string obter_caminho_atual(Diretorio* dir) {
    if (dir->pai == nullptr) return "/";

    vector<string> partes;
    Diretorio* dir_temp = dir;

    while (dir_temp->pai != nullptr) {
        Diretorio* pai = dir_temp->pai;
        string nome_dir_temp = "";
        
        // achra o nome do diretorio atual dentro do diretorio pai
        NoArvoreB* no_pai = pai->arvore->raiz;
        for (int i = 0; i < no_pai->n; ++i) {
            if (no_pai->chaves[i]->tipo == TIPO_DIRETORIO && no_pai->chaves[i]->dados.diretorio == dir_temp) {
                nome_dir_temp = no_pai->chaves[i]->nome;
                break;
            }
        }
        if (!nome_dir_temp.empty()) {
            partes.push_back(nome_dir_temp);
        }
        dir_temp = pai;
    }

    if (partes.empty()) return "/";

    reverse(partes.begin(), partes.end());
    
    stringstream caminho_ss;
    caminho_ss << "/";
    for (size_t i = 0; i < partes.size(); ++i) {
        caminho_ss << partes[i] << (i == partes.size() - 1 ? "" : "/");
    }

    return caminho_ss.str();
}

int main() {
    Diretorio* raiz = new Diretorio;
    raiz->pai = nullptr;
    raiz->arvore = arvoreB_criar(3);

    Diretorio* dir_atual = raiz;
    string linha;
    imprimir_ajuda();

    // p ficar rodando até o fim da exec
    while (true) {
        cout << obter_caminho_atual(dir_atual) << " > ";
        if (!getline(cin, linha)) {
            break; // EOF 
        }

        if (linha.empty()) continue;

        stringstream ss(linha);
        string comando, arg1;
        ss >> comando >> arg1;

        if (comando == "exit") {
            exportar_sistema_para_imagem(raiz, "fs.img");
            cout << "Sistema de arquivos salvo. Ate mais!" << endl;
            break;
        } 
        else if (comando == "help") {
            imprimir_ajuda();
        } 
        else if (comando == "ls") {
            listar_conteudo_diretorio(dir_atual);
        } 
        else if (comando == "mkdir") {
            if (arg1.empty()) cout << "Uso: mkdir <nome_do_diretorio>" << endl;
            else if (arvoreB_buscar(dir_atual->arvore->raiz, arg1.c_str())) {
                cout << "Erro: '" << arg1 << "' ja existe aqui." << endl;
            } else {
                NoArvore* novo_dir = criar_diretorio(arg1.c_str(), dir_atual);
                arvoreB_inserir(dir_atual->arvore, novo_dir);
                cout << "Diretorio '" << arg1 << "' criado com sucesso." << endl;
            }
        } 
        else if (comando == "write") {
            if (arg1.empty()) { cout << "Uso: write <nome_do_arquivo.txt>" << endl; continue; }
            
            if (arvoreB_buscar(dir_atual->arvore->raiz, arg1.c_str())) {
                remover_arquivo_txt(dir_atual, arg1.c_str());
            }
            
            cout << "Digite o texto. Para terminar, digite EOF numa linha nova." << endl;
            string conteudo, linha_conteudo;
            while (getline(cin, linha_conteudo) && linha_conteudo != "EOF") {
                conteudo += linha_conteudo + "\n";
            }
            
            NoArvore* novo_arq = criar_arquivo_txt(arg1.c_str(), conteudo.c_str());
            arvoreB_inserir(dir_atual->arvore, novo_arq);
            cout << "Arquivo '" << arg1 << "' salvo." << endl;
            
        } 
        else if (comando == "cat") {
            if (arg1.empty()) { cout << "Uso: cat <nome_do_arquivo.txt>" << endl; }
            else {
                 NoArvore* no = arvoreB_buscar(dir_atual->arvore->raiz, arg1.c_str());
                 if(no && no->tipo == TIPO_ARQUIVO) {
                    cout << "--- " << arg1 << " ---" << endl;
                    cout << no->dados.arquivo->conteudo;
                    cout << "-----------------" << endl;
                 } else {
                    cout << "Erro: arquivo '" << arg1 << "' nao encontrado." << endl;
                 }
            }
        }
        else if (comando == "cd") {
            if (arg1.empty()) { cout << "Uso: cd <nome_do_diretorio>" << endl; } 
            else { mudar_diretorio(&dir_atual, arg1.c_str()); }
        } 
        else if (comando == "rm") {
            if (arg1.empty()) { cout << "Uso: rm <nome_do_arquivo.txt>" << endl; }
            else { remover_arquivo_txt(dir_atual, arg1.c_str()); }
        } 
        else if (comando == "rmdir") {
            if (arg1.empty()) { cout << "Uso: rmdir <nome_do_diretorio>" << endl; }
            else { remover_diretorio(dir_atual, arg1.c_str()); }
        } 
        else {
            cout << "Comando '" << comando << "' invalido. Use 'help'." << endl;
        }
    }
    
    delete raiz->arvore;
    delete raiz;

    cout << "Saindo do sistema de arquivos.";

    return 0;
}