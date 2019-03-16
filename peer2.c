#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <netdb.h>

//PORT 5003
typedef struct message_header_t {
	char type;
	char error;
	unsigned int room;
	unsigned int payload_length;
} message_header;

typedef struct packet_t {
	struct message_header_t header;
	char payload[1000];
} packet;


//FUNCTION PROTOTYPE 
void parsed_args(int argc, char **argv);
void * read_input(void *ptr);
void send_message(char *msg);  
void* receive_packet(void *ptr);
void receive_message(struct sockaddr_in *sender_addr, packet *pkt);

pthread_mutex_t stdout_lock;
struct sockaddr_in self_addr;
struct sockaddr_in peer_info;
  
int sock;
pthread_t input_thread;

int main(int argc, char **argv)
{
  //peer_info=self_addr;
  sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
  {
    printf("ERROR IN CREATING SOCKET.");
	}
  else
  {
    printf("SOCKET CREATED.");
  }
  
  parsed_args(argc,argv);
  
  if(bind(sock,(struct sockaddr *)&self_addr,sizeof(self_addr))<0)
  {
    printf("ERROR IN BINDING");
  }
  printf("SELF ADDRESS: %x %x %x\n",self_addr.sin_family,self_addr.sin_port, self_addr.sin_addr.s_addr);
  /*struct sockaddr_in peer_info = get_sockaddr_in(self_addr_ip, self_addr_port);
  printf("Hello %d",peer_info.sin_port);*/
  
  pthread_t input_thread;
	pthread_create(&input_thread, NULL, read_input(NULL), NULL);
  //pthread_create(&input_thread, NULL, receive_packet(NULL), NULL);
	pthread_detach(input_thread);
  receive_packet(NULL);
  
  

}

void parsed_args(int argc, char **argv)
{
    if(argc!=2)
    {
      printf("AURGUMENT COUNT IS INCORRECT.");
    }

    short self_port=atoi(argv[1]);
    self_addr.sin_family=AF_INET;
    self_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    self_addr.sin_port=htons(self_port);
  
   short peer_port=5004;
    peer_info.sin_family=AF_INET;
    peer_info.sin_addr.s_addr=htonl(INADDR_ANY);
    peer_info.sin_port=htons(peer_port);
  
   printf("PEER ADDRESS: %x %x %x\n",peer_info.sin_family,peer_info.sin_port, peer_info.sin_addr.s_addr);
  
}

void * read_input(void *ptr)
{
     
      char line[1000];
      char *p;
      while (1) 
      {
        // read input
        
        memset(line, 0, sizeof(line));
        p = fgets(line, sizeof(line), stdin);
        // flush input stream to clear out long message
       
        if (p == NULL) 
        {
          pthread_mutex_lock(&stdout_lock);
          fprintf(stderr, "%s\n", "error - cannot read input");
          pthread_mutex_unlock(&stdout_lock);
          continue;
        }
        
        if (line[strlen(line) - 1] != '\n')
        {
          // flush input stream to clear out long message
          scanf ("%*[^\n]"); 
          (void) getchar ();
        }
         
        line[strlen(line) - 1] = '\0';
        
        send_message(line);
       
  }
  return(NULL);
}

void send_message(char *msg)
{
      int status;
  
      if (msg[0] == '\0')
      {
        pthread_mutex_lock(&stdout_lock);
        fprintf(stderr, "%s\n", "error - no message content.");
        pthread_mutex_unlock(&stdout_lock);
      }
  
      packet pkt;
      pkt.header.type = 'm';
      pkt.header.error = '\0';
      pkt.header.room = 0;
      pkt.header.payload_length = strlen(msg) + 1;
      memcpy(pkt.payload, msg, pkt.header.payload_length);
      
      status = sendto(sock, &pkt, sizeof(pkt.header) + pkt.header.payload_length, 0, (struct sockaddr *)&(peer_info), sizeof(struct sockaddr_in));
      if (status == -1) 
      {
          pthread_mutex_lock(&stdout_lock);
          printf( "error - error sending packet to peer");
          pthread_mutex_unlock(&stdout_lock);
		  }
      else
      {
        //printf("Message Sent!");
        pthread_create(&input_thread, NULL, receive_packet(NULL), NULL);
        pthread_detach(input_thread);
        
      }
}
void* receive_packet(void *ptr) {
struct sockaddr_in sender_addr;
  //sender_addr=peer_info;
  short sender_port=5004;
    sender_addr.sin_family=AF_INET;
    sender_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    sender_addr.sin_port=htons(sender_port);
	socklen_t addrlen = 10;
	packet pkt;
	int status;

	while (1) {
		status = recvfrom(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *)&sender_addr, &addrlen);
		if (status == -1) {
			pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "%s\n", "error - error receiving a packet, ignoring.");
			pthread_mutex_unlock(&stdout_lock);
			continue;
		}
    receive_message(&sender_addr, &pkt);
    
  }
  return(NULL);
}
void receive_message(struct sockaddr_in *sender_addr, packet *pkt) {
	// fetch sender information
	char *sender_ip = inet_ntoa(sender_addr->sin_addr);
	short sender_port = htons(sender_addr->sin_port);

	// display message in stdout if the message is from the chatroom that peer is in
	if (pkt->header.room == 0) {
		pthread_mutex_lock(&stdout_lock);
		printf("(PEER 2) - %s\n",  pkt->payload);
		pthread_mutex_unlock(&stdout_lock);
	}
  pthread_create(&input_thread, NULL, read_input(NULL), NULL);
   pthread_detach(input_thread);
}

