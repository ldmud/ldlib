/*------------------------------------------------------------------
 * call_with_this_player()
 *
 * Author: Alexander Motzkau <Gnomi@UNItopia.de>
 *------------------------------------------------------------------
 */

/*
SYNOPSIS
        mixed call_with_this_player(string fun|closure cl, mixed extra...)

DESCRIPTION
        Calls the function <fun> in the current object resp. the closure <cl>
        with the given arguments <extra>. For this call the current command
        giver (as returned by this_player()) will be set to the current
        object if it is a living object or 0 otherwise. Afterwards
        the current command giver will be restored to its original value.

SEE ALSO
        this_player(E), living(E)
*/
mixed call_with_this_player(mixed fun, varargs mixed *args)
{
    object to = this_object();
    object old_tp = this_player();
    mixed ret;

    efun::set_this_player(living(previous_object()) && previous_object());
    set_this_object(previous_object());	// So static functions will work.

    if(stringp(fun))
        ret = apply(#'call_other, previous_object(), fun, args);
    else
        ret = apply(fun, args);

    set_this_object(to);
    efun::set_this_player(old_tp);
    return ret;
}
