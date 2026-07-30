#include <errno.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PROC_PATH "/proc/teclado_virtual"

static int send_key_to_proc(int fd, int keycode)
{
   char buffer[16];
   int length = snprintf(buffer, sizeof(buffer), "%d\n", keycode);

   if (length < 0 || length >= (int)sizeof(buffer)) {
      errno = EINVAL;
      return -1;
   }

   if (write(fd, buffer, length) != length)
      return -1;

   return 0;
}

int main(void)
{
   int fd = open(PROC_PATH, O_WRONLY);

   if (fd < 0) {
      perror("open /proc/teclado_virtual");
      return 1;
   }

   const int keys[] = { KEY_P, KEY_R, KEY_U, KEY_E, KEY_B, KEY_A };
   size_t count = sizeof(keys) / sizeof(keys[0]);

   for (size_t index = 0; index < count; index++) {
      if (send_key_to_proc(fd, keys[index]) < 0) {
         fprintf(stderr, "Error escribiendo tecla %d: %s\n", keys[index], strerror(errno));
         close(fd);
         return 1;
      }
   }

   close(fd);
   return 0;
}