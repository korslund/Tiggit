#include "comp85.hpp"

#include <string.h>
#include <stdexcept>
#include <stdint.h>
#include <assert.h>

// Digits used in Compact 85
static const char* comp85 = "!()*+,-./0123456789:;<=>@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_abcdefghijklmnopqrstuvwxyz{|}~";

/* Complete ASCII series from ! to ~:

   !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~

   Removed are:
   "#$%&' ? \ `
*/

static void fail85(const std::string &msg)
{
  throw std::runtime_error("Comp85 decode error: " + msg);
}

static uint8_t decode85_char(char ch)
{
  if(ch < '!' || ch > '~' ||
     (ch > '!' && ch < '(') ||
     ch == '?' || ch == '\\' ||
     ch == '`')
    fail85(std::string("Invalid character ") + ch);

  // Subtract away missing ascii values
  if(ch > '`') ch -= '!' + 9;
  else if(ch > '\\') ch -= '!' + 8;
  else if(ch > '?') ch -= '!' + 7;
  else if(ch > '\'') ch -= '!' + 6;
  else ch -= '!';

  assert(ch >= 0 && ch < 85);

  return ch;
}

static uint32_t decode_value(const char *in)
{
  /* It's very important to use 64 bit unsigned values in calculations
     if you want to catch overflows. The compiler doesn't like using
     64 bit values, and will avoid it if possible.
   */
  uint64_t pow = 1;
  uint64_t value = 0;
  for(int i=0; i<5; i++)
    {
      uint64_t dec = decode85_char(in[i]);
      value += pow * dec;
      pow *= 85;
    }

  /* Make sure the total calculated value does not exceed 2^32-1, the
     maximum value of a 32 bit int.

     That is a SECURITY ISSUE, as clients may depend on a strict
     one-to-one map between Comp85 and binary. Breaking this may cause
     unexpected behavior, which exposes the client to security risks.
   */
  if(value > 0xffffffff)
    fail85("Overflow, invalid Comp85 code " + std::string(in,5));

  return value;
}

static void encode_value(uint32_t value, char *out)
{
  for(int i=0; i<5; i++)
    {
      out[i] = comp85[value % 85];
      value /= 85;
    }
}

int Comp85::calc_binary(int ascii_len)
{
  int blocks = ascii_len / 5;
  int rem = ascii_len % 5;
  if(rem > 1) rem--;
  return 4*blocks + rem;
}

int Comp85::calc_ascii(int binary_len)
{
  int blocks = binary_len / 4;
  int rem = binary_len % 4;
  if(rem) rem++;
  return 5*blocks + rem;
}

int Comp85::encode(const void *inptr, int in_size, char *out)
{
  int written = 0;
  const char *in = (const char*)inptr;
  for(;in_size>=4; in_size-=4)
    {
      encode_value(*((uint32_t*)in), out);
      in += 4;
      out += 5;
      written += 5;
    }
  if(in_size)
    {
      assert(in_size > 0 && in_size < 4);

      char buf[5];
      uint32_t val = 0;
      memcpy(&val, in, in_size);
      encode_value(val, buf);

      // Use in_size as output size from this point
      in_size++;

      // Copy data
      memcpy(out, buf, in_size);

      // Cut one byte extra if we can
      if(in_size == 2 && val < 85)
        {
          assert(buf[1] == '!');
          in_size = 1;
        }

      /* Notice that we copy the data BEFORE cutting size. In the case
         where a cut is possible, we still store the last character,
         which will always be '!'.

         This means that if the user chooses to use the length
         returned by calc_ascii() instead of our returned value, they
         will include this extra, valid character. Thus decode() will
         still produce the correct output.
       */
      written += in_size;
    }

  return written;
}

int Comp85::decode(const char *in, int in_size, void *outptr)
{
  int written = 0;
  uint32_t val;

  char *out = (char*)outptr;

  for(;in_size>=5; in_size-=5)
    {
      val = decode_value(in);
      memcpy(out, &val, 4);
      in += 5;
      out += 4;
      written += 4;
    }
  if(in_size)
    {
      assert(in_size > 0 && in_size < 5);

      char buf[6] = "!!!!!";
      memcpy(&buf, in, in_size);
      val = decode_value(buf);

      // Use in_size as output size from this point
      if(in_size > 1)
        in_size--;

      // Is the value too large to store in the allocated output?
      if((in_size == 3 && val >= 0x1000000) ||
         (in_size == 2 && val >= 0x10000) ||
         (in_size == 1 && val >= 0x100))
         fail85("Overflow, invalid Comp85 code.");

      memcpy(out, &val, in_size);
      written += in_size;
    }

  return written;
}

std::string Comp85::encode(const void *binary, int count)
{
  std::string res;
  res.reserve(calc_ascii(count));
  const char *in = (const char*)binary;
  while(count)
    {
      int cnt = count;
      if(cnt > 4) cnt = 4;

      char buf[5];
      int len = encode(in, cnt, buf);
      res.append(buf,len);

      count -= cnt;
      in += cnt;
    }
  return res;
}

int Comp85::decode(const std::string &ascii, void *out)
{
  return decode(ascii.c_str(), ascii.size(), out);
}
