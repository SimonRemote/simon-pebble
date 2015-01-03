#include <pebble.h>

// variables for strtok1
static char *p_start = NULL; /* the pointer to the start of the current fragment */
static char *p_end = NULL; /* the pointer to the end of the current fragment */

char *strtok1(char *str, const char delimiter)
{
  if (str)
  {
    p_start = str;
  }
  else
  {
    if (p_end == NULL) return NULL;
    p_start = p_end+1;
  }
  for (p_end = p_start; true; p_end++)
  {
    if (*p_end == '\0')
    {
      p_end = NULL;
      break;
    }
    if (*p_end == delimiter)
    {
      *p_end = '\0';
      break;
    }
  }
  return p_start;
}
