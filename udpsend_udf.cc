
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include "mysql.h"  // IWYU pragma: keep
#include "mysql/udf_registration_types.h"

extern "C" bool udpsend_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  if (args->arg_count != 3) {
    strcpy(message, "Wrong number of arguments. udpsend(ip, port, data)");
    return true;
  }
  args->arg_type[0] = STRING_RESULT;
  args->arg_type[1] = INT_RESULT;
  args->arg_type[2] = STRING_RESULT;

  return false;
}

extern "C" void udpsend_deinit(UDF_INIT *) {}


extern "C" long long udpsend(UDF_INIT *, UDF_ARGS *args, unsigned char *null_value, unsigned char *) {
  char ip_buff[128];
  unsigned ip_length;
  int port;
  char * data_ptr;
  int data_length;
  int socket_fd;
  struct sockaddr_in server;

  if (!args->args[0] || !(ip_length = args->lengths[0])) {
    *null_value = 1;
    return 0;
  }
  if (ip_length >= sizeof(ip_buff))
    ip_length = sizeof(ip_buff) - 1;

  memcpy(ip_buff, (char *)(args->args[0]), ip_length);
  ip_buff[ip_length] = 0;

  port = (int)*(long long *)(args->args[1]);

  if (!args->args[2] || !(data_length = args->lengths[2])) {
    *null_value = 1;
    return 0;
  }
  data_ptr = (char *)(args->args[2]);

  socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd == -1) {
    /* fprintf(stderr, "socket error %d (%s)", errno, strerror(errno)); */
    *null_value = 1;
    return 0;
  }

  server.sin_addr.s_addr = inet_addr(ip_buff);
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  if (sendto(socket_fd, data_ptr, data_length, 0, (struct sockaddr *)&server, sizeof(server)) < 0) {
    /* fprintf(stderr, "sendto error %d (%s)", errno, strerror(errno)); */
    *null_value = 1;
    return 0;
  }

  close(socket_fd);

  return 1;
}

