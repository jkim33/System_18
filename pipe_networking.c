#include "pipe_networking.h"


/*=========================
  server_handshake
  args: int * to_client

  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
  printf("SERVER: making fifo\n");
  mkfifo("/tmp/fifo", 0644);

  printf("SERVER: making upstream pipe\n");
  int up = open("/tmp/fifo", O_RDONLY);
  char* msg = malloc(HANDSHAKE_BUFFER_SIZE);
  read(up, msg, HANDSHAKE_BUFFER_SIZE);
  printf("SERVER: message recieved\n");
  remove("/tmp/fifo");

  printf("SERVER: making downstream pipe\n");
  int down = open(msg, O_WRONLY);
  write(down, msg, HANDSHAKE_BUFFER_SIZE);
  printf("SERVER: message sent\n");

  read(up, msg, HANDSHAKE_BUFFER_SIZE);
  if (strncmp(ACK, msg, HANDSHAKE_BUFFER_SIZE) == 0) {
    printf("SERVER: connection established\n");
  }
  else {
    printf("Connection not established\n");
    return 1;
  }

  char* input = malloc(BUFFER_SIZE);
  read(up, input, BUFFER_SIZE);
  printf("FROM THE CLIENT: %s\n", input);
  write(down, input, BUFFER_SIZE);

  free(msg);
  free(input);
  
  *to_client = down;
  return up;
}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  char* priv = malloc(HANDSHAKE_BUFFER_SIZE);
  sprintf(priv, "/tmp/%d", getpid());
  printf("CLIENT: making priv\n");
  mkfifo(priv, 0644);
  
  printf("CLIENT: making upstream pipe\n");
  int up = open("/tmp/fifo", O_WRONLY);
  write(up, priv, HANDSHAKE_BUFFER_SIZE);
  printf("CLIENT: message sent\n");

  printf("CLIENT: making downstream pipe\n");
  int down = open(priv, O_RDONLY);
  char* msg = malloc(HANDSHAKE_BUFFER_SIZE);
  read(down, msg, HANDSHAKE_BUFFER_SIZE);
  if (strncmp(priv, msg, HANDSHAKE_BUFFER_SIZE) == 0) {
    printf("CLIENT: message recieved. connection established\n");
  }
  else {
    printf("Connection not established\n");
    return 1;
  }
  remove(priv);

  printf("CLIENT: sending connection message\n");
  write(up, ACK, HANDSHAKE_BUFFER_SIZE);

  printf("Tell the Server Something: ");
  char* input = malloc(BUFFER_SIZE);
  fgets(input, BUFFER_SIZE, stdin);
  input[strlen(input)-1] = 0;
  write(up, input, BUFFER_SIZE);

  char* output = malloc(BUFFER_SIZE);
  read(down, output, BUFFER_SIZE);
  printf("FROM THE SERVER: %s\n", output);

  free(msg);
  free(input);
  free(output);
  
  *to_server = up;
  return down;
}
