/** @file sensor.c
 * @brief simulates a TCN sensor. Simply prints recieved messages to stdout. */

#include <stdlib.h>
#include <signal.h>

#include "util/crc.h"
#include "util/io.h"
#include "util/sockets.h"
#include "data/tats.h"

// we only allow one controller and one sensor
#define MAX_CONNECTIONS 2

#define MAX_BUFFER_SIZE 2<<10

unsigned int configure_server( int argc, char* argv[] );

// convenience method for printing diagnostic methods
void terminate( int status, char* msg );

// interrupt handler for abnormal program termination
void stop( int signal );

// releases all the resources
void cleanup();

int mode = 1;
char *buffer;
int server = INVALID_SOCKET;
int client = INVALID_SOCKET;

/** A simulator of a TATS sensor used for testing Orion.
 * @author Casey Shields
 */
int main( int argc, char *argv[] ) {
    MIDC01 * midc01 = 0;
    buffer = malloc( MAX_BUFFER_SIZE );
    memset( buffer, 0, MAX_BUFFER_SIZE );

    // get server address, should be in dotted quad notation
    char* ip = get_arg( argc, argv, "-ip", "127.0.0.1");

    // get server socket port number
    unsigned short port; // port
    char* arg = get_arg( argc, argv, "-port", "43210" );//8080" );
    if( arg ) port = atoi( arg );

    // register an event handler for abnormal exit
    signal( SIGTERM , cleanup );
    signal( SIGABRT , cleanup );
    signal( SIGINT , cleanup );
    // sigaction(); // is this linux specific?

    // register a cleanup routine with program termination
    atexit( cleanup );

    // load sockets
    int result = socket_load();
    if( result != 0 )
        terminate( result, "Failed to initialize sockets" );

    // create a TCP socket for connecting to the server
    server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( server == INVALID_SOCKET )
        terminate( socket_error(), "Failed to create socket");

    // construct server address structure
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET; // internet address family
    address.sin_addr.s_addr = inet_addr( ip ); // server ip
    address.sin_port = htons( port ); // local port

    // bind socket to host network
    result = bind( server, (struct sockaddr *) &address, sizeof(address) );
    if( result == SOCKET_ERROR )
        terminate( socket_error(), "Failed to bind server socket" );

    // set socket to listen for incoming connections
    result = listen( server, MAX_CONNECTIONS );
    if( result < 0 )
        terminate(result, "Failed to listen");

    // accept Orion clients
    while( mode ) { // todo need a way to gracefully exit

        printf("Awaiting client.\n");
        client = accept(server, NULL, NULL); // Ctrl-C happens on the accept() thread, so my registered Handler doesn't run?
        if (client == INVALID_SOCKET) {
            //terminate(WSAGetLastError(), "failed to accept client connection\n");
//            printf( "[%d] Failed to accept client connection\n\0", socket_error() );
//            mode = 0;
            continue;
        }

        printf("Client connected.\n");
        do {
            memset(buffer, 0, MAX_BUFFER_SIZE);
            int result = recv(client, buffer, MAX_BUFFER_SIZE, 0);
            if (result > 0) {

                char midc = *((char*)buffer);
                switch( midc ) {
                    case TATS_TRK_DATA:
                        midc01 = (MIDC01*)buffer;
                        tats_print_midc01( midc01, stdout );
                        int crc = crc16( (char*) midc01, 20);
                        if( midc01->crc != crc )
                            printf("Error: sent=%04X, calc=%04X\n", midc01->crc, crc);
                        break;

                    // this is not normally a valid message, but we'll use it to force a shutdown.
                    case TATS_LAST_MSG:
                        printf( "Recieved last message:\n", buffer );
                        mode = 0;
                        break;

                    default:
                        printf("unknown message:\n\t" );
                        for(int n=0; n<result; n++)
                            printf( "%X ", buffer[n]);
                        printf("\n");
                }
//              result = send( socket, &entry, sizeof(entry), 0);
//              if( result == SOCKET_ERROR )
//                  terminate( WSAGetLastError(), "Failed to send back the entry");

            } else if (result == 0) {
                // terminate(0, NULL);
                printf( "[%d] Client closed connection\n\0", socket_error());
                socket_close( client );
                mode = 0;
                break;

            } else {
                //terminate(WSAGetLastError(), "Failed to read from client\n");
                printf( "[%d] Failed to read from client\n\0", socket_error());
                socket_close( client );
                mode = 0;
                break;
            }
            fflush(stdout);
        } while ( mode );
    }

    fprintf( stderr, "Main loop exited [%d]\n", mode );

    terminate(0, NULL);
}

void terminate(int status, char* msg) {
    if (msg != NULL)
        fprintf(stderr, "Error : %s : %d\n", msg, status);
    exit( status );
}

void stop( int signal ) {
    // probably shouldn't do IO in a signal handler
//    fprintf(stdout, "signal handler invoked [%d]\n", signal );
//    fflush( stdout );
    mode = 0;
}

void cleanup() {
    fprintf( stderr, "entering cleanup\n" );

    // release resources
    if( buffer != NULL )
        free( buffer );

    if( server != INVALID_SOCKET )
        socket_close( server );

    if( client != INVALID_SOCKET )
        socket_close( client );

    // release socket resources
    socket_unload();
}

// retrieve a list of available sockets for the server
//    struct sockaddr_in hints;
//    mwmser(&hints, 0, sizeof(hints));
//    hints.ai_family = AF_INET;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//    hints.ai_flags = AI_PASSIVE;
//
//    result = getaddrinfo(NULL, DEFAULT_PORT, &hints, server);
//    if( result!=0 ) {
//        result = WSAGetLastError();
//        freeaddrinfo(server);
//        orion_exit( result, "failed to get server socket information" );
//    }
//    freeaddrinfo(address);// free the linked list of results if getaddrinfo() was used


//unsigned int configure_server(int argc, char* argv[]) {
//    int result;
//
//    // get server address, should be in dotted quad notation
//    char* ip = get_arg( argc, argv, "-ip", "127.0.0.1");
//
//    // get server socket port number
//    unsigned short port; // port
//    char* arg = get_arg( argc, argv, "-port", "43210" );//8080" );
//    if( arg ) port = atoi( arg );
//
//    // load the winsock library
//    WSADATA wsadata;
//    result = WSAStartup( MAKEWORD(2, 2), &wsadata);
//    if( result != 0 )
//        terminate( result, "Failed to initialize winsock" );
//
//    // create a TCP socket for connecting to the server
//    unsigned int server = INVALID_SOCKET; // listening socket descriptor
//    server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
//    if( server == INVALID_SOCKET )
//        terminate( WSAGetLastError(), "Failed to create socket");
//
//
//    // construct server address structure
//    struct sockaddr_in address;
//    memset(&address, 0, sizeof(address));
//    address.sin_family = AF_INET; // internet address family
//    address.sin_addr.s_addr = inet_addr( ip ); // server ip
//    //address.sin_addr.s_addr = htonl( INADDR_ANY ); // any incoming interface
//    address.sin_port = htons( port ); // local port
//
//    // bind socket to host network
//    result = bind( server, (struct sockaddr *) &address, sizeof(address) );
//    if( result == SOCKET_ERROR )
//        terminate( WSAGetLastError(), "Failed to bind server socket" );
//
//    //freeaddrinfo(address);// free the linked list of results if getaddrinfo() was used
//
//    // set socket to listen for incoming connections
//    result = listen( server, MAX_CONNECTIONS );
//    if( result < 0 )
//        terminate(result, "Failed to listen");
//
//    return server;
//}

/* Debug note; if sensor crashes you need to release the socket. On windows try;
 * netstat -ano | findstr <port>
 * taskkill /f /im <pid>
 * */