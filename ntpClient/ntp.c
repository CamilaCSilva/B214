#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define NTP_TIMESTAMP_DELTA 2208988800ull

#define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6) // (li   & 11 000 000) >> 6
#define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3) // (vn   & 00 111 000) >> 3
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0) // (mode & 00 000 111) >> 0

 typedef struct{

    uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                             // li.   Two bits.   Leap indicator.
                             // vn.   Three bits. Version number of the protocol.
                             // mode. Three bits. Client will pick mode 3 for client.

    uint8_t stratum;         // Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Eight bits. Precision of the local clock.

    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.

    uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

  } ntp_packet;              // Total: 384 bits or 48 bytes.
  
void error( char* msg ){
    perror( msg ); // Print the error message to stderr.

    exit( 0 ); // Quit the process.
}

ntp_packet enviarPacote2(){ //função que envia um pacote
    
    ntp_packet x = { 0xFE, 0x0C, 0xAC, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x98, 0XFF };
    return x;
}

ntp_packet enviarPacote3(){ //função que envia um outro pacote
    
    ntp_packet x = { 0xFE, 0x0C, 0xB0, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x98, 0XFF };
    return x;
}

//https://acervolima.com/atraso-de-tempo-em-c/
void delay(int number_of_seconds){ //função para dar delay entre os envios de cada pacote
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
  
    // Storing start time
    clock_t start_time = clock();
  
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

int main( int argc, char* argv[ ] ){
  int sockfd, n, n1, n2; // Socket file descriptor and the n return result from writing/reading from the socket.

  int portno = 123; // NTP UDP port number.

  //primeiro servidor
  char* host_name = "us.pool.ntp.org"; // NTP server host-name USA
  
  //segundo servidor
  char* host_name2 = "pool.ntp.org"; // NTP server host-name Global

  // Structure that defines the 48 byte NTP packet protocol.

  // Create and zero out the packet. All 48 bytes worth.

  ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  ntp_packet packet2 = enviarPacote2();
  ntp_packet packet3 = enviarPacote3();

  memset( &packet,0, sizeof( ntp_packet ) );
  memset( &packet2,0, sizeof( ntp_packet ) );
  memset( &packet3,0, sizeof( ntp_packet ) );
  // Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.

 *( ( char * ) &packet + 0 ) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.
 *( ( char * ) &packet2 + 0 ) = 0x1b;
 *( ( char * ) &packet3 + 0 ) = 0x1b;
  // Create a UDP socket, convert the host-name to an IP address, set the port number,
  // connect to the server, send the packet, and then read in the return packet.

  struct sockaddr_in serv_addr; // Server address data structure.
  struct hostent *server;      // Server data structure.

  sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ); // Create a UDP socket.

  if ( sockfd < 0 )
    error( "ERROR opening socket" );
    
  server = gethostbyname( host_name ); // Convert URL to IP.
   
//Passa pro segundo servidor se o primeiro cair
  if ( server == NULL){
    server = gethostbyname( host_name2 );
  }
//retorna erro se os servidores não estiverem disponíveis ou se o servidor não existir
  else if(server == NULL){
    error( "ERROR, no such host" );
  }
         
  // Zero out the server address structure.

  bzero( ( char* ) &serv_addr, sizeof( serv_addr ) );

  serv_addr.sin_family = AF_INET;

  // Copy the server's IP address to the server address structure.

  bcopy( ( char* )server->h_addr, ( char* ) &serv_addr.sin_addr.s_addr, server->h_length );

  // Convert the port number integer to network big-endian style and save it to the server address structure.

  serv_addr.sin_port = htons( portno );

  // Call up the server using its IP address and port number.

  if ( connect( sockfd, ( struct sockaddr * ) &serv_addr, sizeof( serv_addr) ) < 0 )
    error( "ERROR connecting" );

  // Send it the NTP packet it wants. If n == -1, it failed.
  
  n = write( sockfd, ( char* ) &packet, sizeof( ntp_packet ) );
  delay(5000);
  n1 = write( sockfd, ( char* ) &packet2, sizeof( ntp_packet ) );
  delay(6000);
  n2 = write( sockfd, ( char* ) &packet3, sizeof( ntp_packet ) );
  
  if ( n < 0 && n1 < 0 && n2 < 0)
    error( "ERROR writing to socket" );

  // Wait and receive the packet back from the server. If n == -1, it failed.

  n = read( sockfd, ( char* ) &packet, sizeof( ntp_packet ) );
  n1 = read( sockfd, ( char* ) &packet2, sizeof( ntp_packet ) );
  n2 = read( sockfd, ( char* ) &packet3, sizeof( ntp_packet ) );
  
  if ( n < 0 && n1 < 0 && n2 < 0)
    error( "ERROR reading from socket" );

  // These two fields contain the time-stamp seconds as the packet left the NTP server.
  // The number of seconds correspond to the seconds passed since 1900.
  // ntohl() converts the bit/byte order from the network's to host's "endianness".

  //timestamp do pacote 1
  packet.txTm_s = ntohl( packet.txTm_s ); // Time-stamp seconds
  packet.txTm_f = ntohl( packet.txTm_f ); // Time-stamp fraction of a second 

  //timestamp do pacote 2
  packet2.txTm_s = ntohl( packet2.txTm_s ); // Time-stamp seconds.
  packet2.txTm_f = ntohl( packet2.txTm_f ); // Time-stamp fraction of a second.
  
  //timestamp do pacote 3
  packet3.txTm_s = ntohl( packet3.txTm_s ); // Time-stamp seconds.
  packet3.txTm_f = ntohl( packet3.txTm_f ); // Time-stamp fraction of a second.
  
  // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
  // Subtract 70 years worth of seconds from the seconds since 1900.
  // This leaves the seconds since the UNIX epoch of 1970.
  // (1900)------------------(1970)**************(Time Packet Left the Server)

  //Timestamp em GMT 0 e GMT Palo Alto para o pacote 1
  time_t txTm = ( time_t ) ( packet.txTm_s - NTP_TIMESTAMP_DELTA );
  time_t txTm2 = ( time_t ) ( packet.txTm_s - NTP_TIMESTAMP_DELTA  - 7*60*60 );
  
  //Timestamp em GMT 0 e GMT Palo Alto para o pacote 2
  time_t txTm3 = ( time_t ) ( packet2.txTm_s - NTP_TIMESTAMP_DELTA );
  time_t txTm4 = ( time_t ) ( packet2.txTm_s - NTP_TIMESTAMP_DELTA  - 7*60*60 );

  //Timestamp em GMT 0 e GMT Palo Alto para o pacote 3
  time_t txTm5 = ( time_t ) ( packet3.txTm_s - NTP_TIMESTAMP_DELTA );
  time_t txTm6 = ( time_t ) ( packet3.txTm_s - NTP_TIMESTAMP_DELTA  - 7*60*60 );

  // Print the time we got from the server, accounting for local timezone and conversion from UTC time.
    printf("Pacote 1: \n");
    printf( "Time: %s", ctime( ( const time_t* ) &txTm ) );
    printf( "Time: %s", ctime( ( const time_t* ) &txTm2 ) );
    printf("Pacote 2: \n");
    printf( "Time: %s", ctime( ( const time_t* ) &txTm3 ) );
    printf( "Time: %s", ctime( ( const time_t* ) &txTm4 ) );
    printf("Pacote 3: \n");
    printf( "Time: %s", ctime( ( const time_t* ) &txTm5 ) );
    printf( "Time: %s", ctime( ( const time_t* ) &txTm6 ) );
    

  return 0;
}
