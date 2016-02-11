#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define t_buffer 1024

/* Writing a e-mail. */
int main(){
  /* Variables */
  char ligne[t_buffer] = {0};
  FILE* corps = NULL;

  /* Begin */
  corps = fopen("tmp_mail_w.txt","w");
  if(corps == NULL)
    return 1;

  puts("[Nouveau message]");

  printf("TO : ");
  fgets(ligne, 120, stdin);

  fprintf(corps, "TO: %s\n", ligne);

  fclose(corps);

  return 0;
}
