/*------------------------------------------------------------------
 * cond_deep_present()
 *
 * Author: Alexander Motzkau <Gnomi@UNItopia.de>
 *------------------------------------------------------------------
 */
#include "present.h"

/*
SYNOPSIS
        object cond_deep_present(string str, object env [, int flags]
                                          [, mixed func] [, mixed extra...])

DESCRIPTION
        This functions searches for a object in the deep inventory of <env>.
        The first object <ob> that is identified with the id <str> and for
        whom ob->func(extra...) returns a value != 0 is returned.
        if <func> is a closure it will be called with the object as its
        first argument.

        The id will be checked by calling the lfun id() in the object.
        If <str> is 0 or empty the id check will be omitted.
        If <func> is not given no function will be called (i.e. the
        first object with the correct id will be returned).

        The order of traversal can be specified in <flags>:

        CDP_DEPTH_FIRST         Dive recursively into the inventory.

        CDP_BREADTH_FIRST       Look for the object with the least depth.

        CDP_FLAT                Don't recurse. Only look in the immediate
                                inventory.

        CDP_DIRECT_THEN_RECURSE First look at the immediate inventory,
        (default)               after that go recursively into each item.

SEE ALSO
        present(E), id(A)
*/
varargs object cond_deep_present(string was, object wo, int flags, mixed fun, varargs mixed args)
{
    mixed stack;

    if(stringp(fun))
    {
        args = ({fun}) + args;
        fun = #'call_other;
    }
    else if(!closurep(fun))
        fun = 1;

    if(!wo)
        return 0;

    switch(flags & CDP_TRAVERSE_BITS)
    {
        case CDP_DEPTH_FIRST:
            stack = ({first_inventory(wo)})-({0});
            while(sizeof(stack))
            {
                if((!was || stack[0]->id(was)) && apply(fun, stack[0], args))
                    return stack[0];
                stack = ({first_inventory(stack[0]), next_inventory(stack[0])}) - ({0}) + stack[1..<1];
            }
            break;

        case CDP_BREADTH_FIRST:
            stack = ({first_inventory(wo)})-({0});
            while(sizeof(stack))
            {
                if((!was || stack[0]->id(was)) && apply(fun, stack[0], args))
                    return stack[0];
                stack = (({next_inventory(stack[0])}) + stack[1..<1] + ({first_inventory(stack[0])})) - ({0});
            }
            break;

        case CDP_FLAT:
            foreach(object ob: all_inventory(wo))
                if((!was || ob->id(was)) && apply(fun, ob, args))
                    return ob;
            break;

        default:
            stack = ({ wo });
            while(sizeof(stack))
            {
                mixed inv = all_inventory(stack[0]);
                foreach(object ob:inv)
                    if((!was || ob->id(was)) && apply(fun, ob, args))
                        return ob;
                stack = inv + stack[1..<1];
            }
            break;
    }
}
