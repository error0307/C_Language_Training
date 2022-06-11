/************************************/
/*  sakujyo.c                       */
/*    登録削除処理プログラム        */
/************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "main.h"
#include "sakujyo.h"

extern int akicode_tbl[ MEMBER_MAX + 1 ];      /* 空きコード表           */
extern int codedata_tbl[ MEMBER_MAX ];         /* コード・データ対照表   */
extern struct KEISOKU_TBL kojin_keisoku_tbl;   /* 個人別計測データ表     */

/************************************/
/* 登録削除処理                     */
/*   メインルーチン                 */
/*                                  */
/*   パラメータ : なし              */
/*   リターン   : 0:OK              */
/*               -1:NG              */
/************************************/
int touroku_sakujyo( void )
{
    int     ret;            /* リターンコード   */
    int     loop = TRUE;    /* ループフラグ     */
    int     kaiin_code;     /* 退会会員コード   */
    char    work[ 128 ];    /* 入力ワーク       */

    while( loop ) {
        printf( "\n 退会者の会員コードを入力してください" );
        printf( "\n ? " );

        /* 会員コード入力 */
        work[ 0 ] = '\0';
        scanf( "%s", work );

        /* ニューメリック・チェック -> 数値以外 ? */
        if( strspn( work, "1234567890" ) < strlen( work ) ) {
            printf( "\n 数値以外が入力されました" );
            continue;
        }

        /* 入力範囲チェック -> ( 0 < kaiin_code <= MEMBER_MAX ) ? */
        kaiin_code = atoi( work );
        if( kaiin_code > MEMBER_MAX || kaiin_code <= 0 ) {
            printf( "\n 入力ミスです" );
            continue;
        }

        /* コード・データ対照表削除 -> OK ? */
        if( (ret = codedata_tbl_delete( kaiin_code )) == OK ) {
            /* 空きコード表追加 -> OK ? */
            if( (ret = akicode_tbl_add( kaiin_code )) == OK ) {
                /* メインに戻る */
                loop = FALSE;
            }
        }
        else {
            /* メインに戻る */
            loop = FALSE;
        }
    }

    if( ret == OK ) {
        printf( "\n 入会登録削除処理が終了しました" );
    }

    return ret;
}



/************************************/
/* 登録削除処理                     */
/*   コード・データ対照表　削除処理 */
/*                                  */
/*   パラメータ : 退会会員コード    */
/*   リターン   : 0:OK              */
/*                1:CANCEL          */
/*               -1:NG              */
/************************************/
static int codedata_tbl_delete( int kaiin_code )
{
    int     ret;                        /* リターンコード                */
    int     i;                          /* インデックス                  */
    char    msg[ 64 ];                  /* メッセージエリア              */
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
        /* READ エラー */
        printf( "\n コード・データ対照表ファイル READ エラー" );

        /* コード・データ対照表ファイル CLOSE */
        fclose( fp );
        return NG;
    }

    /* 会員登録チェック -> 未登録 ? */
    if( codedata_tbl[ kaiin_code - 1 ] == 0 ) {
        printf( "\n この会員コードは未登録です" );

        /* コード・データ対照表ファイル CLOSE */
        fclose( fp );
        return CANCEL;
    }

    /* 計測データ表 READ -> NG ? */
    if( (ret = kojin_data_read( kaiin_code )) == NG ) {
        /* コード・データ対照表ファイル CLOSE */
        fclose( fp );
        return ret;
    }

    /* 計測データ表示 */
    kojin_data_disp( kaiin_code, "\n ** 削除データ **" );

    /* 計測データ表削除確認 */
    sprintf( msg, "\n\n 上のデータを削除します。よろしいですか( Y/N )" );

    if( (ret = kakunin_input( msg )) == OK ) {

        /* 計測データ表削除 -> OK ? */
        if( (ret = kojin_data_delete( kaiin_code )) == OK ) {

            /* 計測データポインタ更新 */
            for( i = 0; i < MEMBER_MAX; i++ ) {
                if( codedata_tbl[ i ] > codedata_tbl[ kaiin_code - 1 ] ) {
                    codedata_tbl[ i ]--;
                }
            }

            /* 計測データポインタクリア */
            codedata_tbl[ kaiin_code - 1 ] = 0;

            /* コード・データ対照表ファイルの先頭位置に SEEK -> OK ? */
            if( (ret = fseek( fp, 0L, SEEK_SET )) == OK ) {

                /* コード・データ対照表ファイル WRITE -> 1以外 ? */
                if( (ret = fwrite( (char *)codedata_tbl,
                           sizeof( codedata_tbl ),1, fp )) != 1 ) {
                    printf( "\n コード・データ対照表ファイル WRITE エラー" );
                    ret = NG;
                }
                else {
                    ret = OK;
                }
            }
            else {
                printf( "\n コード・データ対照表ファイル SEEK エラー" );
                ret = NG;
            }
        }
    }
    else {
        /* 処理中止 */
        ret = CANCEL;
    }

    /* コード・データ対照表ファイル CLOSE */
    fclose( fp );

    return ret;
}



/************************************/
/* 登録削除処理                     */
/*   個人計測データ表　削除処理     */
/*                                  */
/*   パラメータ : 退会会員コード    */
/*   リターン   : 0:OK              */
/*               -1:NG              */
/************************************/
static int kojin_data_delete( int kaiin_code )
{
    int     ret;                         /* リターンコード                */
    int     i;                           /* インデックス                  */
    FILE    *fp;                         /* 計測データ表ファイルポインタ  */
    FILE    *tmp;                        /* テンポラリファイルポインタ    */
    char    *fname = KEISOKU_TBL_NAME;   /* 計測データ表ファイル          */
    char    *tmpfl = "keisoku.tmp";      /* テンポラリファイル            */

    /* 計測データ表ファイル OPEN -> NULL ? */
    if( (fp = fopen( fname, "rb" )) == NULL ) {
        printf( "\n 計測データ表ファイル OPEN エラー" );
        return NG;
    }

    /* テンポラリファイル OPEN -> NULL ? */
    if( (tmp = fopen( tmpfl, "w+b" )) == NULL ) {
        printf( "\n テンポラリファイル OPEN エラー" );
        fclose( fp );
        return NG;
    }

    i = 0;
    for( ; ; ) {

        /* 計測データ表ファイル READ -> 1以外 ? */
        if( (ret = fread( (char *)&kojin_keisoku_tbl,
                   sizeof( kojin_keisoku_tbl ), 1, fp )) != 1 ) {

            /* READ エラー ? */
            if( ferror( fp ) != 0 ) {
                printf( "\n 計測データ表ファイル READ エラー" );
                ret = NG;
            }
            else {
                /* ファイル EOF でない ? */
                if( feof( fp ) == 0 ) {
                    printf( "\n 計測データ表ファイル READ エラー" );
                    ret = NG;
                }
                else {
                   ret = OK;
                }
            }
            break;
        }

        /* 削除データ ? */
        if( kaiin_code == kojin_keisoku_tbl.kaiin_code ) {
            continue;
        }

        /* テンポラリファイル WRITE -> 1以外 ? */
        if( (ret = fwrite( (char *)&kojin_keisoku_tbl,
                   sizeof( kojin_keisoku_tbl ), 1, tmp )) != 1 ) {
            printf( "\n 計測データ表ファイル WRITE エラー" );
            ret = NG;
            break;
        }
        i++;
    }

    /* テンポラリファイル CLOSE */
    fclose( tmp );

    /* 計測データ表ファイル CLOSE */
    fclose( fp );

    /* 正常終了 ? */
    if( ret == OK ) {

        /* 計測データ表ファイル DELETE -> 0以外 ? */
        if( (ret = remove( fname )) != 0 ) {
            printf( "\n 計測データ表ファイル削除エラー" );
            ret = NG;
        }
        else {
            /* 書込みデータあり ? */
            if( i > 0 ) {

                /* テンポラリファイルを計測データ表ファイルにリネームする -> 0以外 ? */
                if( (ret = rename( tmpfl, fname )) != 0 ) {
                    printf( "\n 計測データ表ファイルリネームエラー" );
                    ret = NG;
                }
            }
            else {
                /* テンポラリファイル削除 */
                remove( tmpfl );
            }
        }
    }
    else {
        /* テンポラリファイル削除 */
        remove( tmpfl );
    }

    return ret;
}



/************************************/
/* 登録削除処理                     */
/*   空きコード表　追加処理         */
/*                                  */
/*   パラメータ : 退会会員コード    */
/*   リターン   : 0:OK              */
/*               -1:NG              */
/************************************/
static int akicode_tbl_add( int kaiin_code )
{
    int     ret;                        /* リターンコード        */
    int     cnt;                        /* 空きコード件数        */
    FILE    *fp;                        /* ファイルポインタ      */
    char    *fname = AKICODE_TBL_NAME;  /* 空きコード表ファイル  */

    /* 空きコード表 READ -> NG ? */
    if( (ret = akicode_tbl_read( )) == NG ) {
        return ret;
    }

    /* 空きコード件数セット */
    cnt = akicode_tbl[ 0 ];

    /* 空きコードテーブルセット */
    akicode_tbl[ cnt + 1 ] = kaiin_code;

    /* 空きコード件数セット */
    akicode_tbl[ 0 ] = cnt + 1;

    /* 空きコード表ファイル OPEN -> NULL ? */
    if( (fp = fopen( fname, "w+b" )) == NULL ) {
        printf( "\n 空きコード表ファイル OPEN エラー" );
        return NG;
    }

    /* 空きコード表ファイル WRITE -> 1以外 ? */
    if( (ret = fwrite( (char *)akicode_tbl,
               sizeof( int ) * ( akicode_tbl[ 0 ] + 1 ), 1, fp )) != 1 ) {
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
