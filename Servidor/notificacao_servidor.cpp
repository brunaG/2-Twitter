#include "notificacao_servidor.hpp"

GerenciaNotificacao::GerenciaNotificacao(){
    InicializaEstruturaNotificacao();
}

void GerenciaNotificacao::defineID(int id){
    server_ID = id;
}

int GerenciaNotificacao::NovoSeguidor(string perfil, string seguidor){
    // Inclui um novo seguidor -> insere novo seguidor no arquivo do perfil
    // Adiciona este seguidor na estrutura de notificações deste perfil
    ifstream inFile;
    string caminho = DEFAULTPATH + to_string(server_ID) + "/";
    string formato = ".txt";
    int retorno, flag_busca;

    /*
        TODO evitar que o .open crie um arquivo novo para um usuario nao existente
    */
    inFile.open(caminho + perfil + formato, std::ios::app);
    // if(inFile == 0){
    //     cout << "PERFIL NAO EXISTENTE" << endl;
    //     return -1;
    // }

    flag_busca = ProcuraSeguidor(caminho + perfil + formato, seguidor);

    if(flag_busca == 0){

        retorno = adicionaLinhaArquivo(caminho + perfil + formato, seguidor);

        if(retorno != 0){
            cout << "---------- ERRO AO ESCREVER NO ARQUIVO ----------";
            return -1;
        }

    }

    NovoSeguidorNotificacao(perfil, seguidor);

    return 0;
}

void GerenciaNotificacao::NovoPerfil(string perfil){
    // Para novos usuários.
    // Cria um arquivo novo, caso não exista este perfil.
    // Cria nova instância de notification para o novo perfil.

    ifstream inFile;
    string caminho = DEFAULTPATH + to_string(server_ID) + "/";
    string formato = ".txt";

    char path[caminho.size()+1];
    strcpy(path, caminho.c_str());

    struct stat st = {0};
    if (stat(path, &st) == -1){
        mkdir(path, 0700);
    }
    inFile.open(caminho + perfil + formato, std::ios::out | std::ios::app);
    NovoPerfilNotificacao(perfil);
}

void GerenciaNotificacao::RemoveTweet(int indice_not, int indice_tweet){

    // Verifica se o próximo tweet é NULL. Se sim, só remover
    if(notification[indice_not].tweet[indice_tweet+1].payload == "NULL"){
        notification[indice_not].tweet[indice_tweet].type = 0;
        notification[indice_not].tweet[indice_tweet].seqn = 0;
        notification[indice_not].tweet[indice_tweet].length = 0;
        notification[indice_not].tweet[indice_tweet].timestamp = 0;
        notification[indice_not].tweet[indice_tweet].envios_faltantes = 0;
        notification[indice_not].tweet[indice_tweet].owner = "NULL";
        notification[indice_not].tweet[indice_tweet].payload = "NULL";
    }
    else{   // Puxa todos os tweets uma posição anterior
        int n = 0;
        while(notification[indice_not].tweet[indice_tweet+n].payload != "NULL"){
            notification[indice_not].tweet[indice_tweet+n].type = notification[indice_not].tweet[indice_tweet+n+1].type;
            notification[indice_not].tweet[indice_tweet+n].seqn = notification[indice_not].tweet[indice_tweet+n+1].seqn;
            notification[indice_not].tweet[indice_tweet+n].length = notification[indice_not].tweet[indice_tweet+n+1].length;
            notification[indice_not].tweet[indice_tweet+n].timestamp = notification[indice_not].tweet[indice_tweet+n+1].timestamp;
            notification[indice_not].tweet[indice_tweet+n].envios_faltantes = notification[indice_not].tweet[indice_tweet+n+1].envios_faltantes;
            notification[indice_not].tweet[indice_tweet+n].owner = notification[indice_not].tweet[indice_tweet+n+1].owner;
            notification[indice_not].tweet[indice_tweet+n].payload = notification[indice_not].tweet[indice_tweet+n+1].payload;

            n++;
        }

    }
}

int GerenciaNotificacao::EnviaTweet(packet* tweet, string perfil){
    // Procura @perfil nas filas de not pendentes
    // Quando encontra o 1o, salva o tweet. Quando encontra um novo, verifica se é mais antigo que este. Se for, substitui na struct
    // No final, tem-se o notification e o índice do tweet mais antigo
    // Com isto, envia o tweet e o remove da fila respectiva
    int indice_not = 0, qtde_fila = 0;
    int not_final = TWEETS, seqn_final = TWEETS, tweet_final = TWEETS, indice_fila_final;
    int i, k, j, seqn_temp;

    // Pega a quantidade de notifications
    while(notification[indice_not].perfil != "NULL"){
        indice_not++;
    }

    // Percorre a fila de cada notification, procurando o @perfil na fila
    i=0;
    while(i < indice_not){

        while(notification[i].fila_notificacoes[qtde_fila].perfil != "NULL"){
            qtde_fila++;
        }

        // Percorre os tweets deste notification
        j=0;
        while(j < qtde_fila){

            // Se é o 1o tweet pendente para o perfil, salva
            if(not_final == TWEETS){

                if(notification[i].fila_notificacoes[j].perfil == perfil){
                    not_final = i;
                    seqn_final = notification[i].fila_notificacoes[j].seqn;
                    indice_fila_final = j;
                    tweet_final = 0;
                    while(notification[not_final].tweet[tweet_final].seqn != seqn_final){
                        tweet_final++;
                    }

                }
            }

            else{ // Procura o timestamp do tweet atual achado da fila

                if(notification[i].fila_notificacoes[j].perfil == perfil){
                    seqn_temp = notification[i].fila_notificacoes[j].seqn;

                    k = 0;
                    while(notification[i].tweet[k].seqn != seqn_temp){
                        k++;
                    }

                    // notification[i].tweet[k] é o tweet atual a ser comparado
                    // notification[not_final].tweet[tweet_final] é o tweet mais antigo

                    if(notification[i].tweet[k].timestamp < notification[not_final].tweet[tweet_final].timestamp){
                        not_final = i;
                        tweet_final = k;
                        seqn_final = notification[i].tweet[k].seqn;
                        indice_fila_final = j;
                    }

                }

            }

            j++;

        }


        i++;

    }
    // No final do laço acima, temos:
    // not_final = índice notification do tweet a ser enviado
    // tweet_final = índice do tweet no notification[not_final] a ser enviado
    cout << "****TWEETS****: " << TWEETS << endl;
    cout << "****not_final****: " << not_final << endl;

    if(not_final == TWEETS){
        return -1;
    }
    else{

        cout << "\n\n-- VERIFICAÇÃO --\n";
        cout << "\n\nÍndice not do tweet mais antigo de @perfil:\n";
        cout << not_final;
        cout << "\nÍndice tweet mais antigo de @perfil:\n";
        cout << tweet_final;


        // Concatena para na msg quem escreveu o tweet
        string nova_msg;

        nova_msg = notification[not_final].tweet[tweet_final].owner;
        nova_msg += ": ";
        nova_msg += notification[not_final].tweet[tweet_final].payload;

        tweet->type = notification[not_final].tweet[tweet_final].type;
        tweet->seqn = notification[not_final].tweet[tweet_final].seqn;
        tweet->length = notification[not_final].tweet[tweet_final].length;
        tweet->timestamp = notification[not_final].tweet[tweet_final].timestamp;
        strcpy(tweet->payload, nova_msg.c_str());

        RemoveFilaPendentes(not_final, indice_fila_final);

        return 0;
    }
}

void GerenciaNotificacao::RemoveFilaPendentes(int indice_not, int indice_notfila){
    // Quando uma notificação é enviada para o Cliente
    // Remove a notificação da fila de notificações pendentes.
    // Decrementa os envios faltantes do tweet em questão.

    int i = 0, seqn;

    // Pega o id do tweet, para achar no vetor de tweets:
    seqn = notification[indice_not].fila_notificacoes[indice_notfila].seqn;

    // Pega o índice do tweet, para decrementar seu contador de envios
    while(notification[indice_not].tweet[i].seqn != seqn){
        i++;
    }

    // Verifica se o próximo da fila é NULL. Se sim, só remover a notificação da fila
    if(notification[indice_not].fila_notificacoes[indice_notfila+1].perfil == "NULL"){
        notification[indice_not].fila_notificacoes[indice_notfila].perfil = "NULL";
        notification[indice_not].fila_notificacoes[indice_notfila].seqn = 0;

        notification[indice_not].tweet[i].envios_faltantes--;
    }
    else{   // Puxa todos os tweets uma posição anterior
        int n = 0;
        while(notification[indice_not].fila_notificacoes[indice_notfila+n].perfil != "NULL"){
            notification[indice_not].fila_notificacoes[indice_notfila+n].perfil = notification[indice_not].fila_notificacoes[indice_notfila+n+1].perfil;
            notification[indice_not].fila_notificacoes[indice_notfila+n].seqn = notification[indice_not].fila_notificacoes[indice_notfila+n+1].seqn;
            n++;
        }
        notification[indice_not].tweet[i].envios_faltantes--;
    }

    if(notification[indice_not].tweet[i].envios_faltantes == 0){
        RemoveTweet(indice_not,i);
    }

}

int GerenciaNotificacao::adicionaLinhaArquivo(string filepath, string line){
    // Função para escrever na última linha do arquivo.

    std::ofstream file;

    file.open(filepath, std::ios::out | std::ios::app);

    if(file.fail()){
        return -1;
    }

    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

    file << line << std::endl;

    return 0;
}

int GerenciaNotificacao::ProcuraSeguidor(string caminho, string perfil_procurado){
    // Retorna uma flag indicando se o seguidor já está no arquivo.
    // Se já existe = retorna 1. Senão, 0.

    // Função usada para não incluir um seguidor já existente no arquivo.

    string linha, perfil;
    int tamanho_perfil, i, flag_igual;

    tamanho_perfil = perfil_procurado.size();

    ifstream inFile(caminho, ios::in);

    if(!inFile){
        cout << "---------- ERRO AO ABRIR ARQUIVO PARA LEITURA ----------\n";
        return -1;
    }
    else{
        while(!inFile.eof()){
            getline(inFile,linha);

            for(i=0; i<tamanho_perfil; i++){
                perfil[i] = linha[i];
            }
            perfil[tamanho_perfil] = '\0';

            flag_igual = 1;
            for(i=0; i<tamanho_perfil; i++){
                if(perfil[i] != perfil_procurado[i]){
                    flag_igual = 0;
                }
            }

            if(flag_igual == 1)
                return flag_igual;
        }

        inFile.close();
        return flag_igual;


    }

}

void GerenciaNotificacao::DefineTamanhoPacote(struct EDPacote* tweet){
    // Seta o tamanho da mensagem do tweet.
    // Não utilizada: este valor já vem do client.

    tweet->length = tweet->payload.size();
}

void GerenciaNotificacao::DefineEnviosPendentes(struct EDPacote* tweet, struct notification* notif){
    // Seta a quantidade total de envios que um tweet terá:
    //           quantidade é o número de seguidores deste perfil;

    int num_seguidores = 0;
    while(notif->seguidores[num_seguidores].compare("\0") != 0){
        num_seguidores++;
    };
    tweet->envios_faltantes = num_seguidores;
}

int GerenciaNotificacao::AdicionaFilaPendentes(int indice_not, int id_tweet){
    // Quando é recebido um novo tweet do perfil, a fila de notificações
    //pendentes é atualizada:
    // Inclui-se [id tweet, seguidor1], [id tweet, seguidor...], [id tweet, seguidorn]

    int i = 0, indice_fila=0, num_seguidores = 0;

    // Busca número de seguidores: vai ser a qntd de notificações pendentes
    while(notification[indice_not].seguidores[num_seguidores] != "NULL"){
        num_seguidores++;

        if(num_seguidores == PERFIS){
            cout << "---------- PERFIL NÃO CADASTRADO ----------\n";
            return -1;
        }
    }


    // Pega a primeira posição vazia da fila de notificações pendentes
    if(notification[indice_not].fila_notificacoes[0].perfil == "NULL"){
        indice_fila = 0;

    }
    else{

        while(notification[indice_not].fila_notificacoes[indice_fila].perfil != "NULL"){
            indice_fila++;
            if(indice_fila == 300){
                cout << "---------- ERRO NA FILA DE NOTIFICAÇÕES ----------\n";
                return -1;
            }

        }
    }

    // Preenche as notificações pendentes
    for(i=0; i<num_seguidores; i++){
        notification[indice_not].fila_notificacoes[indice_fila].seqn = id_tweet;
        notification[indice_not].fila_notificacoes[indice_fila].perfil = notification[indice_not].seguidores[i];
        indice_fila++;
    }

    return 0;

}

int GerenciaNotificacao::AdicionaTweet(struct EDPacote* tweet){
    // Para quando é recebido um tweet de um perfil do Cliente.
    // Adiciona o tweet do seu respectivo perfil na estrutura de notificações.
    // Chama a atualização da fila de notificações.

    string perfil;
    int i = 0, n = 0, num_seguidores = 0;

    perfil = tweet->owner;
    while(notification[i].perfil != perfil){
        i++;

        if(i == PERFIS){
            cout << "---------- PERFIL NÃO CADASTRADO ----------\n";
            return -1;
        }

    }



    while(notification[i].seguidores[num_seguidores] != "NULL"){
        num_seguidores++;
    }

    if(notification[i].tweet[0].payload == "NULL"){

        notification[i].tweet[0].type = tweet->type;
        notification[i].tweet[0].seqn = tweet->seqn;
        notification[i].tweet[0].length = tweet->length;
        notification[i].tweet[0].timestamp = tweet->timestamp;
        notification[i].tweet[0].owner = tweet->owner;
        notification[i].tweet[0].envios_faltantes = num_seguidores;  // ALTERAR!!!
        notification[i].tweet[0].payload = tweet->payload;
    }
    else{

        while(notification[i].tweet[n].payload != "NULL"){
            n++;
        }

        notification[i].tweet[n].type = tweet->type;
        notification[i].tweet[n].seqn = tweet->seqn;
        notification[i].tweet[n].length = tweet->length;
        notification[i].tweet[n].timestamp = tweet->timestamp;
        notification[i].tweet[n].envios_faltantes = num_seguidores;  // ALTERAR!!!
        notification[i].tweet[n].payload = tweet->payload;
        notification[i].tweet[n].owner = tweet->owner;

    }
    cout << "Notificacao: " << notification[i].tweet[n].payload << endl;
    AdicionaFilaPendentes(i, notification[i].tweet[n].seqn);
    return 0;

}

int GerenciaNotificacao::NovoPerfilNotificacao(string perfil){
    // Adiciona um perfil na estrutura de notificações.

    string linha;
    string caminho = DEFAULTPATH + to_string(server_ID) + "/";
    string formato = ".txt";

    int i = 0, n = 0;

    while(notification[i].perfil != "NULL"){
        i++;
    }
    notification[i].perfil = perfil;


    ifstream inFile(caminho + perfil + formato);
    if(!inFile){
        cout << "---------- ERRO AO ABRIR ARQUIVO PARA LEITURA ----------\n";
        return -1;
    }
    else{
        while(inFile >> linha){
            notification[i].seguidores[n] = linha;
            n++;
        }
    }

    return 0;
}

int GerenciaNotificacao::NovoSeguidorNotificacao(string perfil, string seguidor){
    // Adiciona um seguidor ao perfil na estrutura de notificações.

    int indice_not = 0, indice_seg = 0, ja_existe = 0;

    // Procura índice do perfil
    while(notification[indice_not].perfil != perfil){
        indice_not++;

        if(indice_not == PERFIS){
            cout << "---------- PERFIL NÃO CADASTRADO ----------\n";
            return -1;
        }
    }

    // Procura primeiro índice livre dos seguidores
    while(notification[indice_not].seguidores[indice_seg] != "NULL"){
        if(notification[indice_not].seguidores[indice_seg] == seguidor){
            ja_existe = 1;
        }

        indice_seg++;

        if(indice_seg == SEGUIDORES){
            cout << "---------- ERRO VARREDURA DOS SEGUIDORES DO PERFIL ----------\n";
            return -1;
        }

    }

    if(ja_existe == 0){
        notification[indice_not].seguidores[indice_seg] = seguidor;
    }

    return 0;

}

void GerenciaNotificacao::InicializaEstruturaNotificacao(){
    // Inicialização do Módulo de Notificações

    int i, n, k, z;

    for(i=0; i<PERFIS;i++){
        notification[i].perfil = "NULL";

        for(n=0; n<SEGUIDORES; n++){
            notification[i].seguidores[n] = "NULL";
        }

        for(k=0;k<TWEETS; k++){
            notification[i].tweet[k].type = 0;
            notification[i].tweet[k].seqn = 0;
            notification[i].tweet[k].length = 0;
            notification[i].tweet[k].timestamp = 0;
            notification[i].tweet[k].envios_faltantes = 0;
            notification[i].tweet[k].owner = "NULL";
            notification[i].tweet[k].payload = "NULL";
        }

        for(z=0; z<NOTPENDENTES; z++){
            notification[i].fila_notificacoes[z].seqn = 0;
            notification[i].fila_notificacoes[z].perfil = "NULL";
        }

    }

}

