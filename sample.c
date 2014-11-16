// ================================================================
// タイピングソフト用入力チェックモジュール使用サンプル
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
#include <string.h>

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "typingDiff.h"




// ================================================================
// プロトタイプ
// ================================================================
void initalizeMain(void);
void finalizeMain(void);
char getChar(void);




// ================================================================
// 静的グローバル変数
// ================================================================
static struct termio tty_backup; // tty状態を保持、終了時に復元




// ================================================================
// 処理
// ================================================================
int main(void)
{
    FILE *fp; // タイピングするキーワードを記載したテキストをREAD

    char in_char = 0; // キーボードから入力されたキーを保持

    char text[TYPINGDIFF_STRLENGTH]; // タイピングするキーワードの原文
    char type_word[TYPINGDIFF_STRLENGTH]; // タイピングするキーワードの平仮名
    char type[TYPINGDIFF_STRLENGTH]; // タイピングされた文字列を保持
    char fullstr[TYPINGDIFF_STRLENGTH]; // タイピングするべき文字の取得先
    int true_len = 0; // タイピングした文字列の正しい文字数の取得先




    initalizeMain();




    if( ( fp = fopen("intext.txt", "r") ) == NULL ) {
        return 1;
    }




    /* タイピングのサンプル */
    while( fgets(text, 1024, fp) && fgets(type_word, 1024, fp) ) {

        // タイピングキーワード毎の初期化
        type[0] = '\0';
        true_len = 0;

        printf("\n%s", text);
        printf("%s", type_word);

        /* タイピングの入力チェック(キー入力確認) */      
        while((in_char = getChar()) != 0x0a) {

            // 何も入力されていなければ入力処理をしない
            if(in_char == 0){continue;}

            // 入力された文字を代入(最後に'\0'を入れること！)
            type[true_len]   = in_char;
            type[true_len+1] = '\0';

            // 入力された内容のうち何文字目まで正しいか判定、結果文字数の取得
            true_len = getTypingDiff(type_word, type, fullstr);

            printf("\r%d : %s", true_len, type);
            fflush(NULL);

            // -1(タイピングする文字が完全一致)なら次の処理へ
            if(true_len == -1) {
                break;
            }

        }

    }




    finalizeMain();




    return 0;

}

// ----------------------------------------------------------------
// アプリケーションの初期化
// ----------------------------------------------------------------
void initalizeMain(void)
{
    struct termio tty_change;

    /* 現在のtty設定を保持 */
    ioctl(0, TCGETA, &tty_backup);
    tty_change = tty_backup;

    /* tty設定を変更、反映 */
    tty_change.c_lflag &= ~(ECHO | ICANON); /* エコーを止め、RAW モードへ変更 */
    tty_change.c_cc[VMIN] = 0;
    tty_change.c_cc[VTIME] = 1;
    ioctl(0, TCSETAF, &tty_change);

    typingDiffInit();
}




// ----------------------------------------------------------------
// アプリケーションの終了
// ----------------------------------------------------------------
void finalizeMain(void)
{
    // 変更していたtty設定を復元
    ioctl(0, TCSETAF, &tty_backup);

    // タイピング入力チェックモジュールの終了
    typingDiffFinal();
}




// ----------------------------------------------------------------
// 入力されているキーを返す
// ----------------------------------------------------------------
char getChar(void)
{
    char in_char = 0;
    char readed_byte = 0;

    readed_byte = read(0, &in_char, 1);
    if(readed_byte == -1) {
        ioctl(0, TCSETAF, &tty_backup);
        in_char = 0x0a;
    }

    return in_char;
}
