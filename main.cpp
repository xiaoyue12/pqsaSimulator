#include "scheduler.h"
//#include <iostream>
#include <string.h>
#include <math.h>

#define MAX_SEND_RATE 800
#define MAX_RADIO_LINK_BW 1500

#define RESOLUTION 1000
#define MTU 1460

/* wanwenkai */
#define HYSTART_ACK_TRAIN	0x1
#define HYSTART_DELAY		0x2
#define HYSTART_MIN_SAMPLES 3
#define HYSTART_DELAY_MIN	200
#define HYSTART_DELAY_MAX	400
#define TCP_SEQ_GT(a, b) ((int64_t)((a)-(b)) > 0)
#define DOWNLINK_PROPAGATION_DELAY 20000
#define UPLINK_PROPAGATION_DELAY 20000

static int64_t hystart_detect = HYSTART_ACK_TRAIN | HYSTART_DELAY;
static int64_t hystart_ack_delta = 10000;
static int64_t hystart = 1;
static u_int hystart_low_rate = 80;
static u_int alpha = 2; //used to vegas
static u_int beta = 4; //used to vegas
/*added by guoziting*/
int64_t T_THRESHOLD;
double G_gamma;
char *transport_port;
static char *vegas = "TCPvegas";
static char *gr = "PQSAgr";
/* end */

Scheduler *sch = new Scheduler();
IterativePercentile<u_int> percentile(0.95); 


FILE * sum = NULL;

/* wanwenkai */
u_long_long Clamp(u_long_long tmp)
{	
	if (tmp > HYSTART_DELAY_MAX)
		return HYSTART_DELAY_MAX;
	else if (tmp < HYSTART_DELAY_MIN)
		return HYSTART_DELAY_MIN;
	else return tmp;
}

u_int max(u_int a, u_int b)
{
	if (a > b)
		return a;
	else
		return b;
}

void hystart_reset(u_long_long time, hybridss *_hyss)
{	
	_hyss->round_start = _hyss->last_ack = time;
	_hyss->curr_rtt = 0;
	_hyss->sample_cnt = 0;
	_hyss->end_seq = _hyss->snd_nxt;
}

void updateBaseRtt(packet* _packet, hybridss *_hyss)
{
	if ((_packet->queue_time < _hyss->delay_min) || (_hyss->delay_min == 0)) {
		_hyss->delay_min = _packet->queue_time;
		_hyss->baseRTT = _hyss->delay_min + 2*UPLINK_PROPAGATION_DELAY;
	}
}

double updateGamma(packet* _packet, double g, hybridss *_hyss) //added g by guoziting
{
	double gamma;
	//fprintf(stderr, "_hyss->delay_min = %lu, packet->queue_time = %lu\n", _hyss->delay_min, _packet->queue_time);
	u_long_long min_rtt = _hyss->delay_min + 2*UPLINK_PROPAGATION_DELAY;
	u_long_long current_rtt = _packet->queue_time + 2*UPLINK_PROPAGATION_DELAY;
	if (current_rtt > (min_rtt + T_THRESHOLD) ) {
		gamma = (double)min_rtt*1.0/current_rtt;
	} else 
		gamma = g;

	return gamma;	
}

void hybridUpdate(arrival *_arrival_event, packet* _packet, hybridss *_hyss)
{
	u_long_long now;
	u_long_long delay;
	u_long_long min_rtt;

	min_rtt = _hyss->delay_min + 2*UPLINK_PROPAGATION_DELAY;
	delay = _packet->queue_time + 2*UPLINK_PROPAGATION_DELAY;

	if (!(_hyss->found & hystart_detect)) {
		now = _arrival_event->time;
		if ((int64_t)(now - _hyss->last_ack) <= hystart_ack_delta) {
			_hyss->last_ack = now;
			
			if ((int64_t)(now - _hyss->round_start) > (min_rtt >> 2) ) {
				_hyss->found |= HYSTART_ACK_TRAIN;
				hystart_reset(_arrival_event->time, _hyss);
				//printf("#############\n");
			}
		}

		if (_hyss->sample_cnt < HYSTART_MIN_SAMPLES) {
			if (_hyss->curr_rtt == 0 || _hyss->curr_rtt > delay)
				_hyss->curr_rtt = delay;

			_hyss->sample_cnt++;
		} else {
			if (_hyss->curr_rtt > min_rtt + Clamp(_hyss->delay_min >> 3) ) {
				_hyss->found |= HYSTART_DELAY;
				hystart_reset(_arrival_event->time, _hyss);
				//printf("HYSTART_DELAY\n");
			}
		}
	}		
}
/* end */

void initLinkBandwidth(link *_link, char *trace_path)
{
	FILE *bw_trace = fopen(trace_path, "r");

	if (bw_trace == NULL)
		exit(-1);

	double start_time = 0;
	double end_time = 0;
	double sent_bytes;
	u_int bandwidth ;

	char line[1000];
	while(fgets(line, 1000, bw_trace) != NULL)
	{
		if (line[0] != '#') 
		{
			sscanf(line, "%lf %lf",&end_time, &sent_bytes);
	
			bandwidth = sent_bytes / (end_time - start_time) / RESOLUTION;  		
			//fprintf(stderr, "%lf %lf %u\n", start_time, end_time, bandwidth);

			update *_bw_update_event = 
					new update(start_time * RESOLUTION * RESOLUTION, 
					end_time * RESOLUTION * RESOLUTION, bandwidth);

			EventInfo ei(BANDWIDTH_UPDATE, _link, _bw_update_event);
			sch->AddAbsoluteEvent(_bw_update_event->start_time, ei);
			start_time = end_time;

			/*
			if (start_time > _link->link_trace_period)
				break;
			*/
		}
	}
}

void updateLinkBandwidth(update *_update_event, link *_link) 
{
	_link->bw_start_time = _update_event->start_time;
	_link->bw_end_time = _update_event->end_time;
	_link->bandwidth = _update_event->link_rate;
}

/* wanwenkai */
void updateSenderHyssRate(arrival *_arrival_event, packet *_packet, node *_sender, hybridss *_hyss)
{
	u_long_long current_delay;

	current_delay = _packet->queue_time + 2*UPLINK_PROPAGATION_DELAY;

	//if (updateGamma(_packet, G_gamma, _hyss) < 1.0) {
	//	_sender->sending_rate = max(u_int(updateGamma(_packet, G_gamma, _hyss)*_sender->sending_rate), 10);
	if (current_delay > 55000) {
		_hyss->found = HYSTART_DELAY;
		hystart_reset(_arrival_event->time, _hyss);
	} else {
		_sender->sending_rate = _sender->sending_rate + _packet->packet_size*RESOLUTION/current_delay;
	}

	_sender->hybrid_quit_rate = _sender->sending_rate;
	//fprintf(stderr, "sending_rate = %d, current_delay = %ld\n", _sender->sending_rate, current_delay);

	if (!_sender->sending_rate)
		_sender->sending_rate = _sender->sending_rate_min;
}

/* end */

void updateSenderRate(rate_update *_transmit_event, node *_sender)
{
	//fprintf(stderr, "sendrate = %d\n", _sender->sending_rate);
    if (_sender->sliding_avg_window.start_time) {
        if (_sender->sliding_avg_window.estmateInterval(_transmit_event->time) < _sender->sliding_avg_window._interval) {
			/* wanwenkai */
			 u_long_long rcv_bw = _sender->sliding_avg_window.estmateInterval(_transmit_event->time) == 0 ? 
                            (u_long_long)_sender->hybrid_quit_rate : _sender->sliding_avg_window.bytes() * (u_long_long)(RESOLUTION) /
                            (u_long_long)_sender->sliding_avg_window.estmateInterval(_transmit_event->time);
             _sender->sending_rate = max(u_int(_sender->gamma *  _sender->hybrid_quit_rate * (1 - (double)_sender->sliding_avg_window.estmateInterval(_transmit_event->time) / 
                    (double)_sender->sliding_avg_window._interval) + rcv_bw * 
                    ((double)_sender->sliding_avg_window.estmateInterval(_transmit_event->time)/(double)_sender->sliding_avg_window._interval)), 10);            
			 /* end */
            if (!_sender->sending_rate)
                _sender->sending_rate = _sender->sending_rate_min;
            
        } else {
			/* wanwenkai */
            _sender->sending_rate = max(u_int(_sender->gamma * (u_long_long)_sender->sliding_avg_window.bytes() * (u_long_long)(RESOLUTION)/ 
                    (u_long_long)_sender->sliding_avg_window.estmateInterval(_transmit_event->time)), 10);
			/* end */
            if (!_sender->sending_rate)
                _sender->sending_rate = _sender->sending_rate_min;
            
            _sender->sliding_avg_window.nextEstmateSampleTime(_transmit_event->time);
            _sender->sliding_avg_window.another_shift();                                                
        }
    
        rate_update *_rate_update_event = new rate_update(_transmit_event->time + _sender->sliding_avg_window._delta);
        EventInfo ei(RATE_UPDATE, _sender, _rate_update_event);
        sch->AddAbsoluteEvent(_rate_update_event->time, ei);
    }
}

void linkPropagateHandler(propagate *_propagate, packet *_packet, node *_receiver) 
{
    link *radioLink = (link *)_packet->dst;
    
    packet *clone_packet = new packet(_packet);
    clone_packet->src = radioLink;
    clone_packet->dst = _receiver;
    
    if (clone_packet->packet_size < MTU)
        clone_packet->packet_size = MTU;

    arrival *_arrival_event = new arrival(DATA, LINK, RECEIVER, DOWNLINK_PROPAGATION_DELAY + _propagate->time);
    EventInfo ei(RECEIVER_ARRIVAL, clone_packet, _arrival_event);
    sch->AddAbsoluteEvent(DOWNLINK_PROPAGATION_DELAY + _propagate->time, ei);

    //fprintf(stderr, "At %lu link transmits a packet %u\n", _propagate->time, _packet->packet_size);
    radioLink->total_bytes_sent = radioLink->total_bytes_sent + _packet->packet_size;
    
    _packet->flush();
    radioLink->link_buffer.headNext();
    if (radioLink->link_buffer.size()) 
    {
        packet *nxt_packet = radioLink->link_buffer.head();				
        transmit *_nxt_transmit_event = new transmit(DATA, LINK, RECEIVER, _propagate->time);
        EventInfo ei(LINK_TRANSMIT, nxt_packet, _nxt_transmit_event);
        sch->AddAbsoluteEvent(_propagate->time, ei);

        //fprintf(stderr, "At %lu schedules the next transmission\n", _propagate->time);
    }
                               
}

void linkTransmitHandler(transmit *_transmit_event, packet *_packet, node *_receiver)
{	
	link *radioLink = (link *)_packet->dst;
        u_long_long transmission_time;
        
        
        if (radioLink->bandwidth)
            transmission_time = (u_long_long)_packet->packet_size * (u_long_long)RESOLUTION / (u_long_long)radioLink->bandwidth;
        else 
            transmission_time = UINT_MAX;
	
	if (!radioLink->link_start_time)
            radioLink->link_start_time = _transmit_event->time;
		
	if (transmission_time + _transmit_event->time <= radioLink->bw_end_time) 
	{			
            _packet->depart_time = _transmit_event->time;
            _packet->queue_time = _packet->depart_time - _packet->arrival_time + transmission_time;

            percentile.add(_packet->queue_time);
            
            //add propagate event                
            propagate *_propagate = new propagate(DATA, LINK, RECEIVER, transmission_time + _transmit_event->time);
            EventInfo ei(PROPAGATE, _packet, _propagate);
            sch->AddAbsoluteEvent(transmission_time + _transmit_event->time, ei);

	}
	else
	{
            
            if (radioLink->bw_end_time <= _transmit_event->time) {
                fprintf(stderr, "end of trace file\n");
                exit(-1);
            }
            
            _packet->packet_size = _packet->packet_size - (u_long_long)radioLink->bandwidth * 
                    (u_long_long)(radioLink->bw_end_time - _transmit_event->time) / 
                    (u_long_long)RESOLUTION; 

            transmit *_nxt_transmit_event = new transmit(DATA, LINK, RECEIVER, radioLink->bw_end_time);
            EventInfo ei(LINK_TRANSMIT, _packet, _nxt_transmit_event);
            sch->AddAbsoluteEvent(radioLink->bw_end_time, ei);

            //fprintf(stderr, "At %lu transmits the partial packet %u\n", 
            //        radioLink->bw_end_time - _transmit_event->time + _transmit_event->time, 
            //        _packet->packet_size);

            radioLink->total_bytes_sent = radioLink->total_bytes_sent + 
                    (u_long_long)radioLink->bandwidth * (u_long_long)(radioLink->bw_end_time - _transmit_event->time) / (u_long_long)RESOLUTION;

	}
}

void linkArrivalHandler(arrival *_arrival_event, packet *_packet, node* _receiver)
{
	link *radioLink = (link *)_packet->dst;

	//fprintf(stderr, "At %lu link receives one packet %u\n", _arrival_event->time, _packet->packet_size);
	_packet->arrival_time = _arrival_event->time;

	if (!radioLink->link_buffer.size())
	{		
            packet *tail = radioLink->link_buffer.tail();
            tail->clone(_packet);
            radioLink->link_buffer.tailNext();

            packet *nxt_packet = radioLink->link_buffer.head();				
            transmit *_nxt_transmit_event = new transmit(DATA, LINK, RECEIVER, _arrival_event->time);
            EventInfo ei(LINK_TRANSMIT, nxt_packet, _nxt_transmit_event);
            sch->AddAbsoluteEvent(_arrival_event->time, ei);

            //fprintf(stderr, "At %lu link transmits the head packet %u\n", _arrival_event->time, _packet->packet_size);

	}
	else
	{
		if (radioLink->link_buffer.size() >= radioLink->link_buffer.capacity) {
			exit(-1);
		}

		packet *tail = radioLink->link_buffer.tail();
		tail->clone(_packet);
		radioLink->link_buffer.tailNext();
	}
    delete _packet;
              
}

void senderTransmitHandler(transmit *_transmit_event, node *_sender, link *_link, hybridss *_hyss) 
{	                   
	while (TRUE) 
	{   
		u_long_long timeUsed = _sender->sliding_snd_window.timeInterval(_transmit_event->time);
		u_long_long timeInterval = 
				(u_long_long)_sender->sliding_snd_window.bytes() * 
				(u_long_long)(RESOLUTION) / 
				(u_long_long)_sender->sending_rate;
	//	fprintf(stderr, "timeInterval = %ld\n", timeInterval);
		if (timeInterval >= timeUsed && _sender->sliding_snd_window._bytes)
		{			                
			transmit *_nxt_transmit_event = new transmit(DATA, SENDER, LINK, _transmit_event->time + 1);
			EventInfo ei(SENDER_TRANSMIT, _sender, _nxt_transmit_event);
			sch->AddAbsoluteEvent(_nxt_transmit_event->time, ei);
											
			break;
		}
		else
		{
			packet *nxt_packet = new packet(DATA, MTU);
			nxt_packet->src = _sender;
			nxt_packet->dst = _link;
			nxt_packet->seq_no = _sender->snd_nxt;
			_sender->snd_nxt += nxt_packet->packet_size;
		   _hyss->snd_nxt += nxt_packet->packet_size; //add by wanwenkai

			arrival *_arrival_event = new arrival(DATA, SENDER, LINK, _transmit_event->time);
			EventInfo ei (LINK_ARRIVAL, nxt_packet, _arrival_event);
			sch->AddAbsoluteEvent(_transmit_event->time, ei);

			_sender->sliding_snd_window.put(nxt_packet->packet_size, _transmit_event->time, 0);

			if (!_sender->start_tx_time)
				_sender->start_tx_time = _transmit_event->time;
			
							
			if (_transmit_event->time > _sender->transfer_session + _sender->start_tx_time) {
				_sender->end_seq_no = nxt_packet->seq_no; // the last seq_no, transfer session ends
				
				return;
			}
		}
	}	
}

/* wanwenkai */
void tcp_vegas_cong_avoid(node *_sender, hybridss *_hyss, packet* _packet)
{
	u_long_long rtt, diff;
	u_long_long target_cwnd;
	
	_hyss->curr_rtt = _packet->queue_time + 2*UPLINK_PROPAGATION_DELAY; 
	_hyss->cntRTT ++;
	if (_hyss->minRTT > _hyss->curr_rtt)
		_hyss->minRTT = _hyss->curr_rtt;

	if (_packet->seq_no > _hyss->end_seq) {
		if (_hyss->cntRTT > 2) {
			_hyss->end_seq = _sender->snd_nxt;
			
			rtt = _hyss->minRTT;
			target_cwnd = _sender->sending_rate * _hyss->baseRTT/(RESOLUTION * MTU);
			diff = _sender->sending_rate * rtt*(rtt - _hyss->baseRTT)/(_hyss->baseRTT * RESOLUTION * MTU);

			if (diff > beta) {
				_sender->sending_rate = _sender->sending_rate - _packet->packet_size*RESOLUTION/rtt;
			} else if (diff < alpha) {
				_sender->sending_rate = _sender->sending_rate + _packet->packet_size*RESOLUTION/rtt;
			}
			
			_hyss->cntRTT = 0;
			_hyss->minRTT = 0x7777777;
		}
	}	
}

void adaptive_rate_control(arrival *_arrival_event, node *_sender, hybridss *_hyss, packet *_packet)
{
	if ((_packet->seq_no > _hyss->end_seq) && (_hyss->found & hystart_detect) ){
		if (!_sender->sliding_avg_window.start_time) {
			
			/* wanwenkai */
			_sender->gamma = updateGamma(_packet, G_gamma, _hyss);//modified by guoziting
			/* end */
            _sender->sliding_avg_window.start_time = _arrival_event->time;
            rate_update *_rate_update_event = new rate_update(_arrival_event->time + _sender->sliding_avg_window._delta);
            EventInfo ei(RATE_UPDATE, _sender, _rate_update_event);
            sch->AddAbsoluteEvent(_rate_update_event->time, ei);
        } else {
			_sender->gamma = updateGamma(_packet, G_gamma, _hyss);//added by wanwenkai modified by guozitng
            _sender->sliding_avg_window.put(_packet->packet_size, _arrival_event->time, 0);	                
        }
	}
}

void senderArrivalHandler(arrival *_arrival_event, packet* _packet, node* _sender, hybridss *_hyss)	
{
	/* wanwenkai */
	updateBaseRtt(_packet, _hyss);
	if (_sender->sending_rate >= hystart_low_rate && !(_hyss->found & hystart_detect))
		hybridUpdate(_arrival_event, _packet, _hyss);
	if (!(_hyss->found & hystart_detect) && hystart) {
		//printf("_packet->seq_no = %ld, end_seq = %ld\n", _packet->seq_no, _hyss->end_seq);
		if (hystart && TCP_SEQ_GT(_packet->seq_no, _hyss->end_seq)) {
			hystart_reset(_arrival_event->time, _hyss);
		}
		updateSenderHyssRate(_arrival_event, _packet, _sender, _hyss);
	} else {
		if (strcmp(transport_port, vegas) == 0) {
			tcp_vegas_cong_avoid(_sender, _hyss, _packet);	
		} 
		else if (strcmp(transport_port, gr) == 0) {
			adaptive_rate_control(_arrival_event, _sender, _hyss, _packet);
		}

		if (_packet->seq_no >= _sender->end_seq_no)
        {
            //start a new session
			printf("new session\n");
            restart *_restart_event = new restart(_arrival_event->time);
            EventInfo ei(RESTART, _sender, _restart_event);
            sch->AddAbsoluteEvent(_restart_event->time, ei);
            
            //fprintf(stderr, "start a new session at seq no %u\n", _packet->seq_no);
			delete _packet; //update by wanwenkai
        }
	}
}
/* end */

void receiverArrivalHandler(arrival *_arrival_event, packet *_packet, node *_sender)
{        
        _packet->ptype = ACK;
	_packet->src = _packet->dst;
	_packet->dst = _sender;

	arrival *_arrival_sender_event = new arrival(ACK, RECEIVER, SENDER, _arrival_event->time + UPLINK_PROPAGATION_DELAY);
	EventInfo ei (SENDER_ARRIVAL, _packet, _arrival_sender_event);
	sch->AddAbsoluteEvent(_arrival_event->time + UPLINK_PROPAGATION_DELAY, ei);

	//fprintf(stderr, "At %lu receiver receives the packet %u\n", _arrival_event->time, _packet->packet_size);
}

void initSenderTransmit(node *_sender, hybridss *_hyss, u_long_long at)
{
	/* wanwenkai */
	_hyss->found = 0;
	_hyss->snd_nxt = 0;
	_hyss->delay_min = 0;
	if (hystart)
		hystart_reset(at, _hyss);
	/* end */
	transmit *first_transmit_event = new transmit(DATA, SENDER, LINK, at);
	EventInfo ei(SENDER_TRANSMIT, _sender, first_transmit_event);
	sch->AddAbsoluteEvent(first_transmit_event->time, ei);
}

void restartSession(restart *_restart_event, node *_sender, node *_receiver, link *_link, hybridss *hyss)
{
    fprintf(stderr, "throughput %lu\n", _link->total_bytes_sent / 
                    ((_restart_event->time - _link->link_start_time) / RESOLUTION));
    fprintf(stderr, "95th percentile queueing delay: %u\n", percentile.get());
    
    
    fprintf(sum, "%lu %u\n", _link->total_bytes_sent / ((_restart_event->time - _link->link_start_time) / RESOLUTION), percentile.get());
    fflush(sum);        
        
    
    //fprintf(_link->queue_delay, "next session\n");
    //fflush(_link->queue_delay);
        
    _sender->flush();
    _receiver->flush();
    _link->flush();
   
    percentile.clear();
    
    initSenderTransmit(_sender, hyss, _restart_event->time);
}

void printhelp()
{
  printf("simulation 1.0\n");
  printf("Usage: ./main initial_rate(Bytes/ms) window_interval(in microseconds) window_delta(keep it fix to 2000) "
          "window_size(keep it fix to 30000) simulation_period(in microsecond) bandwidth_trace_file gamma T_THRESHOLD(in milliseconds)\n");
  
}

int main (int argc, char **argv)
{
        
    if (argc != 9) {//added gamma, T_THRESHOLD by guoziting
        printhelp();
        return 0;        
    }
    
	printf(stdout, "$$$$$$$$\n");
    node* receiver = new node(RECEIVER, atoi(argv[1]), atoi(argv[4]), atoi(argv[2]), atoi(argv[3]));
    node* sender = new node(SENDER, atoi(argv[1]), atoi(argv[4]), atoi(argv[2]), atoi(argv[3]));
    

	/* added by guoziting & modifited by wanwenkai */
	//transport_port = "TCPrre";
	//transport_port = "TCPvegas";
	transport_port = "PQSAgr";//wanwenkai
	G_gamma = atof(argv[7]);
	T_THRESHOLD = atoi(argv[8]);
	/* end */

    sender->transfer_session = atoi(argv[5]);
    
    char trace_path[120];
    sprintf(trace_path, "%s", argv[6]);
    
    //"/root/bw_bank/downlink_400000/proUDP_385596.txt";    
    
    char path[10][120];

    int n = 0;    
    char *token = strtok(argv[6], "/.");     
    while(token != NULL) {
        sprintf(path[n], "%s", token);
        n ++;
        token = strtok(NULL, "/.");        
    }
    
    for (int i = 0; i < n; i ++) {
        //fprintf(stderr, "%s\n", path[i]);
    }
    
    char *bw_level = path[n-3];
    struct stat st = {0};
        
    if (stat(bw_level, &st) == -1) {
        mkdir(bw_level, 0700);
    }
    
	/*result name changed into:proUDP_***-gamma-T_THRESHOLD-rate-win-period-trace file.sum*/
    strcat(bw_level, "/");
    strcat(bw_level, path[n-2]);
    strcat(bw_level, "-");
	strcat(bw_level, argv[7]);
	strcat(bw_level, "-");
	strcat(bw_level, argv[8]);
	strcat(bw_level, "-");
    strcat(bw_level, argv[1]);
    strcat(bw_level, "-");
    strcat(bw_level, argv[2]);
    strcat(bw_level, "-");
    strcat(bw_level, argv[5]);
        
    link* radioLink = new link(MAX_RADIO_LINK_BW, bw_level);
    
    strcat(bw_level, ".sum");
    //fprintf(stderr, "%s\n", bw_level);    
    sum = fopen(bw_level, "w");
          
    //fprintf(stderr, "sending rate %u sliding window interval %u sliding window capacity %u\n", 
    //        sender->sending_rate, sender->sliding_avg_window._interval, sender->sliding_avg_window.capacity);
    
	hybridss *hyss = new hybridss(0); //wanwenkai
    initLinkBandwidth(radioLink, trace_path);
    initSenderTransmit(sender, hyss, 0); //update by wanwenkai
    while(!sch->IsEmpty())
    {
            EventInfo ei = sch->GetNextEvent();		

            switch(ei.et)
            {
            case LINK_TRANSMIT:
                    linkTransmitHandler((transmit *)ei.Event, (packet *)ei.object, receiver);
                    break;

            case LINK_ARRIVAL:
                    linkArrivalHandler(( arrival*)ei.Event, (packet *)ei.object, receiver);
                    break;

            case SENDER_TRANSMIT:
                    senderTransmitHandler((transmit *)ei.Event, (node *)ei.object, radioLink, hyss);
                    break;

            case SENDER_ARRIVAL: 
                    senderArrivalHandler((arrival *)ei.Event, (packet *)ei.object, sender, hyss);
                    break;

            case RECEIVER_ARRIVAL:
                    receiverArrivalHandler((arrival *)ei.Event, (packet *)ei.object, sender);
                    break;

            case BANDWIDTH_UPDATE:
                    updateLinkBandwidth((update *)ei.Event, radioLink);
                    break;

            case RATE_UPDATE:
                    updateSenderRate((rate_update *)ei.Event, sender);
                    break;

            case PROPAGATE:
                    linkPropagateHandler((propagate *)ei.Event, (packet *)ei.object, receiver);
                    break;
            
            case RESTART:
                    restartSession((restart *)ei.Event, sender, receiver, radioLink, hyss);
                    break;

            }

            sch->DeleteEvent();
            delete ei.Event;


    }

    printf("Simlulation done!\n");
    sch->DisplayEvents();
} 
