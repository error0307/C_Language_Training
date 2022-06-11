/************************************/
/*  nyuukai.h                       */
/*    “ü‰ï“o˜^ˆ—ƒwƒbƒ_ƒtƒ@ƒCƒ‹    */
/************************************/

int  nyuukai_touroku( void );

static int  akicode_tbl_update( void );

#ifdef TOI2
static int  keisoku_tbl_add(long *fptr, int kaiin_code, int age);
#else
static int  keisoku_tbl_add(long *fptr, int kaiin_code);
#endif

static int  codedata_tbl_update( int kaiin_code, long fptr );

#ifdef TOI2
static int age_input( void );
#endif