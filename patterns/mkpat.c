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

/* Compile one of the pattern databases. This takes a database file,
 * e.g. patterns.db, and produces a C code file, in this case
 * patterns.c.
 */

/* See also patterns.h, and the *.db files.
 */

/* Differences when compiling connections patterns (-c) :
 *  '*' means cutting point
 *  '!' is allowed (inhibit connection there), matches as '.'.
 *  '!' will always be written as the first elements
*/

/* FIXME: This file is a horrible mess, especially after pattern
 *	  matching went 1D. Cleaning it will make future work
 *	  with pattern mathing easier.
 */

/* As in the rest of GNU Go, co-ordinate convention (i,j) is 'i' down from
 * the top, then 'j' across from the left
 */

#define USAGE "\
Usage : mkpat [options] <prefix>\n\
  General options:\n\
	-i = one or more input files (typically *.db)\n\
	-o = output file (typically *.c)\n\
	-v = verbose\n\
	-V <level> = dfa verbiage level\n\
  Database type:\n\
	-p = compile general pattern database (the default)\n\
	-c = compile connections database\n\
	-f = compile a fullboard pattern database\n\
	-C = compile a corner pattern database\n\
	-D = compile a dfa database (allows fast matching)\n\
	-T = compile a tree based pattern database (even faster)\n\
  Pattern generation options:\n\
	-O = allow only O to be anchor (the default)\n\
	-X = allow only X to be anchor\n\
	-b = allow both colors to be anchor\n\
	-m = try to place the anchor in the center of the pattern\n\
	     (works best with dfa databases)\n\
	-a = require anchor in all patterns. Sets fixed_anchor flag in db\n\
	-r = pre-rotate patterns before storing in database\n\
If no input files specified, reads from stdin.\n\
If output file is not specified, writes to stdout.\n\
"


#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "liberty.h"
#include "patterns.h"
#include "gg-getopt.h"
#include "gg_utils.h"

#include "dfa.h"


#define DB_GENERAL	((int) 'p')
#define DB_CONNECTIONS	((int) 'c')
#define DB_FULLBOARD	((int) 'f')
#define DB_CORNER	((int) 'C')
#define DB_DFA		((int) 'D')
#define DB_TREE 	((int) 'T')

/* code assumes that ATT_O and ATT_X are 1 and 2 (in either order)
 * An attribute is a candidate for anchor if  (att & anchor) != 0
 */
#define ANCHOR_O    ATT_O
#define ANCHOR_X    ATT_X
#define ANCHOR_BOTH (ATT_O | ATT_X)

#define MAXLINE 500
#define MAXCONSTRAINT 10000
#define MAXACTION 10000
#define MAXPATNO 5000
#define MAXLABELS 20
#define MAXPARAMS 15
#define MAX_INPUT_FILE_NAMES 10

/* Avoid compiler warnings with unused parameters */
#define UNUSED(x)  (void)x

/* valid characters that can appear in a pattern
 * position in string is att value to store
 */
const char VALID_PATTERN_CHARS[]     = ".XOxo,a!*?QY";
const char VALID_EDGE_CHARS[]        = "+-|";
const char VALID_CONSTRAINT_LABELS[] = "abcdefghijklmnpqrstuvwyzABCDEFGHIJKLMNPRSTUVWZ";


/* the offsets into the list are the ATT_* defined in patterns.h
 * The following defns are for internal use only, and are not
 * written out to the compiled pattern database
 */

#define ATT_star  8
#define ATT_wild  9
#define ATT_Q    10
#define ATT_Y    11

/* These arrays control discarding of unnecessary patval elements.
 * Modify them using `goal_elements ...' and `callback_data ..'
 * commands in a database. By default, we don't drop any elements.
 */
int nongoal[8]		 = {0, 0, 0, 0, 0, 0, 0, 0};
int callback_unneeded[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* stuff used in reading/parsing pattern rows */
int maxi, maxj;                 /* (i,j) offsets of largest element */
int mini, minj;                 /* offset of top-left element
				   (0,0) unless there are edge constraints */
unsigned int where;             /* NORTH_EDGE | WEST_EDGE, etc */
int el;                         /* next element number in current pattern */
struct patval_b elements[MAX_BOARD*MAX_BOARD]; /* elements of current pattern */
int num_stars;

int ci = -1, cj = -1;           /* position of origin (first piece element)
				   relative to top-left */
int patno;		        /* current pattern */
int discard_pattern = 0;	/* Set to nonzero to discard a pattern (if e.g.
				 * it is too large or duplicated). */
int pats_with_constraints = 0;  /* just out of interest */
int label_coords[256][2];       /* Coordinates for labeled stones in the 
				   autohelper patterns. */
int current_c_i;		/* Counter for the line number of a 
				   constraint diagram. */
char constraint[MAXCONSTRAINT]; /* Store constraint lines. */
char action[MAXCONSTRAINT];     /* Store action lines. */
static char diagram[MAX_BOARD+2][MAX_BOARD+3];
				/* store pattern diagram*/
static char constraint_diagram[MAX_BOARD+2][MAX_BOARD+3];
				/* store pattern constraint diagram */

/* stuff to maintain info about patterns while reading */
char *prefix;
struct pattern pattern[MAXPATNO];  /* accumulate the patterns into here */
char pattern_names[MAXPATNO][80];  /* with optional names here, */
char helper_fn_names[MAXPATNO][80]; /* helper fn names here */
char autohelper_code[MAXPATNO*300]; /* code for automatically generated */
                                    /* helper functions here */
char *code_pos;                     /* current position in code buffer */
struct autohelper_func {
  const char *name;
  int params;
  int type;	/* 0 - just copy the parameters,
		 * 1 - add parameter count,
		 * 2 - add address of the current pattern.
		 */
  float cost;
  const char *code;
};


/*
 * current_* are useful for debugging broken patterns.
 */
const char *current_file = 0;
int current_line_number = 0;

/* ================================================================ */
/*                                                                  */
/*                Autohelper function definitions                   */
/*                                                                  */
/* ================================================================ */

/* Important notice:
 * If one function has a name which is a prefix of another name, the
 * shorter name must come later in the list. E.g. "lib" must be preceded
 * by "lib2", "lib3", and "lib4".
 */
static struct autohelper_func autohelper_functions[] = {
  {"lib2",			1, 0, 0.01, "worm[%s].liberties2"},
  {"lib3",			1, 0, 0.01, "worm[%s].liberties3"},
  {"lib4",			1, 0, 0.01, "worm[%s].liberties4"},
  {"goallib",			0, 0, 0.01, "goallib"},
  {"lib",			1, 0, 0.01, "countlib(%s)"},
  {"alive",			1, 0, 0.01,
		"(dragon[%s].status == ALIVE)"},
  {"unknown",			1, 0, 0.01,
		"(dragon[%s].status == UNKNOWN)"},
  {"critical",			1, 0, 0.01,
		"(dragon[%s].status == CRITICAL)"},
  {"dead",			1, 0, 0.01, "(dragon[%s].status == DEAD)"},
  {"status",			1, 0, 0.01, "dragon[%s].status"},
  {"ko", 			1, 0, 0.01, "is_ko_point(%s)"},
  {"xdefend_against",		2, 0, 1.00,
		"defend_against(%s, OTHER_COLOR(color), %s)"},
  {"odefend_against",		2, 0, 1.00, "defend_against(%s, color, %s)"},
  {"defend_against_atari",	1, 0, 1.00,
		"defend_against_atari_helper(move, %s)"},
  {"does_defend",		2, 0, 1.00, "does_defend(%s, %s)"},
  {"does_attack",		2, 0, 1.00, "does_attack(%s, %s)"},
  {"attack",			1, 0, 1.00, "ATTACK_MACRO(%s)"},
  {"defend",			1, 0, 1.00, "DEFEND_MACRO(%s)"},
  {"weak",			1, 0, 0.01, "DRAGON_WEAK(%s)"},
  {"safe_xmove", 		1, 0, 1.00, "safe_move(%s, OTHER_COLOR(color))"},
  {"safe_omove", 		1, 0, 1.00, "safe_move(%s, color)"},
  {"legal_xmove",		1, 0, 0.05, "is_legal(%s, OTHER_COLOR(color))"},
  {"legal_omove",		1, 0, 0.05, "is_legal(%s, color)"},
  {"x_suicide",			1, 0, 0.05, "is_suicide(%s, OTHER_COLOR(color))"},
  {"o_suicide",			1, 0, 0.05, "is_suicide(%s, color)"},
  {"x_alive_somewhere",		0, 1, 0.01,
		"somewhere(OTHER_COLOR(color), 1, %d"},
  {"o_alive_somewhere",		0, 1, 0.01, "somewhere(color, 1, %d"},
  {"x_somewhere",		0, 1, 0.01,
		"somewhere(OTHER_COLOR(color), 0, %d"},
  {"o_somewhere",		0, 1, 0.01, "somewhere(color, 0, %d"},
  {"xmoyo_opposite",		1, 0, 0.01,
      "(whose_moyo(INITIAL_INFLUENCE(color), %s) == OTHER_COLOR(color))"},
  {"omoyo_opposite",		1, 0, 0.01,
      "(whose_moyo(INITIAL_INFLUENCE(color), %s) == color)"},
  {"xmoyo",			1, 0, 0.01,
      "(whose_moyo(OPPOSITE_INFLUENCE(color), %s) == OTHER_COLOR(color))"},
  {"omoyo",			1, 0, 0.01,
      "(whose_moyo(OPPOSITE_INFLUENCE(color), %s) == color)"},
  {"xarea",			1, 0, 0.01,
      "(whose_area(OPPOSITE_INFLUENCE(color), %s) == OTHER_COLOR(color))"},
  {"oarea",			1, 0, 0.01, 
      "(whose_area(OPPOSITE_INFLUENCE(color), %s) == color)"},
  {"xterri",			1, 0, 0.01,
      "(whose_territory(OPPOSITE_INFLUENCE(color), %s) == OTHER_COLOR(color))"},
  {"oterri",			1, 0, 0.01,
      "(whose_territory(OPPOSITE_INFLUENCE(color), %s) == color)"},
  {"genus",			1, 0, 0.01, "dragon[%s].genus"},
  {"approx_xlib",		1, 0, 0.03,
		"approxlib(%s, OTHER_COLOR(color), MAX_LIBERTIES, NULL)"},
  {"approx_olib",		1, 0, 0.03,
		"approxlib(%s, color, MAX_LIBERTIES, NULL)"},
  {"xlib",			1, 0, 0.05,
	"accuratelib(%s, OTHER_COLOR(color), MAX_LIBERTIES, NULL)"},
  {"olib",			1, 0, 0.05,
	"accuratelib(%s, color, MAX_LIBERTIES, NULL)"},
  {"xcut",			1, 0, 0.01,
    	"cut_possible(%s, OTHER_COLOR(color))"},
  {"ocut",			1, 0, 0.05, "cut_possible(%s, color)"},
  {"edge_double_sente", 	4, 1, 3.00,
		"edge_double_sente_helper(%s, %s, %s, %s)"},
  {"xplay_defend_both",		2, 1, 3.00,
		"play_attack_defend2_n(OTHER_COLOR(color), 0, %d"},
  {"oplay_defend_both",		2, 1, 3.00, "play_attack_defend2_n(color, 0, %d"},
  {"xplay_attack_either",	2, 1, 3.00,
		"play_attack_defend2_n(OTHER_COLOR(color), 1, %d"},
  {"oplay_attack_either",	2, 1, 3.00, "play_attack_defend2_n(color, 1, %d"},
  {"xplay_defend",		1, 1, 1.00,
		"play_attack_defend_n(OTHER_COLOR(color), 0, %d"},
  {"oplay_defend",		1, 1, 1.00, "play_attack_defend_n(color, 0, %d"},
  {"xplay_attack",		1, 1, 1.00,
		"play_attack_defend_n(OTHER_COLOR(color), 1, %d"},
  {"oplay_attack",		1, 1, 1.00, "play_attack_defend_n(color, 1, %d"},
  {"xplay_break_through",	3, 1, 5.00,
		"play_break_through_n(OTHER_COLOR(color), %d"},
  {"oplay_break_through",	3, 1, 5.00, "play_break_through_n(color, %d"},
  {"oplay_connect",		2, 1,10.00, "play_connect_n(color, 1, %d"},
  {"xplay_connect",		2, 1,10.00,
		"play_connect_n(OTHER_COLOR(color), 1, %d"},
  {"oplay_disconnect",		2, 1,10.00, "play_connect_n(color, 0, %d"},
  {"xplay_disconnect",		2, 1,10.00,
		"play_connect_n(OTHER_COLOR(color), 0, %d"},
  {"seki_helper",		1, 0, 0.0, "seki_helper(%s)"},
  {"threaten_to_save",		1, 0, 0.0, "threaten_to_save_helper(move,%s)"},
  {"threaten_to_capture",	1, 0, 0.0, "threaten_to_capture_helper(move,%s)"},
  {"not_lunch",			2, 0, 0.0, "not_lunch_helper(%s, %s)"},
  {"eye",			1, 0, 0.01, "is_eye_space(%s)"},
  {"proper_eye", 		1, 0, 0.01, "is_proper_eye_space(%s)"},
  {"marginal_eye",		1, 0, 0.01, "is_marginal_eye_space(%s)"},
  {"halfeye",			1, 0, 0.01, "is_halfeye(half_eye,%s)"},
  {"max_eye_value",		1, 0, 0.01, "max_eye_value(%s)"},
  {"owl_topological_eye",	2, 0, 0.01, "owl_topological_eye(%s, board[%s])"},
  {"obvious_false_oeye",	1, 0, 0.01, "obvious_false_eye(%s, color)"},
  {"obvious_false_xeye",	1, 0, 0.01,
		"obvious_false_eye(%s, OTHER_COLOR(color))"},
  {"antisuji",			1, 0, 0.0, "add_antisuji_move(%s)"},
  {"add_connect_move",		2, 0, 0.0, "add_connection_move(move,%s, %s)"},
  {"add_cut_move",		2, 0, 0.0, "add_cut_move(move, %s, %s)"},
  {"test_attack_either_move",	2, 0, 0.0,
		"test_attack_either_move(move, color, %s, %s)"},
  {"add_defend_both_move",	2, 0, 0.0,
		"add_all_move(move, DEFEND_STRING, %s, DEFEND_STRING, %s)"},
  {"same_dragon",		2, 0, 0.01, "is_same_dragon(%s, %s)"},
  {"same_string",		2, 0, 0.01, "same_string(%s, %s)"},
  {"dragonsize", 		1, 0, 0.01, "dragon[%s].size"},
  {"wormsize",			1, 0, 0.01, "countstones(%s)"},
  {"effective_size",		1, 0, 0.01, "dragon[%s].effective_size"},
  {"vital_chain",		1, 0, 0.05, "vital_chain(%s)"},
  {"potential_cutstone",	1, 0, 0.01, "worm[%s].cutstone2 > 1"},
  {"amalgamate_most_valuable_helper", 3, 0, 0.0,
   		"amalgamate_most_valuable_helper(%s, %s, %s)"},
  {"amalgamate", 		2, 0, 0.0, "join_dragons(%s, %s)"},
  {"owl_escape_value",		1, 0, 0.01, "owl_escape_value(%s)"},
  {"owl_goal_dragon",		1, 0, 0.01, "owl_goal_dragon(%s)"},
  {"owl_eyespace",		1, 0, 0.01, "owl_eyespace(%s)"},
  {"owl_big_eyespace",		1, 0, 0.01, "owl_big_eyespace(%s)"},
  {"owl_proper_eye",		1, 0, 0.01, "owl_proper_eye(%s)"},
  {"owl_eye_size",		1, 0, 0.01, "owl_eye_size(%s)"},
  {"owl_strong_dragon",		1, 0, 0.01, "owl_strong_dragon(%s)"},
  {"has_aji",			1, 0, 0.01,
		"(dragon[%s].owl_threat_status == CAN_THREATEN_DEFENSE)"},
  {"finish_ko_helper",		1, 0, 0.05, "finish_ko_helper(%s)"},
  {"squeeze_ko_helper",		1, 0, 0.03, "squeeze_ko_helper(%s)"},
  {"backfill_helper",		3, 0, 1.50, "backfill_helper(%s, %s, %s)"},
  {"connect_and_cut_helper2",	3, 0, 3.00,
		"connect_and_cut_helper2(%s, %s, %s, color)"},
  {"connect_and_cut_helper",	3, 0, 3.00, "connect_and_cut_helper(%s, %s, %s)"},
  {"owl_threatens",		2, 0, 0.01, "owl_threatens_attack(%s, %s)"},
  {"replace",			2, 0, 0.0,  "add_replacement_move(%s, %s)"},
  {"non_oterritory",		1, 0, 0.0,
		"influence_mark_non_territory(%s, color)"},
  {"non_xterritory",		1, 0, 0.0,
		"influence_mark_non_territory(%s, OTHER_COLOR(color))"},
  {"remaining_handicap_stones",	0, 0, 0.0,  "free_handicap_remaining_stones()"},
  {"total_handicap_stones",	0, 0, 0.0,  "free_handicap_total_stones()"},
  {"o_captures_something", 	1, 0, 0.02, "does_capture_something(%s, color)"},
  {"x_captures_something", 	1, 0, 0.02,
		"does_capture_something(%s, OTHER_COLOR(color))"},
  {"false_eye_territory",	1, 0, 0.0, "false_eye_territory[%s]"},
  {"false_eye",			1, 0, 0.01, "is_false_eye(half_eye, %s)"},
  {"o_visible_along_edge",	2, 0, 0.05, "visible_along_edge(color,%s,%s)"},
  {"x_visible_along_edge",	2, 0, 0.05, 
                "visible_along_edge(OTHER_COLOR(color),%s,%s)"},
  {"is_surrounded",		1, 0, 0.01, "is_surrounded(%s)"},
  {"does_surround",		2, 0, 1.00, "does_surround(%s, %s)"},
  {"surround_map",		2, 0, 0.01, "surround_map(%s, %s)"},
  {"value",			0, 2, 0.0,  "(%s->value)"}
};


/* To get a valid function pointer different from NULL. */
static int
dummyhelper(int transformation, int move, int color, int action)
{
  UNUSED(transformation); UNUSED(move); UNUSED(color);
  UNUSED(action);
  return 0;
}


#define PREAMBLE "\
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\
 * This is GNU GO, a Go program. Contact gnugo@gnu.org, or see       *\n\
 * http://www.gnu.org/software/gnugo/ for more information.          *\n\
 *                                                                   *\n\
 * Copyright 1999, 2000, 2001, 2002 and 2003                         *\n\
 * by the Free Software Foundation.                                  *\n\
 *                                                                   *\n\
 * This program is free software; you can redistribute it and/or     *\n\
 * modify it under the terms of the GNU General Public License as    *\n\
 * published by the Free Software Foundation - version 2             *\n\
 *                                                                   *\n\
 * This program is distributed in the hope that it will be useful,   *\n\
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *\n\
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *\n\
 * GNU General Public License in file COPYING for more details.      *\n\
 *                                                                   *\n\
 * You should have received a copy of the GNU General Public         *\n\
 * License along with this program; if not, write to the Free        *\n\
 * Software Foundation, Inc., 59 Temple Place - Suite 330,           *\n\
 * Boston, MA 02111, USA.                                            *\n\
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */\n\n\
#include <stdio.h> /* for NULL */\n\
#include \"liberty.h\"\n\
#include \"patterns.h\"\n\n\
"

int fatal_errors = 0;

/* options */
int verbose = 0;	/* -v */
int database_type = 0;  /* -p (default), -c, -f, -C, -D or -T */
int anchor = 0; 	/* Whether both O and/or X may be anchors.
			* -b for both. -X for only X.
			*/

int choose_best_anchor = 0;  /* -m */
/* FIXME: `fixed anchor' option doesn't work properly yet.
 *	  Probably the first thing to implement is to add
 *	  checks for anchor validity.
 */
int fixed_anchor = 0;        /* -a */
int pre_rotate = 0;          /* -p */

dfa_t dfa;

/**************************
 *
 * stuff to check the constraint diagram
 *
 **************************/

#define CHECK_CHARS "xXoO"
static void
check_constraint_diagram(void)
{
  int i, j, ino = 0, iso = 0, jwo = 0, jeo = 0;

  int have_constraint = (pattern[patno].autohelper_flag & HAVE_CONSTRAINT);
  if (0)
    fprintf(stderr, "patno: %d\n", patno);
    
  if (where & NORTH_EDGE)
    ino = 1;
  if (where & SOUTH_EDGE)
    iso = 1;
  if (where & WEST_EDGE)
    jwo = 1;
  if (where & EAST_EDGE)
    jeo = 1;
    
  if (verbose) {
    for (i = ino; i <= maxi+ino+iso; i++)
      fprintf(stderr, "%02d %s\n", i, diagram[i]);
    for (i = ino; i <= maxi+ino+iso; i++)
      fprintf(stderr, "%02d %s\n", i, constraint_diagram[i]);
  }
  
  if (0)
    fprintf(stderr, "have_constraint: %d\n", have_constraint);
  if (have_constraint && el) {
    for (i = ino; i <= maxi+ino; i++)
      for (j = jwo; j <= maxj+jwo; j++) {
	if (0)
	  fprintf(stderr,"%2d %2d %c %c\n", i, j, constraint_diagram[i][j], 
		  diagram[i][j]);
	if (strchr(CHECK_CHARS, constraint_diagram[i][j])
	    && constraint_diagram[i][j] != diagram[i][j]) {
	  fprintf(stderr, "%s(%d) : Error : "
                  "xXoO not matched in constraint diagram of pattern %s\n",
		  current_file, current_line_number, pattern_names[patno]);
	  fatal_errors++;
	}
    }
  }
}

/**************************
 *
 * stuff to parse the input
 *
 **************************/

/* reset state of all pattern variables */
static void
reset_pattern(void)
{
  int i, j;

  maxi = 0;
  maxj = 0;
  ci = -1;
  cj = -1;
  where = 0;
  el = 0;
  num_stars = 0;
  strcpy(helper_fn_names[patno], "NULL");
  for (i = 0; i < 256; i++)
    label_coords[i][0] = -1;
  current_c_i = 0;
  constraint[0] = 0;
  action[0] = 0;
  for (i = 0; i < MAX_BOARD+2; i++) {
    for (j = 0; j < MAX_BOARD+3; j++) {
      diagram[i][j] = '\0';
      constraint_diagram[i][j] = '\0';
    }
  }
  memset(&pattern[patno], 0, sizeof(struct pattern));
}
  

/* This is called to compute the extents of the pattern, applying
 * edge constraints as necessary.
 */
static void
find_extents(void)
{
  /* When this is called, elements go from (mini,minj) inclusive to
   * (maxi-1, maxj-1) [ie exclusive]. Make them inclusive.
   * Ie maximum element lies on (maxi,maxj).
   */
  
  --maxi;
  --maxj;

  /* apply edge constraints to the size of the pattern */

  if (where & (NORTH_EDGE|SOUTH_EDGE|EAST_EDGE|WEST_EDGE))
    ++pats_with_constraints;

  if (verbose)
    fprintf(stderr, "Pattern %s has constraints 0x%x\n",
	    pattern_names[patno], where);

  pattern[patno].edge_constraints = where;


  /* At this point, (mini,minj) -> (maxi,maxj) contain
   * the extent of the pattern, relative to top-left
   * of pattern, rather than (ci,cj).
   *
   * But we store them in the output file relative
   * to (ci,cj), so that we can transform the corners
   * of the pattern like any other relative co-ord.
   */

  pattern[patno].mini = mini - ci;
  pattern[patno].minj = minj - cj;
  pattern[patno].maxi = maxi - ci;
  pattern[patno].maxj = maxj - cj;
}


/*
 * Here we build the dfa.
 */

static void
write_to_dfa(int index)
{
  char str[MAX_ORDER+1];
  float ratio;
  int ll;
  int rot_start = 0;
  int rot_stop = 1;
  
  assert(ci != -1 && cj != -1);
#if 0
  pattern[index].patn = elements; /* a little modification : keep in touch! */
#endif
  pattern[index].name = &(pattern_names[index][0]); 

  if (verbose)
    fprintf(stderr, "Add   :%s\n", pattern[index].name);

  /* First we create the string from the actual pattern */
  pattern_2_string(pattern+index, elements, str, 0, ci, cj);
      
  if (pre_rotate) {
    if (pattern[index].trfno != 5)
      rot_stop = pattern[index].trfno;
    else {
      rot_start = 2;
      rot_stop = 6;
    }
  }

  for (ll = 0; ll < rot_stop; ll++) {
    /* Then We add this string to the DFA */
    ratio = (dfa_add_string(&dfa, str, index, ll) - 1)*100;
 
    /* Complain when there is more than 10% of increase */ 
    if (dfa_size(&dfa) > 100 && ratio > 10.0) {
      fprintf(stderr, "Pattern %s => %3.1f%% increase: ",
	      pattern[index].name, ratio);
      fprintf(stderr, "another orientation may save memory.\n");
    }
    if (dfa_verbose > 2)
      dump_dfa(stderr, &dfa);
  }
}


/* For good performance, we want to reject patterns as quickly as
 * possible. For each pattern, this combines 16 positions around
 * the anchor stone into a 32-bit mask and value. In the matcher,
 * the same 4x4 grid is precomputed, and then we can quickly
 * test 16 board positions with one test.
 * See matchpat.c for details of how this works - basically, if
 * we AND what is on the board with the and_mask, and get the
 * value in the val_mask, we have a match. This test can be
 * applied in parallel : 2 bits per posn x 16 posns = 32 bits.
 * "Don't care" has and_mask = val_mask = 0, which is handy !
 * FIXME: Looks like element "." (don't care) has and_mask 3, not 0,
 *    as indicated in the comments above.
 * (answer) "Don't care" means '?', not '.'. The comments mean that
 *	    we don't have to search for '?'s directly, since their
 *	    masks are 0, and it is set up by initialization.
 */

static void
compute_grids(void)
{
#if GRID_OPT
  /*                              element: .  X  O  x  o  ,  a  ! */
  static const unsigned int and_mask[] = { 3, 3, 3, 1, 2, 3, 3, 3 };
  static const unsigned int val_mask[] = { 0, 2, 1, 0, 0, 0, 0, 0 };

  int ll;  /* iterate over rotations */
  int k;   /* iterate over elements */

  for (ll = 0; ll < 8; ++ll) {
    for (k = 0; k < el; ++k) {
      int di, dj;

      TRANSFORM2(elements[k].x - ci, elements[k].y - cj, &di, &dj, ll);
      ++di;
      ++dj;
      if (di >= 0 && di < 4 && dj >= 0 && dj < 4) {
	pattern[patno].and_mask[ll]
	  |= and_mask[elements[k].att] << (30 - di * 8 - dj * 2);
	pattern[patno].val_mask[ll]
	  |= val_mask[elements[k].att] << (30 - di * 8 - dj * 2);
      }
    }
  }
#endif
}


/* We've just read a line that looks like a pattern line. Now process it.
 * If the pattern becomes larger than maximal supported board, the function
 * returns zero, so that the pattern can be discarded.
 */
static int
read_pattern_line(char *p)
{
  const char *char_offset;
  char *pcopy = p;
  int j;
  int width;
  int jwo = 0, jeo = 0;

  if (where & SOUTH_EDGE)
    /* something wrong here : pattern line after a SOUTH_EDGE constraint */
    goto fatal;


  if (*p == '+' || *p == '-') {
    /* must be a north/south constraint */

    if (maxi == 0)
      where |= NORTH_EDGE;
    else
      where |= SOUTH_EDGE;

    if (*p == '+') {
      if (maxi > 0 && !(where & WEST_EDGE))
	/* if this is end of pattern, must already know about west */
	goto fatal;

      where |= WEST_EDGE;
      ++p;
    }

    /* count the number of -'s */
    for (width = 0; *p == '-' ; ++p, ++width)
      ;

    if (width == 0)
      goto fatal;

    if (*p == '+') {
      if (maxi > 0 && !(where & EAST_EDGE))
	/* if this is end of pattern, must already know about west */
	goto fatal;
      where |= EAST_EDGE;
    }

    if (maxi > 0 && width != maxj)
      goto notrectangle;

    return 1;
  }

  /* get here => its a real pattern entry, 
   * rather than a north/south constraint 
   */

  /* we have a pattern line - add it into the current pattern */
  if (*p == '|') {
    /* if this is not the first line, or if there is a north
     * constraint, we should already know about it
     */
    if (!(where & WEST_EDGE) && ((where & NORTH_EDGE) || maxi > 0))
      /* we should already know about this constraint */
      goto fatal;

    where |= WEST_EDGE;
    ++p;
  }
  else if (where & WEST_EDGE)
    /* need a | if we are already constrained to west */
    goto fatal;


  for (j = 0; 
       (char_offset = strchr(VALID_PATTERN_CHARS, *p)) != NULL;
       ++j, ++p) {

    /* char_offset is a pointer within the VALID_PATTERN_CHARS string.
     * so  (char-VALID_PATTERN_CHARS) is the att (0 to 11) to write to the
     * pattern element
     */

    /* one of ATT_* - see above */
    int off = char_offset - VALID_PATTERN_CHARS;

    if (off == ATT_wild)
      continue;  /* boring - pad character */

    if (off == ATT_a) /* this were used by halfeye patterns */
      goto fatal;

    if (off == ATT_star) {
      /* '*' */
      pattern[patno].move_offset = OFFSET(maxi, j);
      ++num_stars;
      off = ATT_dot;  /* add a '.' to the pattern instead */
    }

    if (off == ATT_Q) {
      off = ATT_O;
      ci = maxi;
      cj = j;
      pattern[patno].anchored_at_X = (off == ATT_X) ? 3 : 0;
      /*FIXME: Make sure O is valid anchor*/
    }

    if (off == ATT_Y) {
      off = ATT_X;
      ci = maxi;
      cj = j;
      pattern[patno].anchored_at_X = (off == ATT_X) ? 3 : 0;
      /*FIXME: Make sure X is valid anchor*/
    }

    assert(off <= ATT_not);

	
    if ((ci == -1) && (off < 3) && ((off & anchor) != 0)) {
      /* Use this position as the pattern origin. */
      ci = maxi;
      cj = j;
      pattern[patno].anchored_at_X = (off == ATT_X) ? 3 : 0;
    }

    /* Special limitations for fullboard patterns. */
    if (database_type == DB_FULLBOARD) {
      if (off == ATT_dot)
	continue;
      assert(off == ATT_X || off == ATT_O);
    }
    
    /* Range checking. */
    assert(el < (int) (sizeof(elements) / sizeof(elements[0])));
    
    elements[el].x = maxi;
    elements[el].y = j;
    elements[el].att = off;  /* '*' mapped to '.' and 'Q' to 'O' above */
    
    ++el;
  }

  if (*p == '|') {

    /* if this is not the first line, or if there is a north
     * constraint, we should already know about it
     */
    if (!(where & EAST_EDGE) && ((where & NORTH_EDGE) || maxi > 0))
      goto fatal;  /* we should already know about this constraint */

    where |= EAST_EDGE;

  }
  else if (where & EAST_EDGE)
    goto fatal;  /* need a | if we are already constrained to east */


  if (maxi > 0 && j != maxj)
    goto notrectangle;

  if (j > maxj)
    maxj = j;


  if (where & WEST_EDGE)
    jwo = 1;
  if (where & EAST_EDGE)
    jeo = 1;
  if (maxi <= MAX_BOARD)
    strncpy(diagram[maxi], pcopy, maxj + jwo + jeo);
  maxi++;

  return maxi <= MAX_BOARD && maxj <= MAX_BOARD;

fatal:
 fprintf(stderr, "%s(%d) : error : Illegal pattern %s\n", 
         current_file, current_line_number, pattern_names[patno]);
 fatal_errors = 1;
 return 0;

notrectangle:
 fprintf(stderr, "%s(%d) : error : Pattern %s not rectangular\n", 
	 current_file, current_line_number, pattern_names[patno]);
 fatal_errors++;
 return 0;
}


/*
 * We've just read a line that looks like a constraint pattern line.
 * Now process it.
 */

static void
read_constraint_diagram_line(char *p)
{
  int j;
  int jwo = 0, jeo = 0;
  const char *pcopy = p;

  /* North or south boundary, no letter to be found. */
  if (*p == '+' || *p == '-')
    return;

  /* Skip west boundary. */
  if (*p == '|')
    p++;
  
  for (j = 0; 
       strchr(VALID_PATTERN_CHARS, *p) || strchr(VALID_CONSTRAINT_LABELS, *p);
       ++j, ++p) {
    if (strchr(VALID_CONSTRAINT_LABELS, *p) 
	&& label_coords[(int)*p][0] == -1) {

      /* New constraint letter */
      label_coords[(int)*p][0] = current_c_i;
      label_coords[(int)*p][1] = j;
    }
  }

  /* Now j holds the width of this constraint diagram line. Require
   * this to match the main diagram width stored in maxj. However,
   * maxj was modified by find_extents() so we have to compensate for
   * this.
   */
  if (j != maxj + 1) {
    fprintf(stderr, "%s(%d) : error : Mismatching width of constraint line in pattern %s\n", 
	    current_file, current_line_number, pattern_names[patno]);
    fatal_errors++;
    return;
  }

  if (where & WEST_EDGE)
    jwo = 1;
  if (where & EAST_EDGE)
    jeo = 1;
  if (el)
    strncpy(constraint_diagram[current_c_i], pcopy, maxj+jwo+jeo+1);
  current_c_i++;

  return;
}

/* Check that the constraint diagram had the same number of rows as
 * the main diagram.
 */
static void
check_constraint_diagram_size(void)
{
  if (current_c_i != maxi + 1) {
    fprintf(stderr, "%s(%d) : error : Mismatching height of constraint diagram in pattern %s\n", 
	    current_file, current_line_number, pattern_names[patno]);
    fatal_errors++;
  }
}  

/* On reading a line starting ':', finish up and write
 * out the current pattern 
 */

static void
finish_pattern(char *line)
{

  /* end of pattern layout */
  char symmetry;		/* the symmetry character */
  
  mini = minj = 0; /* initially : can change with edge-constraints */

  if (num_stars > 1 || (database_type != DB_CONNECTIONS && num_stars == 0)) {
    fprintf(stderr, "%s(%d) : error : No or too many *'s in pattern %s\n",
	    current_file, current_line_number, pattern_names[patno]);
    fatal_errors = 1;
  }

  if (database_type == DB_FULLBOARD) {
    /* For fullboard patterns, the "anchor" is always at the mid point. */
    ci = (maxi-1)/2;
    cj = (maxj-1)/2;
  }
  else if (database_type == DB_CORNER) {
    ci = 0;
    cj = 0;
  }
  else if (choose_best_anchor) { 

    /* Try to find a better anchor if
     * the -m option is set.
     */
    int mi, mj; /* middle */
    int d, min_d = 36100;
    int k, min_k = -1;
      
    /* We seek the element of suitable value minimizing
     * the distance to the middle.
     */
    mi = (maxi - 1) * 50;
    mj = (maxj - 1) * 50 - 1;
    for (k = 0; k != el; k++)
      if (elements[k].att < 3 && (elements[k].att & anchor) != 0) {
	d = gg_abs(100 * elements[k].x - mi)
	  + gg_abs(100 * elements[k].y - mj);
	if (d < min_d) {
	  min_k = k;
	  min_d = d;
	}
      }
    assert(min_k != -1);
    ci = elements[min_k].x;
    cj = elements[min_k].y;
    pattern[patno].anchored_at_X = (elements[min_k].att == ATT_X) ? 3 : 0;
    
  }
  else if (ci == -1 || cj == -1) {
    fprintf(stderr, "%s(%d) : No origin for pattern %s\n", 
	    current_file, current_line_number, pattern_names[patno]);
    fatal_errors = 1;
    ci = 0;
    cj = 0;
  }

  /* translate posn of * (or Q) to relative co-ords
   */

  if (num_stars == 1) {
    pattern[patno].move_offset -= OFFSET_DELTA(ci, cj);
  }
  else if (num_stars == 0) {
    pattern[patno].move_offset = OFFSET(ci, cj);
  }

  find_extents();

  compute_grids();

  pattern[patno].patlen = el;

  /* Now parse the line. Only the symmetry character and the class
   * field are mandatory. The compiler guarantees that all the fields
   * are already initialized to 0.
   */

  {
    int s;
    char class[80];
    char entry[80];
    char *p = line;
    int n;
    float v = 0.0;
    
    class[0] = 0;  /* in case sscanf doesn't get that far */
    s = sscanf(p, ":%c,%[^,]%n", &symmetry, class, &n);
    p += n;
    
    if (s < 2) {
      fprintf(stderr, ": line must contain symmetry character and class\n");
      fatal_errors++;
    }

    while (sscanf(p, "%*[, ]%[^,]%n", entry, &n) > 0) {
      p += n;

      if (sscanf(entry, "%*[^(](%f)", &v) > 0) {
	if (strncmp(entry, "value", 5) == 0
	    || strncmp(entry, "minvalue", 8) == 0) {
	  pattern[patno].value = v;
	  pattern[patno].class |= VALUE_MINVAL;
	}
	else if (strncmp(entry, "maxvalue", 8) == 0) {
	  pattern[patno].maxvalue = v;
	  pattern[patno].class |= VALUE_MAXVAL;
	}
	else if (strncmp(entry, "terri", 5) == 0
		 || strncmp(entry, "minterri", 8) == 0) {
	  pattern[patno].minterritory = v;
	  pattern[patno].class |= VALUE_MINTERRITORY;
	}
	else if (strncmp(entry, "maxterri", 8) == 0) {
	  pattern[patno].maxterritory = v;
	  pattern[patno].class |= VALUE_MAXTERRITORY;
	}
	else if (strncmp(entry, "shape", 5) == 0) {
	  pattern[patno].shape = v;
	  pattern[patno].class |= VALUE_SHAPE;
	}
	else if (strncmp(entry, "followup", 8) == 0) {
	  pattern[patno].followup = v;
	  pattern[patno].class |= VALUE_FOLLOWUP;
	}
	else if (strncmp(entry, "reverse_followup", 16) == 0) {
	  pattern[patno].reverse_followup = v;
	  pattern[patno].class |= VALUE_REV_FOLLOWUP;
	}
	else {
	  fprintf(stderr, "%s(%d) : error : Unknown value field: %s\n", 
                  current_file, current_line_number, entry);
          fatal_errors++;
	  break;
	}
      }
      else {
	strncpy(helper_fn_names[patno], entry, 79);
	break;
      }
    }
      
    {
      char *p;
      for (p = class; *p; p++) {
	switch (*p) {
	  case 's': pattern[patno].class |= CLASS_s; break;
	  case 'O': pattern[patno].class |= CLASS_O; break;
	  case 'o': pattern[patno].class |= CLASS_o; break;
	  case 'X': pattern[patno].class |= CLASS_X; break;
	  case 'x': pattern[patno].class |= CLASS_x; break;
	  case 'D': pattern[patno].class |= CLASS_D; break;
	  case 'C': pattern[patno].class |= CLASS_C; break;
	  case 'c': pattern[patno].class |= CLASS_c; break;
	  case 'n': pattern[patno].class |= CLASS_n; break;
	  case 'B': pattern[patno].class |= CLASS_B; break;
	  case 'A': pattern[patno].class |= CLASS_A; break;
	  case 'b': pattern[patno].class |= CLASS_b; break;
	  case 'e': pattern[patno].class |= CLASS_e; break;
	  case 'E': pattern[patno].class |= CLASS_E; break;
	  case 'a': pattern[patno].class |= CLASS_a; break;
	  case 'd': pattern[patno].class |= CLASS_d; break;
	  case 'I': pattern[patno].class |= CLASS_I; break;
	  case 'J': pattern[patno].class |= CLASS_J; break;
	  case 'j': pattern[patno].class |= CLASS_j; break;
	  case 't': pattern[patno].class |= CLASS_t; break;
	  case 'T': pattern[patno].class |= CLASS_T; break;
	  case 'U': pattern[patno].class |= CLASS_U; break;
	  case 'W': pattern[patno].class |= CLASS_W; break;
	  case 'F': pattern[patno].class |= CLASS_F; break;
	  case 'N': pattern[patno].class |= CLASS_N; break;
	  case 'Y': pattern[patno].class |= CLASS_Y; break;
	  case '-': break;
	  default:
	    if (!isgraph((int) *p))
	      break;
	    fprintf(stderr,
		    "%s(%d) : error : Unknown classification letter %c. (pattern %s).\n", 
		    current_file, current_line_number, *p,
		    pattern_names[patno]);
	    fatal_errors++;
	    break;
	}
      }
    }
  }

      
  /* Now get the symmetry. There are extra checks we can make to do with
   * square-ness and edges. We do this before we work out the edge constraints,
   * since that mangles the size info.
   */
  
  switch (symmetry) {
  case '+' :
    if (where & (NORTH_EDGE|EAST_EDGE|SOUTH_EDGE|WEST_EDGE))
      fprintf(stderr,
	      "%s(%d) : Warning : symmetry inconsistent with edge constraints (pattern %s)\n", 
	      current_file, current_line_number, pattern_names[patno]);
    pattern[patno].trfno = 2;
    break;

  case 'X' : 
    if (where & (NORTH_EDGE|EAST_EDGE|SOUTH_EDGE|WEST_EDGE))
      fprintf(stderr,
	      "%s(%d) : Warning : X symmetry inconsistent with edge constraints (pattern %s)\n", 
	      current_file, current_line_number, pattern_names[patno]);
    if (maxi != maxj)
      fprintf(stderr,
	      "%s(%d) : Warning : X symmetry requires a square pattern (pattern %s)\n",
	      current_file, current_line_number, pattern_names[patno]);
    pattern[patno].trfno = 2;
    break;

  case '-' :
    if (where & (NORTH_EDGE|SOUTH_EDGE))
      fprintf(stderr,
	      "%s(%d) : Warning : symmetry inconsistent with edge constraints (pattern %s)\n", 
	      current_file, current_line_number, pattern_names[patno]);
    pattern[patno].trfno = 4;
    break;
    
  case '|' :
    if (where & (EAST_EDGE|WEST_EDGE))
      fprintf(stderr,
	      "%s(%d) : Warning : symmetry inconsistent with edge constraints (pattern %s)\n", 
	      current_file, current_line_number, pattern_names[patno]);
    pattern[patno].trfno = 4;
    break;

  case '\\' :
  case '/' :
    /* FIXME: Can't be bothered putting in the edge tests.
    *         (What does this mean?)
    */
    if (maxi != maxj)
      fprintf(stderr,
	      "%s(%d) : Warning : \\ or / symmetry requires a square pattern (pattern %s)\n", 
	      current_file, current_line_number, pattern_names[patno]);

    pattern[patno].trfno = 4;
    break;

  case 'O' :
    if (where & (NORTH_EDGE|EAST_EDGE|SOUTH_EDGE|WEST_EDGE))
      fprintf(stderr,
	      "%s(%d) : Warning : symmetry inconsistent with edge constraints (pattern %s)\n", 
	      current_file, current_line_number, pattern_names[patno]);
    pattern[patno].trfno = 5;  /* Ugly special convention. */
    break;

  default:
    fprintf(stderr,
	    "%s(%d) : Warning : symmetry character '%c' not implemented - using '8' (pattern %s)\n", 
	    current_file, current_line_number, symmetry, pattern_names[patno]);
    /* FALLTHROUGH */
  case '8' :
    pattern[patno].trfno = 8;
    break;
  }

}
      

static void
read_constraint_line(char *line)
{
  /* Avoid buffer overrun. */
  assert(strlen(constraint) + strlen(line) < MAXCONSTRAINT);

  /* Append the new line. */
  strcat(constraint, line);

  pattern[patno].autohelper_flag |= HAVE_CONSTRAINT;
}


static void
read_action_line(char *line)
{
  /* Avoid buffer overrun. */
  assert(strlen(action) + strlen(line) < MAXACTION);

  /* Append the new line. */
  strcat(action, line);

  pattern[patno].autohelper_flag |= HAVE_ACTION;
}


static void
generate_autohelper_code(int funcno, int number_of_params, int *labels)
{
  int i;
  char varnames[MAXPARAMS][8];
  char pattern[MAXLINE];

  for (i = 0; i < number_of_params; i++) {
    if (labels[i] == (int) '*')
      sprintf(varnames[i], "move");
    /* The special label '?' represents a tenuki. We replace this
     * with NO_MOVE value.
     */
    else if (labels[i] == (int) '?')
      sprintf(varnames[i], "NO_MOVE");
    else
      sprintf(varnames[i], "%c", labels[i]);
  }

  switch (autohelper_functions[funcno].type) {
  case 0:
    /* A common case. Just use the labels as parameters. */
    switch (number_of_params) {
    case 0:
      code_pos += sprintf(code_pos, autohelper_functions[funcno].code);
      break;
    case 1:
      code_pos += sprintf(code_pos, autohelper_functions[funcno].code,
			  varnames[0]);
      break;
    case 2:
      code_pos += sprintf(code_pos, autohelper_functions[funcno].code,
			  varnames[0], varnames[1]);
      break;
    case 3:
      code_pos += sprintf(code_pos, autohelper_functions[funcno].code,
			  varnames[0], varnames[1], varnames[2]);
      break;
    case 4:
      code_pos += sprintf(code_pos, autohelper_functions[funcno].code,
			  varnames[0], varnames[1], varnames[2],
			  varnames[3]);
      break;
    default:
      fprintf(stderr, "%s(%d) : error : unknown number of parameters (pattern %s)",
	      current_file, current_line_number, pattern_names[patno]);
      fatal_errors++;
    }
    break;

  case 1:
    /* This is a very special case where there is assumed to be a
     * variable number of parameters and these constitute a series of
     * moves to make followed by a final attack or defense test.
     */
    if (number_of_params < autohelper_functions[funcno].params) {
      fprintf(stderr, "%s(%d) : error : too few parameters (pattern %s)",
	      current_file, current_line_number, pattern_names[patno]);
      fatal_errors++;
      return;
    }

    code_pos += sprintf(code_pos, autohelper_functions[funcno].code,
		number_of_params - autohelper_functions[funcno].params);

    for (i = 0; i < number_of_params; i++)
	code_pos += sprintf(code_pos, ", %s", varnames[i]);

    *code_pos++ = ')'; /* Close parenthesis. */
    break;

  default: /* 2 */
    /* A very special case. We add the address of the current pattern
     * before the actual parameters. So far, used only by `value'.
     */
    sprintf(pattern, "(%s + %d)", prefix, patno);

    switch (number_of_params) {
    case 0:
      code_pos += sprintf(code_pos, autohelper_functions[funcno].code,
			  pattern);
      break;
    case 1:
      code_pos += sprintf(code_pos, autohelper_functions[funcno].code,
			  pattern, varnames[0]);
      break;
    case 2:
      code_pos += sprintf(code_pos, autohelper_functions[funcno].code,
			  pattern, varnames[0], varnames[1]);
      break;
    case 3:
      code_pos += sprintf(code_pos, autohelper_functions[funcno].code,
			  pattern, varnames[0], varnames[1], varnames[2]);
      break;
    default:
      fprintf(stderr, "%s(%d) : error : unknown number of parameters (pattern %s)",
	      current_file, current_line_number, pattern_names[patno]);
      fatal_errors++;
    }
  }
}


/* Parse the constraint and generate the corresponding helper code.
 * We use a small state machine.
 */
static void
parse_constraint_or_action(char *line, float *cost)
{
  int state = 0;
  char *p;
  int n = 0;
  int label = 0;
  int labels[MAXLABELS];
  int N = sizeof(autohelper_functions)/sizeof(struct autohelper_func);
  int number_of_params = 0;
  float cost_factor = 1.0;

  *cost = 0.0;
  for (p = line; *p; p++)
  {
    switch (state) {
      case 0: /* Looking for a token, echoing other characters. */
	for (n = 0; n < N; n++) {
	  if (strncmp(p, autohelper_functions[n].name,
		      strlen(autohelper_functions[n].name)) == 0) {
	    state = 1;
	    p += strlen(autohelper_functions[n].name)-1;
	    *cost += autohelper_functions[n].cost * cost_factor;
	    cost_factor *= 0.6;
	    break;
	  }
	}
	if (state == 0 && *p != '\n')
	  *(code_pos++) = *p;
	break;
	
      case 1: /* Token found, now expect a '('. */
	if (*p != '(') {
	  if (autohelper_functions[n].params == 0) {
	    /* We allow to omit parenthesis when using functions which
	     * have no parameters. In any case, you may still place them,
	     * but having to write `value() = 50' is disgusting.
	     */
	    generate_autohelper_code(n, 0, NULL);
	    p--;
	    state = 0;
	    break;
	  }
	  fprintf(stderr,
		  "%s(%d) : error : Syntax error in constraint or action, '(' expected (pattern %s).\n", 
		  current_file, current_line_number, pattern_names[patno]);
	  fatal_errors++;
	  return;
	}
	else {
	  assert(autohelper_functions[n].params <= MAXPARAMS);
	  number_of_params = 0;
	  if (autohelper_functions[n].params != 0
	      || autohelper_functions[n].type == 1)
	    state = 2;
	  else
	    state = 3;
	}
	break;
	
      case 2: /* Time for a label. */
	if ((*p != '*') && (*p != '?') && !strchr(VALID_CONSTRAINT_LABELS, *p)) {
	  if (strchr("XxOo", *p))
	    fprintf(stderr,
		    "%s(%d) : error : '%c' is not allowed as a constraint label.\n",
		    current_file, current_line_number, *p);
	  else
	    fprintf(stderr,
		    "%s(%d) : error : Syntax error in constraint or action, label expected, found '%c'.\n", 
		    current_file, current_line_number, *p);
	  fatal_errors++;
	  return;
	}
	else {
	  if ((*p == '?') && (number_of_params == 0)) {
	    fprintf(stderr,
		    "mkpat: tenuki (?) cannot be the first label (pattern %s)\n", pattern_names[patno]);
	    return;
	  }

	  label = (int) *p;
	  /* The special label '?' represents a tenuki. */
	  if (*p != '*' && *p != '?' && label_coords[label][0] == -1) {
	    fprintf(stderr,
		    "%s(%d) : error : The constraint or action uses a label (%c) that wasn't specified in the diagram (pattern %s).\n", 
		    current_file, current_line_number, label, pattern_names[patno]);
	    fatal_errors++;
	    return;
	  }
	  labels[number_of_params] = label;
	  number_of_params++;
	  state = 3;
	}
	break;

      case 3: /* A ',' or a ')'. */
	if (*p == ',') {
	  if (autohelper_functions[n].type != 1
	      && number_of_params == autohelper_functions[n].params) {
	    fprintf(stderr,
		    "%s(%d) : error : Syntax error in constraint or action, ')' expected (pattern %s).\n",
		    current_file, current_line_number, pattern_names[patno]);
	    return;
	  }
	  if (number_of_params == MAXPARAMS) {
	    fprintf(stderr,
		    "Error in constraint or action, too many parameters. (pattern %s).\n", 
		    pattern_names[patno]);
	    return;
	  }
	  state = 2;
	  break;
	}
	else if (*p != ')') {
	  fprintf(stderr, 
		  "%s(%d) : error : Syntax error in constraint or action, ',' or ')' expected (pattern %s).\n", 
		  current_file, current_line_number, pattern_names[patno]);
	  return;
	}
	else { /* a closing parenthesis */
	  if ((autohelper_functions[n].type != 1)
	      && (number_of_params < autohelper_functions[n].params)) {
	    fprintf(stderr,
		    "%s(%d) : error : Syntax error in constraint or action, %s() requires %d parameters.\n",
		    current_file, current_line_number, autohelper_functions[n].name, autohelper_functions[n].params);
	    fatal_errors++;
	    return;
	  }
	  generate_autohelper_code(n, number_of_params, labels);
	  state = 0;
	}
	break;
	
      default:
	fprintf(stderr,
		"%s(%d) : error : Internal error in parse_constraint_or_action(), unknown state.\n",
		current_file, current_line_number);
        fatal_errors++;
	return;
	       
    }
  }
}

/* Finish up a constraint and/or action and generate the automatic
 * helper code. The constraint text is in the global variable
 * constraint. */

static void
finish_constraint_and_action(void)
{
  unsigned int i;
  float cost;
  int have_constraint = (pattern[patno].autohelper_flag & HAVE_CONSTRAINT);
  int have_action = (pattern[patno].autohelper_flag & HAVE_ACTION);
  int no_labels = 1;

  /* Mark that this pattern has an autohelper. */
  pattern[patno].autohelper = dummyhelper;
  
  /* Generate autohelper function declaration. */
  code_pos += sprintf(code_pos, 
		      "static int\nautohelper%s%d(int trans, int move, int color, int action)\n{\n  int",
		      prefix, patno);

  /* Generate variable declarations. */
  for (i = 0; i < sizeof(VALID_CONSTRAINT_LABELS); i++) {
    int c = (int) VALID_CONSTRAINT_LABELS[i];

    if (label_coords[c][0] != -1)
      code_pos += sprintf(code_pos, " %c,", c);
  }

  /* Replace the last ',' with ';' */
  if (*(code_pos-1) == ',')
    *(code_pos-1) = ';';
  else {
    code_pos -= 3; /* no variable, erase "int" */
    code_pos += sprintf(code_pos, "UNUSED(trans);");
  }

  /* Include UNUSED statements for two parameters */
  code_pos += sprintf(code_pos, "\n  UNUSED(color);\n");
  if (!have_constraint || !have_action)
    code_pos += sprintf(code_pos, "  UNUSED(action);\n");
  
  /* Generate coordinate transformations. */
  for (i = 0; i < sizeof(VALID_CONSTRAINT_LABELS); i++) {
    int c = (int) VALID_CONSTRAINT_LABELS[i];

    if (label_coords[c][0] != -1) {
      code_pos += sprintf(code_pos,
			  "\n  %c = AFFINE_TRANSFORM(%d, trans, move);",
			  c,
			  OFFSET(label_coords[c][0], label_coords[c][1])
			  - OFFSET_DELTA(ci, cj)
			  - CENTER_OFFSET(pattern[patno].move_offset));
      no_labels = 0;
    }
  }

  /* move might be unused. Add an UNUSED statement to avoid warnings. */
  if (no_labels)
    code_pos += sprintf(code_pos, "\n  UNUSED(move);");

  code_pos += sprintf(code_pos, "\n\n");
  if (have_constraint && have_action)
    code_pos += sprintf(code_pos, "  if (!action)\n  ");
  if (have_constraint) {
    code_pos += sprintf(code_pos, "  return ");
    parse_constraint_or_action(constraint, &cost);
    pattern[patno].constraint_cost = cost;
    code_pos += sprintf(code_pos, ";\n");
  }
  if (have_action) {
    code_pos += sprintf(code_pos, "  ");
    parse_constraint_or_action(action, &cost);
    code_pos += sprintf(code_pos, ";\n");
    code_pos += sprintf(code_pos, "\n  return 0;\n");
  }
  code_pos += sprintf(code_pos, "}\n\n");
  
  /* Check that we have not overrun our buffer. That would be really bad. */
  assert(code_pos <= autohelper_code + sizeof(autohelper_code));
}



/* ================================================================ */
/*           stuff to write the elements of a pattern               */
/* ================================================================ */

/* callback function used to sort the elements in a pattern. 
 * We want to sort the patterns by attribute.
 *
 *  RANK 01234567
 *  ATT  57126340
 *       ,!XOaxo.
 * 
 * so that cheaper / more specific tests are done early on
 * For connections mode, the inhibition points (7)
 * must be first.
 */

static int
compare_elements(const void *a, const void *b)
{
  static char order[] = {7,2,3,5,6,0,4,1};  /* score for each attribute */
  return  order[((const struct patval_b *)a)->att]
    - order[((const struct patval_b *)b)->att];
}

struct element_node {
  struct patval_b e;
  struct element_node *next;
};

/* flush out the pattern stored in elements[]. Don't forget
 * that elements[].{x,y} and min/max{i,j} are still relative
 * to the top-left corner of the original ascii pattern, and
 * not relative to the anchor stone ci,cj
 */

static void
write_elements(FILE *outfile)
{
  int node;
  int used_nodes = 0;

  assert(ci != -1 && cj != -1);

  /* sort the elements so that least-likely elements are tested first. */
  gg_sort(elements, el, sizeof(struct patval_b), compare_elements);

  fprintf(outfile, "static struct patval %s%d[] = {", prefix, patno);

  for (node = 0; node < el; node++) {
    int x = elements[node].x;
    int y = elements[node].y;
    int att = elements[node].att;

    assert(x >= mini && y >= minj);
    if (!(x <= maxi && y <= maxj)) {
      fprintf(stderr, 
	      "%s(%d) : error : Maximum number of elements exceeded in %s.\n",
	      current_file, current_line_number, prefix);
      fatal_errors++;
    }

    /* Check if this element is not needed for goal checking and by
     * callback function. Also, check that pattern class doesn't
     * require dragon status checking on it.
     */
    if ((fixed_anchor || nongoal[att]) && callback_unneeded[att]
	&& (((pattern[patno].class & (CLASS_X | CLASS_x)) == 0)
	    || (att != ATT_X && att != ATT_x))
	&& (((pattern[patno].class & (CLASS_O | CLASS_o)) == 0)
	    || (att != ATT_O && att != ATT_o))) {
      /* Now check that pattern matcher won't need the element for
       * matching itself.
       */

#if GRID_OPT == 1
      /* If we do grid optimization, we can avoid matching 9 pattern elements
       * around its anchor (the 9 elements are the intersection of 16 grid
       * elements for all possible transformations).
       */
      if ((database_type == DB_GENERAL || database_type == DB_CONNECTIONS)
	  && ci-1 <= x && x <= ci+1 && cj-1 <= y && y <= cj+1)
	continue;
#endif /* GRID_OPT == 1 */

      /* DFA pattern matcher doesn't itself need these elements at all. But
       * they might be needed for goal checking or by callback function, so
       * we check it before discarding an element.
       *
       * FIXME: What about tree matcher?
       */
      if (database_type == DB_DFA)
	continue;
    } /* If the element is discardable. */

    if (used_nodes)
      fprintf(outfile, ",");
    fprintf(outfile, used_nodes % 4 ? "\t" : "\n  ");
    used_nodes++;

    fprintf(outfile, "{%d,%d}", OFFSET(x - ci, y - cj), att);
  }

  /* This may happen for fullboard patterns or if we have discarded all
   * the elements as unneeded by the matcher.
   */
  if (!used_nodes)
    fprintf(outfile, "{0,-1}}; /* Dummy element, not used. */\n\n");
  else
    fprintf(outfile, "\n};\n\n");

  pattern[patno].patlen = used_nodes;
}

/* ================================================================ */
/*		    Corner database creation stuff		    */
/* ================================================================ */

#define CORNER_DB_SIZE(patterns, variations)\
  (( patterns * sizeof(struct corner_pattern)\
   + variations * sizeof(struct corner_variation)) / 1024)

static struct corner_variation_b corner_root;
static int second_corner_offset[MAXPATNO];

static int total_variations = 0;
static int variations_written = 0;

static int corner_max_width = 0;
static int corner_max_height = 0;

/* This structure is used by corner_add_pattern() */
struct corner_element {
  int x;
  int y;
  int color;
};


static void
corner_init(void)
{
  corner_root.num_variations = 0;
  corner_root.child = NULL;
}


static int corner_best_element(struct corner_element *el, int n, int x, int y,
			       struct corner_variation_b *variations,
			       int color)
{
  int k;
  int i;
  int best = 0;
  int best_value = 0;

  int candidate[MAX_BOARD * MAX_BOARD];
  int candidates = 0;
  int existing_variation[MAX_BOARD * MAX_BOARD];
  int have_existing_variation = 0;

  for (k = 0; k < n; k++) {
    for (i = 0; i < n; i++) {
      if (i == k)
	continue;

      if (el[k].x >= el[i].x && el[k].y >= el[i].y)
        break;
    }

    if (i == n) {
      struct corner_variation_b *v;
      int move_offset = OFFSET(el[k].x, el[k].y);
      int xor_att = (el[k].color == color ? ATT_O ^ ATT_O : ATT_O ^ ATT_X);

      candidate[candidates] = k;
      existing_variation[candidates] = 0;

      for (v = variations; v != NULL; v = v->next) {
	if (v->move_offset == move_offset
	    && (v->xor_att == xor_att || color == 0)) {
	  existing_variation[candidates] = 1;
	  have_existing_variation = 1;
	  break;
	}
      }

      candidates++;
    }
  }

  for (k = 0; k < candidates; k++) {
    int m = candidate[k];
    int value = 2 * MAX_BOARD * ((el[m].x + 1) * (el[m].y + 1)
	        - (gg_min(el[m].x, x) + 1) * (gg_min(el[m].y, y) + 1))
	        - 2 * gg_abs(el[m].x - el[m].y) + (el[m].x < el[m].y ? 1 : 0);

    if (existing_variation[k] == have_existing_variation
	&& value > best_value) {
      best = k;
      best_value = value;
    }
  }

  return candidate[best];
}


static struct corner_variation_b*
corner_variation_new(int move_offset, char xor_att, char num_stones)
{
  struct corner_variation_b *variation;
   
  variation = malloc(sizeof(struct corner_variation_b));

  variation->move_offset = move_offset;
  variation->xor_att = xor_att;
  variation->num_stones = num_stones;
  variation->num_variations = 0;
  variation->next = NULL;
  variation->child = NULL;
  variation->child_num = -1;
  variation->pattern_num = -1;

  total_variations++;

  return variation;
}


static struct corner_variation_b*
corner_follow_variation(struct corner_variation_b *variation,
			int offset, int color, char num_stones)
{
  char xor_att = color ? ATT_O ^ ATT_O : ATT_O ^ ATT_X;
  struct corner_variation_b *subvariation = variation->child;
  struct corner_variation_b **link = &(variation->child);

  while (subvariation) {
    if (subvariation->move_offset == offset
	&& subvariation->xor_att == xor_att) {
      assert(subvariation->num_stones == num_stones);
      return subvariation;
    }

    link = &(subvariation->next);
    subvariation = subvariation->next;
  }

  variation->num_variations++;
  *link = corner_variation_new(offset, xor_att, num_stones);

  return *link;
}


static void
corner_add_pattern(void)
{
  int k;
  struct corner_element stone[MAX_BOARD * MAX_BOARD];
  int stones = 0;
  int trans;
  int corner_x = 0;
  int corner_y = 0;
  int color = 0;
  int move_pos;
  int move_x;
  int move_y;
  char num_stones;
  struct corner_variation_b *variation = &corner_root;

  switch (where) {
    case SOUTH_EDGE | WEST_EDGE: trans = 5; corner_x = maxi; break;
    case WEST_EDGE | NORTH_EDGE: trans = 0; break;
    case NORTH_EDGE | EAST_EDGE: trans = 7; corner_y = maxj; break;
    case EAST_EDGE | SOUTH_EDGE:
      trans = 2; corner_x = maxi; corner_y = maxj; break;

    default:
      fprintf(stderr, "%s(%d) : error : Illegal edge constraint in pattern %s\n",
	      current_file, current_line_number, pattern_names[patno]);
      return;
  }

  move_pos = AFFINE_TRANSFORM(pattern[patno].move_offset
             - OFFSET_DELTA(corner_x, corner_y), trans, POS(0, 0));
  move_x = I(move_pos);
  move_y = J(move_pos);

  for (k = 0; k < el; k++) {
    if (elements[k].att == ATT_X || elements[k].att == ATT_O) {
      TRANSFORM2(elements[k].x, elements[k].y,
		 &stone[stones].x, &stone[stones].y, trans);
      stone[stones].x += corner_x;
      stone[stones].y += corner_y;
      stone[stones].color = elements[k].att;
      stones++;
    }
    else if (elements[k].att != ATT_dot) {
      fprintf(stderr, "%s(%d) : error : Illegal element in pattern %s\n",
	      current_file, current_line_number, pattern_names[patno]);
      return;
    }
  }

  for (k = 0; k < stones; k++) {
    int i;
    int best;
    struct corner_element stone_t;
     
    if (k > 0) {
      best = k + corner_best_element(stone + k, stones - k, stone[k-1].x,
				     stone[k-1].y, variation->child, color);
      stone_t = stone[k];
      stone[k] = stone[best];
      stone[best] = stone_t;
    }
    else {
      best = corner_best_element(stone, stones, 0, 0, variation->child, color);
      stone_t = stone[0];
      stone[0] = stone[best];
      stone[best] = stone_t;
      color = stone[0].color;

      if (stone[0].x > stone[0].y) {
	int t;

	t = maxi;
	maxi = maxj;
	maxj = t;

	t = move_x;
	move_x = move_y;
	move_y = t;

	for (i = 0; i < stones; i++) {
	  t = stone[i].x;
	  stone[i].x = stone[i].y;
	  stone[i].y = t;
	}
      }
    }

    num_stones = 0;
    for (i = 0; i < k; i++) {
      if (stone[i].x <= stone[k].x && stone[i].y <= stone[k].y)
	num_stones++;
    }

    variation = corner_follow_variation(variation,
		OFFSET(stone[k].x, stone[k].y), stone[k].color == color,
		num_stones);
  }

  num_stones = 0;
  for (k = 0; k < stones; k++) {
    if (stone[k].x <= move_x && stone[k].y <= move_y)
      num_stones++;
  }

  variation = corner_follow_variation(variation, OFFSET(move_x, move_y),
				      ATT_O == color, num_stones);
  if (variation->pattern_num == -1) {
    variation->pattern_num = patno;
    second_corner_offset[patno] = OFFSET(maxi, maxj);
    if (maxi > corner_max_height)
      corner_max_height = maxi;
    if (maxj > corner_max_width)
      corner_max_width = maxj;
  }
  else {
    fprintf(stderr, "%s(%d) : warning : duplicated patterns encountered (%s and %s)\n",
	    current_file, current_line_number,
	    pattern_names[variation->pattern_num], pattern_names[patno]);
    discard_pattern = 1;
  }
}


static int
corner_pack_variations(struct corner_variation_b *variation, int counter)
{
  counter++;
  if (variation->next)
    counter = corner_pack_variations(variation->next, counter);
  if (variation->child) {
    variation->child_num = counter;
    counter = corner_pack_variations(variation->child, counter);
  }

  return counter;
}


static void
corner_write_variations(struct corner_variation_b *variation, FILE *outfile)
{                     
  fprintf(outfile, "  {%d,%d,%d,%d,", variation->move_offset,
	  variation->xor_att, variation->num_stones,
	  variation->num_variations);
  if (variation->child)
    fprintf(outfile, "&%s_variations[%d],", prefix, variation->child_num);
  else
    fprintf(outfile, "NULL,");
  if (variation->pattern_num != -1)
    fprintf(outfile, "&%s[%d]", prefix, variation->pattern_num);
  else
    fprintf(outfile, "NULL");

  variations_written++;
  if (variations_written != total_variations)
    fprintf(outfile, "},\n");
  else
    fprintf(outfile, "}\n};\n\n\n");

  if (variation->next)
    corner_write_variations(variation->next, outfile);
  if (variation->child)
    corner_write_variations(variation->child, outfile);
}


/* sort and write out the patterns */
static void
write_patterns(FILE *outfile)
{
  int j;

  /* Write out the patterns. */
  if (database_type == DB_FULLBOARD)
    fprintf(outfile, "struct fullboard_pattern %s[] = {\n", prefix);
  else if (database_type == DB_CORNER)
    fprintf(outfile, "static struct corner_pattern %s[] = {\n", prefix);
  else
    fprintf(outfile, "static struct pattern %s[] = {\n", prefix);

  for (j = 0; j < patno; ++j) {
    struct pattern *p = pattern + j;

    if (database_type == DB_FULLBOARD) {
      fprintf(outfile, "  {%s%d,%d,\"%s\",%d,%f},\n", prefix, j, p->patlen,
	      pattern_names[j], p->move_offset, p->value);
      continue;
    }

    if (database_type == DB_CORNER) {
      fprintf(outfile, "  {%d,0x%x,\"%s\",%f,%d,", second_corner_offset[j],
	      p->class, pattern_names[j], p->value, p->autohelper_flag);
      if (p->autohelper)
	fprintf(outfile, "autohelper%s%d", prefix, j);
      else
	fprintf(outfile, "NULL");
      fprintf(outfile, ",%f}", p->constraint_cost);

      if (j != patno - 1)
	fprintf(outfile, ",\n");
      else
	fprintf(outfile, "\n};\n\n\n");

      continue;
    }

    /* p->min{i,j} and p->max{i,j} are the maximum extent of the elements,
     * including any rows of '?' which do not feature in the elements list.
     * ie they are the positions of the topleft and bottomright corners of
     * the pattern, relative to the pattern origin. These just transform same
     * as the elements.
     */
    
    fprintf(outfile, "  {%s%d,%d,%d, \"%s\",%d,%d,%d,%d,%d,%d,0x%x,%d",
	    prefix, j,
	    p->patlen,
	    p->trfno,
	    pattern_names[j],
	    p->mini, p->minj,
	    p->maxi, p->maxj,
	    p->maxi - p->mini,   /* height */
	    p->maxj - p->minj,   /* width  */
	    p->edge_constraints,
	    p->move_offset);


#if GRID_OPT
    fprintf(outfile, ",\n    {");
    {
      int ll;

      for (ll = 0; ll < 8; ++ll)
	fprintf(outfile, " 0x%08x%s", p->and_mask[ll], ll<7 ? "," : "");
      fprintf(outfile, "},\n    {");
      for (ll = 0; ll < 8; ++ll)
	fprintf(outfile, " 0x%08x%s", p->val_mask[ll], ll<7 ? "," : "");
    }
    fprintf(outfile, "}\n   ");
#endif

    fprintf(outfile, ", 0x%x,%f,%f,%f,%f,%f,%f,%f,%d,%s,",
	    p->class,
	    p->value,
	    p->maxvalue,
	    p->minterritory,
	    p->maxterritory,
	    p->shape,
	    p->followup,
	    p->reverse_followup,
	    p->autohelper_flag,
	    helper_fn_names[j]);
    if (p->autohelper)
      fprintf(outfile, "autohelper%s%d", prefix, j);
    else
      fprintf(outfile, "NULL");
    fprintf(outfile, ",%d", p->anchored_at_X);
    fprintf(outfile, ",%f", p->constraint_cost);
#if PROFILE_PATTERNS
    fprintf(outfile, ",0,0");
    fprintf(outfile, ",0");
#endif

    fprintf(outfile, "},\n");
  }

  if (database_type == DB_CORNER)
    return;

  if (database_type == DB_FULLBOARD) {
    fprintf(outfile, "  {NULL,0,NULL,0,0.0}\n};\n");
    return;
  }
  
  /* Add a final empty entry. */
  fprintf(outfile, "  {NULL, 0,0,NULL,0,0,0,0,0,0,0,0");
#if GRID_OPT
  fprintf(outfile, ",{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}");
#endif
  fprintf(outfile, ",0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0,NULL,NULL,0,0.0");
#if PROFILE_PATTERNS
  fprintf(outfile, ",0,0");
  fprintf(outfile, ",0");
#endif
  fprintf(outfile, "}\n};\n");
}

/* Write out the pattern db */
static void
write_pattern_db(FILE *outfile)
{
  /* Don't want this for fullboard patterns. */
  if (database_type == DB_FULLBOARD)
    return;

  if (database_type == DB_CORNER) {
    fprintf(outfile, "struct corner_db %s_db = {\n", prefix);
    fprintf(outfile, "  %d,\n", corner_max_width);
    fprintf(outfile, "  %d,\n", corner_max_height);
    fprintf(outfile, "  %d,\n", corner_root.num_variations);
    fprintf(outfile, "  %s_variations\n", prefix);
    fprintf(outfile, "};\n");

    return;
  }

  /* Write out the pattern database. */
  fprintf(outfile, "\n");
  fprintf(outfile, "struct pattern_db %s_db = {\n", prefix);
  fprintf(outfile, "  -1,\n");  /* fixed_for_size */
  fprintf(outfile, "  %d,\n", fixed_anchor);
  fprintf(outfile, "  %s\n", prefix);
  if (database_type == DB_DFA)
    fprintf(outfile, " ,& dfa_%s\n", prefix); /* pointer to the wired dfa */
  else
    fprintf(outfile, " , NULL\n"); /* pointer to a possible dfa */
  fprintf(outfile, "};\n");
}


int
main(int argc, char *argv[])
{
  static char stdin_name[] = "<stdin>";
  int input_files = 0;
  int ifc;
  char *input_file_names[MAX_INPUT_FILE_NAMES];
  char *output_file_name = NULL;
  FILE *input_FILE = stdin;
  FILE *output_FILE = stdout;
  int state = 0;
  char *save_code_pos = autohelper_code;

  transformation_init();

  {
    int i;
    int multiple_anchor_options = 0;

    /* Parse command-line options */
    while ((i = gg_getopt(argc, argv, "i:o:vV:pcfCDTOXbmar")) != EOF) {
      switch (i) {
      case 'i': 
	if (input_files == MAX_INPUT_FILE_NAMES) {
	  fprintf(stderr, "Error : Too many input files (maximum %d supported)\n",
	    MAX_INPUT_FILE_NAMES);
	  return 1;
        }
	input_file_names[input_files++] = gg_optarg;
	break;

      case 'o': output_file_name = gg_optarg; break;
      case 'v': verbose = 1; break;
      case 'V': dfa_verbose = strtol(gg_optarg, NULL, 10); break;

      case 'p':
      case 'c':
      case 'f':
      case 'C':
      case 'D':
      case 'T':
	if (database_type) {
	  fprintf(stderr, "Error : More than one database type specified (-%c and -%c)\n",
		  database_type, i);
	  return 1;
	}
	database_type = i;
	if (i == 'T') {
	  fprintf(stderr, "Error : Tree based matcher is unsupported in this configuration\n");
	  fprintf(stderr, "        Try `configure --enable-experimental-reading'\n");
	  return 1;
	}
	break;

      case 'O':
	if (anchor)
	  multiple_anchor_options = 1;
	anchor = ANCHOR_O;
	break;
      case 'X':
	if (anchor)
	  multiple_anchor_options = 1;
	anchor = ANCHOR_X;
	break;
      case 'b':
	if (anchor)
	  multiple_anchor_options = 1;
	anchor = ANCHOR_BOTH;
	break;

      case 'm': 
        choose_best_anchor = 1;
        if (fixed_anchor)
          fprintf(stderr, "Warning : -m and -a are mutually exclusive.\n");
	break;
      case 'a': 
        fixed_anchor = 1; 
        if (choose_best_anchor)
          fprintf(stderr, "Warning : -m and -a are mutually exclusive.\n");
        break;

      case 'r': pre_rotate = 1; break;
      
      default:
	fprintf(stderr, "\b ; ignored\n");
      }
    }

    if (!database_type)
      database_type = DB_GENERAL;
    if (!anchor)
      anchor = ANCHOR_O;

    if (!input_files)
      input_file_names[input_files++] = stdin_name;
    if (output_file_name) {
      output_FILE = fopen(output_file_name, "wb");
      if (output_FILE == NULL) {
	fprintf(stderr, "Error : Cannot write to file %s\n", output_file_name);
	return 1;
      }
    }

    if (multiple_anchor_options)
      fprintf(stderr, "Warning : Multiple anchor options encountered. The last took precedence\n");
  }

  if (gg_optind >= argc) {
    fputs(USAGE, stderr);
    exit(EXIT_FAILURE);
  }

  prefix = argv[gg_optind];

  if (database_type == DB_DFA) {
    dfa_init();
    new_dfa(&dfa, "mkpat's dfa");
    dfa.pre_rotated = pre_rotate;
  }

  if (database_type == DB_CORNER)
    corner_init();

  fprintf(output_FILE, PREAMBLE);

  /* Initialize pattern number and buffer for automatically generated
   * helper code.
   */
  patno = -1;
  autohelper_code[0] = 0;
  code_pos = autohelper_code;

  /* Parse the input file, output pattern elements as as we find them,
   * and store pattern entries for later output.
   *
   * We do this parsing too with the help of a state machine.
   * state
   *   0     Waiting for a Pattern line.
   *   1     Pattern line found, waiting for a position diagram.
   *   2     Reading position diagram.
   *   3     Waiting for entry line (":" line).
   *   4     Waiting for optional constraint diagram.
   *   5     Reading constraint diagram.
   *   6     Waiting for constraint line (";" line).
   *   7     Reading a constraint
   *   8     Reading an action
   *
   * FIXME: This state machine should be possible to simplify.
   *
   */

  for (ifc = 0; ifc < input_files && !fatal_errors; ifc++) {
    char line[MAXLINE];  /* current line from file */

    if (input_file_names[ifc] != stdin_name) {
      input_FILE = fopen(input_file_names[ifc], "r");
      if (input_FILE == NULL) {
	fprintf(stderr, "Error: Cannot open file %s\n", input_file_names[ifc]);
	return 1;
      }
    }
 
    current_file = input_file_names[ifc];
    current_line_number = 0;

    while (fgets(line, MAXLINE, input_FILE)) {
      int command = 0;
      char command_data[MAXLINE];

      current_line_number++;
      if (line[strlen(line)-1] != '\n') {
	fprintf(stderr, "%s(%d) : Error : line truncated (longer than %d characters)\n",
		current_file, current_line_number, MAXLINE - 1);

	fatal_errors++;
      }
      
      /* Remove trailing white space from `line' */
      {
	int i = strlen(line) - 2;  /* Start removing backwards just before newline */
	while (i >= 0 && isspace(line[i]))
	  i--;
	line[i+1] = '\n';
	line[i+2] = '\0';
      }
      
      if (sscanf(line, "Pattern %s", command_data) == 1)
	command = 1;
      else if (sscanf(line, "goal_elements %s", command_data) == 1)
	command = 2;
      else if (sscanf(line, "callback_data %s", command_data) == 1)
	command = 3;

      if (command) {
	char *p = strpbrk(command_data, " \t");
	
	if (p)
	  *p = 0;

	if (patno >= 0) {
	  switch (state) {
	  case 1:
	    fprintf(stderr, "%s(%d) : Warning: empty pattern %s\n",
		    current_file, current_line_number, pattern_names[patno]);
	    break;
	  case 2:
	  case 3:
	    fprintf(stderr, "%s(%d) : Error : No entry line for pattern %s\n",
		    current_file, current_line_number, pattern_names[patno]);
	    fatal_errors++;
	    break;
	  case 5:
	  case 6:
	    fprintf(stderr,
		    "%s(%d) : Warning: constraint diagram but no constraint line for pattern %s\n",
		    current_file, current_line_number, pattern_names[patno]);
	    break;
	  case 7:
	  case 8:
	    finish_constraint_and_action();
							/* fall through */
	  case 0:
	  case 4:
	    check_constraint_diagram();
	  }
	}

	if (command == 1) { /* Pattern `name' */
	  if (!discard_pattern)
	    patno++;
	  else
	    code_pos = save_code_pos;
	  reset_pattern();

	  if (strlen(command_data) > 79) {
	    fprintf(stderr, "%s(%d) : Warning : pattern name is too long, truncated\n",
		    current_file, current_line_number);
	    command_data[79] = 0;
	  }
	  strcpy(pattern_names[patno], command_data);

	  state = 1;
	  discard_pattern = 0;
	}
	else {
	  int *sort_out = (command == 2 ? nongoal : callback_unneeded);
	  int k;

	  for (k = 0; k < 8; k++)
	    sort_out[k] = 1;

	  if (strcmp(command_data, "none")) {
	    char *c;

	    for (c = command_data; *c; c++) {
	      switch (*c) {
	      case '.':
		sort_out[ATT_dot] = 0;
		if (command == 2) { /* goal_elements */
		  sort_out[ATT_comma] = 0;
		  sort_out[ATT_not]   = 0;
		}
		break;
	      case 'X': sort_out[ATT_X] = 0; break;
	      case 'O': sort_out[ATT_O] = 0; break;
	      case 'x': sort_out[ATT_x] = 0; break;
	      case 'o': sort_out[ATT_o] = 0; break;
	      case ',':
		sort_out[ATT_comma] = 0;
		if (command != 2)
		  break;
	      case '!':
		sort_out[ATT_not] = 0;
		if (command != 2)
		  break;
	      default:
		fprintf(stderr, "%s(%d) : Error : illegal character `%c'\n",
			current_file, current_line_number, *c);
		fatal_errors++;
	      }
	    }
	  }

	  state = 0;
	}
      }
      else if (line[0] == '\n' || line[0] == '#') { 
	/* nothing */
	if (state == 2 || state == 5) {
	  if (state == 5)
	    check_constraint_diagram_size();
	  state++;
	}
      }
      else if (strchr(VALID_PATTERN_CHARS, line[0])
	       || strchr(VALID_EDGE_CHARS, line[0])
	       || strchr(VALID_CONSTRAINT_LABELS, line[0])) { 
	/* diagram line */
	switch (state) {
	case 0:
	case 3:
	case 6:
	case 7:
	case 8:
	  fprintf(stderr, 
		  "%s(%d) : error : What, another diagram here? (pattern %s)\n",
		  current_file, current_line_number, pattern_names[patno]);
	  fatal_errors++;
	  break;
	case 1:
	  state++; /* fall through */
	case 2:
	  if (!read_pattern_line(line)) {
	    discard_pattern = 1;
	    el = 0;
	  }
	  break;
	case 4:
	  state++; /* fall through */
	case 5:
	  read_constraint_diagram_line(line);
	  break;
	}	
      }
      else if (line[0] == ':') {
	if (state == 2 || state == 3) {
	  finish_pattern(line);
	  
	  if (!discard_pattern) {
	    if (database_type == DB_DFA)
	      write_to_dfa(patno);
	    if (database_type != DB_CORNER)
	      write_elements(output_FILE);
	    else
	      corner_add_pattern();
	  }

	  state = 4;
	  save_code_pos = code_pos;
	}
	else {
	  fprintf(stderr,
		  "%s(%d) : warning : Unexpected entry line in pattern %s\n",
		  current_file, current_line_number, pattern_names[patno]);
	}
      } 
      else if (line[0] == ';') {
	if (state == 5)
	  check_constraint_diagram_size();
	
	if (state == 5 || state == 6 || state == 7) {
	  read_constraint_line(line+1);
	  state = 7;
	}
	else {
	  fprintf(stderr, "%s(%d) : Warning: unexpected constraint line in pattern %s\n",
		  current_file, current_line_number, pattern_names[patno]);
	}
      } 
      else if (line[0] == '>') {
	if (state == 4 || state == 5 || state == 6 
	    || state == 7 || state == 8) {
	  if (state == 5)
	    check_constraint_diagram_size();
	  read_action_line(line+1);
	  state = 8;
	}
	else {
	  fprintf(stderr, "Warning: unexpected action line in pattern %s\n",
		  pattern_names[patno]);
	}
      } 
      else {
	int i = strlen(line);
	char c = line[i-1];
	line[i-1] = 0;  /* Chop off \n */
	fprintf(stderr, "%s(%d) : error : Malformed line \"%s\" in pattern %s\n",
		current_file, current_line_number, line, pattern_names[patno]);
	line[i-1] = c;  /* Put it back - maybe not necessary at this point. */
	fatal_errors++;
      }
    } /* while not EOF */
  } /* for each file */
  
  if (patno >= 0) {
    switch (state) {
    case 1:
      fprintf(stderr, "Warning: empty pattern %s\n",
	      pattern_names[patno]);
      break;
    case 2:
    case 3:
      fprintf(stderr, "%s(%d) : Error : no entry line for pattern %s\n",
	      current_file, current_line_number, pattern_names[patno]);
      fatal_errors++;
      break;
    case 5:
    case 6:
      fprintf(stderr,
	      "Warning: constraint diagram but no constraint line for pattern %s\n",
	      pattern_names[patno]);
      break;
    case 7:
    case 8:
      finish_constraint_and_action(); /* fall through */
    case 0:
    case 4:
      check_constraint_diagram();
      patno++;
      reset_pattern();
    }
  } 

  if (discard_pattern) {
    patno--;
    code_pos = save_code_pos;
  }

  *code_pos = 0;

  if (verbose)
    fprintf(stderr, "%d / %d patterns have edge-constraints\n",
	    pats_with_constraints, patno);

#if 0
  /* Forward declaration, which autohelpers might need. */
  if (database_type != DB_FULLBOARD) {
    if (database_type != DB_CORNER)
      fprintf(output_FILE, "extern struct pattern %s[];\n\n", prefix);
    else
      fprintf(output_FILE, "extern struct corner_pattern %s[];\n\n", prefix);
  }
#endif

  /* Write the autohelper code. */
  fprintf(output_FILE, "%s", autohelper_code);

  write_patterns(output_FILE);

  if (database_type == DB_DFA) {
    fprintf(stderr, "---------------------------\n");

    dfa_finalize(&dfa);
    dfa_shuffle(&dfa);

    fprintf(stderr, "dfa for %s\n", prefix);
    fprintf(stderr, "size: %d kB for ", dfa_size(&dfa));
    fprintf(stderr, "%d patterns", patno);
    fprintf(stderr, "(%d states)\n", dfa.last_state);

    if (0 && dfa.pre_rotated)
      dump_dfa(stderr, &dfa);

    strcpy(dfa.name, prefix);
    print_c_dfa(output_FILE, prefix, &dfa);
    fprintf(stderr, "---------------------------\n");

    if (DFA_MAX_MATCHED/8 < patno)
      fprintf(stderr, "Warning: Increase DFA_MAX_MATCHED in 'dfa.h'.\n");

    kill_dfa(&dfa);
    dfa_end();
  }

  if (database_type == DB_CORNER) {
    fprintf(stderr, "---------------------------\n");

    corner_pack_variations(corner_root.child, 0);
    fprintf(output_FILE, "static struct corner_variation %s_variations[] = {\n",
	    prefix);
    corner_write_variations(corner_root.child, output_FILE);

    fprintf(stderr, "corner database for %s\n", prefix);
    fprintf(stderr, "size: %d kB for %d patterns (%d variations)\n",
	    CORNER_DB_SIZE(patno, total_variations), patno, total_variations);
    fprintf(stderr, "---------------------------\n");
  }

  write_pattern_db(output_FILE);

  if (fatal_errors) {
    fprintf(output_FILE, "\n#error: One or more fatal errors compiling %s\n",
	    current_file);
  }

  return fatal_errors ? 1 : 0;
}


/*
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */

