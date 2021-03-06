/* xml.c - XML library for L.
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

/* L'interet c'est de montrer ce qu'on peut faire avec L.

   1/On ecrit le parseur XML
   2/On l'integre dans L


   A la fin, on peut utiliser le generateur de code C, pour integrer
   directement notre extension dans un programme existant.  On rajoute
   donc facilement la possibilite de sortir du XML depuis un programme
   C, simplement, sans rajouter de dependences exterieures!

   Pour pouvoir ecrire de vrais templates XML, il faut pouvoir generer
   des listes, il faudra peut etre avoir un $, (comme @, en Lisp)

    On pourra faire ca alors:

    XML{ <ul>$,{make_list_for(int i=0; i < number; i++)
                   XML {<li>$i</li>}}
	 </ul>}

   */




/* L'autre premier exemple c'est l'ajout de while.  */


/* We write a very naive parser; this is for easy transposition into L
   later.  Even more later, we will define a scanner using L. */


/* This function takes a buffer containing XML data, and transforms it
   into a form. $ is used for escapement into regular L form.

   For instance:

   parse_xml("<toto>truc</toto>") -> ('toto' "truc")
   parse_xml("<toto thing="stuff">truc</toto>") -> ('toto' thing:stuff "truc")
   parse_xml("<toto>${3 + 3}</toto>") -> ('toto' ('$' (+ 3 3)))
   parse_xml("<toto>$thing</toto>") -> ('toto' ('$' 'thing'))
   parse_xml("<toto>truc$thing toto</toto>") -> ('toto' "truc" ('$' 'thing') " toto")
   parse_xml("<toto>truc${thing}toto</toto>") -> ('toto' "truc" ('$' 'thing') "toto")
*/   

/* For instance, if we are in:
<toto><tata>thing</tata><stuff>HERE</stuff></toto> then the list would
contain ('stuff' 'toto'). */
//
//form_t parse_xml (Read_Buffer buf)
//{
//  char c = buffer_getchar (buf);
//
//  
//  switch(c)
//    {
//    case '<':
//      {
//	symbol_t current_symbol = NULL;
//	list_t subform_list;
//	list_t subform_list_ptr = &subform_list;
//
//	assert (buffer_nextchar (buf) != '/');
//#if 0
//	if(buffer_nextchar (buf) == '/')
//	  {
//	    symbol_t symb = parse_id ();
//	    
//	    if(buffer_getchar (buf) != '>')
//	      panic ("Not yet implemented\n");
//	    
//	    if(symb != current_symbol)
//	      panic ("XML symbols do not match\n");
//
//	    *subform_list_ptr = NULL;
//	    return generic_form_symbol (current_symbol, subform_list);
//	  }
//#endif
//	
//	current_symbol = parse_id ();
//
//	while(buffer_nextchar == ' ')
//	  buffer->current++;
//	
//	if(buffer_nextchar (buf) == '/')
//	  panic ("Not yet implemented\n");
//
//	if(buffer_getchar (buf) != '>')
//	  panic ("Not yet implemented\n");
//
//	
//	*parse_xml (buf);
//      }
//	
//      
//    case '$':
//      panic ("Not yet implemented\n");
//    case '>':
//
//    case '0' .. '9':
//      
//    }
//}
//


#include "../compiler/c-to-l.h"

#include "../parser/form.h"
#include <ctype.h> /* We should use isalpha etc... instead.  */
#include <assert.h>
#include <l/sys/panic.h>
#include <l/string.h>


/* An XML data structure and functions.  */

/* This data structure is well suited for generic XML manipulation
   
   If you know what your XML will look like, you could have optimized
   data structures.  For instance, these data structures could be
   generated from the DTD or XML schema.  */

/* A node is a leaf or an interior node.  */
typedef struct xml_node
{ 
  /* If last bit of is_leaf is 1, it is a leaf.  */
  int is_leaf;
} *XML_Node;

typedef enum {XML_Leaf_Int=1, XML_Leaf_String=3} XML_Leaf_Type; 

typedef struct xml_leaf
{
  /* Type is string or int or any atomic data type.  It should end
     with a 1 so that it is recognizable.  */
  XML_Leaf_Type type;
  union
  {
    int number;

    string_t string;
  };

} *xml_leaf_t;

typedef xml_leaf_t XML_Leaf;

typedef struct xml_interior_node
{
  symbol_t tag;

  /* An alist symbol . xml_leaf */
  list_t attributes;

  /* A list of XML_Nodes.  */
  list_t content;
} *xml_interior_node_t;

typedef xml_interior_node_t XML_Interior_Node;

/* The interface is delibarately functional, because XML
   transformation is then mostly a pointer manipulation operation
   (i.e. you can copy a complete subtree between two structures by
   copying a XML_Node, which is a pointer.) 

   This fact, and the fact that XML data structures can be optimized,
   implies that XML_Node data structure should be opaque.  */

XML_Node
make_xml_string_node(string_t string)
{
  XML_Leaf xl = MALLOC(xml_leaf);

  xl->type = XML_Leaf_String;
  xl->string = string;

  return xl;
}

XML_Node
make_xml_int_node(int i)
{
  panic("Unimplemented\n");
}

/* In L, this would be a macro, called like this :
   make_interior_node(tag, NULL, make_node.., make_node...) */
XML_Node
make_xml_interior_node(symbol_t tag,
		       list_t attributes, XML_Node content)
{
  assert(attributes == NULL);

  XML_Interior_Node xi = MALLOC(xml_interior_node);
  xi->tag = tag;
  xi->attributes = NULL;
  xi->content =content;

  return xi;
}




/* XML translator.  */

/* Note: the $construct should not convey side effects (like
   parameters in C function call).  In future versions of L, we will
   be able to check that the code has no side effect.  */

/*<toto>23<stuff>45</stuff>hello</toto> is translated into: 

  make_xml_interior_node(NULL, 'tag', NULL,
  make_xml_int_node(make_xml_interior_node(make_xml_string_node(NULL,"hello"),
  'stuff', NULL, make_xml_int_node(NULL, 45)), 23));
 */
  

generic_form_t 
expand_xml_rec (form_t form)
{
  if(is_form(form, atomic_form))
    {
      assert(is_form(form, string_form));

      return generic_form_symbol(SYMBOL(make_xml_string_node),
				 CONS(form, NULL));
    }

  generic_form_t gform = form;

  if(gform->head == intern("$"))
    {
      /* MMh. Would be easier to implement $@ if we had a list for
	 subnodes.  We could just create a "XML_node_cons" generic
	 which would call the cons function but with the right type.
	 And we could think about a generic way (to do in list.l) to
	 create this generic and associated xml_node_car,
	 xml_node_cdr; that way we could have lists in L.  */

      //      panic("Unimplemented\n");
      return generic_form_symbol(SYMBOL(make_xml_string_node),
				 CONS(CAR(gform->form_list),
				      NULL));
      //
    }

  //We do a MAP on the form list
  list_t xml_node_maker_list;
  {
    list_t *xml_node_maker_list_ptr = &xml_node_maker_list;
    
    FOREACH(element, gform->form_list)
      {
	form_t node_maker = expand_xml_rec(CAR(element));
	
	*xml_node_maker_list_ptr = CONS(node_maker, NULL);
	xml_node_maker_list_ptr = &((*xml_node_maker_list_ptr)->next);
      }

    *xml_node_maker_list_ptr = NULL;
  }

  return generic_form_symbol(SYMBOL(make_xml_interior_node),
			     CONS(symbol_form(gform->head),
				  CONS(int_form(0), 
				       CONS(generic_form_symbol(SYMBOL(list), xml_node_maker_list),
					    NULL))));
}

/* Just strip the first (XML ..)  */
generic_form_t
expand_xml(generic_form_t form)
{
  expand_xml_rec(CAR(form->form_list));
}


#include "../compiler/backend.h"


/* XML printer.  */

void 
print_xml_rec(int indent, XML_Node node)
{
  for(int i=0; i<indent; i++)
    putchar(' ');

  if((node->is_leaf & 1) == 1)
    {
      /* Node is a leaf. */
      XML_Leaf leaf = node;
      assert(leaf->type == XML_Leaf_String);

      printf("%s\n", strndup (leaf->string->content, leaf->string->length));
    }
  else
    {
      XML_Interior_Node inter_node = node;
      printf("<%s>\n", inter_node->tag->name);
      FOREACH(element, inter_node->content)
	{
	  print_xml_rec(indent+2, CAR(element));
	}
      for(int i=0; i<indent; i++)
	putchar(' ');
      printf("</%s>\n", inter_node->tag->name);
    }
}


void 
print_xml(XML_Node node)
{
  print_xml_rec(0, node);
}

/* Note: our XML data structure is for now just a Form; but we could
   use a more efficient data structure.  We could even use
   application-specific data structure, automatically generated from
   the DTD, for optimal performance.  */

/* Note 2: printing a XML data structure for now calls the XML print
   function on the data structure.  We could instead transform this by
   defining the printer of XML data structure as being a macro, that
   efficiently prints the output.

   For instance, for now print (XML (<toto>$i</toto>)) would be expanded
   finally in (using Lisp notation)

   print_xml (generic_form ('toto', int_form(i)));

   while the later could generate:

   print("<toto>", i, "</toto>")

   that could itself expands either in

   print_string("<toto>"); print_int(i); print_string("</toto>");
   or
   printf("<toto>%d</toto>",i);

   which does not create intermediary objects.

   The most difficult part would be transforming transformation of
   lists into looping; in most case, we could just change the
   "make_list" macro into a "loop", and expand everything else
   normally.  For lists not created with "make_list", and XML objects
   not created that way, the consing is still necessary.  */

   



#include <l/expand.h>
#include <l/type.h>

//static struct my_type XML_Node___;
//Type TYPE(XML_Node) = &XML_Node___;

void
init_xml ()
{

  //define_parse (SYMBOL (XML), xml_parse);
  //  DEFINE_GENERIC ("XML", compile_xml);
  define_expander(SYMBOL(XML), expand_xml);

  //  DEFINE_TYPE ("XML_Node", TYPE (XML_Node), sizeof(void *), __alignof (void*));
  //  define_type_string ("XML_Node", sizeof(void *), __alignof__ (void *), NULL);
  eval_cstring( "type XML_Node = Void *;");
  DEFINE_C_FUNCTION(make_xml_interior_node, "(Symbol,Int,List< XML_Node>) -> XML_Node");
  DEFINE_C_FUNCTION(make_xml_string_node, "(String) -> XML_Node");
  DEFINE_C_FUNCTION(print_xml, "(XML_Node) -> Void");

////
////  
//  Read_Buffer buf = Read_Buffer_From_String ("XML (<toto>truc<stuff />thing<hello>3</hello></toto> )");
//
//  generic_form_t form;
//
//  xml_parse (&form, buf);
//
//  lispify (form);
//  
//  exit (3);
}

