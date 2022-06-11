/************************************/
/*  nyuukai.c                       */
/*    入会登録処理プログラム        */
/************************************/
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "main.h"
#include "nyuukai.h"

extern int akicode_tbl[ MEMBER_MAX + 1 ];        /* 空きコード表          */
extern int codedata_tbl[ MEMBER_MAX ];           /* コード・データ対照表  */
extern struct KEISOKU_TBL kojin_keisoku_tbl;     /* 個人別計測データ表    */

/***********************************/
/* 入会登録処理                    */
/*   メインルーチン                */
/*                                 */
/*   パラメータ : なし             */
/*   リターン   : 0:OK             */
/*               -1:NG             */
/***********************************/
int nyuukai_touroku( void )
{
    int    ret;             /* リターンコード      */
    int    kaiin_code;      /* 登録会員コード      */
    long   fptr;            /* ファイルポインタ    */
    char   msg[ 64 ];       /* メッセージエリア    */

    /* 空きコード表 READ -> NG ? */
    if( (ret = akicode_tbl_read( )) == NG ) {
        return ret;
    }

    /* 空きコードあり ? */
    if( akicode_tbl[ 0 ] <= 0 ) {
        printf( "\n 残念ながらただ今メンバーの空きがありません\n" );
        return OK;
    }

    /* 空きコード確認 */
    sprintf( msg, "\n 会員コードは %d です。よろしいですか( Y/N )", akicode_tbl[ 1 ] );

    if( (ret = kakunin_input( msg )) == OK ) {
        /* 登録会員コード退避 */
        kaiin_code = akicode_tbl[ 1 ];

        /* 空きコード表更新 -> OK ? */
        if( (ret = akicode_tbl_update( )) == OK ) {

            /* 計測データ表追加 -> OK ? */
            if( (ret = keisoku_tbl_add( &fptr, kaiin_code )) == OK ) {

                /* コード・データ対照表更新 */
                ret = codedata_tbl_update( kaiin_code, fptr );
            }
        }
    }

    if( ret == OK ) {
        printf( "\n 入会登録処理が終了しました" );
    }

    return ret;
}



/************************************/
/* 入会登録処理                     */
/*   空きコード表　更新処理         */
/*                                  */
/*   パラメータ : なし              */
/*   リターン   : 0:OK              */
/*               -1:NG              */
/************************************/
static int akicode_tbl_update( void )
{
    int     ret;                        /* リターンコード       */
    int     i;                          /* インデックス         */
    int     cnt;                        /* 空きコード件数       */
    FILE    *fp;                        /* ファイルポインタ     */
    char    *fname = AKICODE_TBL_NAME;  /* 空きコード表ファイル */

    /* 空きコード件数セット */
    cnt = akicode_tbl[ 0 ];

    /* 空きコード編集 */
    for( i = 1; i < cnt; i++ ) {
        if( akicode_tbl[ i + 1 ] == 0 ) {
            break;
        }
        akicode_tbl[ i ] = akicode_tbl[ i + 1 ];
    }

    akicode_tbl[ i ] = 0;

    /* 空きコード件数セット */
    akicode_tbl[ 0 ] = cnt - 1;

    /* 空きコード表ファイル OPEN -> NULL ? */
    if( (fp = fopen( fname, "w+b" )) == NULL ) {
        printf( "\n 空きコード表ファイル OPEN エラー" );
        return NG;
    }

    /* 空きコード表ファイル WRITE -> 1以外 ? */
    if( (ret = fwrite( (char *)akicode_tbl,
               sizeof( int ) * (akicode_tbl[ 0 ] + 1), 1, fp )) != 1 ) {
        printf( "\n 空きコード表ファイル WRITE エラー" );
        ret = NG;
    }
    else {
        ret = OK;
    }

    /* 空きコード表ファイル CLOSE */
    fclose( fp );

    return ret;
}



/************************************/
/* 入会登録処理                     */
/*   計測コード表　追加処理         */
/*                                  */
/*   パラメータ : 計測データポインタ*/
/*                登録会員コード    */
/*   リターン   : 0:OK              */
/*               -1:NG              */
/************************************/
static int keisoku_tbl_add( long *fptr, int kaiin_code )
{
    int     ret;                        /* リターンコード         */
    FILE    *fp;                        /* ファイルポインタ       */
    char    *fname = KEISOKU_TBL_NAME;  /* 計測データ表ファイル   */

    /* 計測データ表ファイル OPEN -> NULL ? */
    if( (fp = fopen( fname, "a+b" )) == NULL ) {
        printf( "\n 計測データ表ファイル OPEN エラー" );
        return NG;
    }

    /* 計測データ表ファイル SEEK -> OK でない ? */
    if( (ret = fseek( fp, 0L, SEEK_END )) != OK ) {
        printf( "\n 計測データ表ファイル SEEK エラー" );
        /* 計測データ表ファイル CLOSE */
        fclose( fp );
        return NG;
    }

    /* ファイルポインタ取得 */
    *fptr = ftell( fp );

    /* 計測データ表クリア */
    kojin_keisoku_tbl = init_kojin_keisoku_tbl();

    /* 会員コードセット */
    kojin_keisoku_tbl.kaiin_code = kaiin_code;

    /* 計測データ表ファイル WRITE -> 1以外 ? */
    if( (ret = fwrite( (char *)&kojin_keisoku_tbl, sizeof( kojin_keisoku_tbl ),
               1, fp )) != 1 ) {
        printf( "\n 計測データ表ファイル WRITE エラー" );
        ret = NG;
    }
    else {
        ret = OK;
    }

    /* 計測データ表ファイル CLOSE */
    fclose( fp );

    return ret;
}



/*****************************************/
/* 入会登録処理                     　　 */
/*   コード・データ対照表　更新処理      */
/*                                       */
/*   パラメータ : 登録会員コード         */
/*                計測データポインタ     */
/*   リターン   : 0:OK                   */
/*               -1:NG                   */
/*****************************************/
static int codedata_tbl_update( int kaiin_code, long fptr )
{
    int     ret;                        /* リターンコード                */
    FILE    *fp;                        /* ファイルポインタ              */
    char    *fname = CODEDATA_TBL_NAME; /* コード・データ対照表ファイル  */

    /* コード・データ対照表ファイル OPEN -> NULL ? */
    if( (fp = fopen( fname, "r+b" )) == NULL ) {
        printf( "\n コード・データ対照表ファイル OPEN エラー" );
        return NG;
    }

    /* コード・データ対照表ファイル READ -> 1以外 ? */
    if( (ret = fread( (char *)codedata_tbl, sizeof( codedata_tbl ), 1, fp ) )
               != 1 ) {
        printf( "\n コード・データ対照表ファイル READ エラー" );
        ret = NG;
    }
    else {
        /* 該当データポインタセット */
        codedata_tbl[ kaiin_code - 1 ] = 
            (int)( (fptr / sizeof( struct KEISOKU_TBL )) + 1 );

        /* ファイルポインタを先頭に SEEK -> OK でない ? */
        if( (ret = fseek( fp, 0L, SEEK_SET )) != OK ) {
            printf( "\n コード・データ対照表ファイル SEEK エラー" );

            /* コード・データ対照表ファイル CLOSE */
            fclose( fp );
            return NG;
        }

        /* コード・データ対照表ファイル WRITE -> 1以外 ? */
        if( (ret = fwrite( (char *)codedata_tbl, sizeof( codedata_tbl ), 1,
                   fp )) != 1 ) {
            printf( "\n コード・データ対照表ファイル WRITE エラー" );
            ret = NG;
        }
        else {
            ret = OK;
        }
    }

    /* コード・データ対照表ファイル CLOSE */
    fclose( fp );

    return ret;
}
