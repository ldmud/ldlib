/*------------------------------------------------------------------
 * Bitstring functions read_bits and write_bits.
 *
 * Author: Alexander Motzkau <Gnomi@UNItopia.de>
 *------------------------------------------------------------------
 */

/*
SYNOPSIS
        int read_bits(string str, int start, int length)

DESCRIPTION
        Read a number from the bitstring <str> starting at <start>
        and having <length> bits and return that number as result.

SEE ALSO
        write_bits(), set_bit(E), clear_bit(E), last_bit(E), next_bit(E),
        test_bit(E),  count_bits(E), and_bits(E), or_bits(E), xor_bits(E),
        invert_bits(E), copy_bits(E)
*/
int read_bits(string str, int start, int length)
{
    int result = 0;
    start += length;

    while(length--)
        result = (result<<1) + test_bit(str, --start);
    return result;
}

/*
SYNOPSIS
        string write_bits(string str, int start, int length, int number)

DESCRIPTION
        Writes the number <number> into the bitstring <str> beginning
        at bit <start> and having <length> bits and return the new
        string.

SEE ALSO
        read_bits(), set_bit(E), clear_bit(E), last_bit(E), next_bit(E),
        test_bit(E),  count_bits(E), and_bits(E), or_bits(E), xor_bits(E),
        invert_bits(E), copy_bits(E)
*/
string write_bits(string str, int start, int length, int number)
{
    string result = str;

    while(length--)
    {
        if(number&1)
            result = set_bit(result, start++);
        else
            result = clear_bit(result, start++);
        number >>= 1;
    }

    return result;
}
