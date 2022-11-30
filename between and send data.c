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
