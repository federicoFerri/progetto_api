#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct TapeCell {
	char data;
	struct TapeCell* left;
	struct TapeCell* right;
};

struct Transition {
	int start_state;
	int end_state;
	char expected_char;
	char write_char;
	char direction;
	struct Transition* next_transition;
};

struct TerminalState {
    int state;
    struct TerminalState* next_terminal_state;
};

struct Process {
    int state;
    int steps;
    struct TapeCell* tape;
};

struct Transition* transitions = NULL;
struct TerminalState* terminal_states = NULL;
int steps_limit = 0;

/*
Crea una nuova cella del nastro e restituisce la cella creata con il dato caricato all'interno e i puntatori già aggiustati a NULL
*/
struct TapeCell* create_new_cell() {
	struct TapeCell* new_cell = (struct TapeCell*)malloc(sizeof(struct TapeCell));
	new_cell->left = NULL;
	new_cell->right = NULL;
	return new_cell;
}

/*
Esegui la mossa indicata sul nastro
*/
void move(struct TapeCell** tape, char direction, char data) {
    switch (direction) {
        case 'L':
            if ((*tape)->left == NULL) {
                struct TapeCell* new_cell = create_new_cell();
                (*tape)->left = new_cell;
                new_cell->right = (*tape);
                (*tape) = new_cell;
            } else {
                (*tape) = (*tape)->left;
            }
        break;
        case 'S':
            /* do nothing */
        break;
        case 'R':
            if ((*tape)->right == NULL) {
                struct TapeCell* new_cell = create_new_cell();
                (*tape)->right = new_cell;
                new_cell->left = (*tape);
                (*tape) = new_cell;
            } else {
                (*tape) = (*tape)->right;
            }
        break;
    }
    (*tape)->data = data;
}

/*
Carica transizione da file, data la linea
*/
void load_transition(char* line) {
    struct Transition* new_transition = (struct Transition*)malloc(sizeof(struct Transition));
    sscanf(line, "%d %c %c %c %d\n", &new_transition->start_state, &new_transition->expected_char, &new_transition->write_char, &new_transition->direction, &new_transition->end_state);
    if (transitions == NULL) new_transition->next_transition = NULL;
    else new_transition->next_transition = transitions;
    transitions = new_transition;
}

/*
Carica stato terminale da file, data la linea
*/
void load_terminal_state(char* line) {
    struct TerminalState* new_terminal_state = (struct TerminalState*)malloc(sizeof(struct TerminalState));
    sscanf(line, "%d\n", &new_terminal_state->state);
    if (terminal_states == NULL) new_terminal_state->next_terminal_state = NULL;
    else new_terminal_state->next_terminal_state = terminal_states;
    terminal_states = new_terminal_state;
}

/*
Carica la stringa da verificare nel nastro, data la linea
*/
struct TapeCell* preload_tape(char* line, int length) {
    struct TapeCell* first_cell = create_new_cell();
    first_cell->data = line[0];
    struct TapeCell* left_cell = first_cell;
    for (int c=1; c<length; c++) {
        if (line[c] != '\n') {
            struct TapeCell* new_cell = create_new_cell();
            new_cell->data = line[c];
            new_cell->left = left_cell;
            left_cell->right = new_cell;
            left_cell = new_cell;
        }
    }
    return first_cell;
}

/*
Esegui il process
*/
void launch_execution(char* line, int length) {
    struct TapeCell* tape = preload_tape(line, length);
    struct Process* main_process = (struct Process*)malloc(sizeof(struct Process));
    main_process->state = 0;
    main_process->steps = 0;
    main_process->tape = tape;
    /*append to processes*/
    struct Process* processes = main_process;

}

/*
Crea e carica il nastro con i valori da file
*/
void load_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    int length = 1;
    char mode = ' ';
    while (length > 0) {
        char* line = NULL;
        size_t size = 0;
        length = getline(&line, &size, f);
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
                launch_execution(line, length);
            break;
        }
    }
    fclose(f);
}

int main(int argc, char* argv[]) {
    const char* filename = argv[1];
    load_file(filename);
    return 0;
}
