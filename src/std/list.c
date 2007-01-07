/* list.c - List library for L.
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



/* This is only a temporary list library.  The final list library will
   have a much better syntax.

   For instance, 
   cons(1, li) will be written  1 -> li;

   and push(1, li) : li = 1 -> li;

   and list(1, 2, 3, 4) will be written: 

   either 1 -> 2 -> 3 -> 4 ->| 
   or 1 -> 2 -> 3 -> 4 -> -|
   or 1 -> 2 -> 3 -> 4 -> null
   or 1 -> 2 -> 3 -> 4 -|
   or 1 -> 2 -> 3 -> 4 (we forbid here improper lists, and improper cons. Bad idea.)

   The test of emptiness will be more like C or Lisp:

   if(list) ... or if(list == -|)
*/

/* Translates list(1 2 3 4) into the actual list.  */



/* Translates cons(a,b) into:
   {
     cons_cell_ = alloc_cons_cell();
     cons_cell_->car = (void *) a;
     cons_cell_->cdr = (void *) b;
     (List(type(a)) cons_cell_
   }
*/


#include "../compiler/c-to-l.h"
#include <assert.h>
#include <l/sys/panic.h>

generic_form_t
list_cons(generic_form_t form)
{
  //  form_t type_listed_form =
    //  form_t pair_form = generic_type_form_symbol(SYMBOL(pointer),
    //					      CONS(generic_type_form_symbol(SYMBOL(struct),
  //									    CONS()

  //  form_t alloc_pair = let_form
}


form_t
expand_cons(generic_form_t form)
{
  expanded_form_t car_arg = expand(CAR(form->form_list));
  expanded_form_t cdr_arg = expand(CAR(form->form_list->next));

  /* If we do that :  
     type_form_t type_listed_form = location_type(car_arg)->type_form;

     Then we would allow cons('toto', list_int) with list_int being List(Int).

     We thus do: */
  generic_form_t list_type_form = cdr_arg->type->type_form;

  if(list_type_form->head != SYMBOL(List))
    panic("Second argument of cons must be of the form List(TYPE)\n");

  type_form_t type_listed_form = CAR(list_type_form->form_list);

  /* XXX: It is also the type_form of the type from which List(XXX) is derived.  */
  type_form_t pair_type_form = generic_form_symbol(SYMBOL(pointer),
						   CONS(generic_form_symbol(SYMBOL(struct),
									    CONS(label_form_symbol(SYMBOL(car),
												   type_listed_form),
										 CONS(label_form_symbol(SYMBOL(cdr),
													list_type_form),
										      NULL))),
							NULL));

  symbol_t pair_ = gensym("pair_");

  form_t cast_form = generic_form_symbol(SYMBOL(cast),
					 CONS(pair_type_form,
					      CONS(generic_form_symbol(SYMBOL(alloc_cons_cell), NULL),
						   NULL)));

  form_t alloc_pair_form = seq_form(CONS(let_form(pair_type_form, pair_),
					 CONS(generic_form_symbol(intern("="),
								  CONS(id_form(pair_),
								       CONS(cast_form, 
									    NULL)),
								  NULL),
					      NULL)),
				    NULL);

  form_t car = generic_form_symbol(intern("[]"),
				   CONS(generic_form_symbol(SYMBOL(deref),
							    CONS(id_form(pair_),
								 NULL)),
					CONS(symbol_form(SYMBOL(car)),
					     NULL)));


  form_t setcar = generic_form_symbol(intern("="),
				      CONS(car,
					   CONS(car_arg,
						NULL)));


  form_t cdr = generic_form_symbol(intern("[]"),
				   CONS(generic_form_symbol(SYMBOL(deref),
							    CONS(id_form(pair_),
								 NULL)),
					CONS(symbol_form(SYMBOL(cdr)),
					     NULL)));


  form_t setcdr = generic_form_symbol(intern("="),
				      CONS(cdr,
					   CONS(cdr_arg,
						NULL)));

  form_t complete_form = block_form(CONS(alloc_pair_form,
					 CONS(setcar,
					      CONS(setcdr,
						   CONS(generic_form_symbol(SYMBOL(cast),
									    CONS(list_type_form,
										 CONS(id_form(pair_),
										      NULL))),
							NULL)))));
  //  lispify(complete_form);
  //  print_type(pair_type_form);
  return complete_form;
  //  return compile(list_cons(form));
}

/* Transforms list(1, 2, 3) into cons(1, cons(2, cons(3, cast(List(Int), 0)))). */
form_t
expand_list(generic_form_t form)
{
  if(form->form_list == NULL)
    {
      panic("Due to lack of generic type, we cannot do list() (without args) for now\n");
    }

  /* The problem we have is that we cannot know the type of an
     expression before we compile it for now.  So we actually
     transform list(1, 2, 3) into:

     { let Int temp_ = 1;
       cons(temp_, cons(2, cons(3, List(Int)))) }

     which is equivalent.
  */

  /* XXX: This shows the need for in-place replacement of a form by an
   expanded_form (i.e., a flag that tells that the form has already
   been expanded. ).  And the form interface should not be functional,
   because subtree replication would mean code replication, which is
   not what we want in general.  So few subtrees would be shared;
   contrary to XML.*/

  expanded_form_t first_arg = expand(form->form_list->car); 

  type_form_t type_form = first_arg->type->type_form;

  //  symbol_t temp_ = gensym("temp_");

  //  form_t let_temp_form = let_form(type_form, temp_);
//  form_t assign_form = generic_form_symbol(intern("="),
//					   CONS(id_form(temp_),
//						CONS(CAR(form->form_list),
//						     NULL)));
//
  generic_form_t cons_form;
  { 
    generic_form_t *cons_form_ptr = &cons_form;

    /* Start at the second element.  */
    FOREACH(element, form->form_list->next)
      {
	*cons_form_ptr = generic_form_symbol(SYMBOL(cons),
					     CONS(CAR(element),
						  CONS(NULL,
						       NULL)));
	cons_form_ptr = &((*cons_form_ptr)->form_list->next->car);
      }

    *cons_form_ptr = generic_form_symbol(SYMBOL(cast),
					 CONS(generic_form_symbol(SYMBOL(List),
								  CONS(type_form,
								       NULL)),
					      CONS(int_form(0),
						   NULL)));
  }
  /* Add the already-expanded first element. */
  cons_form = generic_form_symbol(SYMBOL(cons),
				  CONS(first_arg,
				       CONS(cons_form,
					    NULL)));
   
  //  lispify(let_temp_form);
  //  lispify(assign_form);

//form_t block = block_form(CONS(let_temp_form,
//				 CONS(assign_form,
//				      CONS(cons_form,
//					   NULL))));
//  lispify(block);

  lispify(cons_form);
  return cons_form;
  
  //  location_t car_arg = compile(CAR(form->form_list));  
  

  /* Transforms list(a b c..) into cons(a cons(b cons(c, cast(.., 0)))); */

} 


pair_t
alloc_cons_cell(void)
{
  void *pair = MALLOC(pair);
  return pair;
}

#include "../objects/pair.h"
void
print_int_list(pair_t list)
{
  printf("Int List : (");
  FOREACH(element, list)
    {
      printf("%d ", CAR(element));
    }
  printf(")\n");
}


void
init_list(void)
{
  DEFINE_C_FUNCTION2 ("alloc_cons_cell", alloc_cons_cell, "void * <- ()");
  DEFINE_C_FUNCTION2 ("print_int_list", print_int_list, "void <- (List(Int))");

  define_expander(SYMBOL(cons), expand_cons);
  define_expander(SYMBOL(list), expand_list);
  //  DEFINE_GENERIC("cons", compile_cons);
  //  DEFINE_GENERIC("list", compile_list);
}
