 /* 
 * Author: Donnchadh Murphy
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>

#define PORT    "1066" /* Port to listen on */
#define BACKLOG     10  /* Passed to listen() */

int maxlives = 12;
char *word [] = {
# include "words"
};
# define NUM_OF_WORDS (sizeof (word) / sizeof (word [0]))
# define MAXLEN 80 /* Maximum size in the world of Any string */

void handle(int newsock, fd_set *set)
{
    /* send(), recv(), close() */
    /* Call FD_CLR(newsock, set) on disconnection */
}

int main(void)
{
    int sock;
    fd_set socks;
    fd_set readsocks;
    int maxsock;
    int reuseaddr = 1; /* True */
    struct addrinfo hints, *res;

    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        return 1;
    }

    /* Create the socket */
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        perror("socket");
        return 1;
    }

    /* Enable the socket to reuse the address */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }

    /* Bind to the address */
    if (bind(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        return 1;
    }

    freeaddrinfo(res);

    /* Listen */
    if (listen(sock, BACKLOG) == -1) {
        perror("listen");
        return 1;
    }

    /* Set up the fd_set */
    FD_ZERO(&socks);
    FD_SET(sock, &socks);
    maxsock = sock;

    /* Main loop */
    while (1) {
        unsigned int s;
        readsocks = socks;
        if (select(maxsock + 1, &readsocks, NULL, NULL, NULL) == -1) {
            perror("select");
            return 1;
        }
        for (s = 0; s <= maxsock; s++) {
            if (FD_ISSET(s, &readsocks)) {
                printf("socket %d was ready\n", s);
                if (s == sock) {
                    /* New connection */
                    int newsock;
                    struct sockaddr_in their_addr;
                    size_t size = sizeof(struct sockaddr_in);
                    newsock = accept(sock, (struct sockaddr*)&their_addr, &size);
                    if (newsock == -1) {
                        perror("accept");
                    }
                    else {
                        printf("Got a connection from %s on port %d\n",
                                inet_ntoa(their_addr.sin_addr), htons(their_addr.sin_port));
                        FD_SET(newsock, &socks);
                        play_hangman (newsock, newsock);
                        if (newsock > maxsock) {
                            maxsock = newsock;
                        }
                    }
                }
                else {
                    /* Handle read or disconnection */
                    handle(s, &socks);
                    //play_hangman (fd, fd);
                }
            }
        }

    }

    close(sock);

    return 0;
}


/* This block of code contains the hangman logic*/
play_hangman (int in, int out)
 {
    char * whole_word, part_word [MAXLEN],
    guess[MAXLEN], outbuf [MAXLEN];

    int lives = maxlives;
    int game_state = 'I';//I = Incomplete
    int i, good_guess, word_length;
    char hostname[MAXLEN];

    gethostname (hostname, MAXLEN);
    sprintf(outbuf, "Playing hangman on host% s: \n \n", hostname);
    write(out, outbuf, strlen (outbuf));

    /* Pick a word at random from the list */
    whole_word = word[rand() % NUM_OF_WORDS];
    word_length = strlen(whole_word);
    syslog (LOG_USER | LOG_INFO, "server chose hangman word %s", whole_word);

    /* No letters are guessed Initially */
    for (i = 0; i <word_length; i++)
        part_word[i]='-';
    
    part_word[i] = '\0';

    sprintf (outbuf, "%s %d \n", part_word, lives);
    write (out, outbuf, strlen(outbuf));

    while (game_state == 'I')
    /* Get a letter from player guess */
    {
        while (read (in, guess, MAXLEN) <0) {
            if (errno != EINTR)
                exit (4);
            printf ("re-read the startin \n");
            } /* Re-start read () if interrupted by signal */
    good_guess = 0;
    for (i = 0; i <word_length; i++) {
        if (guess [0] == whole_word [i]) {
        good_guess = 1;
        part_word [i] = whole_word [i];
        }
    }
    if (! good_guess) lives--;
    if (strcmp (whole_word, part_word) == 0)
        game_state = 'W'; /* W ==> User Won */
    else if (lives == 0) {
        game_state = 'L'; /* L ==> User Lost */
        strcpy (part_word, whole_word); /* User Show the word */
    }
    sprintf (outbuf, "%s %d \n", part_word, lives);
    write (out, outbuf, strlen (outbuf));
    }
 }

