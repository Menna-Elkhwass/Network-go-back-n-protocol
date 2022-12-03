void wait_for_event(event_type *event){
  if (!physical_layer_reciever.empty()){
    frame temp=physical_layer_reciever.front();
    if (!check_error(temp.info.data)){
      *event=cksum_err;
    }
    else *event=frame_arrival;
  }
  elseif(time){*event=timeout;}
  else{*event=idle;}
}
