#ifndef GAME_WORDLE_WORDS_H
#define GAME_WORDLE_WORDS_H

extern const char wordle_word_bank_5[2315][5];
extern const char wordle_word_valid_5[12972][5];

#define wordle_word_bank	wordle_word_bank_5
#define wordle_word_valid	wordle_word_valid_5

#define WORDLE_WORD_BANK_5_LENGTH	2315
#define WORDLE_WORD_VALID_5_LENGTH	12972

#define WORDLE_WORD_BANK_LENGTH		WORDLE_WORD_BANK_5_LENGTH
#define WORDLE_WORD_VALID_LENGTH	WORDLE_WORD_VALID_5_LENGTH

#endif /* GAME_WORDLE_WORDS_H */