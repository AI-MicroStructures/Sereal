#ifndef SRL_PROTOCOL_H_
#define SRL_PROTOCOL_H_

/*
 * Sereal Protocol version 1, see constants below docs.
 *
 * Generally speaking, structures are serialized depth-first and each item
 * is preceded/defined by a 1-byte control character:
 *
 * +--------+-----------------+-------------+--------------------------------------------------------------------------
 *          |Bit              | follow      | Description
 *          | 7 6 5 4 3 2 1 0 | bytes       | 
 * ---------+-----------------+-------------+------------------------------------------------------------
 *          | F 0 0 s x x x x | -           | tiny ints
 * POS      |       0 x x x x | -           | Positive nibble   0 .. 15
 * NEG      |       1 x x x x | -           | Negative nibble -16 .. -1
 * 
 *          | F 0 1 0 0 x x x
 * VARINT   |           0 0 0 | varint      | varint
 * ZIPPED   |           0 0 1 | varint      | zipped varint
 * 
 * RESERVED - varint indicates length to skip to if a reader does not handle the type, with 0 meaning "die".
 *          |           0 1 0 | varint      | *reserved*
 *          |           0 1 1 | varint      | *reserved*
 *          |           1 0 0 | varint      | *reserved*
 *          |           1 0 1 | varint      | *reserved*
 *          |           1 1 0 | varint      | *reserved* 
 *          |           1 1 1 | varint      | *reserved*
 * 
 * NUMLIST  | F 0 1 0 1 s y y | varint pad? | numeric array (s=0 unsigned, s=1 signed), varint=length, pad if needed for alignment
 *                      s 0 0 |             | varint (unsigned) or zipped (signed)
 *                      s 0 1 |             | of 16 bit
 *                      s 1 0 |             | of 32 bit
 *                      s 1 1 |             | of 64 bit
 * 
 *          | F 0 1 1 0 y y y |             | Ref/Object(ish)
 * FWDREF   |           0 0 0 |             | scalar ref to next item
 * REF      |           0 0 1 | varint?     | scalar ref to the item indicated by varint
 * HASH     |           0 1 0 | varint      | hash, varint=length
 * ARRAY    |           0 1 1 | varint      | array, varint=length 
 * BLESS    |           1 0 0 | TAG(STR) TAG| bless item into class indicated by TAG
 * BLESSV   |           1 0 1 | varint   TAG| bless item into class indicated by varint *provisional*
 * WEAKEN   |           1 1 0 |             | Following item is a reference and it is weakened
 *          |           1 1 1 |             | *reserved* FIXME what to do with this?
 * 
 *          | F 0 1 1 1 y y y |             | Miscellaneous        
 * STRING   |           0 0 x | varint      | string, x= utf8 flag, varint=length
 * ALIAS    |           0 1 0 | varint      | alias to previous item indicated by varint
 * COPY     |           0 1 1 | varint      | copy item at offset
 * UNDEF    |           1 0 0 | -           | undef
 * REGEXP   |           1 0 1 | TAG         | next item is a regexp 
 * FLOAT    |           1 1 0 | (FLOAT)     | float
 * PAD      |           1 1 1 |             | ignored byte, used by encoder to pad if necessary
 * 
 * ASCII    | F 1 x x x x x x | str         | Short ascii string, x=length
 * 
 * 
 * 
 * F = Flag bit to indicate if the item needs to be tracked during deserialization.
 *     The offset of the tag byte should be remembered, so that it can be referenced
 *     later.
 * 
 * * Dealing with self referential and cyclic structures:
 * While dumping any item with a refcount>1 (including weakrefs) the offset of the tag
 * needs to be tracked. The items F flag is NOT set. Should the item later be encountered
 * during dumping an alias or ref item will be generated with the offset in a varint, and
 * the F flag will be set. 
 * 
 * * Handling objects
 * During dumping the dumper is expected to maintain a mapping of class name to id. Whenever
 * it encounters a new class name it emits a "declare class tag" and then emits the appropriate
 * ref tag with the "is class" bit set.
 *    
 * * Varints
 * Varints are variable length integers where the high bit of each segment (normally a byte
 * but in some cases less) indicates if there is another byte to follow, with the bytes in 
 * least significant order first.
 *
 *
 * TODO: FALSE?
 * TODO: What's with floats?
 */

/* Note: both indicating protocol version, keep in sync */
#define SRL_PROTOCOL_VERSION 1
#define SRL_MAGIC_STRING "srl\x01"

/* Useful constants */
/* See also range constants below for the header byte */
#define SRL_ASCII_SHORT_STRING_MAX_LEN (2 << 5) /* six bits */
#define SRL_POS_INT_MAX_SIZE           ((2 << 4) - 1) /* five bits */
#define SRL_NEG_INT_MAX_SIZE           ((2 << 3) - 1) /* four bits */

/* FIXME usefulness of the below are a bit unclear*/

/* All with F bit unset! */
/* _LOW and _HIGH versions refering to INCLUSIVE range boundaries */
#define SRL_HDR_ASCII_LOW     ((char)0b01000000)
#define SRL_HDR_ASCII_HIGH    ((char)0b01111111)
#define SRL_HDR_POS_LOW       ((char)0b00000000)
#define SRL_HDR_POS_HIGH      ((char)0b00001111)
#define SRL_HDR_NEG_LOW       ((char)0b00010000)
#define SRL_HDR_NEG_HIGH      ((char)0b00011111)

#define SRL_HDR_VARINT        ((char)0b00100000)
#define SRL_HDR_ZIPPED        ((char)0b00100001)

/* Note: Can do reserved check with a range now, but as we start using
 *       them, might have to explicit == check later. */
#define SRL_HDR_RESERVED_LOW  ((char)0b00100010)
#define SRL_HDR_RESERVED_HIGH ((char)0b00100111)

#define SRL_HDR_NUMLIST_VAR_U ((char)0b00101000) /* unsigned varint numlist */
#define SRL_HDR_NUMLIST_VAR_S ((char)0b00101100) /* signed varint numlist */
#define SRL_HDR_NUMLIST_16_U  ((char)0b00101001) /* unsigned 16bit numlist */
#define SRL_HDR_NUMLIST_16_S  ((char)0b00101101) /* signed 16bit numlist */
#define SRL_HDR_NUMLIST_32_U  ((char)0b00101010) /* unsigned 32bit numlist */
#define SRL_HDR_NUMLIST_32_S  ((char)0b00101110) /* signed 32bit numlist */
#define SRL_HDR_NUMLIST_64_U  ((char)0b00101011) /* unsigned 64bit numlist */
#define SRL_HDR_NUMLIST_64_S  ((char)0b00101111) /* signed 64bit numlist */

#define SRL_HDR_FWDREF        ((char)0b00110000) /* ref to next item */
#define SRL_HDR_REF           ((char)0b00110001) /* ref with varint offset */
#define SRL_HDR_HASH          ((char)0b00110010)
#define SRL_HDR_ARRAY         ((char)0b00110011)
#define SRL_HDR_BLESS         ((char)0b00110100)
#define SRL_HDR_BLESSV        ((char)0b00110101) /* provisional */
#define SRL_HDR_WEAKEN        ((char)0b00110110)
#define SRL_HDR_FIXME         ((char)0b00110111) /* FIXME do something with this!!! */

#define SRL_HDR_STRING      ((char)0b00111000)
#define SRL_HDR_STRING_UTF8 ((char)0b00111001)
#define SRL_HDR_ALIAS       ((char)0b00111010)
#define SRL_HDR_COPY        ((char)0b00111011)
#define SRL_HDR_UNDEF       ((char)0b00111100)
#define SRL_HDR_REGEXP      ((char)0b00111101)
#define SRL_HDR_FLOAT       ((char)0b00111110)
#define SRL_HDR_PAD         ((char)0b00111111)
 
/* TODO */

#endi
