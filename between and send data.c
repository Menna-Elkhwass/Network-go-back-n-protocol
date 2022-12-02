#define MAX_SEQ 7
typedef enum {frame_arrival, cksum_err, timeout, network_layer_ready} event_type;
#include "protocol.h"

void enable_network_layer(){
network_enabled = true;
}
void disable_network_layer(){
network_enabled = false;
}
static boolean between(seq_nr a, seq_nr b, seq_nr c)
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
   to_physical_layer(&S);
   start_timer(frame_nr);
 } 


void protocol5(void)
{
  seq_nr next_frame_to_send  = 0;       /* MAX SEQ > 1; used for sent stream */
  seq_nr ack_expected = 0; /* oldest frame as yet unacknowledged */  /* next ack expected inbound */
  seq_nr frame_expected = 0; /* next frame expected on recieved stream */  /* number of frame expected inbound */
  frame scratch_var;  /* scratch variable */
  packet buffer[MAX SEQ + 1]; /* buffers for the sent stream */
  seq_nr nbuffered = 0;  /* number of output buffers currently in use */ /* initially no packets are buffered */
  seq_nr i; /* used to index into the buffer array */
  event_type event;
  enable_network_layer(); /* allow network layer ready events */
    
  
  while (true) {
    wait for event(&event);            //  frame_arrival = 0, cksum_err =1, timeout = 2, network_layer_ready = 3
    switch(event) {
      case network_layer_ready:       // packet ready to be sent from network layer
                                       // Accept, save, and transmit a new frame.
      from_network_layer(&buffer[next_frame_to_send]); /* fetch new packet*/
      nbuffered = nbuffered + 1;        // expand the sender’s window
      send data(next_frame_to_send, frame_expected, buffer);    /* transmit the frame */
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
          nbuffered = nbuffered − 1; /* one frame fewer buffered */
          stop_timer(ack_expected); /* frame arrived intact; stop timer */
          inc(ack_expected); /* contract sender’s window */
        }
        
      break;
        
      case cksum_err: break; /* just ignore bad frames */
        
      case timeout: /* trouble; retransmit all outstanding frames */
      next_frame_to_send = ack_expected; /* start retransmitting here */
      for (i = 1; i <= nbuffered; i++) {
      send_data(next_frame_to_send, frame_expected, buffer);/* resend frame */
      inc(next_frame_to_send); /* prepare to send the next one */
      }
   }
      if (nbuffered < MAX SEQ)
        enable_network_layer();
      else
        disable_network_layer();
  }
}
