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



double measure_bandwidth(double accum , double bytes);
void *server_handler(void *args);
void help_func(void);
char *print_ip_addr(uint32_t ip);

//TESTS
//void checking_ip_conversion(uint32_t ip);

int payload = 0;
char ip_address[32];

typedef struct {
  uint32_t  *filename;	
  int  *filedesc;
  int  *iteration;
} name_struct ;


int main(int argc, char **argv)
{

 struct sockaddr_in server_var;  //socket info about our server
 struct sockaddr_in client_var; //socket info about machine connecting to server

 int client_fd;
 int i, port_no;
 int client = 0;
 int server = 0;
 pthread_t t[20];
 socklen_t socksize = sizeof(struct sockaddr_in);
 char host[1024];
 char service[20];


//part fror client:
 int number_data; // data payload we would like to send in bytes
 struct sockaddr_in dest_sock_addr;            // struct to store information about machine we want to connect to
  
 
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
			case 'c':  //client
				printf("%s\n",&argv[1][2]);
                                dest_sock_addr.sin_addr.s_addr=inet_addr(&argv[1][2]); // IP adress [old 1] [new 2]
                                client = 1;
				break;
 
			case 's':  //server
				printf("%s\n",&argv[1][2]);
                                server_var.sin_addr.s_addr=inet_addr(&argv[1][2]); // IP adress [old 1] [new 2]
				server = 1;
                                break;

			case 'p':  //port
				printf("%s\n",&argv[1][2]);
                                port_no = atoi(&argv[1][2]); // Port number [old 2] [new 4]
				break;

			case 'l':  //lenght of TCP segment
				printf("%s\n",&argv[1][2]);
                                payload = atoi(&argv[1][2]);  // TCP segment size [old 3] [new 6]
				break;

                        case 'h':  //calling help function
				printf("%s\n",&argv[1][2]);
				help_func();
                                break;

			default: //calling help functio
				printf("Wrong Argument: %s\n", argv[1]);
				help_func();
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

 //if(1)
 if( client==0 && server==1 )
 {
  int server_fd = socket(AF_INET,SOCK_STREAM,0);
  server_var.sin_port=port_no;
  server_var.sin_family=AF_INET;
  sleep(1);
 
  //bind server informtion to server_fd
  if(bind(server_fd,(struct sockaddr*)&server_var,sizeof(server_var))>=0)
  {
   printf("\nSocket created.\n");
   
   //start listening
   listen(server_fd,0);
   while(1)        //loop 
   {
    for(int i=0 ; i<5 ; i++ )      // 5 clients can served pararerly
    {
     client_fd=accept(server_fd,(struct sockaddr *)&client_var,&socksize);
     printf("\nConnected to client ID: %d\n", client_fd);
     printf("\nIncomming connection from %s .\n", inet_ntoa(client_var.sin_addr));
     name_struct *args = malloc(sizeof *args);
     args->filename=&client_var.sin_addr.s_addr;
     args->filedesc=&client_fd;
     args->iteration=&i;
     pthread_create(&t[i],NULL,*server_handler, args );
     //pthread_join(t[i], NULL);
    }
   }
   close(server_fd);
  }
  else{
   printf("\nSocket creation failed.\n");
  }

 }
 else
 {
   int socket_file_descriptor=socket(AF_INET,SOCK_STREAM,0); // socket which we can use for network connection
  //dest_sock_addr.sin_addr.s_addr=inet_addr("192.168.0.22"); // set a destination IP number
  dest_sock_addr.sin_port=port_no;                             // set a destination port number
  dest_sock_addr.sin_family=AF_INET;                        //
  if(connect(socket_file_descriptor,(struct sockaddr*)&dest_sock_addr,sizeof(dest_sock_addr))>=0) // make a connection to the maachine  specifiedin dest_sock_addr
  {
   int x = 500; //iterations
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
	   write(socket_file_descriptor,payload_data,payload);  // sending the bytes in payload to the connected server
           x--;                                                  // iterator how many times we sent the number_data
          // int usleep(useconds_t usec);
          // sleep(1);                                             // timer between sending the number_data
          usleep(1000000); // 1000 == 1 ms ; 10000 == 10 ms ; 100000 = 100 ms ; 1000000 =1000 ms
          }
  close(socket_file_descriptor);                                  // closing the 
  }
  else
  {
   printf("\nSocket creation failed.\n");
  }
 }
 return 0;
}

// function server handler
void *server_handler(void *args)
{
 FILE *logfile1;
 name_struct *current_args = args; 
 uint32_t temporar =  *current_args->filename;
 int file_desc = *current_args->filedesc;
 int itera = *current_args->iteration ;
 char ip_address_to_file[32];
 char filename[64];
 struct timespec requestStart, requestEnd;
 double accum = 0;
 double timer_count = 0;
 double bytes;
 double bw = 0;
 char buf[sizeof(char)*payload];
 int i = 0;

 int first_segment = 1 ;
 
 printf("\nInside thread : IP Incomming connection from %d .\n", *current_args->filename);
 strcpy(ip_address_to_file, print_ip_addr(temporar));
 printf("\nInside thread : IP Incomming connection from %s .\n", ip_address_to_file);
 printf("\nInside thread : Incomming connection from descriptor %d .\n", file_desc);
 
 sprintf(filename, "%d_%d_%s_test.txt",itera,file_desc,ip_address_to_file );  
 
 logfile1=fopen(filename,"w+"); 
 if(logfile1==NULL) printf("Unable to create file.");

 do
 { 
   if(first_segment == 1 ) 
   {
     clock_gettime(CLOCK_REALTIME, &requestStart);
     bytes = read(*current_args->filedesc,buf,payload);
     clock_gettime(CLOCK_REALTIME, &requestEnd);
     bw = measure_bandwidth(accum,bytes);
     accum = ( requestEnd.tv_sec - requestStart.tv_sec ) + ( requestEnd.tv_nsec - requestStart.tv_nsec ) / BILLION;
     printf("Timer Accum: %lf\n", accum);
     timer_count = timer_count + accum ;
     printf("timer_count: %lf\n", timer_count);
     printf("Bytes read bytes: %lf\n", bytes);
     bw = measure_bandwidth(accum,bytes);
     printf("Avoid First Packet :BW %lf bit/s == %lf bytes/s == %lf kbit/s == %lf Mbit/s\n",bw, bw/8 , bw/1024, bw/(1024*1024));
     first_segment = 0 ;
    }
    else
    {
     clock_gettime(CLOCK_REALTIME, &requestStart);
     bytes = read(file_desc,buf,payload);
     clock_gettime(CLOCK_REALTIME, &requestEnd);
     accum = ( requestEnd.tv_sec - requestStart.tv_sec ) + ( requestEnd.tv_nsec - requestStart.tv_nsec ) / BILLION;
     printf("Timer Accum: %lf\n", accum);
     timer_count = timer_count + accum ;
     printf("timer_count: %lf\n", timer_count);
     printf("Bytes read bytes: %lf\n", bytes);
     bw = measure_bandwidth(accum,bytes);
     printf("BW %lf bit/s == %lf bytes/s == %lf kbit/s == %lf Mbit/s\n",bw, bw/8 , bw/1024, bw/(1024*1024));
     fprintf(logfile1,"%lf %lf\n", timer_count , bw/(1024) ); 
    }
 } while(bytes!=0); //to cosider to define new possibility to stop the loop. right now it checks the o bytes read and exiting loop
 fclose(logfile1);
 printf("Closing clientFileDescriptor: %d\n", file_desc);
 //printf("Goodput: %f bit/s == %lf kbit/s\n",bw, bw/1024);
 free(args);
 
 sleep(1);
 // graph functions
 gnuplot_ctrl * g = gnuplot_init();
 gnuplot_cmd(g, "set terminal canvas");
 gnuplot_cmd(g, "set title \"Goodput\"");
 gnuplot_cmd(g, "set xlabel \"Time\"");
 gnuplot_cmd(g, "set ylabel \"Traffic\"");
 gnuplot_cmd(g, "set output \"%s.html\"",filename);
 gnuplot_cmd(g, "plot '%s' with linespoints ps 4 pt 1",filename);

 sleep(1);
 gnuplot_close(g) ;

 close(file_desc); 

}

double measure_bandwidth(double accum , double bytes)
{
    double bandwidth = 0 ;
    double total_bytes = bytes; 
    printf("BYTES RECIEVED in function: %lf\n", bytes );
    // calculate
    double cur_bw = (total_bytes *8)/accum;
    bandwidth = cur_bw;

    return bandwidth ;
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
    //uint32_t ip = 2110443574;
    struct in_addr ip_addr;
    printf("The IP address before conversion is %d\n", ip);
    ip_addr.s_addr = ip;
    //char ip_address [32];
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
