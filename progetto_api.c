#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define TAPE_BLOCK_SIZE 100
#define TERMINAL_ARRAY_SIZE 10
#define TRANSITION_HASH_SIZE 20

struct TapeCell {
	char data[TAPE_BLOCK_SIZE];
	int position;
	struct TapeCell* left;
	struct TapeCell* right;
};

struct Transition {
    int start_state;
    char match_char;
	int goto_state;
	char write_char;
	char direction;
	struct Transition* next_transition;
};

struct TerminalStates {
    int* states;
    int size;
};

struct Process {
    int state;
    int steps;
    struct TapeCell* tape;
};

struct Transition* transitions[TRANSITION_HASH_SIZE];
struct TerminalStates* terminal_states = NULL;
int steps_limit = 0;

/*
Leggi dal nastro
*/
char read_tape(struct TapeCell** tape) {
    return (*tape)->data[(*tape)->position];
}

/*
Scrivi sul nastro e spostati
*/
void write_and_move_tape(struct TapeCell** tape, char direction, char data) {
    (*tape)->data[(*tape)->position] = data;
    switch (direction) {
        case 'L':
            if ((*tape)->position <= 0) {
                if ((*tape)->left == NULL) {
                    // alloca in memoria la nuova cella del nastro
                    struct TapeCell* new_cell = (struct TapeCell*)malloc(sizeof(struct TapeCell));
                    // inizializza la nuova cella con tutti blank
                    for (int i = 0; i < TAPE_BLOCK_SIZE; i++) new_cell->data[i] = '_';
                    // setta il pointer non usato a NULL e la posizione corretta per un futuro uso
                    new_cell->left = NULL;
                    new_cell->position = TAPE_BLOCK_SIZE - 1;
                    // aggancia la nuova cella alle precendenti
                    (*tape)->left = new_cell;
                    new_cell->right = (*tape);
                    // rendi la nuova cella, la cella di default
                    (*tape) = new_cell;
                } else {
                    (*tape) = (*tape)->left;
                }
            } else (*tape)->position--;
        break;
        case 'S':
            // non fare nulla
        break;
        case 'R':
            if ((*tape)->position >= TAPE_BLOCK_SIZE - 1) {
                if ((*tape)->right == NULL) {
                    // uguale al caso sinistro
                    struct TapeCell* new_cell = (struct TapeCell*)malloc(sizeof(struct TapeCell));
                    for (int i = 0; i < TAPE_BLOCK_SIZE; i++) new_cell->data[i] = '_';
                    new_cell->right = NULL;
                    new_cell->position = 0;
                    (*tape)->right = new_cell;
                    new_cell->left = (*tape);
                    (*tape) = new_cell;
                } else {
                    (*tape) = (*tape)->right;
                }
            } else (*tape)->position++;
        break;
    }
}

/*
Controlla se uno stato è finale
*/
int is_state_terminal(int state) {
    for (int i = 0; i < terminal_states->size; i++) {
        if (state == terminal_states->states[i]) return 1;
    }
    return 0;
}

/*
Genera hash(int) da una stringa
*/
int generate_hash(char *key) {
	unsigned long int hashval;
	int i = 0;
	while (hashval < ULONG_MAX && i < strlen(key)) {
        hashval = hashval << 8;
		hashval += key[i];
		i++;
	}
	return hashval % TRANSITION_HASH_SIZE;
}

/*
Carica transizione da file, data la linea
*/
void load_transition(char* line) {
    struct Transition* new_transition = (struct Transition*)malloc(sizeof(struct Transition));
    sscanf(line, "%d %c %c %c %d\n", &new_transition->start_state, &new_transition->match_char, &new_transition->write_char, &new_transition->direction, &new_transition->goto_state);
    char key[5];
    snprintf(key, 5,"%c%d", new_transition->match_char, new_transition->start_state);
    int index = generate_hash(key);
    new_transition->next_transition = transitions[index];
    transitions[index] = new_transition;
}

/*
Carica stato terminale da file, data la linea
*/
void load_terminal_state(char* line) {
    if (terminal_states == NULL) {
        struct TerminalStates* new_ts = (struct TerminalStates*)malloc(sizeof(struct TerminalStates));
        new_ts->size = 0;
        new_ts->states = malloc(TERMINAL_ARRAY_SIZE * sizeof(int));
        terminal_states = new_ts;
    }
    if (terminal_states->size > ((terminal_states->size / TERMINAL_ARRAY_SIZE) + 1) * TERMINAL_ARRAY_SIZE) {
        terminal_states->states = realloc(terminal_states->states, ((terminal_states->size / TERMINAL_ARRAY_SIZE) + 1) * TERMINAL_ARRAY_SIZE * sizeof(int));
    }
    sscanf(line, "%d\n", &terminal_states->states[terminal_states->size]);
    terminal_states->size++;
}

/*
Carica la stringa da verificare nel nastro, data la linea
*/
struct TapeCell* preload_tape(char* line) {
    struct TapeCell* main_cell = (struct TapeCell*)malloc(sizeof(struct TapeCell));
    main_cell->position = 0;
    main_cell->left = NULL;
    struct TapeCell* curr_cell = main_cell;
    int position = 0;
    for (int c = 0; c < strlen(line) - 1; c++) {
        if (position >= TAPE_BLOCK_SIZE - 1) {
            struct TapeCell* new_cell = (struct TapeCell*)malloc(sizeof(struct TapeCell));
            new_cell->position = 0;
            new_cell->left = curr_cell;
            curr_cell->right = new_cell;
            curr_cell = new_cell;
            position = 0;
        }
        curr_cell->data[curr_cell->position] = line[c];
    }
    for (int i = position; i < TAPE_BLOCK_SIZE; i++) curr_cell->data[i] = '_';
    curr_cell->right = NULL;
    return main_cell;
}

/*
Esegui il process
*/
void launch_execution(char* line) {
    if (strlen(line) > 0) {
        struct TapeCell* tape = preload_tape(line);
        return;
        struct Process* main_process = (struct Process*)malloc(sizeof(struct Process));
        main_process->state = 0;
        main_process->steps = 0;
        main_process->tape = tape;
        /*append to processes*/
        struct Process* processes = main_process;
    }
}

/*
Crea e carica il nastro con i valori da file
*/
void load_stdin() {
    // inizializza le strutture dati
    for (int i = 0; i < TRANSITION_HASH_SIZE; i++) transitions[i] = NULL;
    // inizia lettura
    int length = 1;
    char mode = ' ';
    while (length > 0) {
        char* line = NULL;
        size_t size = 0;
        length = getline(&line, &size, stdin);
        if (strcmp(line, "tr\n") == 0) {
            mode = 't';
            continue;
        }
        if (strcmp(line, "acc\n") == 0) {
            mode = 'a';
            continue;
        }
        if (strcmp(line, "max\n") == 0) {
            mode = 'm';
            continue;
        }
        if (strcmp(line, "run\n") == 0) {
            mode = 'r';
            continue;
        }
        switch (mode) {
            case 't':
                load_transition(line);
            break;
            case 'a':
                load_terminal_state(line);
            break;
            case 'm':
                sscanf(line, "%d\n", &steps_limit);
            break;
            case 'r':
                launch_execution(line);
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    load_stdin();
    return 0;
}
