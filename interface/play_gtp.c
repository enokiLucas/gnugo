/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * This is GNU GO, a Go program. Contact gnugo@gnu.org, or see   *
 * http://www.gnu.org/software/gnugo/ for more information.      *
 *                                                               *
 * Copyright 1999, 2000, 2001 by the Free Software Foundation.   *
 *                                                               *
 * This program is free software; you can redistribute it and/or *
 * modify it under the terms of the GNU General Public License   *
 * as published by the Free Software Foundation - version 2.     *
 *                                                               *
 * This program is distributed in the hope that it will be       *
 * useful, but WITHOUT ANY WARRANTY; without even the implied    *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       *
 * PURPOSE.  See the GNU General Public License in file COPYING  *
 * for more details.                                             *
 *                                                               *
 * You should have received a copy of the GNU General Public     *
 * License along with this program; if not, write to the Free    *
 * Software Foundation, Inc., 59 Temple Place - Suite 330,       *
 * Boston, MA 02111, USA.                                        *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "interface.h"
#include "liberty.h"
#include "gtp.h"
#include "random.h"
#include "gg_utils.h"

/* Internal state that's not part of the engine. */
float komi;
int handicap;

static int report_uncertainty = 0;
static int gtp_orientation = 0;

static void
print_influence(float white_influence[BOARDMAX],
		float black_influence[BOARDMAX],
		int influence_regions[BOARDMAX]);
static void gtp_print_code(int c);
static void gtp_print_vertices2(int n, int *moves);
static void rotate_on_input(int ai, int aj, int *bi, int *bj);
static void rotate_on_output(int ai, int aj, int *bi, int *bj);

/* For undo, we keep the starting position, and a stack
 * listing all moves made.
 */

#define MAX_MOVES 1000
static Position starting_position;

struct game_move_data {
  int i;
  int j;
  int color;
};
static struct game_move_data game_move[MAX_MOVES];
static int move_stack_pointer = 0;

#define DECLARE(func) static int func(char *s, int id)

DECLARE(gtp_aa_confirm_safety);
DECLARE(gtp_all_legal);
DECLARE(gtp_attack);
DECLARE(gtp_combination_attack);
DECLARE(gtp_connect);
DECLARE(gtp_countlib);
DECLARE(gtp_cputime);
DECLARE(gtp_decrease_depths);
DECLARE(gtp_defend);
DECLARE(gtp_disconnect);
DECLARE(gtp_dragon_data);
DECLARE(gtp_dragon_status);
DECLARE(gtp_dragon_stones);
DECLARE(gtp_dump_stack);
DECLARE(gtp_echo);
DECLARE(gtp_echo_err);
DECLARE(gtp_eval_eye);
DECLARE(gtp_final_score);
DECLARE(gtp_final_status);
DECLARE(gtp_final_status_list);
DECLARE(gtp_findlib);
DECLARE(gtp_finish_sgftrace);
DECLARE(gtp_fixed_handicap);
DECLARE(gtp_genmove);
DECLARE(gtp_genmove_black);
DECLARE(gtp_genmove_white);
DECLARE(gtp_get_connection_node_counter);
DECLARE(gtp_get_life_node_counter);
DECLARE(gtp_get_owl_node_counter);
DECLARE(gtp_get_reading_node_counter);
DECLARE(gtp_get_trymove_counter);
DECLARE(gtp_help);
DECLARE(gtp_increase_depths);
DECLARE(gtp_influence);
DECLARE(gtp_is_legal);
DECLARE(gtp_list_stones);
DECLARE(gtp_loadsgf);
DECLARE(gtp_name);
DECLARE(gtp_new_game);
DECLARE(gtp_estimate_score);
DECLARE(gtp_owl_analyze_semeai);
DECLARE(gtp_tactical_analyze_semeai);
DECLARE(gtp_owl_attack);
DECLARE(gtp_owl_defend);
DECLARE(gtp_owl_does_attack);
DECLARE(gtp_owl_does_defend);
DECLARE(gtp_owl_threaten_attack);
DECLARE(gtp_owl_threaten_defense);
DECLARE(gtp_playblack);
DECLARE(gtp_playwhite);
DECLARE(gtp_popgo);
DECLARE(gtp_captures);
DECLARE(gtp_protocol_version);
DECLARE(gtp_query_boardsize);
DECLARE(gtp_query_orientation);
DECLARE(gtp_quit);
DECLARE(gtp_report_uncertainty);
DECLARE(gtp_reset_connection_node_counter);
DECLARE(gtp_reset_life_node_counter);
DECLARE(gtp_reset_owl_node_counter);
DECLARE(gtp_reset_reading_node_counter);
DECLARE(gtp_reset_trymove_counter);
DECLARE(gtp_same_dragon);
DECLARE(gtp_set_boardsize);
DECLARE(gtp_set_orientation);
DECLARE(gtp_set_komi);
DECLARE(gtp_set_level);
DECLARE(gtp_showboard);
DECLARE(gtp_start_sgftrace);
DECLARE(gtp_test_eyeshape);
DECLARE(gtp_top_moves);
DECLARE(gtp_top_moves_white);
DECLARE(gtp_top_moves_black);
DECLARE(gtp_trymove);
DECLARE(gtp_tune_move_ordering);
DECLARE(gtp_undo);
DECLARE(gtp_version);
DECLARE(gtp_what_color);
DECLARE(gtp_worm_cutstone);
DECLARE(gtp_worm_data);
DECLARE(gtp_worm_stones);

/* List of known commands. */
static struct gtp_command commands[] = {
  {"aa_confirm_safety",       gtp_aa_confirm_safety},
  {"all_legal",        	      gtp_all_legal},
  {"attack",           	      gtp_attack},
  {"black",            	      gtp_playblack},
  {"boardsize",        	      gtp_set_boardsize},
  {"captures",        	      gtp_captures},
  {"color",            	      gtp_what_color},
  {"combination_attack",      gtp_combination_attack},
  {"connect",         	      gtp_connect},
  {"countlib",         	      gtp_countlib},
  {"cputime",		      gtp_cputime},
  {"decrease_depths",  	      gtp_decrease_depths},
  {"defend",           	      gtp_defend},
  {"disconnect",       	      gtp_disconnect},
  {"dragon_data",             gtp_dragon_data},
  {"dragon_status",    	      gtp_dragon_status},
  {"dragon_stones",           gtp_dragon_stones},
  {"dump_stack",       	      gtp_dump_stack},
  {"echo" ,                   gtp_echo},
  {"echo_err" ,               gtp_echo_err},
  {"estimate_score",          gtp_estimate_score},
  {"eval_eye",         	      gtp_eval_eye},
  {"final_score",             gtp_final_score},
  {"final_status",            gtp_final_status},
  {"final_status_list",       gtp_final_status_list},
  {"findlib",          	      gtp_findlib},
  {"finish_sgftrace",  	      gtp_finish_sgftrace},
  {"fixed_handicap",   	      gtp_fixed_handicap},
  {"genmove_black",           gtp_genmove_black},
  {"genmove_white",           gtp_genmove_white},
  {"get_connection_node_counter", gtp_get_connection_node_counter},
  {"get_life_node_counter",   gtp_get_life_node_counter},
  {"get_owl_node_counter",    gtp_get_owl_node_counter},
  {"get_reading_node_counter",gtp_get_reading_node_counter},
  {"get_trymove_counter",     gtp_get_trymove_counter},
  {"gg_genmove",              gtp_genmove},
  {"help",                    gtp_help},
  {"increase_depths",  	      gtp_increase_depths},
  {"influence",               gtp_influence},
  {"is_legal",         	      gtp_is_legal},
  {"komi",        	      gtp_set_komi},
  {"level",        	      gtp_set_level},
  {"list_stones",    	      gtp_list_stones},
  {"loadsgf",          	      gtp_loadsgf},
  {"name",                    gtp_name},
  {"new_game",                gtp_new_game},
  {"new_score",               gtp_estimate_score},
  {"owl_analyze_semeai",      gtp_owl_analyze_semeai},
  {"tactical_analyze_semeai", gtp_tactical_analyze_semeai},
  {"owl_attack",     	      gtp_owl_attack},
  {"owl_defend",     	      gtp_owl_defend},
  {"owl_does_attack", 	      gtp_owl_does_attack},
  {"owl_does_defend", 	      gtp_owl_does_defend},
  {"owl_threaten_attack",     gtp_owl_threaten_attack},
  {"owl_threaten_defense",    gtp_owl_threaten_defense},
  {"popgo",            	      gtp_popgo},
  {"orientation",     	      gtp_set_orientation},
  {"protocol_version",        gtp_protocol_version},
  {"query_boardsize",         gtp_query_boardsize},
  {"query_orientation",       gtp_query_orientation},
  {"quit",             	      gtp_quit},
  {"report_uncertainty",      gtp_report_uncertainty},
  {"reset_connection_node_counter", gtp_reset_connection_node_counter},
  {"reset_life_node_counter", gtp_reset_life_node_counter},
  {"reset_owl_node_counter",  gtp_reset_owl_node_counter},
  {"reset_reading_node_counter", gtp_reset_reading_node_counter},
  {"reset_trymove_counter",   gtp_reset_trymove_counter},
  {"same_dragon",    	      gtp_same_dragon},
  {"showboard",        	      gtp_showboard},
  {"start_sgftrace",  	      gtp_start_sgftrace},
  {"test_eyeshape",           gtp_test_eyeshape},
  {"top_moves",               gtp_top_moves},
  {"top_moves_black",         gtp_top_moves_black},
  {"top_moves_white",         gtp_top_moves_white},
  {"trymove",          	      gtp_trymove},
  {"tune_move_ordering",      gtp_tune_move_ordering},
  {"undo",                    gtp_undo},
  {"version",                 gtp_version},
  {"white",            	      gtp_playwhite},
  {"worm_cutstone",           gtp_worm_cutstone},
  {"worm_data",               gtp_worm_data},
  {"worm_stones",             gtp_worm_stones},  
  {NULL,                      NULL}
};


/* Start playing using the Go Text Protocol. */
void
play_gtp(FILE *gtp_input, int gtp_initial_orientation)
{
  /* Try to make sure that we have a useful level of buffering of stdout. */
#ifdef HAVE_SETLINEBUF
  setlinebuf(stdout);
#else
  setbuf(stdout, NULL);
#endif

  /* Inform the GTP utility functions about the board size. */
  board_size = 19;
  gtp_internal_set_boardsize(19);
  gtp_orientation = gtp_initial_orientation;
  gtp_set_vertex_transform_hooks(rotate_on_input, rotate_on_output);
  
  /* Prepare pattern matcher and reading code. */
  reset_engine();
  move_stack_pointer = 0;
  store_position(&starting_position);
  gtp_main_loop(commands, gtp_input);
}


/****************************
 * Administrative commands. *
 ****************************/

/* Function:  Quit
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 */
static int
gtp_quit(char *s, int id)
{
  UNUSED(s);
  gtp_success(id, "");
  return GTP_QUIT;
}


/* Function:  Report protocol version.
 * Arguments: none
 * Fails:     never
 * Returns:   protocol version number
 */
static int
gtp_protocol_version(char *s, int id)
{
  UNUSED(s);
  return gtp_success(id, "1");
}

/* Function:  Start a new game
 * Arguments: none
 * Fails:     always
 * Returns:   nothing
 */
static int
gtp_new_game(char *s, int id)
{
  UNUSED(s);
  return gtp_failure(id, "not implemented");
}


/****************************
 * Program identity.        *
 ****************************/

/* Function:  Report the name of the program.
 * Arguments: none
 * Fails:     never
 * Returns:   program name
 */
static int
gtp_name(char *s, int id)
{
  UNUSED(s);
  return gtp_success(id, "GNU Go");
}




/* Function:  Report the version number of the program.
 * Arguments: none
 * Fails:     never
 * Returns:   version number
 */
static int
gtp_version(char *s, int id)
{
  UNUSED(s);
  return gtp_success(id, VERSION);
}


/***************************
 * Setting the board size. *
 ***************************/

/* Function:  Set the board size to NxN and clear the board.
 * Arguments: integer
 * Fails:     board size outside engine's limits
 * Returns:   nothing
 */
static int
gtp_set_boardsize(char *s, int id)
{
  int boardsize;
  if (sscanf(s, "%d", &boardsize) < 1)
    return gtp_failure(id, "boardsize not an integer");
  
  if (boardsize < MIN_BOARD || boardsize > MAX_BOARD)
    return gtp_failure(id, "unacceptable boardsize");

  board_size = boardsize;
  clear_board();
  gtp_internal_set_boardsize(boardsize);
  reset_engine();
  move_stack_pointer = 0;
  store_position(&starting_position);
  return gtp_success(id, "");
}

/* Function:  Find the current boardsize
 * Arguments: none
 * Fails:     never
 * Returns:   board_size
 */
static int
gtp_query_boardsize(char *s, int id)
{
  UNUSED(s);

  return gtp_success(id, "%d", board_size);
}

/****************************
 * Setting the orientation. *
 ****************************/

/* Function:  Set the orienation to N and clear the board
 * Arguments: integer
 * Fails:     illegal orientation
 * Returns:   nothing
 */
static int
gtp_set_orientation(char *s, int id)
{
  int orientation;
  if (sscanf(s, "%d", &orientation) < 1)
    return gtp_failure(id, "orientation not an integer");
  
  if (orientation < 0 || orientation > 7)
    return gtp_failure(id, "unacceptable orientation");

  clear_board();
  gtp_orientation = orientation;
  gtp_set_vertex_transform_hooks(rotate_on_input, rotate_on_output);
  store_position(&starting_position);
  return gtp_success(id, "");
}

/* Function:  Find the current orientation
 * Arguments: none
 * Fails:     never
 * Returns:   orientation
 */
static int
gtp_query_orientation(char *s, int id)
{
  UNUSED(s);

  return gtp_success(id, "%d", gtp_orientation);
}

/***************************
 * Setting komi.           *
 ***************************/

/* Function:  Set the komi.
 * Arguments: float
 * Fails:     incorrect argument
 * Returns:   nothing
 */
static int
gtp_set_komi(char *s, int id)
{
  if (sscanf(s, "%f", &komi) < 1)
    return gtp_failure(id, "komi not a float");
  
  return gtp_success(id, "");
}


/******************
 * Playing moves. *
 ******************/

/* Function:  Play a black stone at the given vertex.
 * Arguments: vertex
 * Fails:     invalid vertex, illegal move
 * Returns:   nothing
 */
static int
gtp_playblack(char *s, int id)
{
  int i, j;
  char *c;

  for (c = s; *c; c++)
    *c = tolower((int)*c);

  if (strncmp(s, "pass", 4) == 0) {
    i = -1;
    j = -1;
  }
  else if (!gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");

  if (!is_legal(POS(i, j), BLACK))
    return gtp_failure(id, "illegal move");

  play_move(POS(i, j), BLACK);
  game_move[move_stack_pointer].i = i;
  game_move[move_stack_pointer].j = j;
  game_move[move_stack_pointer].color = BLACK;
  if (move_stack_pointer < MAX_MOVES)
    move_stack_pointer++;
  return gtp_success(id, "");
}


/* Function:  Play a white stone at the given vertex.
 * Arguments: vertex
 * Fails:     invalid vertex, illegal move
 * Returns:   nothing
 */
static int
gtp_playwhite(char *s, int id)
{
  int i, j;
  char *c;

  for (c = s; *c; c++)
    *c = tolower((int)*c);

  if (strncmp(s, "pass", 4) == 0) {
    i = -1;
    j = -1;
  }
  else if (!gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");
  
  if (!is_legal(POS(i, j), WHITE))
    return gtp_failure(id, "illegal move");

  play_move(POS(i, j), WHITE);
  game_move[move_stack_pointer].i = i;
  game_move[move_stack_pointer].j = j;
  game_move[move_stack_pointer].color = WHITE;
  if (move_stack_pointer < MAX_MOVES)
    move_stack_pointer++;
  return gtp_success(id, "");
}


/* Function:  Set up fixed placement handicap stones.
 * Arguments: number of handicap stones
 * Fails:     invalid number of stones for the current boardsize
 * Returns:   list of vertices with handicap stones
 */
static int
gtp_fixed_handicap(char *s, int id)
{
  int m, n;
  int first = 1;
  int handicap;
  if (sscanf(s, "%d", &handicap) < 1)
    return gtp_failure(id, "handicap not an integer");
  
  clear_board();
  if (placehand(handicap) != handicap)
    return gtp_failure(id, "invalid handicap");

  gtp_printid(id, GTP_SUCCESS);

  for (m = 0; m < board_size; m++)
    for (n = 0; n < board_size; n++)
      if (BOARD(m, n) != EMPTY) {
	if (!first)
	  gtp_printf(" ");
	else
	  first = 0;
	gtp_mprintf("%m", m, n);
      }
  store_position(&starting_position);
  move_stack_pointer = 0;
  return gtp_finish_response();
}


/* Function:  Load an sgf file, possibly up to a move number or the first
 *            occurence of a move.           
 * Arguments: filename + move number, vertex, or nothing
 * Fails:     missing filename or failure to open or parse file
 * Returns:   color to play
 */
static int
gtp_loadsgf(char *s, int id)
{
  char filename[GTP_BUFSIZE];
  char untilstring[GTP_BUFSIZE];
  SGFNode *sgf;
  Gameinfo gameinfo;
  int nread;
  int color_to_move;
  
  nread = sscanf(s, "%s %s", filename, untilstring);
  if (nread < 1)
    return gtp_failure(id, "missing filename");
  
  if ((sgf = readsgffile(filename)) == NULL)
    return gtp_failure(id, "cannot open or parse '%s'", filename);

  gameinfo_clear(&gameinfo, 19, 5.5); /* Probably unnecessary. */
  gameinfo_load_sgfheader(&gameinfo, sgf);

  if (nread == 1)
    color_to_move = gameinfo_play_sgftree_rot(&gameinfo, sgf, NULL,
					      gtp_orientation);
  else
    color_to_move = gameinfo_play_sgftree_rot(&gameinfo, sgf, untilstring,
                                              gtp_orientation);

  gnugo_force_to_globals(&gameinfo.position);
  movenum = gameinfo.move_number;
  komi = gameinfo.position.komi;
  handicap = gameinfo.handicap;
  gtp_internal_set_boardsize(gameinfo.position.boardsize);
  reset_engine();
  store_position(&starting_position);

  move_stack_pointer = 0;

  gtp_printid(id, GTP_SUCCESS);
  gtp_mprintf("%C", color_to_move);
  sgfFreeNode(sgf);
  return gtp_finish_response();
}


/*****************
 * Board status. *
 *****************/

/* Function:  Return the color at a vertex.
 * Arguments: vertex
 * Fails:     invalid vertex
 * Returns:   "black", "white", or "empty"
 */
static int
gtp_what_color(char *s, int id)
{
  int i, j;
  if (!gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");
  
  return gtp_success(id, color_to_string(BOARD(i, j)));
}


/* Function:  List vertices with either black or white stones.
 * Arguments: color
 * Fails:     invalid color
 * Returns:   list of vertices
 */
static int
gtp_list_stones(char *s, int id)
{
  int i, j;
  int color = EMPTY;
  int vertexi[MAX_BOARD * MAX_BOARD];
  int vertexj[MAX_BOARD * MAX_BOARD];
  int vertices = 0;
  
  if (!gtp_decode_color(s, &color))
    return gtp_failure(id, "invalid color");

  for (i = 0; i < board_size; i++)
    for (j = 0; j < board_size; j++)
      if (BOARD(i, j) == color) {
	vertexi[vertices] = i;
	vertexj[vertices++] = j;
      }

  gtp_printid(id, GTP_SUCCESS);
  gtp_print_vertices(vertices, vertexi, vertexj);
  return gtp_finish_response();
}


/* Function:  Count number of liberties for the string at a vertex.
 * Arguments: vertex
 * Fails:     invalid vertex, empty vertex
 * Returns:   Number of liberties.
 */
static int
gtp_countlib(char *s, int id)
{
  int i, j;
  if (!gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  return gtp_success(id, "%d", countlib(POS(i, j)));
}


/* Function:  Return the positions of the liberties for the string at a vertex.
 * Arguments: vertex
 * Fails:     invalid vertex, empty vertex
 * Returns:   Sorted space separated list of vertices.
 */
static int
gtp_findlib(char *s, int id)
{
  int i, j;
  int libs[MAXLIBS];
  int liberties;
  
  if (!gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  liberties = findlib(POS(i, j), MAXLIBS, libs);
  gtp_printid(id, GTP_SUCCESS);
  gtp_print_vertices2(liberties, libs);
  return gtp_finish_response();
}


/* Function:  Tell whether a move is legal.
 * Arguments: move
 * Fails:     invalid move
 * Returns:   1 if the move is legal, 0 if it is not.
 */
static int
gtp_is_legal(char *s, int id)
{
  int i, j;
  int color;
  
  if (!gtp_decode_move(s, &color, &i, &j))
    return gtp_failure(id, "invalid color or coordinate");

  return gtp_success(id, "%d", is_legal(POS(i, j), color));
}


/* Function:  List all legal moves for either color.
 * Arguments: color
 * Fails:     invalid color
 * Returns:   Sorted space separated list of vertices.
 */
static int
gtp_all_legal(char *s, int id)
{
  int i, j;
  int color;
  int movei[MAX_BOARD * MAX_BOARD];
  int movej[MAX_BOARD * MAX_BOARD];
  int moves = 0;
  
  if (!gtp_decode_color(s, &color))
    return gtp_failure(id, "invalid color");

  for (i = 0; i < board_size; i++)
    for (j = 0; j < board_size; j++)
      if (BOARD(i, j) == EMPTY && is_legal(POS(i, j), color)) {
	movei[moves] = i;
	movej[moves++] = j;
      }

  gtp_printid(id, GTP_SUCCESS);
  gtp_print_vertices(moves, movei, movej);
  return gtp_finish_response();
}


/* Function:  List the number of captures taken by either color.
 * Arguments: color
 * Fails:     invalid color
 * Returns:   Number of captures.
 */
static int
gtp_captures(char *s, int id)
{
  int color;
  
  if (!gtp_decode_color(s, &color))
    return gtp_failure(id, "invalid color");

  if (color == BLACK)
    return gtp_success(id, "%d", white_captured);
  else
    return gtp_success(id, "%d", black_captured);
}


/**********************
 * Retractable moves. *
 **********************/

/* Function:  Play a stone of the given color at the given vertex.
 * Arguments: move (color + vertex)
 * Fails:     invalid color, invalid vertex, illegal move
 * Returns:   nothing
 */
static int
gtp_trymove(char *s, int id)
{
  int i, j;
  int color;
  if (!gtp_decode_move(s, &color, &i, &j))
    return gtp_failure(id, "invalid color or coordinate");

  if (!trymove(POS(i, j), color, "gtp_trymove", NO_MOVE, EMPTY, NO_MOVE))
    return gtp_failure(id, "illegal move");

  return gtp_success(id, "");
}


/* Function:  Undo a trymove.
 * Arguments: none
 * Fails:     stack empty
 * Returns:   nothing
 */
static int
gtp_popgo(char *s, int id)
{
  UNUSED(s);

  if (stackp == 0)
    return gtp_failure(id, "Stack empty.\n");

  popgo();
  return gtp_success(id, "");
}


/*********************
 * Tactical reading. *
 *********************/

/* Function:  Try to attack a string.
 * Arguments: vertex
 * Fails:     invalid vertex, empty vertex
 * Returns:   attack code followed by attack point if attack code nonzero.
 */
static int
gtp_attack(char *s, int id)
{
  int i, j;
  int apos;
  int attack_code;
  
  if (!gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  attack_code = attack(POS(i, j), &apos);
  gtp_printid(id, GTP_SUCCESS);
  gtp_print_code(attack_code);
  if (attack_code > 0) {
    gtp_printf(" ");
    gtp_print_vertex(I(apos), J(apos));
  }
  return gtp_finish_response();
}  


/* Function:  Try to defend a string.
 * Arguments: vertex
 * Fails:     invalid vertex, empty vertex
 * Returns:   defense code followed by defense point if defense code nonzero.
 */
static int
gtp_defend(char *s, int id)
{
  int i, j;
  int dpos;
  int defend_code;
  
  if (!gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  defend_code = find_defense(POS(i, j), &dpos);
  gtp_printid(id, GTP_SUCCESS);
  gtp_print_code(defend_code);
  if (defend_code > 0) {
    gtp_printf(" ");
    gtp_print_vertex(I(dpos), J(dpos));
  }
  return gtp_finish_response();
}  


/* Function:  Increase depth values by one.
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 */
static int
gtp_increase_depths(char *s, int id)
{
  UNUSED(s);
  increase_depth_values();
  return gtp_success(id, "");
}  


/* Function:  Decrease depth values by one.
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 */
static int
gtp_decrease_depths(char *s, int id)
{
  UNUSED(s);
  decrease_depth_values();
  return gtp_success(id, "");
}  


/******************
 * owl reading. *
 ******************/

/* Function:  Try to attack a dragon.
 * Arguments: vertex
 * Fails:     invalid vertex, empty vertex
 * Returns:   attack code followed by attack point if attack code nonzero.
 */
static int
gtp_owl_attack(char *s, int id)
{
  int i, j;
  int attack_point;
  int attack_code;
  int result_certain;

  if (!gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  silent_examine_position(BOARD(i, j), EXAMINE_DRAGONS_WITHOUT_OWL);
  
  /* to get the variations into the sgf file, clear the reading cache */
  if (sgf_dumptree)
    reading_cache_clear();
  
  attack_code = owl_attack(POS(i, j), &attack_point, &result_certain);
  gtp_printid(id, GTP_SUCCESS);
  gtp_print_code(attack_code);
  if (attack_code > 0) {
    gtp_printf(" ");
    gtp_print_vertex(I(attack_point), J(attack_point));
  }
  if (!result_certain && report_uncertainty)
    gtp_printf(" uncertain");
  return gtp_finish_response();
}  


/* Function:  Try to defend a dragon.
 * Arguments: vertex
 * Fails:     invalid vertex, empty vertex
 * Returns:   defense code followed by defense point if defense code nonzero.
 */
static int
gtp_owl_defend(char *s, int id)
{
  int i, j;
  int defense_point;
  int defend_code;
  int result_certain;
  
  if (!gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  silent_examine_position(BOARD(i, j), EXAMINE_DRAGONS_WITHOUT_OWL);
  
  /* to get the variations into the sgf file, clear the reading cache */
  if (sgf_dumptree)
    reading_cache_clear();

  defend_code = owl_defend(POS(i, j), &defense_point, &result_certain);
  gtp_printid(id, GTP_SUCCESS);
  gtp_print_code(defend_code);
  if (defend_code > 0) {
    gtp_printf(" ");
    gtp_print_vertex(I(defense_point), J(defense_point));
  }
  if (!result_certain && report_uncertainty)
    gtp_printf(" uncertain");
  return gtp_finish_response();
}  

/* Function:  Try to attack a dragon in 2 moves.
 * Arguments: vertex
 * Fails:     invalid vertex, empty vertex
 * Returns:   attack code followed by the two attack points if attack code nonzero.
 */
static int
gtp_owl_threaten_attack(char *s, int id)
{
  int i, j;
  int attack_point1;
  int attack_point2;
  int attack_code;

  if (!gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  silent_examine_position(BOARD(i, j), EXAMINE_DRAGONS_WITHOUT_OWL);
  
  /* to get the variations into the sgf file, clear the reading cache */
  if (sgf_dumptree)
    reading_cache_clear();
  
  attack_code = owl_threaten_attack(POS(i, j), &attack_point1, &attack_point2);
  gtp_printid(id, GTP_SUCCESS);
  gtp_print_code(attack_code);
  if (attack_code > 0) {
    gtp_printf(" ");
    gtp_print_vertex(I(attack_point1), J(attack_point1));
    gtp_printf(" ");
    gtp_print_vertex(I(attack_point2), J(attack_point2));
  }
  return gtp_finish_response();
}  


/* Function:  Try to defend a dragon with 2 moves.
 * Arguments: vertex
 * Fails:     invalid vertex, empty vertex
 * Returns:   defense code followed by the 2 defense points if defense code nonzero.
 */
static int
gtp_owl_threaten_defense(char *s, int id)
{
  int i, j;
  int defense_point1;
  int defense_point2;
  int defend_code;
  
  if (!gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  silent_examine_position(BOARD(i, j), EXAMINE_DRAGONS_WITHOUT_OWL);
  
  /* to get the variations into the sgf file, clear the reading cache */
  if (sgf_dumptree)
    reading_cache_clear();

  defend_code = owl_defend(POS(i, j), &defense_point1, &defense_point2);
  gtp_printid(id, GTP_SUCCESS);
  gtp_print_code(defend_code);
  if (defend_code > 0) {
    gtp_printf(" ");
    gtp_print_vertex(I(defense_point1), J(defense_point1));
    gtp_printf(" ");
    gtp_print_vertex(I(defense_point2), J(defense_point2));
  }
  return gtp_finish_response();
}  


/* Function:  Examine whether a specific move attacks a dragon.
 * Arguments: vertex (move), vertex (dragon)
 * Fails:     invalid vertex, empty vertex
 * Returns:   attack code
 */
static int
gtp_owl_does_attack(char *s, int id)
{
  int i, j;
  int ti, tj;
  int attack_code;
  int n;

  n = gtp_decode_coord(s, &ti, &tj);
  if (n == 0)
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(ti, tj) != EMPTY)
    return gtp_failure(id, "move vertex must be empty");

  n = gtp_decode_coord(s + n, &i, &j);
  if (n == 0)
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "dragon vertex must not be empty");

  silent_examine_position(BOARD(i, j), EXAMINE_DRAGONS_WITHOUT_OWL);
  
  /* to get the variations into the sgf file, clear the reading cache */
  if (sgf_dumptree)
    reading_cache_clear();
  
  attack_code = owl_does_attack(POS(ti, tj), POS(i, j));
  gtp_printid(id, GTP_SUCCESS);
  gtp_print_code(attack_code);
  return gtp_finish_response();
}  


/* Function:  Examine whether a specific move defends a dragon.
 * Arguments: vertex (move), vertex (dragon)
 * Fails:     invalid vertex, empty vertex
 * Returns:   defense code
 */
static int
gtp_owl_does_defend(char *s, int id)
{
  int i, j;
  int ti, tj;
  int defense_code;
  int n;

  n = gtp_decode_coord(s, &ti, &tj);
  if (n == 0)
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(ti, tj) != EMPTY)
    return gtp_failure(id, "move vertex must be empty");

  n = gtp_decode_coord(s + n, &i, &j);
  if (n == 0)
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "dragon vertex must not be empty");

  silent_examine_position(BOARD(i, j), EXAMINE_DRAGONS_WITHOUT_OWL);
  
  /* to get the variations into the sgf file, clear the reading cache */
  if (sgf_dumptree)
    reading_cache_clear();
  
  defense_code = owl_does_defend(POS(ti, tj), POS(i, j));
  gtp_printid(id, GTP_SUCCESS);
  gtp_print_code(defense_code);
  return gtp_finish_response();
}  


/* Function:  Analyze a semeai
 * Arguments: dragona, dragonb
 * Fails:     invalid vertices, empty vertices
 * Returns:   status of dragona, dragonb assuming dragona moves first
 */
static int
gtp_owl_analyze_semeai(char *s, int id)
{
  int i, j;
  int k;
  int dragona, dragonb;
  int resulta, resultb, move;
  
  k = gtp_decode_coord(s, &i, &j);

  if (k == 0)
    return gtp_failure(id, "invalid coordinate");
  dragona = POS(i, j);
  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  if (!gtp_decode_coord(s+k, &i, &j))
    return gtp_failure(id, "invalid coordinate");
  dragonb = POS(i, j);
  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  silent_examine_position(BOARD(i, j), EXAMINE_DRAGONS_WITHOUT_OWL);
  /* to get the variations into the sgf file, clear the reading cache */
  if (sgf_dumptree)
      reading_cache_clear();

  owl_analyze_semeai(dragona, dragonb, &resulta, &resultb, &move, 1);
  gtp_printid(id, GTP_SUCCESS);
  gtp_mprintf("%s %s %m", 
	      safety_to_string(resulta),
	      safety_to_string(resultb),
	      I(move), J(move));

  return gtp_finish_response();
}  


/* Function:  Analyze a semeai, not using owl
 * Arguments: dragona, dragonb
 * Fails:     invalid vertices, empty vertices
 * Returns:   status of dragona, dragonb assuming dragona moves first
 */
static int
gtp_tactical_analyze_semeai(char *s, int id)
{
  int i, j;
  int k;
  int dragona, dragonb;
  int resulta, resultb, move;
  
  k = gtp_decode_coord(s, &i, &j);

  if (k == 0)
    return gtp_failure(id, "invalid coordinate");
  dragona = POS(i, j);
  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  if (!gtp_decode_coord(s+k, &i, &j))
    return gtp_failure(id, "invalid coordinate");
  dragonb = POS(i, j);
  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  silent_examine_position(BOARD(i, j), EXAMINE_DRAGONS_WITHOUT_OWL);
  /* to get the variations into the sgf file, clear the reading cache */
  if (sgf_dumptree)
      reading_cache_clear();

  owl_analyze_semeai(dragona, dragonb, &resulta, &resultb, &move, 0);
  gtp_printid(id, GTP_SUCCESS);
  gtp_mprintf("%s %s %m", 
	      safety_to_string(resulta),
	      safety_to_string(resultb),
	      I(move), J(move));

  return gtp_finish_response();
}  


/***********************
 * Connection reading. *
 ***********************/

/* Function:  Try to connect two strings.
 * Arguments: vertex, vertex
 * Fails:     invalid vertex, empty vertex, vertices of different colors
 * Returns:   connect result followed by connect point if successful.
 */
static int
gtp_connect(char *s, int id)
{
  int ai, aj;
  int bi, bj;
  int connect_move = PASS_MOVE;
  int result;
  int n;
  
  n = gtp_decode_coord(s, &ai, &aj);
  if (n == 0)
    return gtp_failure(id, "invalid coordinate");

  if (!gtp_decode_coord(s + n, &bi, &bj))
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(ai, aj) == EMPTY || BOARD(bi, bj) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  if (BOARD(ai, aj) != BOARD(bi, bj))
    return gtp_failure(id, "vertices must have same color");

  result = string_connect(POS(ai, aj), POS(bi, bj), &connect_move);
  gtp_printid(id, GTP_SUCCESS);
  gtp_print_code(result);
  if (result != 0)
    gtp_mprintf(" %m", I(connect_move), J(connect_move));

  return gtp_finish_response();
}  


/* Function:  Try to disconnect two strings.
 * Arguments: vertex, vertex
 * Fails:     invalid vertex, empty vertex, vertices of different colors
 * Returns:   disconnect result followed by disconnect point if successful.
 */
static int
gtp_disconnect(char *s, int id)
{
  int ai, aj;
  int bi, bj;
  int disconnect_move = PASS_MOVE;
  int result;
  int n;
  
  n = gtp_decode_coord(s, &ai, &aj);
  if (n == 0)
    return gtp_failure(id, "invalid coordinate");

  if (!gtp_decode_coord(s + n, &bi, &bj))
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(ai, aj) == EMPTY || BOARD(bi, bj) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  if (BOARD(ai, aj) != BOARD(bi, bj))
    return gtp_failure(id, "vertices must have same color");

  result = disconnect(POS(ai, aj), POS(bi, bj), &disconnect_move);
  gtp_printid(id, GTP_SUCCESS);
  gtp_print_code(result);
  if (result != 0)
    gtp_mprintf(" %m", I(disconnect_move), J(disconnect_move));

  return gtp_finish_response();
}  


/********
 * eyes *
 ********/

/* Function:  Evaluate an eye space
 * Arguments: vertex
 * Fails:     invalid vertex
 * Returns:   Minimum and maximum number of eyes. If these differ an
 *            attack and a defense point are additionally returned.
 *            If the vertex is not an eye space or not of unique color,
 *            a single -1 is returned.
 */

static int
gtp_eval_eye(char *s, int id)
{
  int m, n;
  int max, min;
  int attack_point;
  int defense_point;
  int pos;

  if (!gtp_decode_coord(s, &m, &n))
    return gtp_failure(id, "invalid coordinate");

  silent_examine_position(BLACK, EXAMINE_DRAGONS_WITHOUT_OWL);
  
  if (black_eye[POS(m, n)].color == BLACK_BORDER) {
    pos = black_eye[POS(m, n)].origin;
    compute_eyes(pos, &max, &min, &attack_point, &defense_point,
		 black_eye, half_eye, 0, EMPTY);
  }
  else if (white_eye[POS(m, n)].color == WHITE_BORDER) {
    pos = white_eye[POS(m, n)].origin;
    compute_eyes(pos, &max, &min, &attack_point, &defense_point,
		 white_eye, half_eye, 0, EMPTY);
  }
  else
    /* Not an eye or not of unique color. */
    return gtp_success(id, "-1");

  gtp_printid(id, GTP_SUCCESS);
  gtp_printf("%d %d", min, max);
  if (max != min) {
    gtp_printf(" ");
    gtp_print_vertex(I(attack_point), J(attack_point));
    gtp_printf(" ");
    gtp_print_vertex(I(defense_point), J(defense_point));
  }
  return gtp_finish_response();
}


/*****************
 * dragon status *
 *****************/

/* Function:  Determine status of a dragon.
 * Arguments: vertex
 * Fails:     invalid vertex, empty vertex
 * Returns:   status ("alive", "critical", "dead", or "unknown"),
 *            attack point, defense point. Points of attack and
 *            defense are only given if the status is critical and the
 *            owl code is enabled.
 *
 * FIXME: Should be able to distinguish between life in seki
 *        and life with territory. Should also be able to identify ko.
 */

static int
gtp_dragon_status(char *s, int id)
{
  int i, j;

  if (!gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  silent_examine_position(BLACK, EXAMINE_DRAGONS);
  
  /* FIXME: We should also call the semeai module. */

  if (dragon[POS(i, j)].owl_status == UNCHECKED) {
    if (dragon[POS(i, j)].status == ALIVE)
      return gtp_success(id, "alive");
  
    if (dragon[POS(i, j)].status == DEAD)
      return gtp_success(id, "dead");
  
    if (dragon[POS(i, j)].status == UNKNOWN)
      return gtp_success(id, "unknown");

    assert(dragon[POS(i, j)].status == CRITICAL); /* Only remaining possibility. */
    return gtp_success(id, "critical");
  }

  /* Owl code active. */
  if (dragon[POS(i, j)].owl_status == ALIVE)
    return gtp_success(id, "alive");
  
  if (dragon[POS(i, j)].owl_status == DEAD)
    return gtp_success(id, "dead");
  
  if (dragon[POS(i, j)].owl_status == UNKNOWN)
    return gtp_success(id, "unknown");
  
  assert(dragon[POS(i, j)].owl_status == CRITICAL);
  /* Status critical, need to return attack and defense point as well. */
  gtp_printid(id, GTP_SUCCESS);
  gtp_printf("critical ");
  gtp_print_vertex(I(dragon[POS(i, j)].owl_attack_point),
		   J(dragon[POS(i, j)].owl_attack_point));
  gtp_printf(" ");
  gtp_print_vertex(I(dragon[POS(i, j)].owl_defense_point),
		   J(dragon[POS(i, j)].owl_defense_point));
  return gtp_finish_response();
}


/* Function:  Determine whether two stones belong to the same dragon.
 * Arguments: vertex, vertex
 * Fails:     invalid vertex, empty vertex
 * Returns:   1 if the vertices belong to the same dragon, 0 otherwise
 */

static int
gtp_same_dragon(char *s, int id)
{
  int ai, aj;
  int bi, bj;
  int n;

  n = gtp_decode_coord(s, &ai, &aj);
  if (n == 0)
    return gtp_failure(id, "invalid coordinate");

  if (!gtp_decode_coord(s + n, &bi, &bj))
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(ai, aj) == EMPTY || BOARD(bi, bj) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  silent_examine_position(BLACK, EXAMINE_DRAGONS_WITHOUT_OWL);
  
  return gtp_success(id, "%d", dragon[POS(ai, aj)].id == dragon[POS(bi, bj)].id);
}


/***********************
 * combination attacks *
 ***********************/

/* Function:  Find a move by color capturing something through a
 *            combination attack.
 * Arguments: color
 * Fails:     invalid color
 * Returns:   Recommended move, PASS if no move found
 */

static int
gtp_combination_attack(char *s, int id)
{
  int color;
  int attack_point;
  int n;

  n = gtp_decode_color(s, &color);
  if (!n)
    return gtp_failure(id, "invalid color");

  silent_examine_position(BLACK, EXAMINE_ALL);

  if (!atari_atari(color, &attack_point, NULL, verbose))
    attack_point = NO_MOVE;
  
  gtp_printid(id, GTP_SUCCESS);
  gtp_print_vertex(I(attack_point), J(attack_point));
  return gtp_finish_response();
}

/* Function:  Run atari_atari_confirm_safety().
 * Arguments: move, optional int
 * Fails:     invalid move
 * Returns:   success code, if failure also defending move
 */

static int
gtp_aa_confirm_safety(char *s, int id)
{
  int color;
  int i, j;
  int n;
  int minsize = 0;
  int result;
  int defense_point = NO_MOVE;
  int saved_dragons[BOARDMAX];
  int saved_worms[BOARDMAX];

  n = gtp_decode_move(s, &color, &i, &j);
  if (n == 0)
    return gtp_failure(id, "invalid color or coordinate");

  sscanf(s + n, "%d", &minsize);

  genmove(NULL, NULL, color);
  get_saved_dragons(POS(i, j), saved_dragons);
  get_saved_worms(POS(i, j), saved_worms);
  
  result = atari_atari_confirm_safety(color, POS(i, j),
				      &defense_point, minsize,
				      saved_dragons, saved_worms);
  
  gtp_printid(id, GTP_SUCCESS);
  gtp_mprintf("%d", result);
  if (result == 0)
    gtp_mprintf(" %m", I(defense_point), J(defense_point));
  
  return gtp_finish_response();
}


/********************
 * generating moves *
 ********************/

/* Function:  Generate and play the supposedly best black move.
 * Arguments: none
 * Fails:     never
 * Returns:   a move coordinate (or "PASS")
 */
static int
gtp_genmove_black(char *s, int id)
{
  int i, j;
  UNUSED(s);

  if (stackp > 0)
    return gtp_failure(id, "genmove cannot be called when stackp > 0");

  if (genmove(&i, &j, BLACK) >= 0)
    play_move(POS(i, j), BLACK);

  game_move[move_stack_pointer].i = i;
  game_move[move_stack_pointer].j = j;
  game_move[move_stack_pointer].color = BLACK;
  if (move_stack_pointer < MAX_MOVES)
    move_stack_pointer++;

  gtp_printid(id, GTP_SUCCESS);
  gtp_print_vertex(i, j);
  return gtp_finish_response();
}

/* Function:  Generate and play the supposedly best white move.
 * Arguments: none
 * Fails:     never
 * Returns:   a move coordinate (or "PASS")
 */
static int
gtp_genmove_white(char *s, int id)
{
  int i, j;
  UNUSED(s);

  if (stackp > 0)
    return gtp_failure(id, "genmove cannot be called when stackp > 0");

  if (genmove(&i, &j, WHITE) >= 0)
    play_move(POS(i, j), WHITE);

  game_move[move_stack_pointer].i = i;
  game_move[move_stack_pointer].j = j;
  game_move[move_stack_pointer].color = WHITE;
  if (move_stack_pointer < MAX_MOVES)
    move_stack_pointer++;

  gtp_printid(id, GTP_SUCCESS);
  gtp_print_vertex(i, j);
  return gtp_finish_response();
}

/* Function:  Generate the supposedly best move for either color.
 * Arguments: color to move, optionally a random seed
 * Fails:     invalid color
 * Returns:   a move coordinate (or "PASS")
 */
static int
gtp_genmove(char *s, int id)
{
  int i, j;
  int color;
  int n;
  unsigned int seed;

  n = gtp_decode_color(s, &color);
  if (!n)
    return gtp_failure(id, "invalid color");

  if (stackp > 0)
    return gtp_failure(id, "genmove cannot be called when stackp > 0");

  /* This is intended for regression purposes and should therefore be
   * deterministic. The best way to ensure this is to reset the random
   * number generator before calling genmove(). By default it is
   * seeded with 0, but if an optional unsigned integer is given in
   * the command after the color, this is used as seed instead.
   */
  seed = 0;
  sscanf(s+n, "%u", &seed);
  gg_srand(seed);
  
  genmove_conservative(&i, &j, color);
  gtp_printid(id, GTP_SUCCESS);
  gtp_print_vertex(i, j);
  return gtp_finish_response();
}

/* Function : Generate a list of the best moves in the previous genmove
 *            command (either of genmove_black, genmove_white, gg_genmove).
 *            If no previous genmove command has been issued, the result
 *            of this command will be meaningless.
 * Arguments: none
 * Fails:   : never
 * Returns  : list of moves with weights
 */

static int
gtp_top_moves(char *s, int id)
{
  int k;
  UNUSED(s);
  gtp_printid(id, GTP_SUCCESS);
  for (k = 0; k < 10; k++)
    if (best_move_values[k] > 0.0) {
      gtp_print_vertex(I(best_moves[k]), J(best_moves[k]));
      gtp_printf(" %.2f ", best_move_values[k]);
    }
  gtp_printf("\n\n");
  return GTP_OK;
}

/* Function : Generate a list of the best moves for White with weights
 * Arguments: none
 * Fails:   : never
 * Returns  : list of moves with weights
 */

static int
gtp_top_moves_white(char *s, int id)
{
  int i, j, k;
  UNUSED(s);
  genmove(&i, &j, WHITE);
  gtp_printid(id, GTP_SUCCESS);
  for (k = 0; k < 10; k++)
    if (best_move_values[k] > 0.0) {
      gtp_print_vertex(I(best_moves[k]), J(best_moves[k]));
      gtp_printf(" %.2f ", best_move_values[k]);
    }
  gtp_printf("\n\n");
  return GTP_OK;
}

static int
gtp_top_moves_black(char *s, int id)
{
  int i, j, k;
  UNUSED(s);
  genmove(&i, &j, BLACK);
  gtp_printid(id, GTP_SUCCESS);
  for (k = 0; k < 10; k++)
    if (best_move_values[k] > 0.0) {
      gtp_print_vertex(I(best_moves[k]), J(best_moves[k]));
      gtp_printf(" %.2f ", best_move_values[k]);
    }
  gtp_printf("\n\n");
  return GTP_OK;
}



/* Function:  Set the playing level.
 * Arguments: int
 * Fails:     incorrect argument
 * Returns:   nothing
 */
static int
gtp_set_level(char *s, int id)
{
  int new_level;
  if (sscanf(s, "%d", &new_level) < 1)
    return gtp_failure(id, "level not an integer");
  
  level = new_level;
  return gtp_success(id, "");
}

/* Function:  Undo last move
 * Arguments: int
 * Fails:     If move pointer is 0
 * Returns:   nothing
 */


static int
gtp_undo(char *s, int id)
{
  int k, nb_undo;
  
  nb_undo = 1;
  sscanf(s, "%d", &nb_undo);
  if (move_stack_pointer == 0) 
    return gtp_failure(id, "no moves to take back");
  if ((move_stack_pointer-nb_undo) < 0)
    return gtp_failure(id, "can't undo %d moves", nb_undo);
  if (move_stack_pointer > MAX_MOVES)
    return gtp_failure(id, "move stack overflow");
  restore_position(&starting_position);
  
  move_stack_pointer=move_stack_pointer-nb_undo;
  for (k = 0; k < move_stack_pointer; k++)
    play_move(POS(game_move[k].i, game_move[k].j), game_move[k].color);

  return gtp_success(id, "");
}
  

/***********
 * scoring *
 ***********/

static float final_score;
static int final_status[MAX_BOARD][MAX_BOARD];
static int status_numbers[6] = {ALIVE, DEAD, ALIVE_IN_SEKI, WHITE_TERRITORY,
				BLACK_TERRITORY, DAME};
static const char *status_names[6] = {"alive", "dead", "seki",
				      "white_territory", "black_territory",
				      "dame"};

/* Helper function. */
static void
finish_and_score_game(int seed)
{
  int move_val;
  int i, j;
  int next;
  int pass = 0;
  int moves = 0;
  int saved_board[MAX_BOARD][MAX_BOARD];
  Position saved_pos;
  static int current_board[MAX_BOARD][MAX_BOARD];
  static int current_seed = -1;
  int cached_board = 1;

  if (current_seed != seed) {
    current_seed = seed;
    cached_board = 0;
  }

  for (i = 0; i < board_size; i++)
    for (j = 0; j < board_size; j++)
      if (BOARD(i, j) != current_board[i][j]) {
	current_board[i][j] = BOARD(i, j);
	cached_board = 0;
      }

  /* If this is exactly the same position as the one we analyzed the
   * last time, the contents of final_score and final_status are up to date.
   */
  if (cached_board)
    return;

  doing_scoring = 1;
  store_position(&saved_pos);

  /* FIXME: Letting black always start is a preliminary solution. */
  next = BLACK;
  do {
    move_val = genmove_conservative(&i, &j, next);
    play_move(POS(i, j), next);
    if (move_val >= 0) {
      pass = 0;
      moves++;
    }
    else
      pass++;

    next = OTHER_COLOR(next);
  } while (pass < 2 && moves < board_size * board_size);

  final_score = aftermath_compute_score(next, komi);
  for (i = 0; i < board_size; i++)
    for (j = 0; j < board_size; j++) {
      final_status[i][j] = aftermath_final_status(next, POS(i, j));
      saved_board[i][j] = BOARD(i, j);
    }

  restore_position(&saved_pos);
  doing_scoring = 0;

  /* Update the status for vertices which were changed while finishing
   * the game, up to filling dame.
   */
  for (i = 0; i < board_size; i++)
    for (j = 0; j < board_size; j++) {
      if (BOARD(i, j) == saved_board[i][j])
	continue;

      if (BOARD(i, j) == EMPTY) {
	if (final_status[i][j] == ALIVE
	    || final_status[i][j] == ALIVE_IN_SEKI)
	  final_status[i][j] = DAME;
	else if (final_status[i][j] == DEAD) {
	  if (saved_board[i][j] == BLACK)
	    final_status[i][j] = WHITE_TERRITORY;
	  else
	    final_status[i][j] = BLACK_TERRITORY;
	}
      }
      else if (BOARD(i, j) == BLACK) {
	if (final_status[i][j] == WHITE_TERRITORY)
	  final_status[i][j] = DEAD;
	else if (final_status[i][j] == DAME)
	  final_status[i][j] = ALIVE_IN_SEKI;
	else if (final_status[i][j] == BLACK_TERRITORY)
	  final_status[i][j] = ALIVE;
	else
	  final_status[i][j] = DEAD;
      }
      else if (BOARD(i, j) == WHITE) {
	if (final_status[i][j] == BLACK_TERRITORY)
	  final_status[i][j] = DEAD;
	else if (final_status[i][j] == DAME)
	  final_status[i][j] = ALIVE_IN_SEKI;
	else if (final_status[i][j] == WHITE_TERRITORY)
	  final_status[i][j] = ALIVE;
	else
	  final_status[i][j] = DEAD;
      }
    }
}


/* Function:  Compute the score of a finished game.
 * Arguments: Optional random seed
 * Fails:     never
 * Returns:   Score in SGF format (RE property).
 */
static int
gtp_final_score(char *s, int id)
{
  int seed;
  /* This is intended for regression purposes and should therefore be
   * deterministic. The best way to ensure this is to reset the random
   * number generator before calling genmove(). By default it is
   * seeded with 0, but if an optional unsigned integer is given in
   * the command after the color, this is used as seed instead.
   */
  seed = 0;
  sscanf(s, "%u", &seed);
  gg_srand(seed);

  finish_and_score_game(seed);

  gtp_printid(id, GTP_SUCCESS);
  if (final_score > 0.0)
    gtp_printf("W+%3.1f", final_score);
  else if (final_score < 0.0)
    gtp_printf("B+%3.1f", -final_score);
  else
    gtp_printf("0");
  return gtp_finish_response();
}


/* Function:  Report the final status of a vertex in a finished game.
 * Arguments: Vertex, optional random seed
 * Fails:     invalid vertex
 * Returns:   Status in the form of one of the strings "alive", "dead",
 *            "seki", "white_territory", "black_territory", or "dame".
 */
static int
gtp_final_status(char *s, int id)
{
  int seed;
  int n;
  int ai, aj;
  int k;
  const char *result = NULL;

  n = gtp_decode_coord(s, &ai, &aj);
  if (n == 0)
    return gtp_failure(id, "invalid coordinate");

  /* This is intended for regression purposes and should therefore be
   * deterministic. The best way to ensure this is to reset the random
   * number generator before calling genmove(). By default it is
   * seeded with 0, but if an optional unsigned integer is given in
   * the command after the color, this is used as seed instead.
   */
  seed = 0;
  sscanf(s + n, "%u", &seed);
  gg_srand(seed);

  finish_and_score_game(seed);

  for (k = 0; k < 6; k++)
    if (final_status[ai][aj] == status_numbers[k]) {
      result = status_names[k];
      break;
    }
  assert(result != NULL);

  return gtp_success(id, result);
}


/* Function:  Report vertices with a specific final status in a finished game.
 * Arguments: Status in the form of one of the strings "alive", "dead",
 *            "seki", "white_territory", "black_territory", or "dame".
 *            An optional random seed can be added.
 * Fails:     missing or invalid status string
 * Returns:   Vertices having the specified status. These are split with
 *            one string on each line if the vertices are nonempty (i.e.
 *            for "alive", "dead", and "seki").
 */
static int
gtp_final_status_list(char *s, int id)
{
  int seed;
  int n;
  int i, j;
  int status = UNKNOWN;
  int k;
  char status_string[GTP_BUFSIZE];
  int first;

  if (sscanf(s, "%s %n", status_string, &n) != 1)
    return gtp_failure(id, "missing status");
  
  for (k = 0; k < 6; k++) {
    if (strcmp(status_string, status_names[k]) == 0)
      status = status_numbers[k];
  }

  if (status == UNKNOWN)
    return gtp_failure(id, "invalid status");

  /* This is intended for regression purposes and should therefore be
   * deterministic. The best way to ensure this is to reset the random
   * number generator before calling genmove(). By default it is
   * seeded with 0, but if an optional unsigned integer is given in
   * the command after the color, this is used as seed instead.
   */
  seed = 0;
  sscanf(s + n, "%u", &seed);
  gg_srand(seed);

  finish_and_score_game(seed);

  gtp_printid(id, GTP_SUCCESS);

  first = 1;
  for (i = 0; i < board_size; i++)
    for (j = 0; j < board_size; j++) {
      if (final_status[i][j] != status)
	continue;
      if (BOARD(i, j) == EMPTY) {
	if (!first)
	  gtp_printf(" ");
	else
	  first = 0;
	gtp_print_vertex(i, j);
      }
      else {
	int num_stones;
	int stones[MAX_BOARD * MAX_BOARD];
	if (find_origin(POS(i, j)) != POS(i, j))
	  continue;
	if (!first)
	  gtp_printf("\n");
	else
	  first = 0;
	num_stones = findstones(POS(i, j), board_size * board_size, stones);
	gtp_print_vertices2(num_stones, stones);
      }
    }

  return gtp_finish_response();
}


static int
gtp_estimate_score(char *s, int id)
{
  UNUSED(s);

  silent_examine_position(WHITE, EXAMINE_DRAGONS);
  
  score = estimate_score(&upper_bound, &lower_bound);
  gtp_printid(id, GTP_SUCCESS);
  /* Traditionally W wins jigo */
  if (score >= 0.0) 
    gtp_printf("W+%3.1f (upper bound: %3.1f, lower: %3.1f)", 
	       score, upper_bound, lower_bound);
  else if (score < 0.0)
    gtp_printf("B+%3.1f (upper bound: %3.1f, lower: %3.1f)", 
	       -score, upper_bound, lower_bound);
  return gtp_finish_response();
}  


/**************
 * statistics *
 **************/

/* Function:  Reset the count of life nodes.
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 */
static int
gtp_reset_life_node_counter(char *s, int id)
{
  UNUSED(s);
  reset_life_node_counter();
  return gtp_success(id, "");
}


/* Function:  Retrieve the count of life nodes.
 * Arguments: none
 * Fails:     never
 * Returns:   number of life nodes
 */
static int
gtp_get_life_node_counter(char *s, int id)
{
  int nodes = get_life_node_counter();
  UNUSED(s);
  return gtp_success(id, "%d", nodes);
}


/* Function:  Reset the count of owl nodes.
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 */
static int
gtp_reset_owl_node_counter(char *s, int id)
{
  UNUSED(s);
  reset_owl_node_counter();
  return gtp_success(id, "");
}


/* Function:  Retrieve the count of owl nodes.
 * Arguments: none
 * Fails:     never
 * Returns:   number of owl nodes
 */
static int
gtp_get_owl_node_counter(char *s, int id)
{
  int nodes = get_owl_node_counter();
  UNUSED(s);
  return gtp_success(id, "%d", nodes);
}


/* Function:  Reset the count of reading nodes.
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 */
static int
gtp_reset_reading_node_counter(char *s, int id)
{
  UNUSED(s);
  reset_reading_node_counter();
  return gtp_success(id, "");
}


/* Function:  Retrieve the count of reading nodes.
 * Arguments: none
 * Fails:     never
 * Returns:   number of reading nodes
 */
static int
gtp_get_reading_node_counter(char *s, int id)
{
  int nodes = get_reading_node_counter();
  UNUSED(s);
  return gtp_success(id, "%d", nodes);
}


/* Function:  Reset the count of trymoves/trykos.
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 */
static int
gtp_reset_trymove_counter(char *s, int id)
{
  UNUSED(s);
  reset_trymove_counter();
  return gtp_success(id, "");
}


/* Function:  Retrieve the count of trymoves/trykos.
 * Arguments: none
 * Fails:     never
 * Returns:   number of trymoves/trykos
 */
static int
gtp_get_trymove_counter(char *s, int id)
{
  int nodes = get_trymove_counter();
  UNUSED(s);
  return gtp_success(id, "%d", nodes);
}


/* Function:  Reset the count of connection nodes.
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 */
static int
gtp_reset_connection_node_counter(char *s, int id)
{
  UNUSED(s);
  reset_connection_node_counter();
  return gtp_success(id, "");
}


/* Function:  Retrieve the count of connection nodes.
 * Arguments: none
 * Fails:     never
 * Returns:   number of connection nodes
 */
static int
gtp_get_connection_node_counter(char *s, int id)
{
  int nodes = get_connection_node_counter();
  UNUSED(s);
  return gtp_success(id, "%d", nodes);
}



/*********
 * debug *
 *********/


/* Function:  Test an eyeshape for inconsistent evaluations
 * Arguments: Eyeshape vertices
 * Fails:     Bad vertices
 * Returns:   Failure reports on stderr.
 */
static int
gtp_test_eyeshape(char *s, int id)
{
  int n;
  int i, j;
  int eye_vertices[MAX_BOARD * MAX_BOARD];
  int eyesize = 0;

  n = gtp_decode_coord(s, &i, &j);
  while (n > 0) {
    eye_vertices[eyesize] = POS(i, j);
    eyesize++;
    s += n;
    n = gtp_decode_coord(s, &i, &j);
  }
  
  if (eyesize == 0)
    return gtp_failure(id, "invalid coordinate");

  test_eyeshape(eyesize, eye_vertices);

  return gtp_success(id, "");
}



/* Function:  Returns elapsed CPU time in seconds.
 * Arguments: none
 * Fails:     never
 * Returns:   Total elapsed (user + system) CPU time in seconds.
 */
static int
gtp_cputime(char *s, int id)
{
  UNUSED(s);
  return gtp_success(id, "%.3f", gg_cputime());
}



/* Function:  Write the position to stderr.
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 */
static int
gtp_showboard(char *s, int id)
{
  UNUSED(s);
  showboard(0);
  return gtp_success(id, "");
}


/* Function:  Dump stack to stderr.
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 */
static int
gtp_dump_stack(char *s, int id)
{
  UNUSED(s);
  dump_stack();
  return gtp_success(id, "");
}


/* Function:  Return information about the influence function.
 * Arguments: color to move
 * Fails:     never
 * Returns:   Influence data formatted like:
 *
 * white:
 *   0.51   1.34   3.20   6.60   9.09   8.06   1.96   0.00   0.00 
 *   0.45   1.65   4.92  12.19  17.47  15.92   4.03   0.00   0.00 
 *                   .
 *                   .
 *                   .
 *   0.00   0.00   0.00   0.00   0.00 100.00  75.53  41.47  23.41
 * black:
 *   1.57   2.51   4.10   3.10   3.60   4.54   8.32   4.15   2.71 
 *   2.96   4.62   9.18   5.47   5.89  10.88  20.54  10.19   4.08 
 *                   .
 *                   .
 *                   .
 * 100.00 139.39 100.00 139.39 100.00   0.00   0.00   0.00   0.00
 * regions:
 * -1  0  0  1  1  0 -1 -3 -3
 *              .
 *              .
 *              .
 * -3 -3 -3 -3 -3  3  3  3  3
 *
 * The encoding of the regions is as follows:
 *  3 white territory
 *  2 white moyo
 *  1 white area
 *  0 neutral
 * -1 black area
 * -2 black moyo
 * -3 black territory
 */
static int
gtp_influence(char *s, int id)
{
  int color;
  float white_influence[BOARDMAX];
  float black_influence[BOARDMAX];
  int influence_regions[BOARDMAX];
  
  if (!gtp_decode_color(s, &color))
    return gtp_failure(id, "invalid color");

  silent_examine_position(color, EXAMINE_ALL);

  gtp_printid(id, GTP_SUCCESS);
  get_initial_influence(color, 1, white_influence,
			black_influence, influence_regions);
  print_influence(white_influence, black_influence, influence_regions);
  /* We already have one newline and thus can't use gtp_finish_response(). */
  gtp_printf("\n");
  return GTP_OK;
}

static void
print_influence(float white_influence[BOARDMAX],
		float black_influence[BOARDMAX],
		int influence_regions[BOARDMAX])
{
  int m, n;
  gtp_printf("white:\n");
  for (m = 0; m < board_size; m++) {
    for (n = 0; n < board_size; n++) {
      gtp_printf("%6.2f ", white_influence[POS(m, n)]);
    }
    gtp_printf("\n");
  }

  gtp_printf("black:\n");
  for (m = 0; m < board_size; m++) {
    for (n = 0; n < board_size; n++) {
      gtp_printf("%6.2f ", black_influence[POS(m, n)]);
    }
    gtp_printf("\n");
  }

  gtp_printf("regions:\n");
  for (m = 0; m < board_size; m++) {
    for (n = 0; n < board_size; n++) {
      gtp_printf("%2d ", influence_regions[POS(m, n)]);
    }
    gtp_printf("\n");
  }
}


/* Function:  Return the information in the worm data structure.
 * Arguments: optional vertex
 * Fails:     never
 * Returns:   Worm data formatted like:
 *
 * A19:
 * color           black
 * size            10
 * effective_size  17.83
 * origin          A19
 * liberties       8
 * liberties2      15
 * liberties3      10
 * liberties4      8
 * attack          PASS
 * attack_code     0
 * lunch           B19
 * defend          PASS
 * defend_code     0
 * cutstone        2
 * cutstone2       0
 * genus           0
 * inessential     0
 * B19:
 * color           white
 * .
 * .
 * .
 * inessential     0
 * C19:
 * ...
 *
 * If an intersection is specified, only data for this one will be returned.
 */
static int
gtp_worm_data(char *s, int id)
{
  int i = -1;
  int j = -1;
  int m, n;

  if (sscanf(s, "%*c") >= 0 && !gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid color or coordinate");

  examine_position(EMPTY, EXAMINE_WORMS);

  gtp_printid(id, GTP_SUCCESS);
  
  for (m = 0; m < board_size; m++)
    for (n = 0; n < board_size; n++)
      if (i == -1 || (m == i && n == j)) {
	struct worm_data *w = &worm[POS(m, n)];
	gtp_print_vertex(m, n);
	gtp_printf(":\n");
	gtp_mprintf("color           %C\n",  w->color);
	gtp_printf("size            %d\n",   w->size);
	gtp_printf("effective_size  %.2f\n", w->effective_size);
	gtp_mprintf("origin          %m\n", 
		    I(w->origin), J(w->origin));
	gtp_printf("liberties       %d\n",   w->liberties);
	gtp_printf("liberties2      %d\n",   w->liberties2);
	gtp_printf("liberties3      %d\n",   w->liberties3);
	gtp_printf("liberties4      %d\n",   w->liberties4);
	gtp_mprintf("attack_point    %m\n",  
		    I(w->attack_points[0]), J(w->attack_points[0]));
	gtp_printf("attack_code     %d\n",   w->attack_codes[0]);
	gtp_mprintf("lunch           %m\n",  
		    I(w->lunch), J(w->lunch));
	gtp_mprintf("defense_point   %m\n",  
		    I(w->defense_points[0]), J(w->defense_points[0]));
	gtp_printf("defend_code     %d\n",   w->defend_codes[0]);
	gtp_printf("cutstone        %d\n",   w->cutstone);
	gtp_printf("cutstone2       %d\n",   w->cutstone2);
	gtp_printf("genus           %d\n",   w->genus);
	gtp_printf("inessential     %d\n",   w->inessential);
      }
  
  gtp_printf("\n");
  return GTP_OK;
}

/* Function:  List the stones of a worm
 * Arguments: the location, "BLACK" or "WHITE"
 * Fails:     if called on an empty or off-board location
 * Returns:   list of stones
 */
static int
gtp_worm_stones(char *s, int id)
{
  int i = -1;
  int j = -1;
  int color = EMPTY;
  int m, n;
  int u, v;

  if (sscanf(s, "%*c") >= 0) {
    if (!gtp_decode_coord(s, &i, &j)
	&& !gtp_decode_color(s, &color))
      return gtp_failure(id, "invalid coordinate");
  }
    
  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "worm_stones called on an empty vertex");

  gtp_printid(id, GTP_SUCCESS);
  
  for (u = 0; u < board_size; u++)
    for (v = 0; v < board_size; v++) {
      if (BOARD(u, v) == EMPTY
	  || (color != EMPTY && BOARD(u, v) != color))
	continue;
      if (find_origin(POS(u, v)) != POS(u, v))
	continue;
      if (ON_BOARD2(i, j) 
	  && !same_string(POS(u, v), POS(i, j)))
	continue;
      for (m = 0; m < board_size; m++)
	for (n = 0; n < board_size; n++)
	  if (BOARD(m, n) != EMPTY
	      && same_string(POS(m, n), POS(u, v)))
	    gtp_mprintf("%m ", m, n);
      gtp_printf("\n");
    }
  
  gtp_printf("\n");
  return GTP_OK;
}



/* Function:  Return the cutstone field in the worm data structure.
 * Arguments: non-empty vertex
 * Fails:     never
 * Returns:   cutstone
 */
static int
gtp_worm_cutstone(char *s, int id)
{

  int i, j;
  if (!gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");

  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "vertex must not be empty");

  examine_position(EMPTY, EXAMINE_WORMS);

  return gtp_success(id, " %d", worm[POS(i, j)].cutstone);
}

/* Function:  Return the information in the dragon data structure.
 * Arguments: optional intersection
 * Fails:     never
 * Returns:   Dragon data formatted in the corresponding way to gtp_worm_data.
 */
static int
gtp_dragon_data(char *s, int id)
{
  int i = -1;
  int j = -1;
  int m, n;

  if (sscanf(s, "%*c") >= 0 && !gtp_decode_coord(s, &i, &j))
    return gtp_failure(id, "invalid coordinate");

  if (stackp > 0)
    return gtp_failure(id, "dragon data unavailable when stackp > 0");

  examine_position(EMPTY, EXAMINE_DRAGONS);

  gtp_printid(id, GTP_SUCCESS);

  if (ON_BOARD2(i, j) && BOARD(i, j) == EMPTY)
    gtp_mprintf("%m empty\n", i, j);
  else {
    for (m = 0; m < board_size; m++)
      for (n = 0; n < board_size; n++)
	if ((m == i && n == j)
	    || (i == -1
		&& BOARD(m, n) != EMPTY
		&& dragon[POS(m, n)].origin == POS(m, n))) {
	  int k, ti, tj;
	  struct dragon_data *d = &dragon[POS(m, n)];
	  struct dragon_data2 *d2 = &(dragon2[d->id]);
	  gtp_print_vertex(m, n);
	  gtp_printf(":\n");
	  gtp_printf("color                   %s\n",   
		     color_to_string(d->color));
	  gtp_mprintf("origin                  %m\n", 
		      I(d->origin), J(d->origin));
	  gtp_printf("size                    %d\n",   d->size);
	  gtp_printf("effective_size          %.2f\n", d->effective_size);
	  gtp_printf("heyes                   %d\n",   d2->heyes);
	  gtp_mprintf("heye                    %m\n", 
		      I(d2->heye), J(d2->heye));
	  gtp_printf("genus                   %d\n",   d2->genus);
	  gtp_printf("escape_route            %d\n",   d2->escape_route);
	  /* We would like to use the %1m format used in gprintf
	   * but this is not implemented in gtp_mprintf. Moreover adding this
	   * format would entail including liberty.h in gtp.c, which we
	   * don't want to do in order to maintain independence of the
	   * GNU Go internals. Therefore we use the I and J macros here.
	   */
	  gtp_mprintf("lunch                   %m\n", 
		      I(d2->lunch), J(d2->lunch));
	  gtp_printf("status                  %s\n",   
		     status_to_string(d->status));
	  gtp_printf("owl_status              %s\n",   
		     status_to_string(d->owl_status));
	  gtp_printf("matcher_status          %s\n",   
		     status_to_string(d->matcher_status));
	  gtp_printf("owl_threat_status       %s\n",   
		     status_to_string(d->owl_threat_status));
	  gtp_mprintf("owl_attack              %m\n", 
		      I(d->owl_attack_point), J(d->owl_attack_point));
	  gtp_printf("owl_attack_certain:     %s\n",   
		     (d->owl_attack_certain) ? "YES" : "NO");
	  gtp_mprintf("owl_2nd_attack          %m\n", 
		      I(d->owl_second_attack_point), 
		      J(d->owl_second_attack_point));
	  gtp_mprintf("owl_defend              %m\n", 
		      I(d->owl_defense_point), J(d->owl_defense_point));
	  gtp_printf("owl_defend_certain:     %s\n",   
		     (d->owl_defend_certain) ? "YES" : "NO");
	  gtp_mprintf("owl_2nd_defend          %m\n", 
		      I(d->owl_second_defense_point), 
		      J(d->owl_second_defense_point));
	  gtp_printf("semeai                  %d\n",   
		     d2->semeai);
	  gtp_printf("semeai_margin_of_safety %d\n",   
		     d2->semeai_margin_of_safety);
	  gtp_printf("neighbors:              ");
	  for (k = 0; k < DRAGON2(POS(m, n)).neighbors; k++)
	    gtp_mprintf("%m ", 
			I(DRAGON(DRAGON2(POS(m, n)).adjacent[k]).origin),
			J(DRAGON(DRAGON2(POS(m, n)).adjacent[k]).origin));
	  gtp_printf("\n");
	  gtp_printf("moyo:                   %d\n", DRAGON2(POS(m, n)).moyo);
	  gtp_printf("safety:                 %s\n", 
		     safety_to_string(DRAGON2(POS(m, n)).safety));
	  gtp_printf("strings: ");
	  for (ti = 0; ti < board_size; ti++)
	    for (tj = 0; tj < board_size; tj++)
	      if (worm[POS(ti, tj)].origin == POS(ti, tj)
		  && dragon[POS(ti, tj)].origin == dragon[POS(m, n)].origin)
		gtp_mprintf("%m ", ti, tj);
	  gtp_printf("\n");
	}
  }
  gtp_printf("\n");
  return GTP_OK;
}


/* Function:  List the stones of a dragon
 * Arguments: the location
 * Fails:     if called on an empty or off-board location
 * Returns:   list of stones
 */
static int
gtp_dragon_stones(char *s, int id)
{
  int i = -1;
  int j = -1;
  int color = EMPTY;
  int m, n;
  int u, v;

  if (sscanf(s, "%*c") >= 0) {
    if (!gtp_decode_coord(s, &i, &j)
	&& !gtp_decode_color(s, &color))
    return gtp_failure(id, "invalid coordinate");
  }

  if (BOARD(i, j) == EMPTY)
    return gtp_failure(id, "dragon_stones called on an empty vertex");

  examine_position(EMPTY, EXAMINE_DRAGONS);

  gtp_printid(id, GTP_SUCCESS);

  
  for (u = 0; u < board_size; u++)
    for (v = 0; v < board_size; v++) {
      if (BOARD(u, v) == EMPTY
	  || (color != EMPTY && BOARD(u, v) != color))
	continue;
      if (dragon[POS(u, v)].origin != POS(u, v))
	continue;
      if (ON_BOARD2(i, j) && dragon[POS(i, j)].origin != POS(u, v))
	continue;
      for (m = 0; m < board_size; m++)
	for (n = 0; n < board_size; n++)
	  if (dragon[POS(m, n)].origin == POS(u, v))
	    gtp_mprintf("%m ", m, n);
      gtp_printf("\n");
    }
  
  gtp_printf("\n");
  return GTP_OK;
}


static SGFTree gtp_sgftree;

/* Function:  Start storing moves executed during reading in an sgf
 *            tree in memory. 
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 *
 * Warning: You had better know what you're doing if you try to use this
 *          command.
 */
static int
gtp_start_sgftrace(char *s, int id)
{
  UNUSED(s);
  begin_sgftreedump(&gtp_sgftree);
  count_variations = 1;
  return gtp_success(id, "");
}


/* Function:  Finish storing moves in an sgf tree and write it to file. 
 * Arguments: filename
 * Fails:     never
 * Returns:   nothing
 *
 * Warning: You had better know what you're doing if you try to use this
 *          command.
 */
static int
gtp_finish_sgftrace(char *s, int id)
{
  char filename[GTP_BUFSIZE];
  int nread;
  
  nread = sscanf(s, "%s", filename);
  if (nread < 1)
    return gtp_failure(id, "missing filename");

  end_sgftreedump(filename);
  count_variations = 0;
  return gtp_success(id, "");
}


/* Function:  Tune the parameters for the move ordering in the tactical
 *            reading.
 * Arguments: MOVE_ORDERING_PARAMETERS integers
 * Fails:     incorrect arguments
 * Returns:   nothing
 */
static int
gtp_tune_move_ordering(char *s, int id)
{
  int params[MOVE_ORDERING_PARAMETERS];
  int k;
  int p;
  int n;

  for (k = 0; k < MOVE_ORDERING_PARAMETERS; k++) {
    if (sscanf(s, "%d%n", &p, &n) == 0)
      return gtp_failure(id, "incorrect arguments, expected %d integers",
			 MOVE_ORDERING_PARAMETERS);
    params[k] = p;
    s += n;
  }

  tune_move_ordering(params);
  return gtp_success(id, "");
}

/* Function:  Echo the parameter
 * Arguments: string
 * Fails:     never
 * Returns:   nothing
 */
static int
gtp_echo(char *s, int id)
{
  return gtp_success(id, "%s", s);
}


/* Function:  Echo the parameter to stdout AND stderr
 * Arguments: string
 * Fails:     never
 * Returns:   nothing
 */
static int
gtp_echo_err(char *s, int id)
{
  fprintf(stderr, "%s", s);
  fflush(stdout);
  fflush(stderr);
  return gtp_success(id, "%s", s);
}

/* Function:  List all known commands
 * Arguments: none
 * Fails:     never
 * Returns:   list of known commands, one per line
 */
static int
gtp_help(char *s, int id)
{
  int k;
  UNUSED(s);

  gtp_printid(id, GTP_SUCCESS);

  for (k = 0; commands[k].name != NULL; k++)
    gtp_printf("%s\n", commands[k].name);

  gtp_printf("\n");
  return GTP_OK;
}


/* Function:  Turn uncertainty reports from owl_attack
 *            and owl_defend on or off.
 * Arguments: "on" or "off"
 * Fails:     invalid argument
 * Returns:   nothing
 */
static int
gtp_report_uncertainty(char *s, int id)
{
  if (!strncmp(s, "on", 2)) {
    report_uncertainty = 1;
    return gtp_success(id, "");
  }
  if (!strncmp(s, "off", 3)) {
    report_uncertainty = 0;
    return gtp_success(id, "");
  }
  return gtp_failure(id, "invalid argument");
}
    

static void
gtp_print_code(int c)
{
  gtp_printf("%d", c ? 4-c : c);
}

static void
gtp_print_vertices2(int n, int *moves)
{
  int movei[MAX_BOARD * MAX_BOARD];
  int movej[MAX_BOARD * MAX_BOARD];
  int k;

  for (k = 0; k < n; k++) {
    movei[k] = I(moves[k]);
    movej[k] = J(moves[k]);
  }
  
  gtp_print_vertices(n, movei, movej);
}

static void rotate_on_input(int ai, int aj, int *bi, int *bj)
{
  rotate(ai, aj, bi, bj, board_size, gtp_orientation);
}

static void rotate_on_output(int ai, int aj, int *bi, int *bj)
{
  inv_rotate(ai, aj, bi, bj, board_size, gtp_orientation);
}

/*
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */
