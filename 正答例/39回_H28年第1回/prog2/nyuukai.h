/************************************/
/*  nyuukai.h                       */
/*    入会登録処理ヘッダファイル    */
/************************************/

int  nyuukai_touroku( void );

static int  akicode_tbl_update( void );
static int  keisoku_tbl_add( long *fptr, int kaiin_code );
static int  codedata_tbl_update( int kaiin_code, long fptr );

