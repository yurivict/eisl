#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "eisl.h"
#include "nana.h"
#include "mem.h"
#include "except.h"

#define DBG_PRINTF(msg,arg)     if(gbc_flag) printf(msg,arg)

// ---------garbage collection-----------
DEF_PREDICATE (EMPTY, EMP)
     int gbc (void)
{
  if (gc_sw == 0)
    {
      int addr;

      DBG_PRINTF ("enter M&S-GC free=%d\n", fc);
      gbcmark ();
      gbcsweep ();
      fc = 0;
      for (addr = 0; addr < CELLSIZE; addr++)
	if (IS_EMPTY (addr))
	  fc++;
      DBG_PRINTF ("exit  M&S-GC free=%d\n", fc);
    }
  else
    {
      if (area_sw == 1)
	{
	  DBG_PRINTF ("enter COPY-GC free=%d\n", WORK2 - wp);
	}
      else
	{
	  DBG_PRINTF ("enter COPY-GC free=%d\n", CELLSIZE - wp);
	}
      copygbc ();
      if (area_sw == 1)
	{
	  DBG_PRINTF ("exit  COPY-GC free=%d\n", WORK2 - wp);
	}
      else
	{
	  DBG_PRINTF ("exit  COPY-GC free=%d\n", CELLSIZE - wp);
	}
    }
  return 0;
}


static inline void
MARK_CELL (int addr)
{
  heap[addr].flag = USE;
}

static inline bool
USED_CELL (int addr)
{
  return (heap[addr].flag == USE);
}

void
markcell (int addr)
{
  int i, m, n, x;

  if (addr < 0 || addr >= CELLSIZE)
    return;

  if (USED_CELL (addr))
    return;

  MARK_CELL (addr);
  switch (GET_TAG (addr))
    {
    case EMP:
    case INTN:
    case FLTN:
    case LONGN:
    case CHR:
    case STR:
    case STREAM:
    case BIGX:
      return;
    case VEC:
      n = vector_length (addr);
      for (i = 0; i < n; i++)
	{
	  x = GET_VEC_ELT (addr, i);
	  markcell (x);
	}
      return;

    case ARR:
      m = array_length (addr);
      n = 1;
      while (!nullp (m))
	{
	  n = n * GET_INT (car (m));
	  m = cdr (m);
	}
      for (i = 0; i < n; i++)
	{
	  x = GET_VEC_ELT (addr, i);
	  markcell (x);
	}
      markcell (cdr (addr));	// dimension
      return;

    case SYM:
      markcell (car (addr));
      markcell (cdr (addr));
      markcell (GET_AUX (addr));
      markcell (GET_PROP (addr));
      return;
    case FUNC:
      markcell (car (addr));
      markcell (cdr (addr));
      markcell (GET_AUX (addr));
      markcell (GET_PROP (addr));
      return;
    case MACRO:
    case GENERIC:
    case METHOD:
    case CLASS:
    case INSTANCE:
    case LIS:
      markcell (car (addr));
      markcell (cdr (addr));
      markcell (GET_AUX (addr));
      return;
    case SUBR:
    case FSUBR:
      markcell (GET_AUX (addr));
      return;
    default:
      IP (false, "markcell tag switch default action");
    }
}

void
gbcmark (void)
{
  int i;

  // mark nil and t
  MARK_CELL (NIL);
  MARK_CELL (T);
  // mark local environment
  markcell (ep);
  // mark dynamic environment
  markcell (dp);
  // mark stack
  for (i = 0; i < sp; i++)
    markcell (stack[i]);
  // mark cell binded by argstack
  for (i = 0; i < ap; i++)
    markcell (argstk[i]);

  // mark cell chained from hash table
  for (i = 0; i < HASHTBSIZE; i++)
    markcell (cell_hash_table[i]);

  // mark tagbody symbol
  markcell (tagbody_tag);

  // mark thunk for unwind-protect
  for (i = 0; i < unwind_pt; i++)
    markcell (unwind_buf[i]);

  // mark error_handler
  markcell (error_handler);

  // mark stream
  markcell (standard_input);
  markcell (standard_output);
  markcell (standard_error);
  markcell (input_stream);
  markcell (output_stream);
  markcell (error_stream);

  // mark shelter
  for (i = 0; i < lp; i++)
    markcell (shelter[i]);

  // mark dynamic environment
  for (i = 1; i <= dp; i++)
    markcell (dynamic[i][1]);


  // mark generic_list
  markcell (generic_list);

  // mark symbol list for catch
  markcell (catch_symbols);

}

static inline void
NOMARK_CELL (int addr)
{
  heap[addr].flag = FRE;
}

void
gbcsweep (void)
{
  int addr;

  addr = 0;
  while (addr < CELLSIZE)
    {
      if (USED_CELL (addr))
	NOMARK_CELL (addr);
      else
	{
	  clrcell (addr);
	  SET_CDR (addr, hp);
	  hp = addr;
	}
      addr++;
    }
}

void
clrcell (int addr)
{
  if (IS_VECTOR (addr) || IS_ARRAY (addr))
    FREE (heap[addr].val.car.dyna_vec);


  SET_TAG (addr, EMP);
  FREE (heap[addr].name);
  SET_CAR (addr, 0);
  SET_CDR (addr, 0);
  SET_AUX (addr, 0);
  SET_PROP (addr, 0);
  SET_OPT (addr, 0);
  SET_TR (addr, 0);
}

// when free cells are less FREESIZE, invoke gbc()
int
checkgbc (void)
{
  if (exit_flag)
    {
      exit_flag = 0;
      RAISE (Restart_Repl);
    }
  if (gc_sw == 0 && fc < FREESIZE)
    (void) gbc ();
  else if (gc_sw == 1 && wp < WORK2 && wp > WORK2 - FREESIZE)
    (void) gbc ();
  else if (gc_sw == 1 && wp > WORK2 && wp > CELLSIZE - FREESIZE)
    (void) gbc ();
  return 0;
}



int
freecell (void)
{
  return (fc);
}

int
gbcsw (void)
{
  return (gc_sw);
}

int
getwp (void)
{
  return (wp);
}


void
copygbc (void)
{
  int i;

  if (area_sw == 1)
    {
      area_sw = 2;
      wp = WORK2;
    }
  else
    {
      area_sw = 1;
      wp = WORK1;
    }

  // copy local environment
  ep = copy_work (ep);
  // copy dynamic environment
  dp = copy_work (dp);
  // copy stack
  for (i = 0; i < sp; i++)
    stack[i] = copy_work (stack[i]);
  // copy cell binded by argstack
  for (i = 0; i < ap; i++)
    argstk[i] = copy_work (argstk[i]);

  // copy tagbody symbol
  tagbody_tag = copy_work (tagbody_tag);

  // copy thunk for unwind-protect
  unwind_pt = copy_work (unwind_pt);


  // copy shelter
  for (i = 0; i < lp; i++)
    shelter[i] = copy_work (shelter[i]);

  // copy generic_list
  generic_list = copy_work (generic_list);

  // copy symbol list for catch
  catch_symbols = copy_work (catch_symbols);

  // copy cell chained from hash table
  for (i = 0; i < HASHTBSIZE; i++)
    copy_hash (cell_hash_table[i]);


}
