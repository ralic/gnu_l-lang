/* hash.h - Hash tables handling.
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

#ifndef _HASH_H
#define _HASH_H

typedef void * hash_table;
typedef void ** hash_table_t;


#define MAKE_STATIC_HASH_TABLE(hash_table_name)			\
  static hash_table hash_table_name##__ = NULL;			\
  hash_table_t hash_table_name = & hash_table_name##__;

#include "../../../memory/memory.h"

hash_table_t
make_hash_table (void);


#include <Judy.h>

void *
gethash (void * key, hash_table_t table);

void *
puthash (void * key, void * value, hash_table_t table);



/* Returns 1 if the key was present, 0 otherwise.  */
static inline int
remhash( void* key, hash_table_t table)
{
  int Rc_int;
  JLD( Rc_int, *table, (Word_t) key);
  return Rc_int;
}

/* Hash strings.  */

#include <l/string.h>

typedef void ** hash_string_table_t;

hash_string_table_t
make_hash_string_table (void);

#include <Judy.h>

void *
gethash_string (String key, hash_string_table_t table);


void *
puthash_string (String key, void * value, hash_string_table_t table);



#endif
