#include "notificacao.hpp"

Notificacoes::Notificacoes(){
    buffer_size = 256;

    sem_init(&buffer_to_send_empty, 0, buffer_size);
    sem_init(&buffer_to_send_full, 0, 0);
    sem_init(&mutex_p_send, 0, 1);
    sem_init(&mutex_c_send, 0, 1);
    in_send = 0;
    out_send = 0;
    in_receive = 0;
    out_receive = 0;

    sem_init(&buffer_to_receive_empty, 0, buffer_size);
    sem_init(&buffer_to_receive_full, 0, 0);
    sem_init(&mutex_p_receive, 0, 1);
    sem_init(&mutex_c_receive, 0, 1);

    new_notification = false;
    new_packet_to_send = false;

}

void Notificacoes::AtualizaBufferEnviar(uint16_t type, uint16_t seqn, uint16_t length, int16_t timestamp, char* payload){
    packet message;

    sem_wait(&buffer_to_send_empty);
    sem_wait(&mutex_p_send);

    message.type = type;
    message.length = length;
    message.seqn = seqn;
    message.timestamp = timestamp;
    strcpy(message.payload, payload);

    buffer_to_send[in_send] = message;
    in_send = (in_send + 1) % buffer_size;
    new_packet_to_send = true;

    sem_post(&mutex_p_send);
    sem_post(&buffer_to_send_full);

}

void Notificacoes::AtualizaBufferReceber(uint16_t type, uint16_t seqn, uint16_t length, int16_t timestamp, char* payload){
    packet message;

    sem_wait(&buffer_to_receive_empty);
    sem_wait(&mutex_p_receive);

    message.type = type;
    message.length = length;
    message.seqn = seqn;
    message.timestamp = timestamp;
    strcpy(message.payload, payload);

    buffer_to_receive[in_receive] = message;
    in_receive = (in_receive + 1) % buffer_size;
    new_notification = true;

    sem_post(&mutex_p_receive);
    sem_post(&buffer_to_receive_full);
}

void Notificacoes::ColetaNotificacoesEnviar(packet* message){

    sem_wait(&buffer_to_send_full);
    sem_wait(&mutex_c_send);

    message->type = buffer_to_send[out_send].type;
    message->length = buffer_to_send[out_send].length;
    message->seqn = buffer_to_send[out_send].seqn;
    message->timestamp = buffer_to_send[out_send].timestamp;
    strcpy(message->payload, buffer_to_send[out_send].payload);
    out_send = (out_send + 1) % buffer_size;
    if (out_send == 0){
        new_packet_to_send = false;
    }

    sem_post(&mutex_c_send);
    sem_post(&buffer_to_send_empty);
}

void Notificacoes::ColetaNotificacoesReceber(packet* message){

    sem_wait(&buffer_to_receive_full);
    sem_wait(&mutex_c_receive);

    message->type = buffer_to_receive[out_receive].type;
    message->length = buffer_to_receive[out_receive].length;
    message->seqn = buffer_to_receive[out_receive].seqn;
    message->timestamp = buffer_to_receive[out_receive].timestamp;
    strcpy(message->payload, buffer_to_receive[out_receive].payload);
    out_receive = (out_receive + 1) % buffer_size;

    if (out_receive == 0){
        new_packet_to_send = false;
    }

    sem_post(&mutex_c_receive);
    sem_post(&buffer_to_receive_empty);
}
