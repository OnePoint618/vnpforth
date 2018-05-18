/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     END = 0,
     VARIABLE = 258,
     FVARIABLE = 259,
     CONSTANT = 260,
     FCONSTANT = 261,
     CREATE = 262,
     ALLOT = 263,
     NONAME = 264,
     EXIT = 265,
     IF = 266,
     THEN = 267,
     ENDIF = 268,
     ELSE = 269,
     DO = 270,
     QUERY_DO = 271,
     LOOP = 272,
     PLUS_LOOP = 273,
     LEAVE = 274,
     QUERY_LEAVE = 275,
     BEGIN_ = 276,
     UNTIL = 277,
     WHILE = 278,
     REPEAT = 279,
     AGAIN = 280,
     CASE = 281,
     OF = 282,
     ENDOF = 283,
     ENDCASE = 284,
     RECURSE = 285,
     PSTRING = 286,
     CSTRING = 287,
     ASTRING = 288,
     DOTPARENSTRING = 289,
     INTEGER_LITERAL = 290,
     STRING_LITERAL = 291,
     FLOAT_LITERAL = 292,
     CHAR_LITERAL = 293,
     OBJECT_ADDRESS = 294,
     WORD = 295,
     CODE = 296,
     ASM = 297,
     END_CODE = 298,
     COND_IFDEF = 299,
     COND_IFUNDEF = 300,
     COND_ELSE = 301,
     COND_THEN = 302,
     COND_ENDIF = 303
   };
#endif
/* Tokens.  */
#define END 0
#define VARIABLE 258
#define FVARIABLE 259
#define CONSTANT 260
#define FCONSTANT 261
#define CREATE 262
#define ALLOT 263
#define NONAME 264
#define EXIT 265
#define IF 266
#define THEN 267
#define ENDIF 268
#define ELSE 269
#define DO 270
#define QUERY_DO 271
#define LOOP 272
#define PLUS_LOOP 273
#define LEAVE 274
#define QUERY_LEAVE 275
#define BEGIN_ 276
#define UNTIL 277
#define WHILE 278
#define REPEAT 279
#define AGAIN 280
#define CASE 281
#define OF 282
#define ENDOF 283
#define ENDCASE 284
#define RECURSE 285
#define PSTRING 286
#define CSTRING 287
#define ASTRING 288
#define DOTPARENSTRING 289
#define INTEGER_LITERAL 290
#define STRING_LITERAL 291
#define FLOAT_LITERAL 292
#define CHAR_LITERAL 293
#define OBJECT_ADDRESS 294
#define WORD 295
#define CODE 296
#define ASM 297
#define END_CODE 298
#define COND_IFDEF 299
#define COND_IFUNDEF 300
#define COND_ELSE 301
#define COND_THEN 302
#define COND_ENDIF 303




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


