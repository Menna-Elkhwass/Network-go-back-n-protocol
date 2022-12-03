#include "protocol.h"

using namespace std;

void wait_for_event(event_type *event){
    char sendOrRecieve;
    cin>> sendOrRecieve;
    if(sendOrRecieve == 's')
        from_network_layer();
    else(sendOrRecieve == 'r')
        from_physical_layer();
}

void from_network_layer(packet *p){
    cin>>p->data;
}

void from_physical_layer(frame *r){
    cout<<"Data:"<<endl;
    cin>>r->info->data;
    cout<<"sequence:"<<endl;
    cin>>r->seq;
    cout<<"acknowledgment:"<<endl;
    cin>>r->ack;
    cout<<"frame kind:"<<endl;
    cin>>r->kind;
}

void to_network_layer(packet *p){
    network_Send.push(p)
    cout<<"Data received at network layer:"<<endl();
    cout<<p->data<<endl;
}

void to_physical_layer(frame *s){
    physical_Recieve.push(s);
    cout<<"Data sent from physical layer:"<<endl;
    cout<<s->info->data<<endl;
    cout<<"sequence:"<<endl;
    cout<<s->seq<<endl;
    cout<<"acknowledgment:"<<endl;
    cout<<s->ack<<endl;
    cout<<"frame kind:"<<endl;
    cout<<s->kind<<endl;
}

int checksum(char *data){
        int index = sizeof(data);
        int counter = 0;
        for(int i=0; i<index; i++){
         if(data[i]== '1') counter++;
        }
        return counter%2;
    }

