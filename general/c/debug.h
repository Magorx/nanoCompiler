#ifndef GENERAL_DEBUG
#define GENERAL_DEBUG

//=============================================================================
//<KCTF> Debug ================================================================

#define KCTF_DEBUG_LEVEL 0 ///< Just a mode for debugging

int           DEBUG_NUMBER = 1;   ///< Just an int for debugging
unsigned char DEBUG_LETTER = 'a'; ///< Just a char for debugging

#define DEBUG_NUMBER_PRINT() printf("[deb] %d [deb]\n", DEBUG_NUMBER++);
#define DEBUG_LETTER_PRINT() printf("[deb] %c [deb]\n", DEBUG_LETTER++);

#define DEBUG_PRINTF() DEBUG_NUMBER_PRINT();
#define DP DEBUG_PRINTF();

#define DEBUG_NUMBER_INCR() DEBUG_NUMBER++;
#define DEBUG_LETTER_INCR() DEBUG_LETTER++;

#define DEBUG(LEVEL) if (LEVEL <= KCTF_DEBUG_LEVEL)

#endif // GENERAL_DEBUG