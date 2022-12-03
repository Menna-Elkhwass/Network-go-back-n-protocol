#define MAX_SEQ 7
#include "protocol.h"
using namespace std;

int checksum(string data);
istream& operator>>(istream& is, frame_kind& kind)
{
    int a =0;
    is >> a;
    kind = static_cast<frame_kind>(a);
    return is;
}

void wait_for_event(event_type *event){
    if (!network_enabled)
        return;



    char sendOrRecieve = '0';

    cout<<"send from network layer (s) or receive from physical layer (r)?";
    while(1){ //while to fix wrong input error
    cin>> sendOrRecieve;

        if(sendOrRecieve == 's'){ //send input from console
            if(time){
                *event = timeout;
                return;
            }
            *event = network_layer_ready;
            return;
        }
        else if(sendOrRecieve == 'r'){ //receive input from console
            frame temp;
           // cout<<"Data: ";
            cin>>temp.info.data;
           // cout<<"sequence: ";
            cin>>temp.seq;
           // cout<<"acknowledgment: ";
            cin>>temp.ack;
           // cout<<"frame kind: ";
            cin>>temp.kind;

            if(checksum(temp.info.data)){
                cout<<"checksum error!"<<endl;
                *event = cksum_err;
                return;
            }
            else{
                physical_Recieve.push(temp);
                *event = frame_arrival;
                return;
            }

        }
    }
}

void from_network_layer(packet *p){
    cout<<"Receive data from network layer: ";
    cin>>p->data;

}

void from_physical_layer(frame *r){
    cout<<"Receive data from physical layer: ";
    frame p= physical_Recieve.front();
    *r = p;
    physical_Recieve.pop();
    return;
}

void to_network_layer(packet *p){
    network_Send.push(*p);
    cout<<"Data received at network layer: "<<endl;
    cout<<p->data<<endl;
}

void to_physical_layer(frame *s){
    physical_send.push(*s);

    cout<<s->info.data<<" ";
    cout<<s->seq<<" ";
    cout<<s->ack<<" ";
    cout<<s->kind<<endl;
}

int checksum(string data){
        int index = data.size();
        int counter = 0;
        for(int i=0; i<index; i++){
         if(data[i]== '1') counter++;
        }
        return counter%2;
}


void stop_timer(seq_nr k){
 cout<<"Stop timer for the packet:"<<k<<endl;
 time --;
 }

 void start_timer(seq_nr k){
  cout<<"Start timer for the packet:"<<k<<endl;
  time ++;
  }

void enable_network_layer(){
    cout<<"network layer is enabled"<<endl;
    network_enabled = true;
}
void disable_network_layer(){
    cout<<"network layer is disabled"<<endl;
    network_enabled = false;
}


static bool between(seq_nr a, seq_nr b, seq_nr c)
{
  if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
      return (true);
  else
      return (false);
 }

 static void send_data (seq_nr frame_nr, seq_nr frame_expected, packet buffer[])
 {
   frame scratch;
   scratch.info = buffer[frame_nr];
   scratch.seq = frame_nr;
   scratch.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ +1);
   scratch.kind = frame_kind::data;
   to_physical_layer(&scratch);
   start_timer(frame_nr);
 }


void protocol5(void)
{
  seq_nr next_frame_to_send  = 0;       /* MAX SEQ > 1; used for sent stream */
  seq_nr ack_expected = 0; /* oldest frame as yet unacknowledged */  /* next ack expected inbound */
  seq_nr frame_expected = 0; /* next frame expected on recieved stream */  /* number of frame expected inbound */
  frame scratch_var;  /* scratch variable */
  packet buffer[MAX_SEQ + 1]; /* buffers for the sent stream */
  seq_nr nbuffered = 0;  /* number of output buffers currently in use */ /* initially no packets are buffered */
  seq_nr i=0; /* used to index into the buffer array */
  event_type event = frame_arrival;
  enable_network_layer(); /* allow network layer ready events */


  while (true) {
    wait_for_event(&event);            //  frame_arrival = 0, cksum_err =1, timeout = 2, network_layer_ready = 3
    switch(event) {
      case network_layer_ready:       // packet ready to be sent from network layer
                                       // Accept, save, and transmit a new frame.
      from_network_layer(&buffer[next_frame_to_send]); /* fetch new packet*/
      nbuffered = nbuffered + 1;        // expand the sender’s window
      send_data(next_frame_to_send, frame_expected, buffer);    /* transmit the frame */
      inc(next_frame_to_send); /* advance sender’s upper window edge */
      break;

      case frame_arrival: /* a data or control frame has arrived */
        from_physical_layer(&scratch_var); /* get incoming frame from physical layer */
        if (scratch_var.seq == frame_expected) {  /* Frames are accepted only in order. */
          to_network_layer(&scratch_var.info);  /* pass packet to network layer */
          inc(frame_expected); /* advance lower edge of receiver’s window */
        }

        /* Ack n implies n − 1, n − 2, etc. Check for this. */
        while (between(ack_expected, scratch_var.ack, next_frame_to_send)) {/* Handle piggybacked ack. */
          nbuffered = nbuffered - 1; /* one frame fewer buffered */
          stop_timer(ack_expected); /* frame arrived intact; stop timer */
          inc(ack_expected); /* contract sender’s window */
        }

      break;

      case cksum_err: break; /* just ignore bad frames */

      case timeout: /* trouble; retransmit all outstanding frames */
        cout<<"timeout resending data"<<endl;
      next_frame_to_send = ack_expected; /* start retransmitting here */
      for (i = 1; i <= nbuffered; i++) {
      send_data(next_frame_to_send, frame_expected, buffer);/* resend frame */
      inc(next_frame_to_send); /* prepare to send the next one */

      }
   }
      if (nbuffered < MAX_SEQ)
        enable_network_layer();
      else
        disable_network_layer();
  }
}




int main()
{
    protocol5();
}
