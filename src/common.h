/************************************/
/*  common.h                        */
/*    共通ヘッダファイル            */
/************************************/

#define MEMBER_MAX          200             /* メンバー数 MAX                   */
#define AKICODE_TBL_NAME    "akicode.tbl"   /* 空きコード表ファイル名           */
#define CODEDATA_TBL_NAME   "codedata.tbl"  /* コード・データ対照表ファイル名   */
#define KEISOKU_TBL_NAME    "keisoku.tbl"   /* 計測データ表ファイル名           */

#define  TRUE          1    /* 真            */
#define  FALSE         0    /* 偽            */
#define  OK            0    /* 正常          */
#define  CANCEL        1    /* 処理中止      */
#define  NG           -1    /* 異常          */

/* 計測データテーブル */
struct   KEISOKU_TBL {
    int  kaiin_code;       /* 会員コード     */
    int  count;            /* 計測回数       */
    char first_date[ 9 ];  /* 初回日付       */
    int  first_data;       /* 初回データ     */
    char max_date[ 9 ];    /* 最高記録日付   */
    int  max_data;         /* 最高記録データ */
    char soku_date[ 9 ];   /* 最新計測日付   */
    int  soku_data[ 10 ];  /* 計測データ     */
};

/* 入力計測データ */
struct   KEISOKU_INPUT {
    int  huka;             /* 負荷   */
    int  set;              /* セット */
    int  kaisuu;           /* 回数   */
};
