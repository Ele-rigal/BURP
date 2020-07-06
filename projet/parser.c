#include <glib.h>
#include <glib/gprintf.h>
#include <gmodule.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"


#define G_CSET_DIGITS "0123456789"
#define SIZE 1024

/**
*
* Parser pour le langage BURP.
* A compiler avec GLIB.
* UTILISATION : L'executable prend le fichier a parser en parametre
* EXEMPLE : ./parser file
*
*/

//gcc `pkg-config --cflags glib-2.0` parser.c `pkg-config --libs glib-2.0` -o parser -lm -lncursesw

/*
Fonction qui renvoie un token initialisé avec les valeurs passé en paramètre
*/
token *initToken(value val, type type, int data, token **arg, int nbArg)
{
  token *res = malloc(sizeof(token));
  res->type = type;
  res->val = val;
  res->data = data;
  res->nbArg = nbArg;
  res->arg = arg;
  return res;
}

/*
Fonction qui lit un nombre positif. Arrete le programme si ce n’est pas un nombre ou s’il         est négatif
@return le token correspondant au nombre
*/
token *number(GScanner *scanner)
{
  GTokenType res = g_scanner_get_next_token(scanner);

  if (res == G_TOKEN_INT)
  {
    GTokenValue v = g_scanner_cur_value(scanner);
    int data = v.v_int;
    if (data < 0)
    {
      fprintf(stderr, " erreur nombre négatif dans number\n ");
      exit(0);
    }
    return initToken(VOID, NUMBER, data, NULL, 0);
  }
  else
  {
    fprintf(stderr, " Number : token pas de type int\n ");
    exit(0);
  }
}

/*
Fonction qui lit un token.identifier +, -, *, / ou % et retourne un token le décrivant
@return le token correspondant a opérateur lu en char *
*/
token *operator(GScanner *scanner)
{
  GTokenType res = g_scanner_get_next_token(scanner);
  if (res == G_TOKEN_IDENTIFIER)
  {
    GTokenValue v = g_scanner_cur_value(scanner);
    char *tok = v.v_identifier;
    if (strcmp(tok, "+") == 0)
    initToken(PLUS, OPERATOR, 0, NULL, 0);
    else if (strcmp(tok, "-") == 0)
    initToken(MINUS, OPERATOR, 0, NULL, 0);
    else if (strcmp(tok, "*") == 0)
    initToken(TIMES, OPERATOR, 0, NULL, 0);
    else if (strcmp(tok, "/") == 0)
    initToken(DIV, OPERATOR, 0, NULL, 0);
    else if (strcmp(tok, "%") == 0)
    initToken(MOD, OPERATOR, 0, NULL, 0);

    else
    fprintf(stderr, "Operator unsupported\n");
    exit(0);
  }
  else
  {
    fprintf(stderr, "erreur pas de Token identifier\n");
    exit(0);
  }
}

/*
Fonction qui lit une expression et retourne le token décrivant ce qu’il a lu
@return un token correctement remplit
*/
token *expression(GScanner *scanner)
{
  GTokenType res = g_scanner_get_next_token(scanner);

  if (res == G_TOKEN_INT)
  {
    GTokenValue v = g_scanner_cur_value(scanner);
    int data = v.v_int;
    return initToken(INT, EXPRESSION, data, NULL, 0);
  }

  if (res == G_TOKEN_LEFT_PAREN)
  {
    token **arg = malloc(sizeof(token) * 3);
    arg[0] = expression(scanner);
    arg[1] = operator(scanner);
    arg[2] = expression(scanner);
    if (g_scanner_get_next_token(scanner) != G_TOKEN_RIGHT_PAREN)
    {
      fprintf(stderr, "pas de parenthèse fermante\n");
      exit(0);
    }
    return initToken(OP, EXPRESSION, 0, arg, 3);
  }

  if (res == G_TOKEN_IDENTIFIER)
  {
    GTokenValue v = g_scanner_cur_value(scanner);
    char *tok = v.v_identifier;

    if (strcmp(tok, "PEEK") == 0)
    {
      token **arg = malloc(sizeof(token));
      arg[0] = expression(scanner);
      return initToken(PEEK, EXPRESSION, 0, arg, 1);
    }
    if (strcmp(tok, "RAND") == 0)
    {
      token **arg = malloc(sizeof(token));
      arg[0] = expression(scanner);
      return initToken(RAND, EXPRESSION, 0, arg, 1);
    }
    if (strcmp(tok, "CARDINAL") == 0)
    {
      return initToken(CARDINAL, EXPRESSION, 0, NULL, 0);
    }
    if (strcmp(tok, "SELF") == 0)
    {
      return initToken(SELF, EXPRESSION, 0, NULL, 0);
    }
    if (strcmp(tok, "SPEED") == 0)
    {
      return initToken(SPEED, EXPRESSION, 0, NULL, 0);
    }
    if (strcmp(tok, "STATE") == 0)
    {
      token **arg = malloc(sizeof(token));
      arg[0] = expression(scanner);
      return initToken(STATE, EXPRESSION, 0, arg, 1);
    }
    if (strcmp(tok, "GPSX") == 0)
    {
      token **arg = malloc(sizeof(token));
      arg[0] = expression(scanner);
      return initToken(GPSX, EXPRESSION, 0, arg, 1);
    }
    if (strcmp(tok, "GPSY") == 0)
    {
      token **arg = malloc(sizeof(token));
      arg[0] = expression(scanner);
      return initToken(GPSY, EXPRESSION, 0, arg, 1);
    }
    if (strcmp(tok, "ANGLE") == 0)
    {
      token **arg = malloc(sizeof(token) * 4);
      arg[0] = expression(scanner);
      arg[1] = expression(scanner);
      arg[2] = expression(scanner);
      arg[3] = expression(scanner);
      return initToken(ANGLE, EXPRESSION, 0, arg, 4);
    }
    if (strcmp(tok, "TARGETX") == 0)
    {
      token **arg = malloc(sizeof(token) * 3);
      arg[0] = expression(scanner);
      arg[1] = expression(scanner);
      arg[2] = expression(scanner);
      return initToken(TARGETX, EXPRESSION, 0, arg, 3);
    }
    if (strcmp(tok, "TARGETY") == 0)
    {
      token **arg = malloc(sizeof(token) * 3);
      arg[0] = expression(scanner);
      arg[1] = expression(scanner);
      arg[2] = expression(scanner);
      return initToken(TARGETY, EXPRESSION, 0, arg, 3);
    }
    if (strcmp(tok, "DISTANCE") == 0)
    {
      token **arg = malloc(sizeof(token) * 4);
      arg[0] = expression(scanner);
      arg[1] = expression(scanner);
      arg[2] = expression(scanner);
      arg[3] = expression(scanner);
      return initToken(DISTANCE, EXPRESSION, 0, arg, 4);
    }
  }


  fprintf(stderr, "pas de token identifier\n");
  exit(0);

}

/*
Fonction qui lit une comparaison et retourne le token décrivant ce qu’il a lu
@return un token correctement remplit
*/
token *comparaison(GScanner *scanner)
{
  GTokenType res = g_scanner_get_next_token(scanner);
  if (res == G_TOKEN_IDENTIFIER)
  {
    GTokenValue v = g_scanner_cur_value(scanner);
    char *tok = v.v_identifier;
    if (strcmp(tok, "<") == 0)
    return initToken(INF, COMPARAISON, 0, NULL, 0);
    if (strcmp(tok, ">") == 0)
    return initToken(SUPP, COMPARAISON, 0, NULL, 0);
    if (strcmp(tok, "<>") == 0)
    return initToken(DIFF, COMPARAISON, 0, NULL, 0);
    if (strcmp(tok, "<=") == 0)
    return initToken(INF_EQ, COMPARAISON, 0, NULL, 0);
    if (strcmp(tok, ">=") == 0)
    return initToken(SUPP_EQ, COMPARAISON, 0, NULL, 0);
    if (strcmp(tok, "=") == 0)
    return initToken(EQ, COMPARAISON, 0, NULL, 0);
    fprintf(stderr, "mauvais token dans comparaison");
    exit(0);
  }
  else
  {
    fprintf(stderr, "Pas de token identifier dans comparaison\n");
    exit(0);
  }
}

/*
Fonction qui lit une comparaison et retourne le token décrivant ce qu’il a lu
@return un token correctement remplit
*/
token *condition(GScanner *scanner)
{
  token **arg = malloc(sizeof(token) * 3);
  arg[0] = expression(scanner);
  arg[1] = comparaison(scanner);
  arg[2] = expression(scanner);
  return initToken(VOID, CONDITION, 0, arg, 3);
}

/*
Fonction qui lit une commande et retourne le token décrivant ce qu’il a lu
@return un token correctement remplit
*/
token *command(GScanner *scanner)
{
  GTokenType res = g_scanner_get_next_token(scanner);
  if (res == G_TOKEN_IDENTIFIER)
  {
    GTokenValue v = g_scanner_cur_value(scanner);
    char *tok = v.v_identifier;
    if (strcmp(tok, "WAIT") == 0)
    {
      token **arg = malloc(sizeof(token));
      arg[0] = expression(scanner);
      return initToken(WAIT, COMMAND, 0, arg, 1);
    }
    if (strcmp(tok, "POKE") == 0)
    {
      token **arg = malloc(sizeof(token) * 2);
      arg[0] = expression(scanner);
      arg[1] = expression(scanner);
      return initToken(POKE, COMMAND, 0, arg, 2);
    }
    if (strcmp(tok, "GOTO") == 0)
    {
      res = g_scanner_get_next_token(scanner);
      if (res == G_TOKEN_INT)
      {
        GTokenValue v = g_scanner_cur_value(scanner);
        int data = v.v_int;
        return initToken(GOTO, COMMAND, data, NULL, 0);
      }
      else
      fprintf(stderr, "pas de number dans condition\n");
      exit(0);
    }
    if (strcmp(tok, "IF") == 0)
    {
      token **arg = malloc(sizeof(token) * 2);
      arg[0] = condition(scanner);

      res = g_scanner_get_next_token(scanner);
      if (res == G_TOKEN_IDENTIFIER)
      {
        GTokenValue v = g_scanner_cur_value(scanner);
        char *tok = v.v_identifier;
        if (strcmp(tok, "THEN") == 0)
        {
          arg[1] = number(scanner);
        }
        else
        {
          fprintf(stderr, "Error : Expecting THEN\n");
          exit(0);
        }
      }

      return initToken(IF, COMMAND, 0, arg, 2);
    }
    if (strcmp(tok, "ENGINE") == 0)
    {
      token **arg = malloc(sizeof(token) * 2);
      arg[0] = expression(scanner);
      arg[1] = expression(scanner);
      return initToken(ENGINE, COMMAND, 0, arg, 2);
    }
    if (strcmp(tok, "SHOOT") == 0)
    {
      token **arg = malloc(sizeof(token) * 2);
      arg[0] = expression(scanner);
      arg[1] = expression(scanner);
      return initToken(SHOOT, COMMAND, 0, arg, 2);
    }
  }

  fprintf(stderr, "erreur pas de token identifier dans commande\n");
  exit(0);

}

token *line(GScanner *scanner)
{
  token **arg = malloc(sizeof(token) * 2);
  arg[0] = number(scanner);
  arg[1] = command(scanner);
  // fprintf(stderr,"TYPE NUMBER IN LINE %d\n", arg[0]->type);
  return initToken(VOID, LINE, 0, arg, 2);
}

token *program(GScanner *scanner)
{
  int nbArg = 0;
  int nbArgMax = 50;
  token **arg = malloc(sizeof(token) * nbArgMax);
  if (arg == NULL)
  {
    fprintf(stderr, "Probleme malloc\n");
    exit(-1);
  }
  while (g_scanner_peek_next_token(scanner) != G_TOKEN_EOF)
  {
    if (nbArg >= nbArgMax)
    {
      nbArgMax += 50;
      arg = realloc(arg, sizeof(token) * nbArgMax);
      if (arg == NULL)
      {
        fprintf(stderr, "Probleme realloc\n");
        exit(-1);
      }
    }
    arg[nbArg] = line(scanner);
    // fprintf(stderr,"TYPE NUMBER IN PROGRAM %d\n", arg[nbArg]->arg[0]->type);

    nbArg++;
  }
  return initToken(VOID, PROGRAM, 0, arg, nbArg);
}

void infoChat()
{
  fprintf(stderr,"\n------------------INFO CHAT : LES VALEURS DES ENUMS ------------------\n");

  fprintf(stderr,"\nTYPES : \n\n");
  fprintf(stderr,"DIGIT : %d\n", DIGIT);
  fprintf(stderr,"INTEGER : %d\n", INTEGER);
  fprintf(stderr,"NUMBER : %d\n", NUMBER);
  fprintf(stderr,"OPERATOR : %d\n", OPERATOR);
  fprintf(stderr,"EXPRESSION : %d\n", EXPRESSION);
  fprintf(stderr,"COMPARAISON : %d\n", COMPARAISON);
  fprintf(stderr,"CONDITION : %d\n", CONDITION);
  fprintf(stderr,"COMMAND : %d\n", COMMAND);
  fprintf(stderr,"LINE : %d\n", LINE);
  fprintf(stderr,"PROGRAM : %d\n", PROGRAM);

  fprintf(stderr,"\nVALEURS : \n\n");
  fprintf(stderr,"INT : %d\n", INT);
  fprintf(stderr,"OP : %d\n", OP);
  fprintf(stderr,"PEEK : %d\n", PEEK);
  fprintf(stderr,"RAND : %d\n", RAND);
  fprintf(stderr,"CARDINAL : %d\n", CARDINAL);
  fprintf(stderr,"SELF : %d\n", SELF);
  fprintf(stderr,"SPEED : %d\n", SPEED);
  fprintf(stderr,"STATE : %d\n", STATE);
  fprintf(stderr,"GPSX : %d\n", GPSX);
  fprintf(stderr,"GPSY : %d\n", GPSY);
  fprintf(stderr,"ANGLE : %d\n", ANGLE);
  fprintf(stderr,"TARGETX : %d\n", TARGETX);
  fprintf(stderr,"TARGETY : %d\n", TARGETY);
  fprintf(stderr,"DISTANCE : %d\n", DISTANCE);
  fprintf(stderr,"WAIT : %d\n", WAIT);
  fprintf(stderr,"POKE : %d\n", POKE);
  fprintf(stderr,"IF : %d\n", IF);
  fprintf(stderr,"ENGINE : %d\n", ENGINE);
  fprintf(stderr,"SHOOT : %d\n", SHOOT);
  fprintf(stderr,"GOTO  : %d\n", GOTO);
  fprintf(stderr,"PLUS : %d\n", PLUS);
  fprintf(stderr,"MINUS : %d\n", MINUS);
  fprintf(stderr,"TIMES : %d\n", TIMES);
  fprintf(stderr,"DIV : %d\n", DIV);
  fprintf(stderr,"MOD : %d\n", MOD);
  fprintf(stderr,"INF_EQ : %d\n", INF_EQ);
  fprintf(stderr,"SUPP : %d\n", SUPP);
  fprintf(stderr,"SUPP_EQ : %d\n", SUPP_EQ);
  fprintf(stderr,"EQ : %d\n", EQ);
  fprintf(stderr,"DIFF : %d\n", DIFF);
  fprintf(stderr,"VOID : %d\n", VOID);
}

static void config(GScannerConfig *gsc)
{
  gsc->skip_comment_multi = 1;
  gsc->skip_comment_single = 1;
  gsc->scan_comment_multi = 1;
  gsc->scan_identifier = 1;
  gsc->scan_identifier_1char = 1;
  gsc->scan_identifier_NULL = 0;
  gsc->scan_symbols = 1;
  gsc->scan_binary = 0;
  gsc->scan_octal = 0;
  gsc->scan_float = 0;
  gsc->scan_hex = 0;
  gsc->scan_hex_dollar = 0;
  gsc->scan_string_sq = 1;
  gsc->scan_string_dq = 1;
  gsc->numbers_2_int = 1;
  gsc->int_2_float = 0;
  gsc->identifier_2_string = 0;
  gsc->char_2_token = 1;
  gsc->symbol_2_token = 1;
  gsc->scope_0_fallback = 1;
  gsc->store_int64 = 0;

  gsc->cset_identifier_first = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-%/<>=";
  gsc->cset_identifier_nth = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ<>=0123456789";
}

/*
configure le scanner pour pouvoir lire le langage
@param scanner : scanner à configurer
@param argv: nom du fichier a scanner
*/
token *start(const
  char *argv)
  {
    GScanner *scanner = g_scanner_new(NULL);
    scanner = g_scanner_new(NULL);
    const char *pathname = argv;
    int descripteur = open(pathname, O_RDONLY);
    if (descripteur == -1)
    {
      perror("open");
      exit(-1);
    }

    // scanner->config->cset_identifier_first = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789<>=-";
    // scanner->config->cset_identifier_nth = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789<>=+-/%*";
    GScannerConfig *gsc = scanner->config;
    config(gsc);

    g_scanner_input_file(scanner, descripteur);

    return program(scanner);
  }

  void printTokenRec(token *reponse, int shift)
  {
    for (size_t i = 0; i < shift; i++)
    {
      fprintf(stderr,"\t");
    }
    if (reponse->type == NUMBER || reponse->val == INT){
      fprintf(stderr,"type = %d\t val = %d\t data = %d\n", reponse->type, reponse->val, reponse->data);
    }else{
      fprintf(stderr,"type = %d\t val = %d\n", reponse->type, reponse->val);
    }

    for (int i = 0; i < reponse->nbArg; i++)
    {
      printTokenRec(reponse->arg[i], shift + 1);
    }
  }
