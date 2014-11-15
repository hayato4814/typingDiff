// ================================================================
// タイピングソフト用入力チェックモジュール
// ================================================================
// 
// Write          : hayato
// Character Code : UTF-8
// Line ending    : LF
// 
// ----------------------------------------------------------------




// ================================================================
// include module
// ================================================================
#include <stdio.h>
#include <stdlib.h>

#include "typingDiff.h"




// ================================================================
// プロトタイプ
// ================================================================
static int typingDiffColumn(char *text);
int typingDiffSearchCoincide(char* kana);
static int typingDiffSearchColumn(char*typed, int row);
static void typingDiffReplace(char* text, int row);




// ================================================================
// 定義
// ================================================================
#define CODE_ROW 414    // wordlist rows
#define CODE_COL 110    // wordlist cols

#define TRUE 1
#define FALSE 0




// ================================================================
// 静的グローバル変数
// ================================================================

// ローマ字変換パターン格納用配列
typedef struct {
    char* word;
    int length;
} _wordlist;

static _wordlist wordlist[CODE_ROW][CODE_COL];




// ================================================================
// 処理
// ================================================================

// ----------------------------------------------------------------
// ローマ字変換パターン格納用配列に変換データ(csv)を格納する
// ----------------------------------------------------------------
int typingDiffInit(void)
{
    FILE *fp;
    char text[2048];
    int row;
    int col;
    int offset;
    int textlen;
    int copycnt;



    // initalize wordlist[][].length zero
    for(row = 0; row < CODE_ROW; row++) {
        for(col = 0; col < CODE_COL; col++) {
            wordlist[row][col].length = 0;
        }
    }




    if( ( fp = fopen("wordlist", "r") ) == NULL ) {
        return 1;
    }



    row = 0;
    while( fgets( text, 2048, fp ) ) {

        col = 0;
        offset = 0;
        textlen = -1;

        do {
            textlen++;

            if( text[offset + textlen] == ',' || text[offset + textlen] == '\0' || text[offset + textlen] == '\r' || text[offset + textlen] == '\n' ) {
                wordlist[row][col].word = (char *)malloc(textlen+1);
                wordlist[row][col].length = textlen;
                for(copycnt = 0; copycnt < textlen ; copycnt++) {
                    wordlist[row][col].word[copycnt] = text[offset + copycnt];
                }
                wordlist[row][col].word[copycnt] = '\0';
                offset += textlen + 1;
                textlen = -1;
                col++;

            }

        } while( text[offset + textlen] != '\0' && text[offset + textlen] != '\r' && text[offset + textlen] != '\n' );

        row++;
    }



    fclose(fp);

    return 0;
}




// ----------------------------------------------------------------
// ローマ字変換パターン格納用配列の解放処理
// ----------------------------------------------------------------
int typingDiffFinal(void)
{
    int row,col;

    for(row = 0; row < CODE_ROW; row++) {
        for(col = 0; col < CODE_COL; col++) {
            free(wordlist[row][col].word);
            wordlist[row][col].length = 0;
        }
    }


    return 0;
}




// ----------------------------------------------------------------
// タイピングで何文字目まで合っているかを調べる
// ----------------------------------------------------------------
int getTypingDiff(char* kana, char* typed,char* fullstr)
{

    int lengthFullStr = 0;
    int row;
    int typedOffset = 0;
    int fullstrOffset = 0;
    int trueStrings = 0;

    // kanaの内容をfullstrにコピー
    while( *(kana + lengthFullStr) != '\0' && *(kana + lengthFullStr) != '\r' && *(kana + lengthFullStr) != '\n' /*buffer over flow*/ ) {
        *(fullstr + lengthFullStr) = *(kana + lengthFullStr);
        lengthFullStr++;
    }
    *(fullstr + lengthFullStr) = '\0';




    // fullstr の内容をローマ字に置き換え(typed の内容から自動切り替え)
    while( *(fullstr + fullstrOffset) != '\r' &&  *(fullstr + fullstrOffset) != '\n' &&  *(fullstr + fullstrOffset) != '\0') {
        row = typingDiffSearchCoincide( &fullstr[fullstrOffset] );

        if( row != -1 ) {
            typingDiffSearchColumn(&fullstr[fullstrOffset], row);
            typedOffset += typingDiffSearchColumn(&typed[typedOffset], row);

            if(fullstrOffset + wordlist[row][1].length >= TYPINGDIFF_STRLENGTH -1) {
                return -2;
            }

            typingDiffReplace(&fullstr[fullstrOffset], row);
        }

        fullstrOffset++;
    }



    // 何文字目まで合っているかを調べる
    while( *(typed + trueStrings) == *(fullstr + trueStrings) ) {
        trueStrings++;

        if( *(fullstr + trueStrings) == '\0' ) {
            trueStrings = -1;
            break;
        }
    }

    return trueStrings;

}




// ----------------------------------------------------------------
// ローマ字変換パターン(csv)のカラム数を調べる
// ----------------------------------------------------------------
static int typingDiffColumn(char *text)
{
    int offset = 0;
    int col = 1;




    // 最初からNULL文字ならカラム数は0
    if( *(text) == '\r' || *(text) == '\n' || *(text) == '\0' ) {
        col = 0;
    }




    // NULL文字が見つかるまでカラム数を調べる(,で区切る)
    while( *(text + offset) != '\r' && *(text + offset) != '\n' && *(text + offset) != '\0' ) {
        if( *(text + offset) == ',' ) {
            col ++;
        }

        offset ++;
    }

    return col;
}




// ----------------------------------------------------------------
// ローマ字変換用の「かな」を検索
// ----------------------------------------------------------------
int typingDiffSearchCoincide(char* kana)
{
    int searchpos;
    int row;
    int coinside;

    for(row = 0; row < CODE_ROW; row++ ) {

        coinside = TRUE;

        for(searchpos = 0; searchpos < wordlist[row][0].length; searchpos++) {

            if( *(kana + searchpos) == '\r' || *(kana + searchpos) == '\n' || *(kana + searchpos) == '\0' ) {
                coinside = FALSE;
                break;
            }

            if( *(kana + searchpos) != wordlist[row][0].word[searchpos] ) {
                coinside = FALSE;
                break;
            }
        }

        if( coinside == TRUE ) {
            return row;
        }

    }

    return -1;
}




// ----------------------------------------------------------------
// 入力された文字列からローマ字変換後に一致するパターンを探す
// ----------------------------------------------------------------
static int typingDiffSearchColumn(char*typed, int row)
{
    int disable[CODE_COL];
    int enable;
    int strExcess;
    int colCnt;
    int strCnt;

    _wordlist swapWordlist;

    for(colCnt = 0; colCnt < CODE_COL; colCnt++) {
        disable[colCnt] = TRUE;
    }

    strExcess = 0;
    enable = CODE_COL -1;
    strCnt = 0;
    while(1) {
        for(colCnt = 1; colCnt < CODE_COL; colCnt++) {

            // if wordlist[row][] col enable continue;
            if(disable[colCnt] == FALSE) {
                continue;
            }

            if(wordlist[row][colCnt].length == 0) {
                disable[colCnt] = FALSE;
                enable--;
                continue;
            }

            if(wordlist[row][colCnt].length < strCnt) {
                disable[colCnt] = FALSE;
                enable--;
                continue;
            }

            // typed is end
            if(*(typed + strCnt) == '\0' || *(typed + strCnt) == '\r' || *(typed + strCnt) == '\n') {
                enable = 0;
                break;
            }

            // typed not equal wordlist[row][colCnt]
            if( (wordlist[row][colCnt].length > strCnt) && (wordlist[row][colCnt].word[strCnt] != *(typed + strCnt)) ) {
                disable[colCnt] = FALSE;
                enable--;
                continue;
            }

            // wordlist[row][colCnt].length[strCnt] is '\0'
            if( wordlist[row][colCnt].length <= strCnt && enable > 1 ) {
                disable[colCnt] = FALSE;
                enable--;
                continue;
            }

            // move to enable column wordlist[row][1]
            if(disable[1] == FALSE) {
                swapWordlist.length = wordlist[row][1].length;
                wordlist[row][1].length = wordlist[row][colCnt].length;
                wordlist[row][colCnt].length = swapWordlist.length;

                swapWordlist.word = wordlist[row][1].word;
                wordlist[row][1].word = wordlist[row][colCnt].word;
                wordlist[row][colCnt].word = swapWordlist.word;

                disable[colCnt] = FALSE;
                disable[1] = TRUE;
            }

        }

        if(enable <= 1) {
            break;
        }

        strCnt++;
    }

    // strCnt get
    strCnt = 0;
    while(
        *(typed + strCnt) != '\r'                           &&
        *(typed + strCnt) != '\n'                           &&
        *(typed + strCnt) != '\0'                           &&
        wordlist[row][1].length > strCnt                    &&
        wordlist[row][1].word[strCnt] == *(typed + strCnt)
    ){
        strCnt++;
    }

    return strCnt;
}




// ----------------------------------------------------------------
// ひらがなをローマ字に変換(入力された内容に合わせて自動変換)
// ----------------------------------------------------------------
static void typingDiffReplace(char* text, int row)
{
    char *cloneText = text;
    int targetOffset = wordlist[row][0].length;
    int replaceOffset = 0;

    for(replaceOffset = 0; replaceOffset < wordlist[row][1].length; replaceOffset++) {
        *(text + replaceOffset) = wordlist[row][1].word[replaceOffset];
    }

    while( *(cloneText + targetOffset) != '\r' && *(cloneText + targetOffset) != '\n' && *(cloneText + targetOffset) != '\0') {
        *(text + replaceOffset) = *(cloneText + targetOffset);
        replaceOffset++;
        targetOffset++;
    }

    *(text + replaceOffset) = '\0';

}
