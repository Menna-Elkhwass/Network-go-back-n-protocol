#define MAX_SEQ 7
typedef enum {frame_arrival, cksum_err, timeout, network_layer_ready} event_type;
#include "protocol.h"

static boolean between(seq_nr a, seq_nr b, seq_nr c)
{
  if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
      return (true);
  else 
      return (false);
 }
 
 static void send_data (seq_nr frame_nr, seq_nr frame_expected, packet buffer[])
 { 
   frame s;
   s.info = buffer[frame_nr];
   s.seq = frame_nr;
   s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ +1);
   to_physical_layer(&S);
   start_timer(frame_nr);
 } 


void protocol5(void)
{
  seq nr next frame to send; /* MAX SEQ > 1; used for outbound stream */
  seq nr ack expected; /* oldest frame as yet unacknowledged */
  seq nr frame expected; /* next frame expected on inbound stream */
  frame r; /* scratch variable */
  packet buffer[MAX SEQ + 1]; /* buffers for the outbound stream */
  seq nr nbuffered; /* number of output buffers currently in use */
  seq nr i; /* used to index into the buffer array */
  event type event;
  enable network layer(); /* allow network layer ready events */
  ack expected = 0; /* next ack expected inbound */
  next frame to send = 0; /* next frame going out */
  frame expected = 0; /* number of frame expected inbound */
  nbuffered = 0; /* initially no packets are buffered */
  
  
  while (true) {
    wait for event(&event);            //  frame_arrival = 0, cksum_err =1, timeout = 2, network_layer_ready = 3
    switch(event) {
      case network layer ready:       // packet ready to be sent from network layer
                                       // Accept, save, and transmit a new frame.
      from network layer(&buffer[next frame to send]); /* fetch new packet */
      nbuffered = nbuffered + 1;        // expand the sender’s window
      send data(next frame to send, frame expected, buffer);    /* transmit the frame */
      inc(next frame to send); /* advance sender’s upper window edge */
      break;

      case frame arrival: /* a data or control frame has arrived */
        from physical layer(&r); /* get incoming frame from physical layer */
        if (r.seq == frame expected) {
          /* Frames are accepted only in order. */
          to network layer(&r.info); /* pass packet to network layer */
          inc(frame expected); /* advance lower edge of receiver’s window */
        }    /* Ack n implies n − 1, n − 2, etc. Check for this. */
        while (between(ack expected, r.ack, next frame to send)) {/* Handle piggybacked ack. */
          nbuffered = nbuffered − 1; /* one frame fewer buffered */
          stop timer(ack expected); /* frame arrived intact; stop timer */
          inc(ack expected); /* contract sender’s window */
        }
      break;
        
      case cksum err: break; /* just ignore bad frames */
        
      case timeout: /* trouble; retransmit all outstanding frames */
      next frame to send = ack expected; /* start retransmitting here */
      for (i = 1; i <= nbuffered; i++) {
       send data(next frame to send, frame expected, buffer);/* resend frame */
       inc(next frame to send); /* prepare to send the next one */
      }
   }
      if (nbuffered < MAX SEQ)
        enable network layer();
      else
        disable network layer();
  }
}
