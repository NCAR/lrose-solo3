#include <sys/types.h>
#include <LittleEndian.hh>

/* c------------------------------------------------------------------------ */
/* Check if this is a little-endian host */

static bool endianSet = false;
static bool littleEndian = false;

bool hostIsLittleEndian()

{

  if (endianSet) {
    return littleEndian;
  }

  typedef union {
    u_int16_t d;
    u_int8_t bytes[2];
  } short_int_t;
  short_int_t short_int;
  short_int.d = 1;
  if (short_int.bytes[1] != 0) {
    littleEndian = false;
  } else {
    littleEndian = true;
  }

  endianSet = true;
  return littleEndian;

}

