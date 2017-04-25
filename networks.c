#include<netinet/in.h>
#include<errno.h>
#include<netdb.h>
#include<stdio.h>   
#include<stdlib.h>  
#include<string.h>
#include<netinet/ip_icmp.h> 
#include<netinet/udp.h> 
#include<netinet/tcp.h> 
#include<netinet/ip.h>  
#include<netinet/if_ether.h>
#include<net/ethernet.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>
#include<pthread.h>

void dump_ip(unsigned char* ,int);
void dump_tcp(unsigned char * ,int );
void dump_udp(unsigned char * ,int );
void dump_icmp(unsigned char* ,int );
void dump_data(unsigned char* , int );
void dump_eth(unsigned char*, int );
void ProcessPacket(unsigned char* buffer, int sz);
FILE *out;

int src_port;
int dest_port;
struct sockaddr_in source,dest;

void debug1()
{
    //printf("Hi\n");
    int a=0;
    double b;
    char arr[100];
    for(a=0;a<50;a++)
        b = 4.0;
    //printf("Debug\n");
}

struct capture_details
{
    int tcp;
    int udp;
    int icmp;
    int others;
    int igmp;
    int total;
};
int i,j; 

struct capture_details capt;


int main()
{
    char *sa = (char *)(malloc(sizeof(char)*20));
    int saddr_size , data_size;
    struct sockaddr saddr;
        
    unsigned char *buffer = (unsigned char *) malloc(65536);
    debug1();
    out=fopen("output.txt","w");
    if(out==NULL) 
    {
        printf("file error");
    }
    
    int sock_raw = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;   
    if(sock_raw < 0)
    {
        perror("Socket Error");
        return 1;
    }
    debug1();
    printf("start your capture, open some website\n");
    while(1)
    {
        saddr_size = sizeof saddr;
        //Receive a packet
        data_size = recvfrom(sock_raw , buffer , 65536 , 0 , &saddr , (socklen_t*)&saddr_size);
        if(data_size <0 )
        {
            printf("Error in Recieveing\n");
            return 1;
        }
        debug1();
        //Get the IP Header part of this packet , excluding the ethernet header
        struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
        ++capt.total;
        if(iph->protocol==1)
        {  
            ++capt.icmp;
            dump_icmp( buffer , data_size);
        }
        
        else if(iph->protocol==2) 
        {
            ++capt.igmp;
        }
    
        else if(iph->protocol==6)
        { 
            ++capt.tcp;
            dump_tcp(buffer , data_size);
        }
    
        else if(iph->protocol==17)
        {
            ++capt.udp;
            dump_udp(buffer , data_size);
        }
    
        else
        { 
            ++capt.others;
        }
    printf("TCP : %d   UDP : %d   ICMP : %d   IGMP : %d   Others : %d   Total : %d\n", capt.tcp , capt.udp , capt.icmp , capt.igmp , capt.others , capt.total);
    }
    close(sock_raw);
    printf("Finished");
    return 0;
    debug1();
}



void dump_eth(unsigned char* Buffer, int sz)
{
    struct ethhdr *eth = (struct ethhdr *)Buffer;
    
    fprintf(out , "\n");
    fprintf(out , "Ethernet Header\n");
    fprintf(out , "   |-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5] );
    fprintf(out , "   |-Source Address      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4] , eth->h_source[5] );
    fprintf(out , "   |-Protocol            : %u \n",(unsigned short)eth->h_proto);
}

void dump_ip(unsigned char* Buffer, int sz)
{
    dump_eth(Buffer , sz);
  
    unsigned short iphdrlen;
        
    struct iphdr *iph = (struct iphdr *)(Buffer  + sizeof(struct ethhdr) );
    iphdrlen =iph->ihl*4;
    
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = iph->saddr;
    
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = iph->daddr;
    
    fprintf(out , "\n");
    fprintf(out , "IP Header\n");
    fprintf(out , "   |-IP Version        : %d\n",(unsigned int)iph->version);
    fprintf(out , "   |-IP Header Length  : %d DWORDS or %d Bytes\n",(unsigned int)iph->ihl,((unsigned int)(iph->ihl))*4);
    fprintf(out , "   |-Type Of Service   : %d\n",(unsigned int)iph->tos);
    fprintf(out , "   |-IP Total Length   : %d  Bytes(sz of Packet)\n",ntohs(iph->tot_len));
    fprintf(out , "   |-Identification    : %d\n",ntohs(iph->id));
    //fprintf(out , "   |-Reserved ZERO Field   : %d\n",(unsigned int)iphdr->ip_reserved_zero);
    //fprintf(out , "   |-Dont Fragment Field   : %d\n",(unsigned int)iphdr->ip_dont_fragment);
    //fprintf(out , "   |-More Fragment Field   : %d\n",(unsigned int)iphdr->ip_more_fragment);
    fprintf(out , "   |-TTL      : %d\n",(unsigned int)iph->ttl);
    fprintf(out , "   |-Protocol : %d\n",(unsigned int)iph->protocol);
    fprintf(out , "   |-Checksum : %d\n",ntohs(iph->check));
    fprintf(out , "   |-Source IP        : %s\n",inet_ntoa(source.sin_addr));
    fprintf(out , "   |-Destination IP   : %s\n",inet_ntoa(dest.sin_addr));
}

void dump_tcp(unsigned char* Buffer, int sz)
{
    unsigned short iphdrlen;
    
    struct iphdr *iph = (struct iphdr *)( Buffer  + sizeof(struct ethhdr) );
    iphdrlen = iph->ihl*4;
    
    struct tcphdr *tcph=(struct tcphdr*)(Buffer + iphdrlen + sizeof(struct ethhdr));
            
    int header_size =  sizeof(struct ethhdr) + iphdrlen + tcph->doff*4;
    
    fprintf(out , "\n\n***********************TCP Packet*************************\n");  
    int src_port = ntohs(tcph->source);
    int dest_port = ntohs(tcph->dest);



    dump_ip(Buffer,sz);
    debug1();
    fprintf(out , "\n");
    fprintf(out , "TCP Header\n");
    fprintf(out , "   |-Source Port      : %u\n",ntohs(tcph->source));
    fprintf(out , "   |-Destination Port : %u\n",ntohs(tcph->dest));
    fprintf(out , "   |-Sequence Number    : %u\n",ntohl(tcph->seq));
    fprintf(out , "   |-Acknowledge Number : %u\n",ntohl(tcph->ack_seq));
    fprintf(out , "   |-Header Length      : %d DWORDS or %d BYTES\n" ,(unsigned int)tcph->doff,(unsigned int)tcph->doff*4);
    //fprintf(out , "   |-CWR Flag : %d\n",(unsigned int)tcph->cwr);
    //fprintf(out , "   |-ECN Flag : %d\n",(unsigned int)tcph->ece);
    fprintf(out , "   |-Urgent Flag          : %d\n",(unsigned int)tcph->urg);
    fprintf(out , "   |-Acknowledgement Flag : %d\n",(unsigned int)tcph->ack);
    fprintf(out , "   |-Push Flag            : %d\n",(unsigned int)tcph->psh);
    fprintf(out , "   |-Reset Flag           : %d\n",(unsigned int)tcph->rst);
    fprintf(out , "   |-Synchronise Flag     : %d\n",(unsigned int)tcph->syn);
    fprintf(out , "   |-Finish Flag          : %d\n",(unsigned int)tcph->fin);
    fprintf(out , "   |-Window         : %d\n",ntohs(tcph->window));
    fprintf(out , "   |-Checksum       : %d\n",ntohs(tcph->check));
    fprintf(out , "   |-Urgent Pointer : %d\n",tcph->urg_ptr);
    fprintf(out , "\n");
    fprintf(out , "                        DATA Dump                         ");
    fprintf(out , "\n");
        
    fprintf(out , "IP Header\n");
    dump_data(Buffer,iphdrlen);
        
    fprintf(out , "TCP Header\n");
    dump_data(Buffer+iphdrlen,tcph->doff*4);
        
    fprintf(out , "Data Payload\n");
     if(ntohs(tcph->source) == 53 || ntohs(tcph->dest)== 53)
    {
        printf("it's a DNS Packet\n" );
        debug1();
    }  
    if(ntohs(tcph->source) == 80 || ntohs(tcph->dest) == 80)
    {
        char *temp = Buffer + header_size;
        int sz1 = sz - header_size;
        int i;
        for(i=0;i<sz1;i++)
        {
            if(i<(sz1-1) && temp[i]=='.' && temp[i+1]=='.')
            {
                i++;
                i++;
               fprintf(out,"\n");
            }
            else
                fprintf(out,"%c",temp[i]);
        }
        debug1();
    }   
    
         dump_data(Buffer + header_size , sz - header_size );
    
    char *s = (char *)(malloc(sizeof(char)*strlen(Buffer)/10));

    fprintf(out , "\n\n");

}

void dump_udp(unsigned char *Buffer , int sz)
{
    
    unsigned short iphdrlen;
    
    struct iphdr *iph = (struct iphdr *)(Buffer +  sizeof(struct ethhdr));
    iphdrlen = iph->ihl*4;
    
    struct udphdr *udph = (struct udphdr*)(Buffer + iphdrlen  + sizeof(struct ethhdr));
    char *s = (char *)(malloc(sizeof(char)*strlen(Buffer)/10));
    
    int header_size =  sizeof(struct ethhdr) + iphdrlen + sizeof udph;
    
    fprintf(out , "\n\n***********************UDP Packet*************************\n");
    
    dump_ip(Buffer,sz);           
    
    fprintf(out , "\nUDP Header\n");
    fprintf(out , "   |-Source Port      : %d\n" , ntohs(udph->source));
    fprintf(out , "   |-Destination Port : %d\n" , ntohs(udph->dest));
    fprintf(out , "   |-UDP Length       : %d\n" , ntohs(udph->len));
    fprintf(out , "   |-UDP Checksum     : %d\n" , ntohs(udph->check));
    
    fprintf(out , "\n");
    fprintf(out , "[IP] Header\n");
    dump_data(Buffer , iphdrlen);
        
    fprintf(out , "[UDP] Header\n");
    dump_data(Buffer+iphdrlen , sizeof udph);
        
    fprintf(out , "[Data] Payload\n"); 
    if(ntohs(udph->source) == 53 || ntohs(udph->dest)== 53)
    {
        fprintf(out, "it's a DNS Packet\n" );
        debug1();
    } 
    
    //Move the pointer ahead and reduce the sz of string
    dump_data(Buffer + header_size , sz - header_size);
    
    fprintf(out , "\n###########################################################");
}

void dump_icmp(unsigned char* Buffer , int sz)
{
    unsigned short iphdrlen;
    char *s = (char *)(malloc(sizeof(char)*strlen(Buffer)/10));
    
    struct iphdr *iph = (struct iphdr *)(Buffer  + sizeof(struct ethhdr));
    iphdrlen = iph->ihl * 4;
    
    struct icmphdr *icmph = (struct icmphdr *)(Buffer + iphdrlen  + sizeof(struct ethhdr));
    
    int header_size =  sizeof(struct ethhdr) + iphdrlen + sizeof icmph;
    
    fprintf(out , "\n\n***********************ICMP Packet*************************\n"); 
    
    dump_ip(Buffer , sz);
            
    fprintf(out , "\n");
        
    fprintf(out , "ICMP Header\n");
    fprintf(out , "   |-Type : %d",(unsigned int)(icmph->type));
            
    if((unsigned int)(icmph->type) == 11)
    {
        fprintf(out , "  (TTL Expired)\n");
    }
    else if((unsigned int)(icmph->type) == ICMP_ECHOREPLY)
    {
        fprintf(out , "  (ICMP Echo Reply)\n");
    }
    
    fprintf(out , "   |-Code : %d\n",(unsigned int)(icmph->code));
    fprintf(out , "   |-Checksum : %d\n",ntohs(icmph->checksum));
    //fprintf(out , "   |-ID       : %d\n",ntohs(icmph->id));
    //fprintf(out , "   |-Sequence : %d\n",ntohs(icmph->sequence));
    fprintf(out , "\n");

    fprintf(out , "IP Header\n");
    dump_data(Buffer,iphdrlen);
        
    fprintf(out , "UDP Header\n");
    dump_data(Buffer + iphdrlen , sizeof icmph);
        
    fprintf(out , "Data Payload\n");    
    
    //Move the pointer ahead and reduce the sz of string
    dump_data(Buffer + header_size , (sz - header_size) );
    
    fprintf(out , "\n###########################################################");
}

void dump_data(unsigned char* data , int sz)
{
    int i , j;
    for(i=0 ; i < sz ; i++)
    {
        if( i!=0 && i%32==0)   //if one line of hex printing is complete...
        {
            fprintf(out , "         ");
            for(j=i-32 ; j<i ; j++)
            {
                if(data[j]>=32 && data[j]<=128)
                    fprintf(out , "%c",(unsigned char)data[j]); //if its a number or alphabet
                
                else fprintf(out , "."); //otherwise print a dot
            }
            fprintf(out , "\n");
        } 
        char *s = (char *)(malloc(sizeof(char)*50));
        if(i%32==0) fprintf(out , "   ");
            fprintf(out , " %02X",(unsigned int)data[i]);
        debug1();       
        if( i==sz-1)  //print the last spaces
        {
            for(j=0;j<15-i%32;j++) 
            {
              fprintf(out , "   "); //extra spaces
            }
            debug1();
            
            fprintf(out , "         ");
            
            for(j=i-i%32 ; j<=i ; j++)
            {
                if(data[j]>=32 && data[j]<=128) 
                {
                  fprintf(out , "%c",(unsigned char)data[j]);
                }
                else 
                {
                  fprintf(out , ".");
                }
            }
            
            fprintf(out ,  "\n" );
        }
    }
}