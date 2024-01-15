/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_DEALYACC_TAB_H_INCLUDED
# define YY_YY_DEALYACC_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 28 "../src/dealyacc.y"

    /* :JGM: Entries here go to both the dealyacc.tab.h and the dealyacc.tab.c files */
    /* :JGM: these definitions are required by the %union declaration */
  extern int yylex( void );             /* yylex comes from the flex object file  at link time */
  void yyerror( char *s ) ;             /* yyerror defined later in this file in code provides section */
#include "../include/dealdefs.h"       /* symbolic constants for everything */
#include "../include/dealtypes.h"      /* types; some needed by union aka YYSTYPE so put them here */
#include "../include/dealexterns.h"    /* extern statements for the various global vars. */
#include "../include/dbgprt_macros.h"  /* DBGLOC definition */


#line 61 "dealyacc.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    QUERY = 258,                   /* QUERY  */
    COLON = 259,                   /* COLON  */
    OR2 = 260,                     /* OR2  */
    AND2 = 261,                    /* AND2  */
    CMPEQ = 262,                   /* CMPEQ  */
    CMPNE = 263,                   /* CMPNE  */
    CMPLT = 264,                   /* CMPLT  */
    CMPLE = 265,                   /* CMPLE  */
    CMPGT = 266,                   /* CMPGT  */
    CMPGE = 267,                   /* CMPGE  */
    ARMINUS = 268,                 /* ARMINUS  */
    ARPLUS = 269,                  /* ARPLUS  */
    ARTIMES = 270,                 /* ARTIMES  */
    ARDIVIDE = 271,                /* ARDIVIDE  */
    ARMOD = 272,                   /* ARMOD  */
    NOT = 273,                     /* NOT  */
    UMINUS = 274,                  /* UMINUS  */
    HCP = 275,                     /* HCP  */
    SHAPE = 276,                   /* SHAPE  */
    ANY = 277,                     /* ANY  */
    EXCEPT = 278,                  /* EXCEPT  */
    CONDITION = 279,               /* CONDITION  */
    ACTION = 280,                  /* ACTION  */
    PRINT = 281,                   /* PRINT  */
    PRINTALL = 282,                /* PRINTALL  */
    PRINTEW = 283,                 /* PRINTEW  */
    PRINTNS = 284,                 /* PRINTNS  */
    PRINTSIDE = 285,               /* PRINTSIDE  */
    PRINTPBN = 286,                /* PRINTPBN  */
    PRINTCOMPACT = 287,            /* PRINTCOMPACT  */
    PRINTONELINE = 288,            /* PRINTONELINE  */
    AVERAGE = 289,                 /* AVERAGE  */
    HASCARD = 290,                 /* HASCARD  */
    FREQUENCY = 291,               /* FREQUENCY  */
    PREDEAL = 292,                 /* PREDEAL  */
    POINTCOUNT = 293,              /* POINTCOUNT  */
    ALTCOUNT = 294,                /* ALTCOUNT  */
    CONTROL = 295,                 /* CONTROL  */
    LOSER = 296,                   /* LOSER  */
    QUALITY = 297,                 /* QUALITY  */
    CCCC = 298,                    /* CCCC  */
    TRICKS = 299,                  /* TRICKS  */
    NOTRUMPS = 300,                /* NOTRUMPS  */
    EVALCONTRACT = 301,            /* EVALCONTRACT  */
    ALL = 302,                     /* ALL  */
    NONE = 303,                    /* NONE  */
    SCORE = 304,                   /* SCORE  */
    IMPS = 305,                    /* IMPS  */
    RND = 306,                     /* RND  */
    PT0 = 307,                     /* PT0  */
    PT1 = 308,                     /* PT1  */
    PT2 = 309,                     /* PT2  */
    PT3 = 310,                     /* PT3  */
    PT4 = 311,                     /* PT4  */
    PT5 = 312,                     /* PT5  */
    PT6 = 313,                     /* PT6  */
    PT7 = 314,                     /* PT7  */
    PT8 = 315,                     /* PT8  */
    PT9 = 316,                     /* PT9  */
    PRINTES = 317,                 /* PRINTES  */
    OPC = 318,                     /* OPC  */
    LTC = 319,                     /* LTC  */
    DDS = 320,                     /* DDS  */
    PAR = 321,                     /* PAR  */
    EXPORT = 322,                  /* EXPORT  */
    DEAL = 323,                    /* DEAL  */
    CSVRPT = 324,                  /* CSVRPT  */
    TRIX = 325,                    /* TRIX  */
    PRINTRPT = 326,                /* PRINTRPT  */
    BKTFREQ = 327,                 /* BKTFREQ  */
    USEREVAL = 328,                /* USEREVAL  */
    PARCONTRACT = 329,             /* PARCONTRACT  */
    NUMBER = 330,                  /* NUMBER  */
    HOLDING = 331,                 /* HOLDING  */
    STRING = 332,                  /* STRING  */
    IDENT = 333,                   /* IDENT  */
    COMPASS = 334,                 /* COMPASS  */
    VULN = 335,                    /* VULN  */
    SUIT = 336,                    /* SUIT  */
    CARD = 337,                    /* CARD  */
    CONTRACT = 338,                /* CONTRACT  */
    DISTR = 339,                   /* DISTR  */
    DISTR_OR_NUMBER = 340,         /* DISTR_OR_NUMBER  */
    DECNUM = 341,                  /* DECNUM  */
    SIDE = 342,                    /* SIDE  */
    VULNERABILITY = 343            /* VULNERABILITY  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 49 "../src/dealyacc.y"

        int            y_int;       /* 4 bytes */
        char          *y_str;       /* 8 bytes */
        struct tree   *y_tree;      /* 8 bytes */
        struct action *y_action;    /* 8 bytes */
        struct expr   *y_expr;      /* 8 bytes */
        char           y_distr[8];  /* 8 bytes was 4 bytes when ints and ptrs were 4. We need 5 to hold distr + NULL */
        float          y_flt;       /* 4 bytes */
        int            y_arr2[2];   /* 8 bytes */
        short int      y_arr4[4];   /* 8 bytes */
        char           y_arr8[8];   /* 8 bytes */
        struct csvterm_st *y_csv ;  /* 8 bytes */

#line 180 "dealyacc.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;

int yyparse (void);

/* "%code provides" blocks.  */
#line 181 "../src/dealyacc.y"
   /* code that is needed by the parser action clauses It also goes to tab.h !! */

 #include "../include/dealprotos.h"  /* prototypes for procedures incl some called in parse action clauses */
  extern int yylex() ;  /* yylex will be provided by the scanner produced file */


#line 216 "dealyacc.tab.h"

#endif /* !YY_YY_DEALYACC_TAB_H_INCLUDED  */
