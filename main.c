//  wordleGuessSecretWord.c
//  Have the program do the guessing to discover the secret wordle word.
//
//  Author: Dale Reed, 11/6/22
//  System: CLion and XCode
//
//  Links to wordle dictionary words at:
//    https://www.reddit.com/r/wordle/comments/s4tcw8/a_note_on_wordles_word_list/
/*  Running the program looks like:
        Using file wordsLarge.txt with 12947 words.
        -----------------------------------------------------------

        enter a secret word or just r to choose one at random: dream
        Trying to find secret word:
               d r e a m

            1. s o r e s
                   * *
            2. s e r e r
                 * *
            3. r a r e r
               * *   *
            4. r a r e e
               * *   *
            5. s a r e e
                 * * *
            6. p a r e r
                 * * *
            7. c a r e r
                 * * *
            8. b a r e r
                 * * *
            9. f a r e r
                 * * *
           10. D a r e r
                 * * *
           11. D e a r e
                 * * *
           12. D e b A r
                 *     *
           13. D R E A r
                       *
           14. D R E A d

           15. D R E A M

        Got it!

     ... and then it runs two more times ...
 */
#include <stdio.h>    // for printf(), scanf()
#include <stdlib.h>   // for exit( -1)
#include <string.h>   // for strcpy
#include <assert.h>   // for assert() sanity checks
#include <ctype.h>    // for toupper()
#include <time.h>     // for time()

// Declare globals
#define WORD_LENGTH 5     // All words have 5 letters, + 1 NULL at the end when stored
//#define WORDS_FILE_NAME "wordsLarge.txt"
#define WORDS_FILE_NAME  "wordsTiny.txt"
#define MAX_NUMBER_OF_WORDS 12947   // Number of words in the full set of words file
#define true 1   // Make boolean logic easier to understand
#define false 0  // Make boolean logic easier to understand

/*
 * struct: wordCountStruct
 * member variables: (char[6]) word, (int) score
 * word: an accepted word of 5 letters, with a null string char
 * score: score to be assigned to each word relative to how 'good' it is to be a guess word.
 */
typedef struct wordCount wordCountStruct;
struct wordCount{
    char word[ WORD_LENGTH + 1];   // The word length plus NULL
    int score;                     // Score for the word
};

typedef struct letterCount letterCountStruct;
struct letterCount{
    char letter;          // The letter character
    int appearances;      // Appearance count of the letter
};
//-----------------------------------------------------------------------------------------
// Comparator for use in built-in qsort(..) function.  Parameters are declared to be a
// generic type, so they will match with anything.
// This is a two-part comparison.  First the scores are compared.  If they are the same,
// then the words themselves are also compared, so that the results are in descending
// order by score, and within score they are in alphabetic order.
int compareFunction( const void * a, const void * b) {
    // Before using parameters we have cast them into the actual type they are in our program
    // and then extract the numerical value used in comparison
    int firstScore = ((wordCountStruct *) a)->score;
    int secondScore = ((wordCountStruct *) b)->score;

    // If scores are different, then that's all we need for our comparison.
    if (firstScore != secondScore) {
        // We reverse the values, so the result is in descending vs. the otherwise ascending order
        // return firstScore - secondScore;   // ascending order
        return secondScore - firstScore;      // descending order
    }
    else {
        // Scores are equal, so check words themselves, to put them in alphabetical order
        return strcmp( ((wordCountStruct *)a)->word,  ((wordCountStruct *)b)->word );
    }
} //end compareFunction(..)

int compareFunctionLetter( const void * a, const void * b) {
    // Before using parameters we have cast them into the actual type they are in our program
    // and then extract the numerical value used in comparison
    int firstScore = ((letterCountStruct *) a)->appearances;
    int secondScore = ((letterCountStruct *) b)->appearances;

    // If scores are different, then that's all we need for our comparison.
    if (firstScore != secondScore) {
        // We reverse the values, so the result is in descending vs. the otherwise ascending order
        // return firstScore - secondScore;   // ascending order
        return secondScore - firstScore;      // descending order
    }
    else {
        // Scores are equal, so check words themselves, to put them in alphabetical order
        return strcmp( ((letterCountStruct *) a)->letter,  ((letterCountStruct *) b)->letter );
    }
} //end compareFunction(..)

/*
 * Copy an array of wordCountStruct, in particular, a dynamically allocated array, granted the size is known (or specifically,
 * the first amount of objects to be copied is known)
 * Param: (wordCountStruct*) source array of wordCountStruct to copy from (a pointer copy), (int) how many first
 * wordCountStruct objects to be copied from the source array.
 * Output: Return the pointer to the dynamically allocated memory of the array post-copying.
 */
wordCountStruct *wordStructArrayCopy(wordCountStruct *src, int size) {
    int i = 0;
    wordCountStruct *dest;
    // malloc was used here, must free after this copying if used elsewhere
    // than main (in which frees are automatically called, but should be called for good practice).
    dest = (wordCountStruct *)malloc(sizeof(wordCountStruct) * size);
    for (; i < size; i++) {
        // to copy wordCountStruct is to create another object with its variables - word and score, being the same as the source.
        strcpy((dest + i)->word, (src + i)->word);
        (dest + i)->score = (src + i)->score;
    }
    return dest;
}

//-----------------------------------------------------------------------------------------
// Read in words from file into array.  We've previously read the data file once to
// find out how many words are in the file.
void readWordsFromFile(
        char fileName[],        // Filename we'll read from
        wordCountStruct *words, // Array of words where we'll store words we read from file
        int *wordCount)          // How many words.  Gets updated here and returned
{
    FILE *inFilePtr  = fopen(fileName, "r");  // Connect logical name to filename
    assert( inFilePtr != NULL);               // Ensure file open worked correctly

    // Read each word from file and store into array, initializing the score for that word to 0.
    char inputString[ 6];
    *wordCount = 0;
    while( fscanf( inFilePtr, "%s", inputString) != EOF) {
        strcpy( words[ *wordCount].word, inputString);
        words[ *wordCount].score = 0;
        (*wordCount)++;
    }

    // Close the file
    fclose( inFilePtr);
} // end readWordsFromFile(..)

void scoreReset(wordCountStruct allWords[], int counter) {
    int i = 0;
    for (; i < counter; i++) {
        (allWords+i)->score = 0;
    }
    qsort(allWords, counter, sizeof(wordCountStruct), compareFunction);
}

/*
 * Take a word of reference and a word that is choice of guess, assign score to how well-matched the guess word was
 * to the reference word. With proper precedent,
 * assign 3 points to a perfectly matched letter of the same position
 * after assigning all the 3-point possible, assign 1 point if there exists letters matching (at this point, letters are
 * guaranteed to not be of the same location, so 1-point assignment well-defined).
 * Param: (char*) word of reference to calculate score on, (char*) word that is guessing to be compared to word of reference
 * Output: Score of guess word relative to the reference word.
 */
int scoreAssigning(char *wordRef, char *wordGuess) {
    int scoreAssigned = 0;
    int k = 0;
    // first assign the 3-points, match index by index of string, then blank out to indicate the letter not to be counted for
    // for the loop assigning 1-points.
    for (; k < 5; k++) {
        if (wordGuess[k] == wordRef[k]) {
            scoreAssigned += 3;
            // blanking out used different letters to differentiate (the guess word's 'letter' should not still be found)
            wordRef[k] = ' ';
            wordGuess[k] = '-';
        }
    }
    k = 0; // double loop since precise location should take precedent, no
    // way to ensure that if implementing parallelly
    for (; k < 5; k++) {
        if (strchr(wordRef, wordGuess[k])) {
            scoreAssigned += 1;
            *(strchr(wordRef, wordGuess[k])) = ' ';
            // don't need to blank pos k of guess word since this is
            // the last time it is encountered.
        }
    }
    return scoreAssigned;
}

/*
 * Compute scores of however many words indicated by size, starting from a certain word in the wordCountStruct array.
 * Scores are computed relative to the array of answers (with a specified amount of answers of consideration).
 * Param: (wordCountStruct*) the pointer at the beginning of the wordCountStruct array in consideration for score assignment
 * (wordCountStruct*) the pointer at the beginning of the array of answers to compare all words to for scores, (int)
 * amount of answer words of consideration, (int) amount of words to have scores computed
 */
void scoreCompute(wordCountStruct *begin, wordCountStruct *answerBegin,
                  int answersCounter, int size) {
    int i = 0;
    for (; i < size; i++) {
        int j = 0;
        int pointAccumulated = 0;
        for (; j < answersCounter; j++) {
            // the process of computing scores demand mutating the words, but since a word is computed scores multiple times,
            // the words should not be permanently mutated, so operate this score assigning on a copy of the answer/guess word instead.
            char wordOfReference[6];
            char wordOfCurrentGuess[6];
            strcpy(wordOfReference, (answerBegin+j)->word);
            strcpy(wordOfCurrentGuess, begin->word);
            // score of a wordCountStruct is defined to be the sum of all its scores relative to the answerWord.
            pointAccumulated += scoreAssigning(wordOfReference, wordOfCurrentGuess);
        }
        begin->score = pointAccumulated;
        begin++;
    }
}

/*
 * In consideration of the second best words, the words in the array of answerWords are mutated: the letters are blanked
 * out; thus, the function first creates a copy of the answerWords, modify the words of this copy array based on which word
 * meant to be used as a blanking out. Afterwards, utilize the score computing function to finish the rest.
 * Param: (wordCountStruct*) the pointer at the beginning of the wordCountStruct array in consideration for score assignment
 * (wordCountStruct*) the pointer at the beginning of the array of answers to compare all words to for scores, (int)
 * amount of answer words of consideration, (int) amount of words to have scores computed, (char[]) string of the word based upon which
 * to blank out all the answersWord from.
 */
void secondScoreCompute(wordCountStruct *begin, wordCountStruct *answerBegin,
                        int answersCounter, int size, char wordToRemove[]) {
    wordCountStruct* filteredWordArray = wordStructArrayCopy(answerBegin, answersCounter);
    int i = 0;
    char cpyRemoveWord[6]; //score assigning function applies onto char array, which is forced as pass by reference (due to array construction), so require a copy to not completely mutate
    for (; i < answersCounter; i ++) {
        strcpy(cpyRemoveWord, wordToRemove);
        // this function is less about assigning score, but more so to mutate the words (blanking letters)
        scoreAssigning((filteredWordArray + i)->word, cpyRemoveWord);
//        printf(" %d. %s\n", i, (filteredWordArray + i)->word); // debug: print all the words post-blanking
    }
    // compute the scores, assigning scores to each word in the word bank based on answersWords that are already blanked out at this point
    scoreCompute(begin, filteredWordArray, answersCounter, size);
    free(filteredWordArray);
}

/*
 * Composite function to parse answers, guesses from fileNames indicated, calculate how many answersWords and guessesWords are,
 * initialize the array of all wordCountStruct objects as well as just the answerWords wordCountStruct. Afterwards, immediately
 * calculate the best first word to guess, assigning scores to each of the word in the array of all words and sort based on score/alphabetically.
 * Param: (char[]) file name of all answer words, (int*) integer passed by reference to indicate how many answerWords there are,
 * (char[]) file name of all guess words, (int*) integer passed by reference to indicate how many guessesWords there are,
 * (wordCountStruct**) the pointer to the first object of the dynamically allocated array of all wordCountStruct objects
 * passed in by reference, (wordCountStruct**) the pointer to the first object of all answer words wordCountStruct objects.
 */
void parseAndCompute(wordCountStruct** allWords, int* answersCounter, int* guessesCounter, wordCountStruct** allAnswers) {
    // the space reserved for guesses in the array of all words start after all answers
    // as a consequence, all answer words are meant to belong in the first [amount of answerWords] objects of the array
    // save a copy of the answer words for later usage.
    (*allAnswers) = wordStructArrayCopy(*allWords, *answersCounter);
    scoreCompute(*allWords, *allWords, *answersCounter,
                 *answersCounter + *guessesCounter);
    // Sort the allWords array in descending order by score, and within score they
    // should also be sorted into ascending order alphabetically.  Use the built-in
    // C quick sort qsort(...).
    qsort(*allWords, *guessesCounter + *answersCounter, sizeof(wordCountStruct),
          compareFunction);
}

/*
 * Process which word is the best second word post-first best word computation. Extract all the highest scored words,
 * process second-best words in the same manner as the first-best word, but based on the answer words that had letters
 * struck out from the highest scored words, instead of full answer words.
 * Param: (wordCountStruct**) the pointer to the first object of the dynamically allocated array of all wordCountStruct objects
 * passed in by reference, which at this point is sorted based on scores relative to full-letter answerr words,
 * (wordCountStruct**) the pointer to the first object of all answer words wordCountStruct objects, (int) count of all
 * answer words, (int) count of all guess words.
 */
void bestSecondWordsProcessing(wordCountStruct** allWords, wordCountStruct** allAnswers, int answersCounter, int guessesCounter) {
    // first extract all the highest scored words, separate it into a specific array, since the array of all words
    // are going to be mutated after the consideration with the first highest scoring word.
    int highestScore = (*allWords)->score;
    int highestScoredWordsTie = 0;
    while ((*allWords + highestScoredWordsTie)->score == highestScore) {
        highestScoredWordsTie++;
    }
    wordCountStruct* highestScoredWords = wordStructArrayCopy(*allWords, highestScoredWordsTie);
    int i = 0;

    while (i < highestScoredWordsTie) {
        /* Debug:
        printf("answerWordsCopy after letters from %s removed:\n",
               (highestScoredWords + i)->word);
         */
        // compute score (second compute score) based on what word to blank out, then sort afterwards with now new scores assigned
        // relative to the answer words array, assumed to have letters struck out.
        secondScoreCompute(*allWords, *allAnswers, answersCounter, answersCounter + guessesCounter, (highestScoredWords + i)->word);
        qsort(*allWords, guessesCounter + answersCounter, sizeof(wordCountStruct),
              compareFunction);
        int j = 0;
        /* Debug: Words and their scores relative to the answer words that were blanked out by the best first word
        for (; j < answersCounter + guessesCounter; j++) {
            printf("    %s %d\n", (*allWords + j)->word, (*allWords + j)->score);
        }
         */
        printf("%s %d\n", (highestScoredWords + i)->word, highestScore);
        int highestSecondWordScore = (*allWords)->score;
        j = 0;
        while (((*allWords) + j)->score == highestSecondWordScore) {
            printf("   %s %d", ((*allWords) + j)->word, highestSecondWordScore);
            j++;
        }
        printf("\n");
        i++;
    }
    free(highestScoredWords);
}

// -----------------------------------------------------------------------------------------

int main2() {
    char answersFileName[81]; // Stores the answers file name
    char guessesFileName[81]; // Stores the guesses file name
    int answersCounter = 0;
    int guessesCounter = 0;
    // Set default file names
    // ...
    // Construct containers for all words, both guesses and answers, and all answers, for later usage of blanking out letters of answers based on "best first words"
    wordCountStruct *allWords;
    wordCountStruct *allAnswers;
    // Read in the file, count answers and guesses words, parse into the array of structs and assign scores,
    // compute best first word(s), turn array of all words into sorted order and save a copy of the answer words.
    parseAndCompute(answersFileName, &answersCounter, guessesFileName, &guessesCounter, &allWords, &allAnswers);
    printf("%s has %d words\n%s has %d words\n", answersFileName, answersCounter, guessesFileName, guessesCounter);
    printf("\nWords and scores for top first words and second words:\n");
    // if option 2, re-process the allWords array based on the best first words
    bestSecondWordsProcessing(&allWords, &allAnswers, answersCounter, guessesCounter);
    free(allWords);
    free(allAnswers);
    printf("Done\n");
    return 0;
} // end main()

void initializeLetterCountStruct(letterCountStruct* allLetters) {
    int i = 0;
    for (; i <= 122 - 97; i++) {
        allLetters->letter = i + 97;
        allLetters->appearances = 0;
        allLetters++;
    }
}

void letterCompute(wordCountStruct allWords[], letterCountStruct allLetters[], int counter) {
    //97->122
    int i = 0;
    for (; i < counter; i++) {
        if (allWords->score >= 0) {
            int j = 0;
            for (; j < 5; j++) {
                (allLetters+((allWords->word)[j] - 97))->appearances++;
            }
        }
    }
    qsort(allLetters, 26, sizeof(letterCountStruct), compareFunctionLetter);
    i = 0;
    for (; i < counter; i++) {
        if (allWords->score >= 0) {
            int j = 0;
            for (; j < 5; j++) {
                int k = 0;
                for (; k < 5; k++) {
                    if ((allWords+i)->word[j] == (allLetters+k)->letter) {
                        (allWords+i)->score+=10-k;
                        break;
                    }
                }
            }
        }
    }
    qsort(allWords, counter, sizeof(wordCountStruct), compareFunction);
}

void wordCompare()

void wordGuessAlgo(wordCountStruct allWords[], int guessCount) {
    while (guessCount < 3) {

    }

}

// -----------------------------------------------------------------------------------------
// Find a secret word
void findSecretWord(
        wordCountStruct allWords[],    // Array of all the words
        int wordCount,                  // How many words there are in allWords
        char secretWord[])              // The word to be guessed
{
    char computerGuess[ 6];  // Allocate space for the computer guess

    printf("Trying to find secret word: \n");
    // Display secret word with a space between letters, to match the guess words below.
    printf("       ");
    for( int i=0; i<WORD_LENGTH; i++) {
        printf("%c ", secretWord[ i]);
    }
    printf("\n");
    printf("\n");

    letterCountStruct* letterCommonalities;
    letterCommonalities = (letterCountStruct*) malloc(sizeof(letterCountStruct) * (122 - 97 + 1));
    // Loop until the word is found
    int guessNumber = 1;
    while(guessNumber < 3) {
        // Lots of code to go here ...
        letterCompute(allWords, wordCount, letterCommonalities);
        // ...
        printf("%5d. \n", guessNumber);

        // Update guess number
        guessNumber++;
    } //end for( int i...)
} //end findSecretWord


// -----------------------------------------------------------------------------------------
int main() {
    char wordsFileName[81];                   // Stores the answers file name
    strcpy(wordsFileName, WORDS_FILE_NAME);   // Set the filename, defined at top of program.
    srand( (unsigned) time( NULL));           // Seed the random number generator to be current time
    // Declare space for all the words, of a maximum known size.
    wordCountStruct allWords[ MAX_NUMBER_OF_WORDS];
    // Start out the wordCount to be the full number of words.  This will decrease as
    //    play progresses each time through the game.
    int wordCount = 0;
    // The secret word that the computer will try to find, plus the return character from fgets.
    char secretWord[ WORD_LENGTH + 1];
    char userInput[ 81];                // Used for menu input of secret word

    // Read in words from file, update wordCount and display information
    readWordsFromFile( wordsFileName, allWords, &wordCount);
    printf("Using file %s with %d words. \n", wordsFileName, wordCount);

    // Run the word-guessing game three times
    for( int i=0; i<3; i++) {
        // Reset secret Word
        strcpy( secretWord, "");
        // Display prompt
        printf("-----------------------------------------------------------\n");
        printf("\n");
        printf("Enter a secret word or just r to choose one at random: ");
        scanf(" %s", userInput);
        // Eliminate the return character at end or userInput if it is there
        int length = (int) strlen( userInput);
        if( userInput[ length] == '\n') {
            userInput[ length] = '\0';
        }
        strcpy( secretWord, userInput);   // Store the secret word from user input

        // If input was 'r' then choose a word at random.
        if( strlen( secretWord) <= 1) {
            // Randomly select a secret word to be guessed.
            int randomIndex = rand() % wordCount;
            strcpy( secretWord, allWords[ randomIndex].word);
        }

        // Run the game once with the current secret word
        findSecretWord( allWords, wordCount, secretWord);
    }

    printf("Done\n");
    printf("\n");
    return 0;
} // end main()
