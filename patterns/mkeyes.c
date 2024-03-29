/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * This is GNU GO, a Go program. Contact gnugo@gnu.org, or see       *
 * http://www.gnu.org/software/gnugo/ for more information.          *
 *                                                                   *
 * Copyright 1999, 2000, 2001, 2002 and 2003                         *
 * by the Free Software Foundation.                                  *
 *                                                                   *
 * This program is free software; you can redistribute it and/or     *
 * modify it under the terms of the GNU General Public License as    *
 * published by the Free Software Foundation - version 2             *
 *                                                                   *
 * This program is distributed in the hope that it will be useful,   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 * GNU General Public License in file COPYING for more details.      *
 *                                                                   *
 * You should have received a copy of the GNU General Public         *
 * License along with this program; if not, write to the Free        *
 * Software Foundation, Inc., 59 Temple Place - Suite 330,           *
 * Boston, MA 02111, USA.                                            *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Compile the eye database. This produces eyes.c. */

/* see also eyes.db, eyes.h and engine/optics.c */


#define MAX_BOARD 19
#define MAXLINE 80
#define MAXDIMEN 20
#define MAXSIZE 20
#define MAXPATNO 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "eyes.h"


int fatal_errors = 0;

int
main(void)
{
  char line[MAXLINE];
  int patno = 0;
  int p;
  char vertex[MAXDIMEN][MAXDIMEN];
  char marginal[MAXDIMEN][MAXDIMEN];
  char edge[MAXDIMEN][MAXDIMEN];
  char flags[MAXDIMEN][MAXDIMEN];
  int neighbors[MAXSIZE];
  int k, l, h;
  int m = 0, n = 0;
  int vi[MAXSIZE];
  int vj[MAXSIZE];
  int eye_number[MAXPATNO];
  int esize[MAXPATNO];
  int msize[MAXPATNO];
  int value_a[MAXPATNO];
  int value_b[MAXPATNO];
  int value_c[MAXPATNO];
  int value_d[MAXPATNO];
  int ends[MAXPATNO];
  int two_neighbors[MAXPATNO];
  int three_neighbors[MAXPATNO];
  int num_attacks = 0;
  int num_defenses = 0;
  int debug = 0;
  
  printf("/* This file is automatically generated by mkeyes. Do not\n");
  printf(" * edit it directly. Instead, edit the eye shape database.\n");
  printf(" */\n\n");
  printf("#include <stdio.h> /* for NULL */\n");
  printf("#include \"eyes.h\"\n\n");

  memset(ends, 0, sizeof(ends));
  memset(two_neighbors, 0, sizeof(two_neighbors));
  memset(three_neighbors, 0, sizeof(three_neighbors));
  memset(esize, 0, sizeof(esize));

  while (fgets(line, MAXLINE, stdin) && !fatal_errors) {
    int last = strlen(line) - 1;

    if (line[last] != '\n') {
      fprintf(stderr, "mkeyes: line truncated: %s\n", line);
      return 1;
    }

    /* remove trailing whitespace */
    for (last--; last >= 0 && isspace(line[last]); last--) {
      line[last]   = '\n';
      line[last+1] = '\0';
    }

    /* New pattern. */
    if (sscanf(line, "Pattern %d", &p)) {
      eye_number[patno] = p;
      if (patno > 0 && eye_number[patno] <= eye_number[patno-1]) {
	fprintf(stderr, "mkeyes: Pattern %d out of sequence\n",
		eye_number[patno]);
	return 1;
      }
      if (debug)
	fprintf(stderr, "parsing pattern %d\n", eye_number[patno]);
      
      memset(vertex, 0, sizeof(vertex));
      memset(marginal, 0, sizeof(marginal));
      memset(edge, 0, sizeof(edge));
      memset(flags, 0, sizeof(flags));
      
      m = 0;
      esize[patno] = 0;
      msize[patno] = 0;
      num_attacks = 0;
      num_defenses = 0;
      continue;
    }
    
    /* Empty line or comment line, skip. */
    if (strncmp("#", line, 1) == 0 || strncmp("\n", line, 1) == 0)
      continue;

    if (strncmp(":", line, 1) != 0) {
      /* diagram line. */
      for (n = 0; n < MAXDIMEN && strncmp("\n", line + n, 1); n++) {
	/* space, harmless CR, or corner symbol */
	if (line[n] == ' ' || line[n] == '\r' || line[n] == '+')
	  continue;

	/* vertical edge */
	if (line[n] == '|') {
	  if (n == 0)
	    edge[m][n+1]++;
	  else
	    edge[m][n-1]++;
	  continue;
	}

	/* horizontal edge */
	if (line[n] == '-') {
	  if (m == 0)
	    edge[m+1][n]++;
	  else
	    edge[m-1][n]++;
	  continue;
	}

	/* All other symbols. */
	vi[esize[patno]] = m;
	vj[esize[patno]] = n;
	vertex[m][n] = line[n];
	if (debug)
	  fprintf(stderr, "%c", line[n]);
	switch (line[n]) 
	{
	  case '.':
	    marginal[m][n] = 0;
	    flags[m][n] = CAN_BE_EMPTY;
	    break;
	    
	  case '!':
	    msize[patno]++;
	    marginal[m][n] = 1;
	    flags[m][n] = CAN_BE_EMPTY;
	    break;
	    
	  case '@':
	    msize[patno]++;
	    marginal[m][n] = 1;
	    flags[m][n] = CAN_BE_EMPTY | EYE_DEFENSE_POINT | EYE_ATTACK_POINT;
	    num_attacks++;
	    num_defenses++;
	    break;
	    
	  case '$':
	    msize[patno]++;
	    marginal[m][n] = 1;
	    flags[m][n] = CAN_CONTAIN_STONE;
	    break;
	    
	  case '(':
	    msize[patno]++;
	    marginal[m][n] = 1;
	    flags[m][n] = CAN_BE_EMPTY | EYE_ATTACK_POINT;
	    num_attacks++;
	    break;
	    
	  case ')':
	    msize[patno]++;
	    marginal[m][n] = 1;
	    flags[m][n] = CAN_BE_EMPTY | EYE_DEFENSE_POINT;
	    num_defenses++;
	    break;
	    
	  case 'x':
	    marginal[m][n] = 0;
	    flags[m][n] = CAN_BE_EMPTY | CAN_CONTAIN_STONE;
	    break;
	    
	  case '*':
	    marginal[m][n] = 0;
	    flags[m][n] = CAN_BE_EMPTY | EYE_ATTACK_POINT | EYE_DEFENSE_POINT;
	    num_attacks++;
	    num_defenses++;
	    break;
	    
	  case '<':
	    marginal[m][n] = 0;
	    flags[m][n] = CAN_BE_EMPTY | EYE_ATTACK_POINT;
	    num_attacks++;
	    break;

	  case '>':
	    marginal[m][n] = 0;
	    flags[m][n] = CAN_BE_EMPTY | EYE_DEFENSE_POINT;
	    num_defenses++;
	    break;
	    
	  case 'X':
	    marginal[m][n] = 0;
	    flags[m][n] = CAN_CONTAIN_STONE;
	    break;
	    
	  default:
	    fprintf(stderr, 
		    "mkeyes: invalid character %c in pattern %d\n",
		    line[n], eye_number[patno]);
	    fatal_errors++;
	    break;
	}
	esize[patno]++;
      }
      m++;
      if (debug)
	fprintf(stderr, "\n");
    }
    else {
      /* Colon line. */
      sscanf(line, ":%1d%1d%1d%1d", &value_a[patno], &value_b[patno],
	     &value_c[patno], &value_d[patno]);
      if (debug)
	fprintf(stderr, "value=%d%d%d%d\n", value_a[patno], value_b[patno],
		value_c[patno], value_d[patno]);

      if (value_b[patno] != value_c[patno]) {
	if (num_attacks == 0 || num_defenses == 0) {
	  fprintf(stderr,
		  "mkeyes: missing attack or defense point in pattern %d\n",
		  eye_number[patno]);
	  fatal_errors++;
	}
      }
      
      if (value_b[patno] == value_c[patno]) {
	if (num_attacks > 0 || num_defenses > 0) {
	  fprintf(stderr,
		  "mkeyes: attack or defense point in settled pattern %d\n",
		  eye_number[patno]);
	  fatal_errors++;
	}
      }
      
      printf("static struct eye_vertex eye%d[] = {\n", eye_number[patno]);
      
      for (l = 0; l < esize[patno]; l++) {
	int ni[4];
	int nj[4];
	int nb[4];
	int mx[MAXDIMEN][MAXDIMEN];
	int count = 0;
	int i = vi[l];
	int j = vj[l];
	
	memset(mx, -1, sizeof(mx));
	
	neighbors[l] = 0;
	
	for (h = 0; h < 4; h++) {
	  ni[h] = -1;
	  nj[h] = -1;
	  nb[h] = -1;
	}
	
	mx[i][j] = 0;
	
	if (i > 0 && vertex[i-1][j]) {
	  ni[neighbors[l]] = i-1;
	  nj[neighbors[l]] = j;
	  neighbors[l]++;
	  count++;
	  mx[i-1][j] = l;
	}
	
	if (i < MAXDIMEN-1 && vertex[i+1][j]) {
	  ni[neighbors[l]] = i+1;
	  nj[neighbors[l]] = j;
	  neighbors[l]++;
	  count++;
	  mx[i+1][j] = l;
	}
	
	if (j > 0 && vertex[i][j-1]) {
	  ni[neighbors[l]] = i;
	  nj[neighbors[l]] = j-1;
	  neighbors[l]++;
	  mx[i][j-1] = l;
	}
	
	if (j < MAXDIMEN-1 && vertex[i][j+1]) {
	  ni[neighbors[l]] = i;
	  nj[neighbors[l]] = j+1;
	  neighbors[l]++;
	  mx[i][j+1] = l;
	}
	
	
	if (neighbors[l] == 1)
	  ends[patno]++;
	else if (neighbors[l] == 2)
	  two_neighbors[patno]++;
	else if (neighbors[l] == 3)
	  three_neighbors[patno]++;
	
	for (h = 0; h < esize[patno]; h++) {
	  
	  for (k = 0; k < 4; k++)
	    if (ni[k] != -1 && vi[h] == ni[k] && vj[h] == nj[k])
	      nb[k] = h;
	}
	
	
	printf("  {%d, %d, %2d, %d, {%2d, %2d, %2d, %2d}}",
	       marginal[i][j], (int) edge[i][j], (int) flags[i][j],
	       neighbors[l], nb[0], nb[1], nb[2], nb[3]);
	
	if (l < esize[patno]-1)
	  printf(",\n");
	else
	  printf("\n};\n\n");
      }
      
      patno++;
      if (patno >= MAXPATNO) {
	fprintf(stderr,
		"mkeyes: Too many eye patterns. Increase MAXPATNO in mkeyes.c\n");
	fatal_errors++;
      }
    }
  }

  
  printf("\nstruct eye_graph graphs[] = {\n");
  for (l = 0; l < patno; l++) {

    printf("  {eye%d, %d, %d, %d, %d, %d, %d, {%d, %d, %d, %d}}",
	   eye_number[l], eye_number[l], esize[l], msize[l], ends[l],
	   two_neighbors[l], three_neighbors[l],
	   value_a[l], value_b[l], value_c[l], value_d[l]);
    if (l < patno-1)
      printf(",\n");
    else
      printf(",\n  {NULL, 0, 0, 0, 0, 0, 0, {0, 0, 0, 0}}\n};\n");
  }

  if (fatal_errors) {
    printf("\n\n#error in eye database.  Rebuild.\n\n");
  }

  return fatal_errors;
}

/*
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */
