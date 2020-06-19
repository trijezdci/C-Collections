/* Simple overhead-free function-local exception handling
 *
 *  @file bailout.h
 *  Structured goto
 *  
 *  This file ("bailout.h") is hereby released into the Public Domain.
 *
 *  Goto is considered harmful,  but there are cases where goto makes sense:
 *
 *  The for, while, repeat and other loops in structured programming languages
 *  are all  gotos in disguise,  so are if-else and case or switch statements.
 *  These statements have been introduced in order to replace unstructured and
 *  potentially dangerous or otherwise undesirable usage cases of goto  with a
 *  structured equivalent.  Where such structured equivalents  are  available,
 *  they should be used instead of goto.  Unfortunately,  there is  one  usage
 *  scenario for goto  which does not have  any structured equivalent  in most
 *  languages, if any: the use of goto in order to bail out of a function when
 *  an unrecoverable error has occurred,  but not before a neccessary sequence
 *  of clean-up operations has been performed.
 *
 *  For this scenario, C programmers often use goto and if done properly, then
 *  this can lead to both better readability as well as better performance due
 *  to the reduced chance of instruction cache misses in modern CPUs with long
 *  instruction pipelines.  Language purists often frown upon  even this usage
 *  of goto  even though it should be considered justified.  Perhaps it is the
 *  fact that goto is used verbatim  which is most irritating.  The problem is
 *  simply the absence of a structured equivalent for this usage scenario.
 *
 *  In the same manner in which for, while, repeat, if and case or switch have
 *  been introduced to replace other usage scenarios of goto,  the use of goto
 *  for cleaning up and bailing out of a function  could also be replaced by a
 *  a structured equivalent.  One possible way to design such a replacement is
 *  to introduce two new statements  "bailout" and "on error" to the syntax of
 *  the target language.  In C,  the same could be achieved by using macros to
 *  define those statements  and  let the pre-processor replace them with goto
 *  statements and labels accordingly.  This file introduces two such macros.
 *
 *  An example how the macros are used is shown below:
 *
 *      foo = alloc(foo_size);
 *      if (foo == NULL) BAILOUT(foo_alloc_failure);
 *
 *      bar = alloc(bar_size);
 *      if (bar == NULL) BAILOUT(bar_alloc_failure);
 *
 *      baz = alloc(baz_size);
 *      if (baz == NULL) BAILOUT(baz_alloc_failure);
 *
 *      ...
 *
 *      *status = SUCCESS;
 *      return result;  // successful return
 *
 *      // error handling section
 *
 *      ON_ERROR(baz_alloc_failure) :
 *          free(bar);  // undo allocation of bar
 *
 *      ON_ERROR(bar_alloc_failure) :
 *          free(foo);  // undo allocation of foo
 *
 *      ON_ERROR(foo_alloc_failure) :
 *          *status = ALLOCATION_FAILED;
 *          return NULL;  // unsuccessful return
 *
 *  } // end of function
 *
 */


#ifndef BAILOUT_H
#define BAILOUT_H


// ---------------------------------------------------------------------------
// macro:  BAILOUT ( error_label )
// ---------------------------------------------------------------------------
//
// Transfers program control to label <error_label>  within the error handling
// section at the end of the function in which BAILOUT() is invoked.
//
// Invocation of this macro will cause a compilation error if no corresponding
// ON_ERROR() macro is present within the function.

#define BAILOUT(_label) { goto _bailout_ ## _label ; }


// ---------------------------------------------------------------------------
// macro:  BAILOUT ( error_label )
// ---------------------------------------------------------------------------
//
// Defines a label <error_label>  for error handline code in an error handling
// section at the end of a function.  The macro must be followed by a colon.
//
// Error labels should only be placed at the end of a function  within the top
// level block of the function.  Neither BAILOUT()  nor goto should be invoked
// within a function following an invocation of ON_ERROR().

#define ON_ERROR(_label) _bailout_ ## _label


#endif /* BAILOUT_H */

// END OF FILE