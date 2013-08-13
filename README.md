random-copy
===========

cp clone losely based on P2P client behaviour or a P2P download simulator

It tries to mimic behaviour of P2P clients such as
 * Pre-allocating space for the file on-disk
 * Copy start and end of the file first
 * Get random segments of the file with some timeout in between
