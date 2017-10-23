#include <stdio.h>
#include <stdlib.h>
#include "include.h"
#include "string.h"
//#include <string>
#include <limits.h>

#include <vector>
#include <algorithm>
#include <functional>

#include "sys/types.h"
#include "sys/stat.h"


typedef unsigned int u_int;
typedef int BOOL;
typedef unsigned char u_char;
typedef unsigned long long u_long_long;


class node;
class link;     

/* wanwenkai 
template<class T>
T Clamp(T x, T min, T max)
{
	if (x > max)
		return max;
	if (x < min)
		return min;
	return x;
}
 end */

template<class T>
class IterativePercentile {
    public:
          /// Percentile has to be in range [0, 1(
        IterativePercentile(double percentile)
                : _percentile(percentile)
        {}
		
		// Adds a number in O(log(n))
		void add(const T& x) {
                    if (_lower.empty() || x <= _lower.front()) {
                      _lower.push_back(x);
                      std::push_heap(_lower.begin(), _lower.end(), std::less<T>());
                    } else {
                      _upper.push_back(x);
                      std::push_heap(_upper.begin(), _upper.end(), std::greater<T>());
                    }

                    
                    unsigned size_lower = (unsigned)((_lower.size() + _upper.size()) * _percentile) + 1;
                    if (_lower.size() > size_lower) {
                      // lower to upper
                      std::pop_heap(_lower.begin(), _lower.end(), std::less<T>());
                      _upper.push_back(_lower.back());
                      std::push_heap(_upper.begin(), _upper.end(), std::greater<T>());
                      _lower.pop_back();
                    } else if (_lower.size() < size_lower) {
                      // upper to lower
                      std::pop_heap(_upper.begin(), _upper.end(), std::greater<T>());
                      _lower.push_back(_upper.back());
                      std::push_heap(_lower.begin(), _lower.end(), std::less<T>());
                      _upper.pop_back();
                    }            
                }

		/// Access the percentile in O(1)
		const T& get() const {
                    return _lower.front();
		}

		void clear() {
			_lower.clear();
			_upper.clear();
		}


		double _percentile;
		std::vector<T> _lower;
		std::vector<T> _upper;
};


enum EventType 
{	
	LINK_TRANSMIT,
	LINK_ARRIVAL,
        PROPAGATE,
	SENDER_TRANSMIT,
	SENDER_ARRIVAL,
	RECEIVER_TRANSMIT,
	RECEIVER_ARRIVAL,
	BANDWIDTH_UPDATE,
	//HYBRID_SLOW_START,//add by wanwenkai
        RATE_UPDATE,
        RESTART
};

enum NODE
{
	SENDER,
	RECEIVER,
	LINK
};

enum PACKET
{
	NA,
	DATA,
	ACK
};

struct update
{
	u_long_long start_time;
	u_long_long end_time;
	u_int link_rate;        

	update (u_long_long start, u_long_long end, u_int rate)
	{
            start_time = start;
            end_time = end;
            link_rate = rate;               
	}

};

struct rate_update
{
    u_long_long time;
    
    rate_update (u_long_long t) {
        time = t;
    }
};

struct restart 
{
    u_long_long time;
    
    restart(u_long_long t) {
        time = t;
    }
};

struct propagate
{
    PACKET packet;
    
    u_long_long time;
    NODE src;
    NODE dst;
    
    propagate(PACKET p, NODE _src, NODE _dst, u_long_long t)
    {
        packet = p;
        src = _src;
        dst = _dst;
        time = t;
    }
};

struct transmit 
{
	PACKET packet;
	
	u_long_long time;
	NODE src;
	NODE dst;

	transmit(PACKET p, NODE _src, NODE _dst, u_long_long t)
	{
		packet = p;
		src = _src;
		dst = _dst;
		time = t;
	}
	
};

struct arrival
{
	PACKET packet;
	NODE src;
	NODE dst;
	u_long_long time;

	arrival(PACKET p, NODE _src, NODE _dst, u_long_long t)
	{
		time = t;
		src = _src;
		dst = _dst;
		packet = p;
	}
};

/* wanwenkai */
struct hybridss {
	u_long_long delay_min;
	u_long_long last_ack;
	u_long_long curr_rtt;
	u_long_long baseRTT;
	u_long_long minRTT;
	u_long_long cntRTT;
	u_long_long found;
	u_long_long sample_cnt;
	u_long_long round_start;
	u_long_long end_seq;
	u_long_long snd_nxt;
	u_long_long ack;

	hybridss(u_long_long current_time) {
		curr_rtt = 0;
		sample_cnt = 0;
		cntRTT = 0;
		last_ack = current_time;
		round_start = current_time;
		found = 0;
		delay_min = 0;
		baseRTT = 0x7777777;
		minRTT = 0x7777777;
		end_seq = 0;
		snd_nxt = 0;
		ack = 0;
	}
};
/* end */

class EventInfo
{
public:

	EventType et;
	void* object;
	void* Event;
	
	EventInfo(){}
	EventInfo(EventType e, void *o, void *ev)
	{
		et = e;
		object = o;
		Event = ev;
	}

	~EventInfo()
	{
		object = NULL;
		Event = NULL;
	}
};

class packet
{
public:

	PACKET ptype;
	u_int packet_size;
	u_long_long arrival_time;
	u_long_long depart_time;
	u_int queue_time;
	u_int delay_min;//wanwenkai
        u_int seq_no;
        u_int ack_no;
        
	void *src;
	void *dst;

	packet(PACKET P, u_int size)
	{
		ptype = P;
		packet_size = size;
		arrival_time = 0;
		depart_time = 0;
		queue_time = 0;
		delay_min = 0;//wanwenkai
		src = NULL;
		dst = NULL;
                
                seq_no = 0;
                ack_no = 0;
	}

	packet(packet *_packet) 
	{
		ptype = _packet->ptype;
		packet_size = _packet->packet_size;
		arrival_time = _packet->arrival_time;
		depart_time = _packet->depart_time;
		queue_time = _packet->queue_time;
		delay_min = _packet->delay_min;//wanwenkai
		src = _packet->src;
		dst = _packet->dst;
                
                seq_no = _packet->seq_no;
                ack_no = _packet->ack_no;
                
	}

	void clone(packet *_packet)
	{
		ptype = _packet->ptype;
		packet_size = _packet->packet_size;
		arrival_time = _packet->arrival_time;
		depart_time = _packet->depart_time;
		queue_time = _packet->queue_time;
		src = _packet->src;
		dst = _packet->dst;
                
                seq_no = _packet->seq_no;
                ack_no = _packet->ack_no;
	}

	void flush()
	{
		ptype = NA;
		packet_size = 0;
		arrival_time = depart_time = queue_time = 0;
		src = NULL;
		dst = NULL;
                
                seq_no = 0;
                ack_no = 0;
	}

};

struct ForwardPktBuffer
{
	packet* pktQueue;
	u_int capacity, _size, _head, _tail;
    	    
	ForwardPktBuffer(u_int size)	
	{
		capacity = size;
		pktQueue = (packet *)malloc(sizeof(packet)*capacity);
		for (int i = 0; i < capacity; i ++)
			pktQueue[i].flush();

		_head = _tail = _size = 0;
	}

	void flush()
	{
		for (int i = 0; i < capacity; i ++)
			pktQueue[i].flush();

		_head = _tail = _size = 0;
	}

	~ForwardPktBuffer()
	{
		free(pktQueue);
	}
	
	inline packet* head() { return pktQueue + (_head % capacity); }
	inline void headNext() { _head = (_head + 1) % capacity; _size --; }	
	inline packet* tail() { return pktQueue + (_tail % capacity); }
	inline void tailNext() { _tail = (_tail + 1) % capacity; _size ++; }
	inline u_int size() { return _size; }
                 
};

struct Packet
{
	u_int seqNo;
	u_long_long time;
	u_int len;
          
	Packet()
	{
		len = seqNo = time = 0;
	}

	void flush()
	{
		len = seqNo = time = 0;
	}
};

struct SlideWindow
{
	Packet* window;   
        
	u_int _size, _bytes, capacity, _delta, _interval;
	u_long_long start_time;
            
	u_int M;        
        u_int k;
        
	SlideWindow(u_int size, u_int interval, u_int delta)
	{
            capacity = size;
            _delta = delta;
            _interval = interval;
            M = interval/delta;
            window = (Packet *)malloc(sizeof(Packet) * capacity);

            for (int i = 0; i < capacity; i ++)
                window[i].flush();

            _size = _bytes = start_time = 0;            
            k = 1;
	}

	~SlideWindow()
	{
            free(window);
	}

	u_int size()
	{
            return _size;
	}

	u_int bytes()
	{
            if (_bytes)           
                return _bytes;
            else
                return 0;
        }

#define TRUE 1
#define FALSE 0

	BOOL isEmpty()
	{
            if (_size == 0)
                return TRUE;
            else
                return FALSE;
        }
	
	u_long_long frontTime()
	{
            if (isEmpty())
                return 0;
            else
                return window[0].time;
	}

	u_long_long tailTime()
	{
            if (isEmpty())
                return 0;
            else
                return window[_size-1].time;
	}

        u_long_long estmateInterval(u_long_long current_time)
	{
            if (!start_time)
                return 0;
            else
                return (current_time <= start_time ? 0 :current_time - start_time);
	}

	u_long_long nextEstmateSampleTime(u_long_long current_time)
	{
            if (!start_time)
            {
                start_time = current_time;
                return start_time;
            }
            else
            {
                start_time += _delta;
                return start_time;
            }
	}

	void another_shift()
	{
         
            while (frontTime() < start_time && _bytes > 0 && _size > 0)
            {
                shift();
                _size --;
            }

	}
	
	u_long_long timeInterval(u_long_long current_time)
	{
            if (isEmpty())
                return 0;
            else
                return (current_time <= window[0].time ? 0 : current_time - window[0].time);
        }

	void shift()
        {
            _bytes -= window[0].len;

            for (u_int i = 0; i < _size - 1; i ++)
            {
                window[i].len = window[i+1].len;
                window[i].time = window[i+1].time;
                window[i].seqNo = window[i+1].seqNo;
            }

            window[_size - 1].flush();
	}
        
        void pop(u_int len) 
        {            
            if (_bytes > len && window[_size - 1].len >= len)
            {
                window[_size - 1].len -= len;
                _bytes -= len; 
            }
        }
        
	void put(u_int len, u_long_long time, u_int seqNo)
	{
            if (_size < capacity)
            {
                window[_size].len = len;
                window[_size].time = time;
                window[_size].seqNo = seqNo;
                _bytes += window[_size].len;
                _size ++;
            }
            else
            {                
                shift();
                window[_size-1].len = len;
                window[_size-1].time = time;
                window[_size-1].seqNo = seqNo;
                _bytes += window[_size-1].len;
            }
	}

	void flush()
	{      

            _size = _bytes = start_time = 0;
            for (u_int i = 0; i < capacity; i ++)
                window[i].flush();
            
            k = 1;

	}
     
        void put_tail(u_int len, u_int seqNo)
        {
            if (_size)
            {
                window[_size-1].len += len;            
                window[_size-1].seqNo = seqNo;
                _bytes += window[_size-1].len;
            }
            else
            {
                window[_size].len = len;            
                window[_size].seqNo = seqNo;
                _bytes += window[_size].len;
            }

        }

        u_int tail()
        {
            if (isEmpty())
                return 0;
            else
                return window[_size-1].len;
        }

        u_int head()
        {
            if (isEmpty())
                return 0;
            else
                return window[0].len;
        }
                
};


#define SND_WIN_SIZE 2
#define SLIDING_WIN_SIZE 30000
#define SLIDING_TIME_INTERVAL 250000
#define SLIDING_TIME_DELTA 2000
#define SENDING_RATE_MIN 1

class node
{
public:

	NODE ntype;
	u_int sending_rate;
	u_int initial_sending_rate;
	u_int hybrid_quit_rate; //added by wanwenkai
	u_int sending_rate_min;
	u_int snd_nxt;
	u_int snd_una;
	double gamma;//added by wanwenlai
        
	SlideWindow sliding_snd_window;
	SlideWindow sliding_avg_window;
        
        u_int end_seq_no;
        u_long_long transfer_session;
        u_long_long start_tx_time;
                
	node(NODE n, u_int initial_rate, u_int _capacity, u_int _interval, u_int _delta):
	sliding_avg_window (_capacity, _interval, _delta), 
	sliding_snd_window (SND_WIN_SIZE, _interval, _delta) 
	{
		ntype = n;
		sending_rate = initial_sending_rate = initial_rate;
		snd_nxt = snd_una = 0;
		transfer_session = 0;
		start_tx_time = 0;
		end_seq_no = UINT_MAX;
		sending_rate_min = SENDING_RATE_MIN;
		gamma = 1;//added by wanwenkai
		hybrid_quit_rate = initial_sending_rate;//added by wanwenkai
	}
        
        void flush() 
        {
            sending_rate = initial_sending_rate;
            sliding_snd_window.flush();
            sliding_avg_window.flush();
            //transfer_session = 0;
            end_seq_no = UINT_MAX;
            snd_nxt = snd_una = 0;
            start_tx_time = 0;
            sending_rate_min = SENDING_RATE_MIN;
        }

	
};

#define LINK_BUFFER_SIZE 5000

class link 
{
public:

	u_int residual_bytes;
	u_int bandwidth;
	u_long_long bw_start_time;
	u_long_long bw_end_time;

	u_long_long total_bytes_sent;
	u_long_long link_start_time;
	ForwardPktBuffer link_buffer;
        
        FILE *queue_delay;
        u_long_long simulation_period;
        u_long_long link_trace_period;
        
	link(u_int initial_bw, char *file_path):link_buffer(LINK_BUFFER_SIZE)
	{
		residual_bytes = 0;
		bw_start_time = 0; 
		bw_end_time = ULONG_LONG_MAX;
		bandwidth = initial_bw;	
		link_start_time = 0;
		total_bytes_sent = 0;
               /* 
                queue_delay = fopen(file_path, "r");
                if (queue_delay) {
                    struct stat st;
                    if (stat(file_path, &st) == 0)
                            fprintf(stderr, "size: %u\n", st.st_size);   
                    if (st.st_size < 2500)
                        queue_delay = fopen(file_path, "w");
                    else
                        exit(-1);
                }
                else
                    queue_delay = fopen(file_path, "w");
                */
                simulation_period = 0;
                link_trace_period = 0;
	}
        
        void flush()
        {
            link_start_time = 0;
            total_bytes_sent = 0;
            //queue_delay = fopen("queue_delay.txt", "w");
            simulation_period = 0;
            link_trace_period = 0;
            link_buffer.flush();
            residual_bytes = 0;
        }

};


                  


class Scheduler 
{
public :

	LinkedList<EventInfo, u_long_long> EventList;
	u_long_long now;
	
	Scheduler()
	{
		now = 0;
	}

	~Scheduler()
	{
		EventList.RemoveAll();
	}

	void* AddAbsoluteEvent (u_long_long at_time, EventInfo pevent)
	{
		return EventList.Add(pevent, at_time);
	}

	void* AddRelativeEvent (u_long_long rel_time, EventInfo pevent)
	{
		return EventList.Add(pevent, rel_time + now);
	}
	
	double GetCurrentTime()
	{
		return EventList.GetListHeadKey();
	}

	EventInfo DeleteEvent (void)
	{
		return EventList.RemoveAt(EventList.GetHeadPosition());
	}
	
	EventInfo GetNextEvent (void)
	{
		return EventList.GetNext(EventList.GetHeadPosition());
	}

	double Clock(void)
	{
		return now;
	}

	void DisplayEvents(void)
	{
		EventList.DisplayList();
	}
    
	bool IsEmpty(void)
	{
		return EventList.IsEmpty();
	}
};


