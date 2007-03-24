/* analysis.c - Generic pre-analysis
   Copyright (C) 2007 Matthieu Lemerre <racin@free.fr>

   This file is part of the L programming language.

   The L programming language is free software; you can redistribute it 
   and/or modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.
   
   The L programming language is distributed in the hope that it will be 
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
   GNU Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public License
   along with the L programming language; see the file COPYING.  If not,
   write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
   Boston, MA  02110-1301  USA.  */


/* This code handle all the processing needed before compiling can
   begin. For example:

   - Type definitions, and relationship
   - Function definition
   - Overload definition
   - Class definition
   - Generic definition
   - Parse definition ...

*/   




/* XXX: type definitions should be treated here.  */

/* XXX: also here should be recorded the association generic =>
   function + type of the function.  */


/* Associate to each defined generic a type.  Returns a list of forms
   to compile. (i.e. a list of (define function ... ), the rest is
   stripped.  Or maybe we should have another generic to enclose form
   generation in it.)  */

#include <assert.h>

#include <l/string.h>

#include "../objects/pair.h"
#include "../objects/function.h"
#include "../parser/form.h"

#include "c-to-l.h"
#include "analysis.h"

#include <l/sys/panic.h>
#include <l/sys/global.h>
#include <l/access.h>
#include <l/creator.h>

/*__thread*/ analysis_t current_analysis;

/*__thread*/ /*list_t to_compile_list;*/  /* List of things to compile,
  with type parameter.

  Note: this list should be global; this would allow for instance
  one-pass compilation of closures.

  Note: some things need to be compiled first, like generics and type
  functions.  

  XXX: it is in fact a list of function_t
  */

//list_t generic_compile_list;
/* List of generics to compile.  They should be compiled before the
   function list.  */

/* If generic compilation depends on another generics, we should add
   another level.  Maybe the "absolute dependencies" should be stored
   instead, i.e. the need to compile sth before sth else. */


/* Definers and define form.  */

/* The function used to define an object.

   Exemple: (define class 'toto'
               (define member 'truc' ...))

	       Truc is a member of class toto.

   Standard definers: function, type.

   Other definer: overloaded_function, global, thread_local ...
                  generic
                  macro
                  parse_hook
		  constant
		  pci_interface (par exemple pour un device driver dans un noyau)
		  service (pour du CORBA)
   */
MAKE_STATIC_HASH_TABLE (definer_hash);

/* A hash Symbol -> Global.  */
MAKE_STATIC_HASH_TABLE (global_hash);
extern hash_table_t generic_hash;

typedef void (*definer_t)(symbol_t, symbol_t, form_t);

/* XXX: for recursive types: just use a lazy evaluation technique, we
   transform a type definition to a type only when the type is needed. */
void
define_type (symbol_t define_symbol, symbol_t name, type_form_t type_form)
{
  Base_Type t = define_type_type_form (id_form (name), -1, -1, type_form);

  assert (t->type_type == BASE_TYPE);
  
  /*  Does not really belong here, but more to a "high-level type
     definition processing" part of the compiler.  */
  define_derived_creator( t, name);
  define_accesser (t, derived_accesser);
  define_left_accesser (t, derived_left_accesser);
}

/* A type alias is just another name for the same type.  It is
   equivalent to C's typedef. */
void
define_type_alias (symbol_t define_symbol, symbol_t name,
		   type_form_t type_form)
{
  Type type = intern_type (type_form);
  associate_type_with_type_form( id_form( name), type);
  // puthash (name, type, type_hash);
}


void *
compile_function_call_unknown_function (generic_form_t form);

location_t
compile_function_call (generic_form_t form);

static void
analysis_define_global( symbol_t define_symbol, symbol_t name,
	       form_t value);

#include <l/expand.h>
void
define_function (symbol_t define_symbol, symbol_t name,
		 generic_form_t form)
{
  form_t return_type = CAR (form->form_list);
  generic_form_t parameters = CAR (form->form_list->next);
  form_t body = CAR (form->form_list->next->next);

  /* XXX: construct function type from tuple type.  */

  
  form_t function_type = function_type_form (return_type, parameters);

  /* A function also declares a global variable.  */
  analysis_define_global( NULL, name, function_type);
  
//  generic_t cfcuf = MALLOC (generic);
//  cfcuf->compile = compile_function_call;//_unknown_function;
//  puthash (name, cfcuf, generic_hash);
  define_expander(name, expand_function);
  
  /* XXX: put fn on the to_compile list.  */

  current_analysis->functions = CONS (CONS (name, form), current_analysis->functions);
  //  panic ("Define_function should define the generic, and the function hash.");
  //  panic ("Put on the function to compile list.\n");
}

/* XXX For now it is assumed that generics don't depend on each
   other,i.e. that all generics are already defined. */


/* We need to transform:

def generic whilee(Form condition, Form body)
{
  loop_form (seq_form (if_form (condition, break_form (), null),
		       body))
}

into:

def function Form<-(Generic_Form _form)
{
  let List _form_alist = _form->form_alist;
  let Form condition = (Form) CDR(CAR(_form_alist));
  let Form body = (Form) CDR(CAR(CDR((_form_alist))));

  loop_form (seq_form (if_form (condition, break_form (), null),
		       body));

}
*/
void
define_constant(symbol_t define_symbol, symbol_t name,
		form_t value)
{
  panic( "TODO\n");

}

static void
analysis_define_global( symbol_t define_symbol, symbol_t name,
			form_t value)
{
  Type type = intern_type( value);
  global_t glob = MALLOC( global);
  glob->global_type = NORMAL_GLOBAL;
  glob->type = type;
  glob->handling_backend = NULL;
  glob->for_backend = NULL;

  puthash( name, glob, global_hash);
}


void
define_generic (symbol_t define_symbol, symbol_t name,
		generic_form_t form)
{
  /* XXX: we should have lazy compilation instead.  */


  /* Create a new symbol for the function generic.  */
  unsigned int str_len = strlen (name->name) + 10;
  char str[str_len];
  strcpy (str, name->name);
  strcat (str, "__generic");
  symbol_t new_symbol = intern (str);

  /* Create a new body that deconstructs the parameters from the
     list.  */
    /* XXX: In the future, generic could be called with the "funcall"
     function, that take a list and pass all the arguments in the
     stack, according to the function type.  This step would then be
     useless.  */

  assert (form->head == SYMBOL (lambda));

  generic_form_t parameters = CAR (form->form_list->next);
  form_t body = CAR (form->form_list->next->next);

  lispify (parameters);
  lispify (body);

  assert (parameters->head == SYMBOL (tuple));


  generic_form_t new_body;
  
  {
    list_t form_list;
    list_t *form_list_ptr = &form_list;

    int i = 0;
    panic ("To reimplement\n");
#if 0    
    FOREACH (element, parameters->form_alist)
      {
	id_form_t sf = CAR (CAR (element));
	assert (is_form (sf, id_form));
	symbol_t symbol = sf->value;
	
	form_t type_form = CDR (CAR (element));
	
	*form_list_ptr = CONS (let_form (type_form, symbol), NULL);
	form_list_ptr = &((*form_list_ptr)->next);

	form_t assignement = equal_form (id_form (symbol),
      				       generic_form_symbol (SYMBOL (ith_form),
							    CONS (id_form (SYMBOL (__form)),
								  CONS (int_form (i), NULL))));
	i++;

	*form_list_ptr = CONS (assignement, NULL);
	form_list_ptr = &((*form_list_ptr)->next);
	
      //      *form_list_ptr = CONS (
      }
#endif
    *form_list_ptr = CONS (body, NULL);

    new_body = block_form (form_list);
  }

  /* XXX: this is only for macros (currently), not for generics.  */
  new_body = generic_form_symbol (SYMBOL (compile), CONS (new_body, NULL));
  
  list_t lambda_parameters = CONS (id_form (SYMBOL (__form)), base_type_form (SYMBOL (Form)));
  
  generic_form_t lf = lambda_form (base_type_form (SYMBOL (Form)),
				   CONS (lambda_parameters, NULL),
				   new_body);

  form_t df = define_form (SYMBOL (function), new_symbol, lf);

  lispify (df);

  /* XXX: In the future, generic could be called with the "funcall"
     function, that take a list and pass all the arguments in the
     stack, according to the function type.  */

  
  define_function (define_symbol, new_symbol, form);
  
  generate (df);

  //  DEFINE_GENERIC_SYMBOL (name, function->address);

  
  
}


list_t
analyze (list_t form_list)
{
  current_analysis = MALLOC (analysis);
  
  list_t result = NULL;

  list_t *result_ptr = &result;
  
  FOREACH (element, form_list)
    {
      form_t form = CAR (element);
      //      *result_ptr = CONS (form, NULL);
      //      result_ptr = &((*result_ptr)->next);
      
      if(is_form (form, generic_form))
	{
	  generic_form_t gform = (generic_form_t) form;
	  if(gform->head == SYMBOL (define))
	    {
	      symbol_t type_defined, name;
	      type_defined = ((id_form_t) (CAR (gform->form_list)))->value;
	      name = ((id_form_t) (CAR (gform->form_list->next)))->value;
	      form_t subform = CAR (gform->form_list->next->next);
	      
	      *result_ptr = CONS (form, NULL);
	      result_ptr = &((*result_ptr)->next);
	      
	      definer_t definer = gethash (type_defined, definer_hash);

	      if(definer == NULL)
		panic ("Unknown definer\n");
	      definer (type_defined, name, subform);
	    }
	}
    }

  return result;
}




#if 0
void
run_analysis (list_t form_list)
{
  FOREACH (element, form_list)
    {
      form_t form = CAR (element);

      if(is_form (form, generic_form))
	{
	  generic_form_t gform = form;
	  if(gform->head == intern ("define"))
	    {
	      /* XXX: we should have define function int->int toto(a)
		 instead of define function (lambda int toto (int)).
		 Lambda is useless. */

	      symbol_form_t sf = CDR (CAR (gform->form_alist));
	      
	      assert (is_form (sf, symbol_form));
	      symbol_t type_symbol = sf->value; 

	      if(type_symbol == SYMBOL (function))
		{
		  
		  symbol_form_t name_form = CDR (CAR (gform->form_alist->next));
		  symbol_t name = name_form->value;

		  lambda_form_t lf = CDR (CAR (gform->form_alist->next->next));
		}
	      
	    }
	}
    }
  
}
#endif

#define DEFINE_DEFINER(string__, function__)			\
do {								\
  puthash (intern (string__), function__, definer_hash);	\
 } while(0)


void
init_analysis ()
{
  DEFINE_DEFINER ("function", define_function);
  DEFINE_DEFINER ("global", analysis_define_global);
  DEFINE_DEFINER ("type", define_type);
  DEFINE_DEFINER ("type_alias", define_type_alias);
  DEFINE_DEFINER ("generic", define_generic);

  define_type_string ("Int", sizeof(int), __alignof__ (int), NULL);
  define_type_string ("Bool", sizeof(int), __alignof__ (int), NULL); 
  define_type_string ("Type", sizeof(int), __alignof__ (int), NULL);
  define_type_string ("Function", sizeof(int), __alignof__ (int), NULL);
  define_type_string ("Symbol", sizeof(int), __alignof__ (int), NULL);
  define_type_string ("String", sizeof(int), __alignof__ (int), NULL);

  /* Label should have a type of -1, like functions, but for now
     we need them to be of size 0.  */
  define_type_string ("Label", 0, 0, NULL);

    /* XXX: void is not really the () type : because we allow (int) to
     coerce into void? Or is it? */
  //define_type_string ("Void", -1, -1, NULL); /* Void has no size.  */
  eval_cstring( "type_alias Void = ();");

}
