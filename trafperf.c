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



#define BILLION 1E9 // nano



double measure_bandwidth(double timer_count , double bytes, int file_desc);
void *server_handler(void *arguments);
void help_func(void);
char *print_ip_addr(uint32_t ip);

//TESTS
//void checking_ip_conversion(uint32_t ip);

int file_desc = 5;
int payload = 0;
char ip_address[32];
double total_bytes[5];

typedef struct {
  uint32_t  *filename;	// IP of the connected client
  int       *filedesc;  // file descriptor of connected client
  int       *iteration; // iteration of threads
} struct_data ;


int main(int argc, char **argv)
{

 struct sockaddr_in server_var;  //socket info about our server
 struct sockaddr_in client_var;  //socket info about machine connecting to server

 int client_fd;
 int i, port_no;
 int client = 0;
 int server = 0;
 pthread_t work_thread[20];
 socklen_t socksize = sizeof(struct sockaddr_in);


//part fror client:
 // number_data payload we would like to send in bytes
 int number_data; 
 // struct to store information about machine we want to connect to
 struct sockaddr_in dest_sock_addr;            
  
 
	printf("Program name: %s\n", argv[0]);

	 if(argc < 4 ) {
	   printf("Three arguments expected.\n");
	   printf("-s<Server IP adress> -p<TCP Port> -l<Size of Segment>\n");
	   printf("-c<Server IP adress> -p<TCP Port> -l<Size of Segment>\n");
	   return 1 ; 
	 }
	 else if(argc >=5) {
	   printf("Three arguments expected.\n");
	   printf("-s<Server IP adress> -p<TCP Port> -l<Size of Segment>\n");
	   printf("-c<Server IP adress> -p<TCP Port> -l<Size of Segment>\n");
	   return 1;
	 }

	while ((argc > 1) && (argv[1][0] == '-'))
	{
		switch (argv[1][1])
		{
			case 'c': // client
				printf("%s\n",&argv[1][2]);
                                // inet_addr() converts from IP to binary data in network byte order
                                dest_sock_addr.sin_addr.s_addr=inet_addr(&argv[1][2]);
                                client = 1;
				break;
 
			case 's': // server
				printf("%s\n",&argv[1][2]);
                                // inet_addr() converts from IP to binary data in network byte order
                                server_var.sin_addr.s_addr=inet_addr(&argv[1][2]); 
				server = 1;
                                break;

			case 'p': // port
				printf("%s\n",&argv[1][2]);
                                // atoi() - converts string to integer
                                port_no = atoi(&argv[1][2]);
				break;

			case 'l': // lenght of TCP segment
				printf("%s\n",&argv[1][2]);
                                // atoi() - converts string to integer
                                payload = atoi(&argv[1][2]);
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

 printf("client = %d ; server = %d \n", client , server);
 printf("server_var.sin_addr.s_addr = %d \n", server_var.sin_addr.s_addr);
 printf("port_no = %d \n",  port_no);
 printf("payload = %d \n", payload); // getting value for data payload
 
 uint32_t test;
 //test=369141952; //test
 //checking_ip_conversion(test);
 //struct sockaddr_in dest_sock_addr;            // struct to store information about machine we want to connect to
 char payload_data[payload]; // the table of char in bytes as a payload to sent to

 if( client==0 && server==1 )
 {
  int server_fd = socket(AF_INET,SOCK_STREAM,0);
  server_var.sin_port=port_no;
  server_var.sin_family=AF_INET;
  sleep(1);
 
  //bind server informtion to server_fd
  if(bind(server_fd,(struct sockaddr*)&server_var,sizeof(server_var))>=0)
  {
   printf("\nSocket has been created.\n");
   
   //start listening
   listen(server_fd,0);
   while(1)        //loop 
   {
    // to be done -> creation of logic for waiting the thread to finish when we would like to open a new thread.
    for(int i=0 ; i<5 ; i++ )  // 5 clients can served pararerly
    {     
     client_fd=accept(server_fd,(struct sockaddr *)&client_var,&socksize); // file descriptor for the accepted socket
     printf("\nConnected to client ID: %d .\n", client_fd);
     printf("\nIncomming connection from: %s .\n", inet_ntoa(client_var.sin_addr));
     struct_data *arguments = malloc(sizeof *arguments);  // memory alocation
     arguments->filename=&client_var.sin_addr.s_addr;     // the filename of IP client
     arguments->filedesc=&client_fd;                      // the client file descriptor
     arguments->iteration=&i;                             // iteration number of client
     pthread_create(&work_thread[i],NULL,*server_handler, arguments ); //creating thread
    }
   }
   close(server_fd);
  }
  else{
   printf("\nSocket creation has been failed.\n");
  }

 }
 else
 {
  int socket_file_descriptor=socket(AF_INET,SOCK_STREAM,0); // socket which we can use for network connection
  dest_sock_addr.sin_port=port_no;                          // set a destination port number
  dest_sock_addr.sin_family=AF_INET;                        // stores a code for the address family. set to symbolic constant AF_INET
 
  // make a connection to the maachine  specifiedin dest_sock_addr
  if(connect(socket_file_descriptor,(struct sockaddr*)&dest_sock_addr,sizeof(dest_sock_addr))>=0) 
  {
   usleep(1000000); // wait 1 sec after sending first TCP segment to the server
   int x = 100; //iterations
   // filling the payload
   for(int i = 0 ; i < payload ; i++)
   {
     payload_data[i] = 'N';
   }

   // printing the payload
   for(int i = 0 ; i< payload; i++)
   {
    printf("%c",payload_data[i]);
   }
   //
   printf("Connected to server %d\n",socket_file_descriptor);
	  while(x>0)
	  {
           int j =0 ;
	   printf("Iteration_%d_\n",x);

           for(int i = 0 ; i < payload ; i++)
           {
             if(payload_data[i] != '\0') j++ ;
           }
           printf("\nSending counted to the Server: %d kbit == %d bits == %d bytes\n ", j*8/1024, j*8, j);
	   write(socket_file_descriptor,payload_data,payload); // sending the bytes in payload to the connected server
           x--;                                                // iterator how many times we sent the number_data
           // sleep(1);                                             // timer between sending the number_data
           usleep(1000000); // 1000 == 1 ms ; 10000 == 10 ms ; 100000 = 100 ms ; 1000000 =1000 ms
          }
  close(socket_file_descriptor);                               // closing socket file descriptor 
  }
  else
  {
   printf("\nSocket creation failed.\n");
  }
 }
 return 0;
}

// function server handler
void *server_handler(void *arguments)
{
 FILE *logfile1;
 struct_data *current_args = arguments; 
 uint32_t temporar =  *current_args->filename;
 int file_desc = *current_args->filedesc;
 int itera = *current_args->iteration ;
 char ip_address_to_file[32];
 char filename[64];
 struct timespec requestStart, requestEnd;
 double bytes_read;
 double mbw = 0;
 char buf[sizeof(char)*payload];
 int i = 0;
 double timer_count = 0;
 double timer_read = 0;
 
 printf("\nInside thread : IP Incomming connection from %d .\n", *current_args->filename);
 strcpy(ip_address_to_file, print_ip_addr(temporar));
 printf("\nInside thread : IP Incomming connection from %s .\n", ip_address_to_file);
 printf("\nInside thread : Incomming connection from descriptor %d .\n", file_desc);
 
 sprintf(filename, "%d_%d_%s_test.txt",itera,file_desc,ip_address_to_file );  
 
 logfile1=fopen(filename,"w+"); 
 if(logfile1==NULL) printf("Unable to create file.");

 do
 { 
     clock_gettime(CLOCK_REALTIME, &requestStart);
     bytes_read = read(file_desc,buf,payload);
     clock_gettime(CLOCK_REALTIME, &requestEnd);
     timer_read = ( requestEnd.tv_sec - requestStart.tv_sec ) + ( requestEnd.tv_nsec - requestStart.tv_nsec ) / BILLION;
     printf("Timer timer_read: %lf\n", timer_read);
     timer_count = timer_count + timer_read ;
     printf("timer_count: %lf\n", timer_count);
     printf("Bytes read bytes: %lf\n", bytes_read);
     if (bytes_read == 0) break; // it is not good way to skip the loop. Better way it needs to be done.
     mbw = measure_bandwidth(timer_count,bytes_read,file_desc);
     printf("BW %lf bit/s == %lf bytes/s == %lf kbit/s == %lf Mbit/s\n",mbw, mbw/8 , mbw/1024, mbw/(1024*1024));
     fprintf(logfile1,"%lf %lf\n", timer_count , mbw/(1024) ); 

 } while(bytes_read!=0); //to cosider to define new possibility to stop the loop. right now it checks the o bytes read and exiting loop
 
 fclose(logfile1);
 printf("Closing clientFileDescriptor: %d\n", file_desc);
 //printf("Goodput: %f bit/s == %lf kbit/s\n",mbw, mbw/1024);
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
 //gnuplot_cmd(g, "plot '%s' with linespoints ps 4 pt 1",filename);
 gnuplot_cmd(g, "plot '%s' with linespoints ps 1 pt 1",filename);

 sleep(1);
 gnuplot_close(g) ;

 close(file_desc); 

}

double measure_bandwidth(double timer_count , double bytes_read, int file_desc)
{
    double bandwidth[file_desc];
    //double total_bytes ;
    total_bytes[file_desc] += bytes_read; 
    printf("BYTES RECIEVED in function: %lf\n", bytes_read );
    // calculate
    double current_bw = (total_bytes[file_desc] *8)/timer_count;
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
	exit (8);
}

//converting IP address
char *print_ip_addr(uint32_t ip)
{
    struct in_addr ip_addr;
    printf("The IP address before conversion is %d\n", ip);
    ip_addr.s_addr = ip;
    strcpy(ip_address,inet_ntoa(ip_addr));
    printf("The IP address is %s\n", inet_ntoa(ip_addr));
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
