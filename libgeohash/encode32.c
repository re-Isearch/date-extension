#include <stdio.h>

// edz: lets speed-up things

static const char char_map[33] = "0123456789bcdefghjkmnpqrstuvwxyz";


static int index_for_char (char ch)
{
  const char *tcp = char_map;
  while (*tcp && *tcp != ch) tcp++;
  if (*tcp) return tcp - char_map;
  return -1;
}

int main()
{
  printf ("int char_map[] = {");
  for (int i = 0; i <= 127; i++) {
     if (i % 16 == 0) printf("\n  ");
      printf ("%*d, ", 3, index_for_char(i));
  }

  printf("\n};\n");
}
;
