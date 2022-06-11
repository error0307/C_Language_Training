/**************************************/
/*  keisoku.c                         */
/*  　計測記録入力処理プログラム      */
/**************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "main.h"
#include "keisoku.h"

extern int akicode_tbl[ MEMBER_MAX + 1 ];                  /* 空きコード表           */
extern int codedata_tbl[ MEMBER_MAX ];                     /* コード・データ対照表   */
extern struct KEISOKU_TBL kojin_keisoku_tbl;               /* 個人別計測データ表     */
extern struct KEISOKU_TBL sort_keisoku_tbl[ MEMBER_MAX ];  /* ソート用計測データ表   */


/* 入力計測データテーブル */
static struct  KEISOKU_INPUT   keisoku_indata[ 5 ];

/************************************/
/* 計測記録入力処理                 */
/*   メインルーチン                 */
/*                                  */
/*   パラメータ : なし              */
/*   リターン   : 0:OK              */
/*               -1:NG              */
/************************************/
int keisoku_input( void )
{
    int     ret;                /* リターンコード    */
    int     i;                  /* インデックス      */
    int     kaiin_code;         /* 計測会員コード    */
    int     undou_sisuu;        /* 算出運動指数      */
    char    msg[ 64 ];          /* メッセージエリア  */
    char    keisoku_date[ 9 ];  /* 計測日付          */

    /* 空きコード表 READ -> NG ? */
    if( (ret = akicode_tbl_read( )) == NG ) {
        return ret;
    }

    /* 入会者チェック */
    if( akicode_tbl[ 0 ] >= MEMBER_MAX ) {
        printf( "\n 現在，入会者がいません" );
        return NG;
    }

    /* コード・データ対照表ファイル READ -> NG ? */
    if( (ret = codedata_tbl_read( )) == NG ) {
        return ret;
    }

    /* 計測会員コード入力 */
    kaiin_code_input( &kaiin_code );

    /* 計測日付入力 */
    keisoku_date_input( keisoku_date );

    /* 計測値入力処理 */
    for( i = 0; i < 5; i++ ) {
        keisoku_data_input( i );
    }

    /* 入力データ表示 */
    keisoku_data_disp( kaiin_code, keisoku_date );

    /* 入力確認 */
    strcpy( msg, "\n 入力はよろしいですか( Y/N )" );

    /* 'N' OR 'n'入力 -> OK でない ? */
    if( (ret = kakunin_input( msg )) != OK ) {
        return NG;
    }

    /* 計測データ表 READ -> NG ? */
    if( (ret = kojin_data_read( kaiin_code )) == NG ) {
        return ret;
    }

    /* 計測値計算処理 */
    undou_sisuu_keisan( &undou_sisuu );

    /* 計測データ表 SET */
    kojin_data_update( kaiin_code, keisoku_date, undou_sisuu );

    /* 計測結果データ表示 */
    kojin_data_disp( kaiin_code, "\n ** 計測結果データ **" );

    /* キー入力待ち */
    while( getchar( ) != '\n' ) ;
    printf( "\n リターンキーを押してください" );
    getchar( );

    /* 計測結果順位ソート表示 */
    ret = keisoku_rank( );

    return ret;
}



/**************************************/
/* 計測記録入力処理                   */
/*   コード・データ対照表　読込み処理 */
/*                                    */
/*   パラメータ : なし                */
/*   リターン   : 0:OK                */
/*               -1:NG                */
/**************************************/
static int codedata_tbl_read( void )
{
    int     ret;                        /* リターンコード               */
    FILE    *fp;                        /* ファイルポインタ             */
    char    *fname = CODEDATA_TBL_NAME; /* コード・データ対照表ファイル */

    /* コード・データ対照表ファイル OPEN -> NULL ? */
    if( (fp = fopen( fname, "rb" )) == NULL ) {
        printf( "\n コード・データ対照表ファイル OPEN エラー" );
        return NG;
    }

    /* コード・データ対照表ファイル READ -> 1以外 ? */
    if( (ret = fread( (char *)codedata_tbl, sizeof( codedata_tbl ), 1, fp ))
               != 1 ) {
        printf( "\n コード・データ対照表ファイル READ エラー" );
        ret = NG;
    }
    else {
        ret = OK;
    }

    /* コード・データ対照表ファイル CLOSE */
    fclose( fp );

    return ret;
}



/************************************/
/* 計測記録入力処理                 */
/*   会員コード入力処理             */
/*                                  */
/*   パラメータ : 入力会員コード    */
/*   リターン   : なし              */
/************************************/
static void kaiin_code_input( int *kaiin_code )
{
    int     loop = TRUE;    /* ループフラグ */
    char    work[ 128 ];    /* 入力ワーク   */

    while( loop ) {
        printf( "\n 計測者の会員コードを入力してください" );
        printf( "\n ? " );

        /* 会員コード入力 */
        work[ 0 ] = '\0';
        scanf( "%s", work );

        /* ニューメリック・チェック -> 数値以外 ? */
        if( strspn( work, "1234567890" ) < strlen( work ) ) {
            printf( "\n 数値以外が入力されました" );
            continue;
        }

        /* 入力範囲チェック( 0 < kaiin_code <= MEMBER_MAX ) */
        *kaiin_code = atoi( work );
        if( *kaiin_code > MEMBER_MAX || *kaiin_code <= 0 ) {
            printf( "\n 入力ミスです" );
            continue;
        }

        /* 会員コード登録チェック -> 未登録 ? */
        if( codedata_tbl[ *kaiin_code - 1 ] == 0 ) {
            printf( "\n この会員コードは未登録です" );
            continue;
        }
        break;
    }
    return;
}



/************************************/
/* 計測記録入力処理                 */
/*   計測日付入力処理               */
/*                                  */
/*   パラメータ : 入力日付          */
/*   リターン   : なし              */
/************************************/
static void keisoku_date_input( char *keisoku_date )
{
    int     loop = TRUE;    /* ループフラグ  */
    int     chk_date;       /* 日付数値      */
    char    conv[ 3 ];      /* 数値変換用    */
    char    work[ 128 ];    /* 入力ワーク    */

    while( loop ) {
        printf( "\n 日付を入力してください( YYYYMMDD )" );
        printf( "\n ? " );

        /* 日付入力 */
        work[ 0 ] = '\0';
        scanf( "%s", work );

        /* 入力桁数チェック -> 8以外 ? */
        if( strlen( work ) != 8 ) {
            printf( "\n 入力ミスです" );
            continue;
        }

        /* ニューメリック・チェック -> 数値以外 ? */
        if( strspn( work, "1234567890" ) < strlen( work ) ) {
            printf( "\n 数値以外が入力されました" );
            continue;
        }

        /* 月チェック */
        conv[0] = work[4];
        conv[1] = work[5];
        conv[2] = '\0';

        chk_date = atoi( conv );
        if( chk_date > 12 || chk_date < 1 ) {
            printf( "\n 日付( 月 )入力エラーです" );
            continue;
        }

        /* 日チェック */
        conv[0] = work[6];
        conv[1] = work[7];
        conv[2] = '\0';
        chk_date = atoi( conv );
        if( chk_date > 31 || chk_date < 1 ) {
            printf( "\n 日付( 日 )入力エラーです" );
            continue;
        }

        break;
    }

    /* 入力データ セット */
    strcpy(keisoku_date, work);
    return;
}



/************************************/
/* 計測記録入力処理                 */
/*   計測データ入力処理             */
/*                                  */
/*   パラメータ : インデックス      */
/*   リターン   : なし              */
/************************************/
static void keisoku_data_input( int idx )
{
    int     i;                  /* インデックス   */
    int     loop = TRUE;        /* ループフラグ   */
    char    work[ 3 ][ 128 ];   /* 入力ワーク     */

    while( loop ) {
        printf( "\n 運動 %d の計測データを入力してください", idx + 1 );
        printf( "\n   負荷( 999 )  セット( 100 )  回数( 100 )" );
        printf( "\n ? " );

        /* 計測データ入力 */
        work[ 0 ][ 0 ] = '\0';
        work[ 1 ][ 0 ] = '\0';
        work[ 2 ][ 0 ] = '\0';
        scanf( "%s %s %s", work[ 0 ], work[ 1 ], work[ 2 ] );

        for( i = 0; i < 3; i++ ) {

            /* 入力桁数チェック -> 3より大きい ? */
            if( strlen( work[ i ] ) > 3 ) {
                printf( "\n 入力ミスです" );
                break;
            }

            /* ニューメリック・チェック -> 数値以外 ? */
            if( strspn( work[ i ], "1234567890" ) < strlen( work[ i ] ) ) {
                printf( "\n 数値以外が入力されました" );
                break;
            }

            /* 負荷以外の上限チェック -> 100より大きい ? */
            if( i != 0 ) {
                if( atoi( work[ i ] ) > 100 ) {
                    printf( "\n 上限( 100 )を超えています" );
                    break;
                 }
            }
        }

        if( i < 3 ) {
            continue;
        }

        break;
    }

    /* 入力データテーブル セット */
    keisoku_indata[ idx ].huka = atoi( work[ 0 ] );
    keisoku_indata[ idx ].set = atoi( work[ 1 ] );
    keisoku_indata[ idx ].kaisuu = atoi( work[ 2 ] );

    return;
}



/************************************/
/* 計測記録入力処理                 */
/*   入力計測データ　表示処理       */
/*                                  */
/*   パラメータ : 会員コード        */
/*                日付              */
/*   リターン   : なし              */
/************************************/
static void keisoku_data_disp( int kaiin_code, char *keisoku_date )
{
    int     i;      /* インデックス      */

    printf( "\n ** 入力計測値データ **" );
    printf( "\n 会員コード %3d", kaiin_code );
    printf( "\n 日付       %4.4s-%2.2s-%2.2s", 
        ( keisoku_date + 0 ), ( keisoku_date + 4 ), ( keisoku_date + 6 ) );

    printf( "\n\n 運動   負荷  セット   回数" );

    for( i = 0; i < 5; i++ ) {
        printf( "\n   %d     %3d    %3d     %3d", i + 1, 
            keisoku_indata[ i ].huka,
            keisoku_indata[ i ].set,
            keisoku_indata[ i ].kaisuu );
    }

    return;
}



/************************************/
/* 計測記録入力処理                 */
/*   運動指数計算処理               */
/*                                  */
/*   パラメータ : 運動指数          */
/*   リターン   : なし              */
/************************************/
static void undou_sisuu_keisan( int *undou_sisuu )
{
    int     i;              /* インデックス  */
    double  sisuu;          /* 計算ワーク    */
    double  sisuu_total;    /* 計算値合計    */
    static  double  huka_sisuu[ 5 ] = { 0.24, 0.36, 0.52, 1.05, 2.13 };

    sisuu_total = 0.0;
    for( i = 0; i < 5; i++ ) {

        /* 回数 1以下 ? */
        if( keisoku_indata[ i ].kaisuu <= 1 ) {
            continue;
        }

        sisuu = huka_sisuu[ i ] * (double)keisoku_indata[ i ].huka * 
               (double)keisoku_indata[ i ].set *
               ((pow( (double)keisoku_indata[ i ].kaisuu, 2.0 )) /
               ((double)keisoku_indata[ i ].kaisuu - 1.0));

        /* シグマ(累計)の計算 */
        sisuu_total += sisuu;
    }

    /* 平方根の算出 */
    *undou_sisuu = (int)sqrt( sisuu_total );

    return;
}



/************************************/
/* 計測記録入力処理                 */
/*   個人計測データ表　更新処理     */
/*                                  */
/*   パラメータ : 会員コード        */
/*                日付              */
/*                運動指数          */
/*   リターン   : 0:OK              */
/*               -1:NG              */
/************************************/
static int kojin_data_update( int kaiin_code, char *keisoku_date, int undou_sisuu )
{
    int     ret;                        /* リターンコード         */
    long    fptr;                       /* 計測データポインタ     */
    FILE    *fp;                        /* ファイルポインタ       */
    char    *fname = KEISOKU_TBL_NAME;  /* 計測データ表ファイル   */
    int     i;                          /* インデックス           */

    /* 1回目 ? */
    if( kojin_keisoku_tbl.count <= 0 ) {
        strcpy( kojin_keisoku_tbl.first_date, keisoku_date );
        kojin_keisoku_tbl.first_data = undou_sisuu;

        strcpy( kojin_keisoku_tbl.max_date, keisoku_date );
        kojin_keisoku_tbl.max_data = undou_sisuu;
    }
    else {

        /* 最高記録 ? */
        if( kojin_keisoku_tbl.max_data < undou_sisuu ) {
            strcpy( kojin_keisoku_tbl.max_date, keisoku_date );
            kojin_keisoku_tbl.max_data = undou_sisuu;
        }
        for (i = sizeof kojin_keisoku_tbl.soku_data / sizeof(int) - 1; i > 0; i-- )
            kojin_keisoku_tbl.soku_data[ i ] = kojin_keisoku_tbl.soku_data[ i - 1 ];
    }

    strcpy( kojin_keisoku_tbl.soku_date, keisoku_date );
    kojin_keisoku_tbl.soku_data[ 0 ] = undou_sisuu;
    kojin_keisoku_tbl.count++;

    /* 計測データ表ファイル OPEN -> NULL ? */
    if( (fp = fopen( fname, "r+b" )) == NULL ) {
        printf( "\n 計測データ表ファイル OPEN エラー" );
        return NG;
    }

    /* 該当データポインタ セット */
    fptr = ( codedata_tbl[ kaiin_code - 1 ] - 1 ) * 
        sizeof( struct KEISOKU_TBL );

    /* 計測データ表ファイル SEEK -> OK でない ? */
    if( (ret = fseek( fp, fptr, SEEK_SET )) != OK ) {
        printf( "\n 計測データ表ファイル SEEK エラー" );

        /* 計測データ表ファイル CLOSE */
        fclose( fp );
        return NG;
    }

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



/************************************/
/* 計測記録入力処理                 */
/*   計測結果順位ソート・表示処理   */
/*                                  */
/*   パラメータ : なし              */
/*   リターン   : 0:OK              */
/*               -1:NG              */
/************************************/
static int keisoku_rank( void )
{
    int     ret;                        /* リターンコード         */
    int     i;                          /* インデックス           */
    FILE    *fp;                        /* ファイルポインタ       */
    char    *fname = KEISOKU_TBL_NAME;  /* 計測データ表ファイル   */

    /* 計測データ表ファイル OPEN -> NULL ? */
    if( (fp = fopen( fname, "rb" )) == NULL ) {
        printf( "\n 計測データ表ファイル OPEN エラー" );
        return NG;
    }

    i = 0;
    for( ; ; ) {

        /* 計測データ表ファイル READ -> 1以外 ? */
        if( (ret = fread( (char *)&sort_keisoku_tbl[ i ],
                   sizeof( struct KEISOKU_TBL ), 1, fp )) != 1 ) {

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
        }

        /* READ エラー ? */
        if( ret == NG ) {
            break;
        }

        /* 計測データあり ? */
        if( sort_keisoku_tbl[ i ].count != 0 ) {
            i++;
        }

        /* eof ? */
        if( ret == OK ) {
            break;
        }
    }

    /* 計測データ表ファイル CLOSE */
    fclose( fp );

    if( ret == OK ) {
        /* 計測データソート */
        keisoku_data_sort( i );
        /* 計測データソート結果表示 */
        keisoku_sort_disp( i );
    }

    return ret;
}



/************************************/
/* 計測記録入力処理                 */
/*   計測結果順位ソート処理         */
/*                                  */
/*   パラメータ : 計測データ件数    */
/*   リターン   : なし              */
/************************************/
static void keisoku_data_sort( int cnt )
{
    int     i;                      /* インデックス      */
    int     j;                      /* インデックス      */
    struct  KEISOKU_TBL  work;      /* スワップ用エリア  */

    /* データソート */
    for( i = 0; i < cnt - 1; i++ ) {
        for( j = i + 1; j < cnt; j++ ) {
            if( sort_keisoku_tbl[ i ].max_data < 
                sort_keisoku_tbl[ j ].max_data ) {

                work = sort_keisoku_tbl[ i ];
                sort_keisoku_tbl[ i ] = sort_keisoku_tbl[ j ];
                sort_keisoku_tbl[ j ] = work;
            }
        }
    }
    return;
}



/************************************/
/* 計測記録入力処理                 */
/*   計測結果順位表示処理           */
/*                                  */
/*   パラメータ : 計測データ件数    */
/*   リターン   : なし              */
/************************************/
static void keisoku_sort_disp( int cnt )
{
    int     i;  /* インデックス  */

    printf( "\n\n ** 順位表 **" );
    printf( "\n 順位 会員コード 最高データ     日付" );

    for( i = 0; i < cnt; i++ ) {

        /* 10位まで表示する */
        if( i >= 10 ) {
            break;
        }

        printf( "\n   %2d   %3d         %4d     %4.4s-%2.2s-%2.2s", i + 1,
            sort_keisoku_tbl[ i ].kaiin_code,
            sort_keisoku_tbl[ i ].max_data,
            &sort_keisoku_tbl[ i ].max_date[ 0 ],
            &sort_keisoku_tbl[ i ].max_date[ 4 ],
            &sort_keisoku_tbl[ i ].max_date[ 6 ] );
    }
    return;
}

#ifdef TOI2
/************************************/
/* 個人計測データ初期化処理         */
/*   メインルーチン                 */
/*                                  */
/*   パラメータ : なし              */
/*   リターン   : 0:OK              */
/*               -1:NG              */
/************************************/
int clear_data(void)
{
    int     ret;            /* リターンコード               */
    int     loop = TRUE;    /* ループフラグ                 */
    int     kaiin_code;     /* 計測データ初期化会員コード   */
    char    work[128];      /* 入力ワーク                   */
    char    msg[64];        /* メッセージエリア             */

    /* コード・データ対照表ファイル READ -> NG ? */
    if ((ret = codedata_tbl_read()) == NG) {
        return ret;
    }

    while (loop) {
        printf("\n 計測データを初期化する会員コードを入力してください(E:終了)");
        printf("\n ? ");

        /* 会員コード入力 */
        work[0] = '\0';
        scanf("%s", work);

        /* 計測データ初期化処理終了*/
        if (work[0] == 'E' || work[0] == 'e') {
            break;
        }
        /* ニューメリック・チェック -> 数値以外 ? */
        if (strspn(work, "1234567890") < strlen(work)) {
            printf("\n 数値以外が入力されました");
            continue;
        }

        /* 入力範囲チェック -> ( 0 < kaiin_code <= MEMBER_MAX ) ? */
        kaiin_code = atoi(work);
        if (kaiin_code > MEMBER_MAX || kaiin_code <= 0) {
            printf("\n 1～200以外の値が入力されました");
            continue;
        }

        /* 会員の存在チェック */
        if (codedata_tbl[kaiin_code - 1] == 0) {
            printf("\n 指定された会員は存在しません");
            continue;
        }

        /* 計測データ表 READ -> NG ? */
        if ((ret = kojin_data_read(kaiin_code)) == NG) {
            /* コード・データ対照表ファイル CLOSE */
            return ret;
        }

        /* 計測データありの判定 */
        if (kojin_keisoku_tbl.count == 0)
        {
            printf("\n 指定された会員の計測データはありません");
            continue;
        }

        /* 計測データ表示 */
        kojin_data_disp(kaiin_code, "\n ** 初期化データ **");

        /* 計測データ表初期化確認 */
        sprintf(msg, "\n\n 上のデータを初期化します。よろしいですか( Y/N )");

        if ((ret = kakunin_input(msg)) == OK) {
            /* 個人計測データ表情報を初期化する -> OK ? */
            if ((ret = codedata_tbl_clear(kaiin_code)) == OK) {
                printf("\n 会員コード(%3d)の計測データを初期化しました", kaiin_code);
            }
            else {
                /* エラー発生 */
                loop = FALSE;
            }
        }
    }

    return ret;
}

/************************************/
/* 個人計測データ表情報初期化処理   */
/*   個人計測データ初期化処理       */
/*                                  */
/*   パラメータ : 初期化会員コード  */
/*   リターン   : 0:OK              */
/*               -1:NG              */
/************************************/
static int codedata_tbl_clear(int kaiin_code)
{
    int     ret;                        /* リターンコード         */
    long    fptr;                       /* 計測データポインタ     */
    FILE    *fp;                        /* ファイルポインタ       */
    char    *fname = KEISOKU_TBL_NAME;  /* 計測データ表ファイル   */

    /* 計測データ表ファイル OPEN -> NULL ? */
    if ((fp = fopen(fname, "r+b")) == NULL) {
        printf("\n 計測データ表ファイル OPEN エラー");
        return NG;
    }

    /* 該当データポインタ セット */
    fptr = (codedata_tbl[kaiin_code - 1] - 1) *
        sizeof(struct KEISOKU_TBL);

    /* 計測データ表ファイル SEEK -> OK でない ? */
    if ((ret = fseek(fp, fptr, SEEK_SET)) != OK) {
        printf("\n 計測データ表ファイル SEEK エラー");

        /* 計測データ表ファイル CLOSE */
        fclose(fp);
        return NG;
    }

    /* 初期化する計測データの設定 */
    kojin_keisoku_tbl = init_kojin_keisoku_tbl();
    kojin_keisoku_tbl.kaiin_code = kaiin_code;

    /* 計測データ表ファイル WRITE -> 1以外 ? */
    if ((ret = fwrite((char *)&kojin_keisoku_tbl, sizeof(kojin_keisoku_tbl),
        1, fp)) != 1) {
        printf("\n 計測データ表ファイル WRITE エラー");
        ret = NG;
    }
    else {
        ret = OK;
    }

    /* 計測データ表ファイル CLOSE */
    fclose(fp);

    return ret;
}
#endif
