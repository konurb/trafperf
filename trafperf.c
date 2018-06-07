/*
 * 
 *  Compilation of program we can use:
 *  gcc trafperf.c gnuplot_i.c -o traffic -lpthread
 *  or you can use Makefile
 * 
 *  for compilation this module the gnuplot_i.c gnuplot_i.h are used from:
 *  http://people.sc.fsu.edu/~jburkardt/c_src/gnuplot_i/gnuplot_i.html
 * 
 *  gnuplot: gnuplot 5.2 patchlevel 3 https://sourceforge.net/projects/gnuplot/files/gnuplot/5.2.3/
 * Four arguments expected.
 * -s<Server IP adress> -p<TCP Port> -l<Size of Segment> -n<Max no of clients>
 * -c<Server IP adress> -p<TCP Port> -l<Size of Segment> -i<No of iterations>
 * 
 */

#include"gnuplot_i.h"    // for gnu plot for drawnings the graphs 
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<string.h>
#include<unistd.h>
#include<ncurses.h>
#include<stdbool.h>
#include<time.h>



#define BILLION 1E9 // nano

typedef struct {
  uint32_t  *filename;	 // IP of the connected client
  int       *filedesc;   // file descriptor of connected client
  int       *iteration;  // iteration of threads
  int       *payload;    // payload of data
  int       *no_clients; // number of clients allowed by server
} struct_data ;


typedef struct entry_points {
  struct sockaddr_in dest_sock_addr; // socket info about clients
  struct sockaddr_in server_var;     // socket info about our server
  int port_no;                       // port number
  int payload;                       // payload to sent
  int num_iteration;                 // number of iterations running by client
  int node_id;                       // 1 - server; 0 client
  int num_clients;
} entry_points ;


int randomize();
int random_sleep();
double measure_bandwidth(double timer_count , double bytes, int file_desc);
void *server_handler(void *arguments);
void help_func(void);
char *print_ip_addr(uint32_t ip);
void server_runner(struct entry_points* data);
void client_runner(struct entry_points* data);
struct entry_points set_entry_points(int argc, char **argv);
void print_itera(double mbw, char* ip_address_to_file, int file_desc, int itera, int number_clients);

//TESTS -- to be done
//void checking_ip_conversion(uint32_t ip);

int payload = 0;          // payload
char ip_address[15];      // IP address to conversion
double total_bytes[100];  // total bytes recieved


// MAIN FUNCTION
int main(int argc, char **argv)
{
 int seed;
 time_t tt_number;        // number time in seconds
 seed = time(&tt_number); // seed for rand
 srand(seed);

 struct entry_points new_entries = set_entry_points( argc, argv);

 printf("client :%d\n", new_entries.dest_sock_addr.sin_addr.s_addr);
 printf("serwer :%d\n", new_entries.server_var.sin_addr.s_addr);
 printf("port no :%d\n", new_entries.port_no);
 printf("payload : %d\n", new_entries.payload);
 printf("iterat :%d\n", new_entries.num_iteration);
 printf("node id :%d\n", new_entries.node_id);

 uint32_t test;
 //test=369141952; //test
 //checking_ip_conversion(test);
 //struct sockaddr_in dest_sock_addr;            // struct to store information about machine we want to connect to
 //char payload_data[new_entries.payload]; // the table of char in bytes as a payload to sent to

 // running server part of program
 if( new_entries.node_id == 1 ) server_runner(&new_entries);

 // running client part of program
 if( new_entries.node_id == 0 ) client_runner(&new_entries);

 return 0;
}
// END OF MAIN FUNCTION



// Functions used:
// choosing the numbers from 1 - 10
int randomize() {
 int number = rand()%100000 + 1;
 while(number>10) {
    number = number%10;
 }
 if(number == 0 )
   return 10;
 else
   return number;
}

// choosing the numbers for sleep function
int random_sleep() {
  int basement = 100000;
  int returned = 1;
  while(basement!=1) {
    returned = returned + basement*(randomize()-1);
    basement = basement /10 ;  
  }
  printf("%d\n", returned);
  return returned;
}


// choosing server or client and parameters from arguments from command line
struct entry_points set_entry_points(int argc, char **argv){
   struct entry_points entry;
   printf("Program name: %s\n", argv[0]);

   if(argc == 1 && argv[1][0]=='-' && argv[1][1]=='h') {
	 help_func();
   }
   else if(argc < 5 ) {
    printf("Four arguments expected.\n");
    printf("-s<Server IP adress> -p<TCP Port> -l<Size of Segment> -n<Max no of clients>\n");
    printf("-c<Server IP adress> -p<TCP Port> -l<Size of Segment> -i<No of iterations>\n");
    help_func();
    return entry;
   }
   else if(argc >=6) {
    printf("Four arguments expected.\n");
    printf("-s<Server IP adress> -p<TCP Port> -l<Size of Segment> -n<Max no of clients>\n");
    printf("-c<Server IP adress> -p<TCP Port> -l<Size of Segment> -i<No of iterations>\n");
    help_func();
    return entry;
 }
   while ((argc > 1) && (argv[1][0] == '-'))
   {
    switch (argv[1][1])
        {
            case 'c': // client
                printf("%s\n",&argv[1][2]);
                // inet_addr() converts from IP to binary data in network byte order
                entry.dest_sock_addr.sin_addr.s_addr=inet_addr(&argv[1][2]);
                entry.node_id = 0;
                break;

            case 's': // server
                printf("%s\n",&argv[1][2]);
                // inet_addr() converts from IP to binary data in network byte order
                entry.server_var.sin_addr.s_addr=inet_addr(&argv[1][2]);
                entry.node_id = 1;
                break;

            case 'p': // port
                printf("%s\n",&argv[1][2]);
                // atoi() - converts string to integer
                entry.port_no = atoi(&argv[1][2]);
                break;

            case 'l': // lenght of TCP segment
                printf("%s\n",&argv[1][2]);
                // atoi() - converts string to integer
                entry.payload = atoi(&argv[1][2]);
                break;

            case 'i': // number of iterations for client
                printf("%s\n",&argv[1][2]);
                entry.num_iteration = atoi(&argv[1][2]);
                break;

            case 'n': // number of iterations for client
                printf("%s\n",&argv[1][2]);
                entry.num_clients = atoi(&argv[1][2]);
                break;

            case 'h': // calling help function
                printf("%s\n",&argv[1][2]);
                help_func();
                break;

            default: // calling help functio
                printf("Wrong Argument: %s\n", argv[1]);
                help_func();
                break;
        }
        ++argv;
        --argc;
    }
    if(entry.node_id == 1) {
        entry.num_iteration = 0 ;
        entry.dest_sock_addr.sin_addr.s_addr = 0;
    }
    if(entry.node_id == 0) entry.server_var.sin_addr.s_addr = 0;
 return entry;
}

// running server part of program
void server_runner(struct entry_points* new_entries)
{
  int no_iterate;
  int client_fd;
  //int value = 10 ;
  pthread_t work_thread[new_entries->num_clients];
  socklen_t socksize = sizeof(struct sockaddr_in);
  
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);          // socket which we can use for network connection
  new_entries->server_var.sin_port = new_entries->port_no;  // set a destination port number
  new_entries->server_var.sin_family = AF_INET;             // stores a code for the address family. set to symbolic constant AF_INET
  sleep(1);
  
  //bind server informtion to server_fd
  if(bind(server_fd, (struct sockaddr*)&new_entries->server_var, sizeof(new_entries->server_var)) >= 0)
  {
   printf("\nSocket has been created.\n");
   
   //start listening
   listen(server_fd, 0);
   while(1)        //loop 
   {
    no_iterate = 0;
    // int no_iteration = 0 ;
    // to be done -> creation of logic for waiting the thread to finish when we would like to open a new thread.
    while(no_iterate <= new_entries->num_clients)
    //for(no_iterate = 0 ; no_iterate <= new_entries->num_clients ; ++no_iterate )
    {
     //no_iterate = no_iteration;
     client_fd=accept(server_fd, (struct sockaddr *)&new_entries->dest_sock_addr, &socksize); // file descriptor for the accepted socket
     // printf("\nConnected to client ID: %d .\n", client_fd);
     // printf("\nIncomming connection from: %s .\n", inet_ntoa(client_var.sin_addr));
     struct_data *arguments = malloc(sizeof *arguments);  // memory alocation
     arguments->filename=&new_entries->dest_sock_addr.sin_addr.s_addr;  // the filename of IP client
     arguments->filedesc=&client_fd;                                    // the client file descriptor
     arguments->iteration=&no_iterate;                                  // iteration number of client
     arguments->payload=&new_entries->payload;                          // payload
     arguments->no_clients=&new_entries->num_clients;                   // number of clients connected
     initscr(); // init ncurses mode
     pthread_create(&work_thread[no_iterate], NULL, *server_handler, arguments); //creating thread
     no_iterate++;
    }
   }
   close(server_fd);
  }
  else{
   printf("\nSocket creation has been failed.\n");
  }
}


// running client part of program
void client_runner(struct entry_points* new_entries)
{
  char payload_data[new_entries->payload];
  int socket_file_descriptor=socket(AF_INET, SOCK_STREAM, 0);  // socket which we can use for network connection
  new_entries->dest_sock_addr.sin_port=new_entries->port_no;   // set a destination port number
  new_entries->dest_sock_addr.sin_family=AF_INET;              // stores a code for the address family. set to symbolic constant AF_INET
 
  // make a connection to the maachine  specifiedin dest_sock_addr
  if(connect(socket_file_descriptor, (struct sockaddr*)&new_entries->dest_sock_addr, sizeof(new_entries->dest_sock_addr)) >= 0) 
  {
   usleep(1000000); // wait 1 sec before sending first TCP segment to the server
                    // 1000 == 1 ms ; 10000 == 10 ms ; 100000 = 100 ms ; 1000000 =1000 ms
   // filling the payload
   for(int i = 0 ; i < new_entries->payload ; i++)
   {
     payload_data[i] = 'N';
   }
   // printing the payload
   for(int i = 0 ; i< new_entries->payload ; i++)
   {
    printf("%c",payload_data[i]);
   }
   //
   printf("Connected to server %d\n",socket_file_descriptor);
      while(new_entries->num_iteration>0)
      {
       int j =0 ;
       printf("Iteration_%d_\n",new_entries->num_iteration);
       for(int i = 0 ; i < new_entries->payload ; i++)
       {
         if(payload_data[i] != '\0') j++ ;
       }
         printf("\nSending counted to the Server: %d kbit == %d bits == %d bytes\n ", j*8/1024, j*8, j);
         write(socket_file_descriptor, payload_data, new_entries->payload); // sending the bytes in payload to the connected server
         new_entries->num_iteration--;  // iterator how many times we sent the number_data
         usleep(random_sleep());        // timer between sending the number_data [random]
       }
       close(socket_file_descriptor);          // closing socket file descriptor
  }
  else
  {
   printf("\nSocket creation failed.\n");
  }
}

// function printing the throughput on the screen from every client connected to the server
void print_itera(double mbw, char* ip_address_to_file, int file_desc, int itera, int number_clients){
  mvprintw(itera,0,"BW %lf bytes/s == %lf kbit/s IP: %s FD: %d Iter: %d", mbw/8, mbw/1024, ip_address_to_file, file_desc, itera);
  if(itera == number_clients){
    mvprintw(itera+1,0,"MAXIMUM NUMBER OF CLIENTS REACHED");
  }
}

// function server handler
void *server_handler(void *arguments)
{
 //system("clear");
 FILE *logfile1;                                 // pointer to file
 struct_data *current_args = arguments;          // pointer to struct
 struct timespec req_start, req_end;             // start stop
 char ip_address_to_file[32];                    // ip address
 char filename[64];                              // filename

 int file_desc = *current_args->filedesc;        // file secriptor from thread
 int itera = *current_args->iteration ;          // iteration number of thread
 int payload = *current_args->payload;           // payload
 int number_clients = *current_args->no_clients; // no of clients
 uint32_t temporar =  *current_args->filename;   // IP address for filename
 double bytes_read;
 double mbw = 0;
 char buf[sizeof(char)*payload];               // buffer to read 
 double timer_count = 0;
 double timer_read = 0;

 //printf("\nInside thread : IP Incomming connection from %d .\n", *current_args->filename); // for debug purposes
 strcpy(ip_address_to_file, print_ip_addr(temporar));                                        // copy IP address
 //printf("\nInside thread : IP Incomming connection from %s .\n", ip_address_to_file);      // for debug purposes
 //printf("\nInside thread : Incomming connection from descriptor %d .\n", file_desc);       // for debug purposes
  
 sprintf(filename, "%d_%d_%s_test.txt", itera, file_desc, ip_address_to_file); // create name of file to store data  
 //sleep(2);
 logfile1=fopen(filename, "w+");                                               // creating file of name
 if(logfile1==NULL) printf("Unable to create file.");

 do
 {
     clock_gettime(CLOCK_REALTIME, &req_start);
     bytes_read = read(file_desc, buf, payload);      // reading bytes
     clock_gettime(CLOCK_REALTIME, &req_end);
     timer_read = (req_end.tv_sec - req_start.tv_sec) + (req_end.tv_nsec - req_start.tv_nsec) / BILLION;
     //printf("Timer timer_read: %lf\n", timer_read); // for debug purposes
     timer_count = timer_count + timer_read ;
     //printf("timer_count: %lf\n", timer_count);     // for debug purposes
     //printf("Bytes read bytes: %lf\n", bytes_read); // for debug purposes
     if (bytes_read == 0) break; // it is not good way to skip the loop. Better way it needs to be done.
     mbw = measure_bandwidth(timer_count, bytes_read, file_desc); // measure bandwidth
     // printf("BW %lf bit/s == %lf bytes/s == %lf kbit/s == %lf Mbit/s\n",mbw, mbw/8 , mbw/1024, mbw/(1024*1024));
     curs_set(0); // hide cursor
     //print iteration of counting bytes
     print_itera(mbw, ip_address_to_file, file_desc, itera, number_clients);  // print iteration of mesure bandwidth
     refresh();
     fprintf(logfile1,"%lf %lf\n", timer_count , mbw/(1024)); // kbit/s - output to the file

 } while(bytes_read!=0); //to cosider to define new possibility to stop the loop. right now it checks the o bytes read and exiting loop
 
 fclose(logfile1);
 //printf("\nClosing clientFileDescriptor: %d\n", file_desc); // for debug purposes
 total_bytes[file_desc] = 0 ;
 free(arguments);
 sleep(1);

 // graph functions
 gnuplot_ctrl * g ;   // referenced by a handle of type (pointer to) gnuplot_ctrl
 g = gnuplot_init();  // open a new gnuplot session
 gnuplot_cmd(g, "set terminal canvas");
 gnuplot_cmd(g, "set title \"Goodput\"");
 gnuplot_cmd(g, "set xlabel \"Time\"");
 gnuplot_cmd(g, "set ylabel \"Traffic\"");
 gnuplot_cmd(g, "set output \"%s.html\"",filename);
 //gnuplot_cmd(g, "set style func linespoints");
 //gnuplot_cmd(g, "plot '%s' with linespoints ps 1 pt 1",filename);
 gnuplot_cmd(g, "set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 0.2");
 gnuplot_cmd(g, "plot '%s' with linespoints ls 1", filename);
 //gnuplot_cmd(g, "plot '%s' with linespoints lt -1 pi -3 pt 7 ps 0.2",filename);
 sleep(1);
 gnuplot_close(g) ;

 close(file_desc); 
 sleep(1);
 endwin();
 return 0;
}

double measure_bandwidth(double timer_count , double bytes_read, int file_desc)
{
 double bandwidth[file_desc];
 total_bytes[file_desc] = total_bytes[file_desc] + bytes_read;
 //printf("BYTES RECIEVED in function: %lf\n", bytes_read );   // for debug purposes
 double current_bw = (total_bytes[file_desc] *8)/timer_count;  // calculate
 bandwidth[file_desc] = current_bw;
 return bandwidth[file_desc] ;
}

// printing help function
void help_func(void)
{
 printf("Usage:\n");
 printf(" -c<Server IP> or\n");
 printf(" -s<Server IP>\n");
 printf(" -p<Port Number>\n");
 printf(" -s<TCP Segment Size>\n");
 exit (1);
}

//converting IP address
char *print_ip_addr(uint32_t ip)
{
 struct in_addr ip_addr;
 // printf("The IP address before conversion is %d\n", ip);
 ip_addr.s_addr = ip;
 strcpy(ip_address,inet_ntoa(ip_addr));
 // printf("The IP address is %s\n", inet_ntoa(ip_addr));
 return ip_address;
}

// TESTS

//checking print_ip_addr 369141952 == 192.168.0.22
// TO BE DONE
/*
void checking_ip_conversion(uint32_t ip){
     struct in_addr ip_addr;
     ip_addr.s_addr = ip;
     char ip_address_to_file[32]; //comment
     strcpy(ip_address_to_file, print_ip_addr(ip));
     printf("IP after conversion: %s\n", ip_address_to_file);
     //printf("strlen 192.168.0.22 : %s\n", strlen("192.168.0.122"));
     printf("strlen ip_address_to_file: %s\n", ip_address_to_file);
     if( strcmp( "192.168.0.22", ip_address_to_file)){
       printf("TC:1: checking IP adress conversion: PASSED OK.\n"); 
     }
     else
     {
       printf("TC:1 checking IP adress conversion: NOK\n"); 
     }
}*/
