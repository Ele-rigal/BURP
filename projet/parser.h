#include <glib.h>
#include <glib/gprintf.h>
#include <gmodule.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


/*
Les énumérations utilisés par le parser et l’interpreteur.
type
*/
typedef enum
{
  DIGIT,
  INTEGER,
  NUMBER,
  OPERATOR,
  EXPRESSION,
  COMPARAISON,
  CONDITION,
  COMMAND,
  LINE,
  PROGRAM
} type;

typedef enum
{
  INT,
  OP,
  PEEK,
  RAND,
  CARDINAL,
  SELF,
  SPEED,
  STATE,
  GPSX,
  GPSY,
  ANGLE,
  TARGETX,
  TARGETY,
  DISTANCE,
  WAIT,
  POKE,
  IF,
  ENGINE,
  SHOOT,
  GOTO,
  PLUS,
  MINUS,
  TIMES,
  DIV,
  MOD,
  INF,
  INF_EQ,
  SUPP,
  SUPP_EQ,
  EQ,
  DIFF,
  VOID
} value;

typedef struct token
{
  value val;
  type type;
  int data;
  struct token **arg;
  int nbArg;
} token;
