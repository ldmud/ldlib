/*------------------------------------------------------------------
 * escape_string
 *
 * Author: Andreas Klauer <Menaures@UNItopia.de>
 *------------------------------------------------------------------
 */
#include <regexp.h>
#include "escape_string.h"

private static closure escape_pcre_wildcard;
private static closure escape_regexp_wildcard;
private static closure escape_regexp_nocase;
private static closure escape_replace;
private static closure escape_getdir;

/*
SYNOPSIS
        string escape_string(string|string* str [, int mode])

DESCRIPTION
        This function inserts escape characters (\) into the input
        string according to serveral rules and returns the result.
        The applied rules depend on <mode>.

        Such escaped strings can be used in regular expressions
        even if they contain special characters. So this function
        is suitable to process inputs from players.

        For use with regular expressions the following modes
        can be used and combined:

          ESCAPE_REGEXP:       (Default)
            Returns a regular expression that matches <str>.

          ESCAPE_PCRE:
            Returns a PCRE that matches <str>.

          ESCAPE_ANY:          (Default)
            The returned regular expression matches strings
            that contain <str>.

          ESCAPE_EXACT:
            The returned regular expression matches <str> exactly.
            (No characters before or after.)

          ESCAPE_WORD:
            The returned regular expression matches <str>
            as a word in a givens string.

          ESCAPE_CASE:         (Default)
            The returned regular expression is case sensitive.

          ESCAPE_NOCASE:
            The returned regular expression is case insensitive.

          ESCAPE_LIST:
            If <str> is a string it is interpreted as a comma separated
            list of strings that should be matched. <str> can also be
            an array of strings to be matched, in that case
            ESCAPE_LIST is automatically assumed.

          ESCAPE_WILDCARD:
            <str> can contain simple wildcards that will translated
            into the regular expression accordingly:

              ?    for any single character
              *    for any number of characters (or none at all)

            These special characters can be escaped themselves.

              \?   for ?
              \*   for *

        The following values for <mode> are not combinable:

          ESCAPE_REPLACE:
            Returns a replace pattern for regreplace(), which will
            replace a matched string exactly with <str> even if
            it contains special characters.

          ESCAPE_GETDIR:
            Returns a string for use with get_dir() that matches
            files with the name <str> even if it contains
            wildcards or escape characters.

EXAMPLES
        scape_string("(foo|bar)")
          -> The returned pattern matches "(foo|bar)", "bla (foo|bar) bla".

        escape_string("(foo|bar)", ESCAPE_PCRE)
          -> The same but as a PCRE.

        escape_string("(foo|bar)", ESCAPE_EXACT)
          -> The pattern matches solely "(foo|bar)".

        escape_string("bla*", ESCAPE_WILDCARD)
          -> The returned pattern matches "bla", "blafasel",
             "/pfad/zum/blablubb"

        escape_string("bla*", ESCAPE_WILDCARD|ESCAPE_EXACT)
          -> Now the pattern doesn't match "/pfad/zum/blablubb".

        escape_string("\\1&\\2&\\3", ESCAPE_REPLACE)
          -> Used as a replace pattern, "\1&\2&\3" will be inserted.

        escape_string("table, chair, couch", ESCAPE_LIST)
          -> The pattern matches "table", "chair" or "couch",
             but also "stable", "wheelchair" or "couching".

        escape_string("table, chair, couch",
                      ESCAPE_LIST|ESCAPE_CASE|ESCAPE_WORD)
          -> Now the pattern only matches "table", "chair" or "cauch",
             but not "stable" and the like. Also it's case insensitive.

        escape_string("", ESCAPE_LIST)
        escape_string(" ", ESCAPE_LIST)
        escape_string(" , ,,,", ESCAPE_LIST)
          -> These are empty lists, so 0 is returned.

SEE ALSO
        regexp(E), regreplace(E), get_dir(E)
*/
varargs string escape_string(mixed str, int mode)
{
#define ESCAPE_LAMBDA(x) lambda( ({'s}), ({#'||, ({#'[, x, 's}), "\\\\&"}) )

    mixed ret;

    // --- Parameter: ---

    if(intp(mode))
    {
        if(mode & ESCAPE_GETDIR)
        {
            if(mode != ESCAPE_GETDIR)
            {
                raise_error("Bad arg 2 to escape_string: ESCAPE_GETDIR not passed exclusively\n");
            }

            else if(!stringp(str))
            {
                raise_error("Bad arg 1 to escape_string: ESCAPE_GETDIR requires string\n");
            }
        }

        else if(mode & ESCAPE_REPLACE)
        {
            if(mode != ESCAPE_REPLACE)
            {
                raise_error("Bad arg 2 to escape_string: ESCAPE_REPLACE not passed exclusively\n");
            }

            else if(!stringp(str))
            {
                raise_error("Bad arg 1 to escape_string: ESCAPE_REPLACE requires string\n");
            }
        }
    }

    else
    {
        raise_error("Bad arg 2 to escape_string: not an int\n");
    }

    if(stringp(str))
    {
        if(mode & ESCAPE_LIST)
        {
            ret = str = regexplode(str, "[ \t]*,[ \t]*", RE_OMIT_DELIM) - ({""});
        }

        else
        {
            ret = ({ str });
        }
    }

    else if(pointerp(str))
    {
        mode |= ESCAPE_LIST;
        ret = str;
    }

    else
    {
        raise_error("Bad argument 1 to escape_string(): not a string or pointer\n");
    }

    // --- Exklusive flags: ---
    if(mode == ESCAPE_GETDIR)
    {
        escape_getdir ||= ESCAPE_LAMBDA(
            ([ "\\\\": "\\\\\\\\\\\\\\\\", // escaped backslash
               "\\":   "\\\\\\\\",         // normal backslash
               "\\?":  "\\\\\\\\\\\\?",    // escaped ?
               "\\*":  "\\\\\\\\\\\\*",    // escaped *
            ]) );

        return regreplace(str, "\\\\\\\\|\\\\[?*]|[?*\\\\]",
                          escape_getdir, RE_GLOBAL);
    }

    if(mode == ESCAPE_REPLACE)
    {
        // No differences between traditional regexps and PCRE?
        escape_replace ||= ESCAPE_LAMBDA(
            ([ "\\\\":  "\\\\\\\\\\\\\\\\", // escaped backslash
               "\\":    "\\\\\\\\",         // normal backslash
               "\\&":   "\\\\\\\\\\\\\\&",  // escaped &
            ]) );

        return regreplace(str, "\\\\\\\\|\\\\[0-9&]|[&\\\\]",
                               escape_replace, RE_GLOBAL);
    }

    // --- PCRE: ---
    if(mode & ESCAPE_PCRE)
    {
        if(mode & ESCAPE_WILDCARD)
        {
            escape_pcre_wildcard ||= ESCAPE_LAMBDA(
                ([ "\\\\": "\\\\",                // escaped backslash
                   "\\E":  "\\\\E\\\\\\\\E\\\\Q", // PCRE quote
                   "\\*":  "*",                   // escaped *,
                   "*":    "\\\\E.*\\\\Q",        // wildcard *
                   "\\?":  "?",                   // escapted ?
                   "?":    "\\\\E.\\\\Q"          // wildcard ?
                ]) );

            ret = map(ret, #'regreplace, "\\\\\\\\|\\\\E|\\\\[*?]|[\\\\?*]",
                                         escape_pcre_wildcard, RE_GLOBAL);
        }

        else
        {
            ret = map(ret, #'regreplace, "\\\\E",
                                         "\\\\E\\\\\\\\E\\\\Q", RE_GLOBAL);
        }

        return sprintf("%s(%s\\Q%s\\E)%s",
                      mode & ESCAPE_EXACT ? "^"
                                          : mode & ESCAPE_WORD ? "\\b"
                                                               : "",
                      mode & ESCAPE_NOCASE ? "(?i)" : "",
                      implode(ret, "\\E|\\Q"),
                      mode & ESCAPE_EXACT ? "$"
                                          : mode & ESCAPE_WORD ? "\\b"
                                                               : "");
    }

    // --- REGEXP: ---
    else // mode & ESCAPE_REGEXP
    {
        if(mode & ESCAPE_WILDCARD)
        {
            escape_regexp_wildcard ||= ESCAPE_LAMBDA(
                ([ "\\\\":  "\\\\\\\\", // escaped backslash
                   "\\*":   "\\\\*",    // escaped *
                   "*":     ".*",       // wildcard *
                   "\\?":   "?",        // escaped ?
                   "?":     "."         // wildcard ?
                ]) );

            ret = map(ret, #'regreplace, "\\\\\\\\|\\\\[<>*?]|[\\\\?*.^$|()+[\\]]",
                                         escape_regexp_wildcard, RE_GLOBAL);
        }

        else
        {
            // Escape regexp special characters
            ret = map(ret, #'regreplace, "\\\\[<>]|[\\\\*.^$|()+[\\]]",
                                         "\\\\&", RE_GLOBAL);
        }

        if(mode & ESCAPE_NOCASE)
        {
            escape_regexp_nocase ||= function string (string s)
            {
                // This line looks that cryptic because it's from Gnomi.
                return ("["+implode(map(transpose_array(({explode(upper_case(s),""),explode(lower_case(s),"")})),#'implode,""),"][")+"]");
            };

            ret = map(ret, #'regreplace, "[A-Za-z]+",
                                         escape_regexp_nocase, RE_GLOBAL);
        }

        return sprintf("%s(%s)%s",
                       mode & ESCAPE_EXACT ? "^"
                                           : mode & ESCAPE_WORD ? "\\<"
                                                                : "",
                       implode(ret, "|"),
                       mode & ESCAPE_EXACT ? "$"
                                           : mode & ESCAPE_WORD ? "\\>"
                                                                : "");
    }

#undef ESCAPE_LAMBDA
}
