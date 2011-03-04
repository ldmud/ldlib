/*------------------------------------------------------------------
 * Higher-order functions foldl and foldr.
 *
 * Author: Alexander Motzkau <Gnomi@UNItopia.de>
 *------------------------------------------------------------------
 */

/*
SYNOPSIS
        mixed foldl(mixed *arg, mixed start, closure func, mixed extra...)

DESCRIPTION
        Walk through the array <arg> from left to right and call

          func(start, element, extra...)

        for each element in <arg>. The result is taken as the
        <start> value for the next element. The last result
        is returned by foldl.

EXAMPLES
        To get the sum of the weights of all items in <ob> call:

          foldl(all_inventory(ob), 0, (: $1 + $2->query_weight() :))

NOTES
        foldr() does not only process the given array in the opposite
        order, also the arguments to the closure are interchanged.

SEE ALSO
        foldr(), map(E), filter(E), funcall(E), apply(E)
*/
mixed foldl(mixed *array, mixed startwert, closure func, varargs mixed * args)
{
    mixed wert = startwert;

    foreach(mixed elem: array)
        wert = apply(func, wert, elem, args);

    return wert;
}

/*
SYNOPSIS
        mixed foldr(mixed *arg, mixed start, closure func, mixed extra...)

DESCRIPTION
        Walk through the array <arg> from right to left and call

          func(element, start, extra...)

        for each element in <arg>. The result is taken as the
        <start> value for the next element. The last result
        is returned by foldr.

EXAMPLES
        To get the sum of the weights of all items in <ob> call:

          foldr(all_inventory(ob), 0, (: $1->query_weight() + $2 :))

NOTES
        foldl() does not only process the given array in the opposite
        order, also the arguments to the closure are interchanged.

SEE ALSO
        foldl(), map(E), filter(E), funcall(E), apply(E)
*/
mixed foldr(mixed *array, mixed startwert, closure func, varargs mixed * args)
{
    mixed wert = startwert;

    for(int i=sizeof(array); i--; )
        wert = apply(func, array[i], wert, args);

    return wert;
}
