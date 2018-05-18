/* vi: set ts=2 shiftwidth=2 expandtab:
 *
 * VNPForth demonstration program.
 *
 * This module is the main module for a call to reimann_.ft.  Demonstrates
 * calling Forth words from C.
 *
 * Prints a summation of a Reimann Zeta function.
 */

#include "assert.h"
#include "string.h"
#include "forthlib.h"

extern void v4_reimann_Minuszeta (void); /* Forth reimann-zeta */

int main (int argc, char **argv) {
  static const char * message = "Reimann-zeta (x=2, n=10000) = ";
  assert (sizeof (char *) == sizeof (int));

  v4_c_push (2);                         /* Push 2 onto data stack */
  v4_c_push (10000);                     /* Push 10000 onto data stack */
  v4_reimann_Minuszeta ();               /* reimann-zeta */

  v4_c_push ((int) message);             /* Push result message address */
  v4_c_push (strlen (message));          /* Push result message length */
  v4_type ();                            /* Print, using Forth TYPE */

  v4_f_Dot ();                           /* Print result with F. */
  v4_cr ();                              /* CR */

  return 0;
}
