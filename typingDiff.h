#ifndef DEF_TYPINGDIFF_H

int typingDiffInit(void);
int typingDiffFinal(void);
int getTypingDiff(char*, char*, char*);

// バッファオーバーフロー対策
#define TYPINGDIFF_STRLENGTH   2048

#endif
