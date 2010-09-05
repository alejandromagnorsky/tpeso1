     #include <stdio.h>
     #include <errno.h>
     #include <stdlib.h>
     #include <unistd.h>
     #include <string.h>
     #include <sys/types.h>
     #include <sys/socket.h>
     #include <netinet/in.h>
     #include <netdb.h>

     #define PORT            5555
     #define MESSAGE         "Yow!!! Are we having fun yet?!?"
     #define SERVERHOST     INADDR_LOOPBACK
     
 void
     init_sockaddr (struct sockaddr_in *name,
                    uint16_t port)
     {
       struct hostent *hostinfo;
     
       name->sin_family = AF_INET;
       name->sin_port = htons (port);
       name->sin_addr.s_addr = htonl(SERVERHOST);
     }


     void
     write_to_server (int filedes)
     {
       int nbytes;
     
       nbytes = write (filedes, MESSAGE, strlen(MESSAGE) + 1);
       if (nbytes < 0)
         {
           perror ("write");
           exit (EXIT_FAILURE);
         }
     }
     
     
     int
     main (void)
     {
       extern void init_sockaddr (struct sockaddr_in *name,
                                  uint16_t port);
       int sock;
       struct sockaddr_in servername;
     
       /* Create the socket. */
       sock = socket (PF_INET, SOCK_STREAM, 0);
       if (sock < 0)
         {
           perror ("socket (client)");
           exit (EXIT_FAILURE);
         }
     
       /* Connect to the server. */
       init_sockaddr (&servername, PORT);
       if (0 > connect (sock,
                        (struct sockaddr *) &servername,
                        sizeof (servername)))
         {
           perror ("connect (client)");
           exit (EXIT_FAILURE);
         }
     
       /* Send data to the server. */
       write_to_server (sock);
       close (sock);
       exit (EXIT_SUCCESS);
     }

