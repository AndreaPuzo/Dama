#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#if _WIN32
# define TERM_ANSI 0
#else
# define TERM_ANSI 1
#endif

#define N_ROWS   8
#define N_COLS   8
#define SUCCESS  0
#define FAILURE  1
#define PL_BLANK '#'
#define PL_BLACK 'B'
#define PL_WHITE 'W'

typedef char     chr_t ;
typedef uint8_t  u8_t  ;
typedef uint16_t u16_t ;
typedef uint32_t u32_t ;
typedef int8_t   i8_t  ;
typedef int16_t  i16_t ;
typedef int32_t  i32_t ;

typedef struct dama_s dama_t ;

struct dama_s {
  u32_t row                     ;
  u32_t col                     ;
  chr_t tab [ N_ROWS * N_COLS ] ;
} ;

int dama_vprintf (dama_t * dama, const chr_t * fmt, va_list ap) ;
int dama_printf (dama_t * dama, const chr_t * fmt, ...) ;
void dama_setchr (dama_t * dama, u32_t row, u32_t col, u32_t chr) ;
int dama_getchr (dama_t * dama, u32_t row, u32_t col) ;
int dama_verify (dama_t * dama, u32_t row, u32_t col) ;
void dama_rand (dama_t * dama, u32_t n, u32_t pl) ;
void dama_draw (dama_t * dama, FILE * fp) ;
void dama_calc (dama_t * dama, u32_t pl) ;

int main (int argc, char ** argv)
{
  if (0 == argc)
    abort() ;

  if (2 == argc && 0 == strcmp(argv[1], "-h")) {
    fprintf(stderr, "usage: %s [-B<n>|-W<n>]\n", argv[0]) ;
    exit(EXIT_SUCCESS) ;
  }

  srand(time(NULL)) ;

  u32_t Bn, Wn ;
  dama_t dama ;

  Bn = Wn = 5 ;

  for (int i = 1 ; i < argc ; ++i) {
    if (0 == strncmp(argv[i], "-B", 2)) {
      Bn = strtoul(argv[i] + 2, NULL, 10) ;
    } else if (0 == strncmp(argv[i], "-W", 2)) {
      Wn = strtoul(argv[i] + 2, NULL, 10) ;
    }
  }

  if (N_ROWS * N_COLS < Bn + Wn) {
    fprintf(stderr, "panic: out of chessboard.\n") ;
    exit(EXIT_FAILURE) ;
  }

  memset(dama.tab, PL_BLANK, sizeof(dama.tab)) ;
  dama_rand(&dama, Bn, PL_WHITE) ;
  dama_rand(&dama, Wn, PL_BLACK) ;
  dama_draw(&dama, stdout) ;
  dama_calc(&dama, PL_WHITE) ;
  dama_calc(&dama, PL_BLACK) ;

  exit(EXIT_SUCCESS) ;
}

int dama_vprintf (dama_t * dama, const chr_t * fmt, va_list ap)
{
  int n = 0 ;

  n += fprintf(stderr, "(%u,%c) ", N_ROWS - dama->row, 'A' + dama->col) ;
  n += vfprintf(stderr, fmt, ap) ;

  return n ;
}

int dama_printf (dama_t * dama, const chr_t * fmt, ...)
{
  int n ;

  va_list ap ;
  va_start(ap, fmt) ;
  n = dama_vprintf(dama, fmt, ap) ;
  va_end(ap) ;

  return n ;
}

void dama_setchr (dama_t * dama, u32_t row, u32_t col, u32_t chr)
{
  dama->row = row ;
  dama->col = col ;

  if (N_ROWS <= row) {
    dama_printf(dama, "error: out of rows.\n") ;
    exit(EXIT_FAILURE) ;
  }

  if (N_COLS <= col) {
    dama_printf(dama, "error: out of cols.\n") ;
    exit(EXIT_FAILURE) ;
  }

  dama->tab[row * N_COLS + col] = chr ;
}

int dama_getchr (dama_t * dama, u32_t row, u32_t col)
{
  dama->row = row ;
  dama->col = col ;

  if (N_ROWS <= row) {
    dama_printf(dama, "error: out of rows.\n") ;
    exit(EXIT_FAILURE) ;
  }

  if (N_COLS <= col) {
    dama_printf(dama, "error: out of cols.\n") ;
    exit(EXIT_FAILURE) ;
  }

  return dama->tab[row * N_COLS + col] ;
}

int dama_verify (dama_t * dama, u32_t row, u32_t col)
{
  if (N_ROWS <= row || N_COLS <= col)
    return FAILURE ;

  return SUCCESS ;
}

void dama_rand (dama_t * dama, u32_t n, u32_t pl)
{
  u32_t i, j ;

  while (0 < n) {
    i = (u32_t)rand() % N_ROWS ;
    j = (u32_t)rand() % N_COLS ;

    //     0 1 2 3 4 5 6 7
    // 0 - 1 0 1 0 1 0 1 0
    // 1 - 0 1 0 1 0 1 0 1
    // 2 - 1 0 1 0 1 0 1 0
    // 3 - 0 1 0 1 0 1 0 1
    // 4 - 1 0 1 0 1 0 1 0
    // 5 - 0 1 0 1 0 1 0 1
    // 6 - 1 0 1 0 1 0 1 0
    // 7 - 0 1 0 1 0 1 0 1

    if (
      (  ((i * N_COLS + j) & 1) == (i & 1)  ) &&
      ( PL_BLANK == dama_getchr(dama, i, j) )
    ) {
      dama_setchr(dama, i, j, pl) ;
      --n ;
    }
  }
}

void dama_draw (dama_t * dama, FILE * fp)
{
  u32_t i, j ;

  fputc('\n', fp) ;
#if TERM_ANSI
  fprintf(fp, "\x1B[96m") ;
#endif
  fprintf(fp, "     ") ;
  for (j = 0 ; j < N_COLS ; ++j) {
    fprintf(fp, "%c ", 'A' + j) ;
  }
  fputc('\n', fp) ;

  fprintf(fp, "   .-") ;
  for (j = 0 ; j < 2 * N_COLS - 1 ; ++j) {
    fputc('-', fp) ;
  }
  fprintf(fp, "-.\n") ;

  for (i = 0 ; i < N_ROWS ; ++i) {
    fprintf(fp, " %u | ", N_ROWS - i) ;

    for (j = 0 ; j < N_COLS ; ++j) {
      int chr = dama_getchr(dama, i, j) ;

      if (((i * N_COLS + j) & 1) == (i & 1)) {
#if TERM_ANSI
        fprintf(fp, "\x1B[37m%c\x1B[96m", chr) ;
#else
        fprintf(fp, "%c", chr) ;
#endif
      } else {
#if TERM_ANSI
        fprintf(fp, "\x1B[90m%c\x1B[96m", chr) ;
#else
        fprintf(fp, "%c", chr) ;
#endif
      }

      fputc(' ', fp) ;
    }

    fprintf(fp, "|\n") ;
  }

  fprintf(fp, "   '-") ;
  for (j = 0 ; j < 2 * N_COLS - 1 ; ++j) {
    fputc('-', fp) ;
  }
  fprintf(fp, "-'\n") ;
#if TERM_ANSI
  fprintf(fp, "\x1B[0m") ;
#endif
  fputc('\n', fp) ;
}

void dama_calc (dama_t * dama, u32_t pl)
{
  u32_t i, j ;

  for (i = 0 ; i < N_ROWS ; ++i) {
    for (j = 0 ; j < N_COLS ; ++j) {
      if (dama_getchr(dama, i, j) == pl) {
        fprintf(stderr, "(%u:%c) `%c`:\n", N_ROWS - i, 'A' + j, pl) ;

        int chr ;

        // Note: change this condition to change the direction
        if (PL_BLACK == pl) {
          if (
            SUCCESS == dama_verify(dama, i - 1, j - 1) &&
            SUCCESS == dama_verify(dama, i - 2, j - 2)
          ) {
            chr = dama_getchr(dama, i - 1, j - 1) ;

            if (pl != chr && PL_BLANK != chr) {
              dama->row = i ;
              dama->col = j ;

              dama_printf(dama,
                "`%c` can eat `%c` at (%u,%c)\n",
                pl, chr, N_ROWS - (i - 1), 'A' + (j - 1)
              ) ;
            }
          }

          if (
            SUCCESS == dama_verify(dama, i - 1, j + 1) &&
            SUCCESS == dama_verify(dama, i - 2, j + 2)
          ) {
            chr = dama_getchr(dama, i - 1, j + 1) ;

            if (pl != chr && PL_BLANK != chr) {
              dama->row = i ;
              dama->col = j ;

              dama_printf(dama,
                "`%c` can eat `%c` at (%u,%c)\n",
                pl, chr, N_ROWS - (i - 1), 'A' + (j + 1)
              ) ;
            }
          }
        } else {
          if (
            SUCCESS == dama_verify(dama, i + 1, j - 1) &&
            SUCCESS == dama_verify(dama, i + 2, j - 2)
          ) {
            chr = dama_getchr(dama, i + 1, j - 1) ;

            if (pl != chr && PL_BLANK != chr) {
              dama->row = i ;
              dama->col = j ;

              dama_printf(dama,
                "`%c` can eat `%c` at (%u,%c)\n",
                pl, chr, N_ROWS - (i + 1), 'A' + (j - 1)
              ) ;
            }
          }

          if (
            SUCCESS == dama_verify(dama, i + 1, j + 1) &&
            SUCCESS == dama_verify(dama, i + 2, j + 2)
          ) {
            chr = dama_getchr(dama, i + 1, j + 1) ;

            if (pl != chr && PL_BLANK != chr) {
              dama->row = i ;
              dama->col = j ;

              dama_printf(dama,
                "`%c` can eat `%c` at (%u,%c)\n",
                pl, chr, N_ROWS - (i + 1), 'A' + (j + 1)
              ) ;
            }
          }
        }
      }
    } // for N_COLS
  } // for N_ROWS
}