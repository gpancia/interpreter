#include <stdlib.h>
#include <stdio.h>
#include "lexer_utils.h"

int main(int argc, char *argv[])
{
  update_interrupts();
  print_all_tokens2(argc, argv);
  return 0;
}
