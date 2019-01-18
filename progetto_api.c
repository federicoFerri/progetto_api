#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define TAPE_BLOCK_SIZE 1000
#define TERMINAL_ARRAY_SIZE 100
#define MAX_PROCESS_STEPS 100
#define LINEAR_THRESHOLD 5

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
    struct Process* prev;
    struct Process* next;
};

struct Transition*** transitions = NULL;
int min_state, max_state;
char min_char, max_char;
struct TerminalStates* terminal_states = NULL;
int steps_limit = 0;

/*
Leggi dal nastro
*/
char read_tape(struct TapeCell* tape) {
    return tape->data[tape->position];
}

/*
Scrivi sul nastro e spostati
*/
void write_and_move_tape(struct TapeCell** tape, char write_char, char direction) {
    (*tape)->data[(*tape)->position] = write_char;
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
Copia il nastro in un nuovo nastro e restituiscilo
*/
struct TapeCell* make_tape_copy(struct TapeCell* source_tape) {
    struct TapeCell* copy_tape = (struct TapeCell*)malloc(sizeof(struct TapeCell));
    copy_tape->position = source_tape->position;
    for (int i = 0; i < TAPE_BLOCK_SIZE; i++) copy_tape->data[i] = source_tape->data[i];
    // copia lato sinistro nastro
    struct TapeCell* tmp_source_tape = source_tape->left;
    struct TapeCell* tmp_copy_tape = copy_tape;
    while (tmp_source_tape != NULL) {
        struct TapeCell* new_cell = (struct TapeCell*)malloc(sizeof(struct TapeCell));
        new_cell->position = tmp_source_tape->position;
        for (int i = 0; i < TAPE_BLOCK_SIZE; i++) new_cell->data[i] = tmp_source_tape->data[i];
        new_cell->right = tmp_copy_tape;
        tmp_copy_tape->left = new_cell;
        tmp_source_tape = tmp_source_tape->left;
        tmp_copy_tape = new_cell;
    }
    tmp_copy_tape->left = NULL;
    // copia lato destro nastro
    tmp_source_tape = source_tape->right;
    tmp_copy_tape = copy_tape;
    while (tmp_source_tape != NULL) {
        struct TapeCell* new_cell = (struct TapeCell*)malloc(sizeof(struct TapeCell));
        new_cell->position = tmp_source_tape->position;
        for (int i = 0; i < TAPE_BLOCK_SIZE; i++) new_cell->data[i] = tmp_source_tape->data[i];
        new_cell->left = tmp_copy_tape;
        tmp_copy_tape->right = new_cell;
        tmp_source_tape = tmp_source_tape->right;
        tmp_copy_tape = new_cell;
    }
    tmp_copy_tape->right = NULL;
    return copy_tape;
}

/*
Libera la memoria usata da un nastro
*/
void free_tape(struct TapeCell* tape) {
    struct TapeCell* tape_copy = tape->left;
    while (tape_copy != NULL) {
        struct TapeCell* tmp = tape_copy;
        tape_copy = tape_copy->left;
        free(tmp);
    }
    tape_copy = tape->right;
    while (tape_copy != NULL) {
        struct TapeCell* tmp = tape_copy;
        tape_copy = tape_copy->right;
        free(tmp);
    }
    free(tape);
}

/*
Esegui il processo
*/
void run_process(struct Process* process) {
    int start_state;
    char match_char;
    int start_steps;
    int one_stalled = 0;
    int max_process_steps;
    int concurrent_executions = 0;
    while (1) {
        start_steps = process->steps;
        while (concurrent_executions < LINEAR_THRESHOLD || process->steps - start_steps < MAX_PROCESS_STEPS) {
            // se i passi sono al limite o oltre, esci
            if (is_state_terminal(process->state)) {
                printf("1\n");
                struct Process* curr = process->next;
                while (curr->next != process) {
                    struct Process* tmp = curr;
                    free_tape(tmp->tape);
                    free(tmp);
                    curr = curr->next;
                }
                free_tape(process->tape);
                free(process);
                return;
            }
            if (process->steps >= steps_limit) {
                one_stalled = 1;
                if (process == process->next) {
                    printf("U\n");
                    free_tape(process->tape);
                    free(process);
                    return;
                } else {
                    struct Process* prev = process->prev;
                    struct Process* next = process->next;
                    prev->next = next;
                    next->prev = prev;
                    struct Process* tmp = process;
                    free_tape(tmp->tape);
                    free(tmp);
                    process = next;
                    break;
                }
            }
            match_char = read_tape(process->tape);
            start_state = process->state;
            struct Transition* available_transitions;
            if (start_state < min_state || start_state > max_state || match_char < min_char || match_char > max_char) {
                available_transitions = NULL;
            } else {
                available_transitions = transitions[start_state - min_state][match_char - min_char];
            }
            struct Transition* first_matching_transition = NULL;
            while (available_transitions != NULL) {
                if (first_matching_transition == NULL) {
                    // conservo la prima transizione così da poterla applicare dopo aver copiato i nastri
                    first_matching_transition = available_transitions;
                } else {
                    // qui eseguo il "fork", creando un nuovo processo
                    struct Process* new_process = (struct Process*)malloc(sizeof(struct Process));
                    new_process->tape = make_tape_copy(process->tape);
                    write_and_move_tape(&new_process->tape, available_transitions->write_char, available_transitions->direction);
                    new_process->state = available_transitions->goto_state;
                    new_process->steps = process->steps + 1;
                    // linka a precendenti processi
                    struct Process* next = process->next;
                    new_process->prev = process;
                    new_process->next = next;
                    next->prev = new_process;
                    process->next = new_process;
                    concurrent_executions++;
                }
                available_transitions = available_transitions->next_transition;
            }
            // se non trova nessuna transizione valida, esci
            if (first_matching_transition == NULL) {
                if (process == process->next) {
                    if (one_stalled) printf("U\n"); else printf("0\n");
                    free_tape(process->tape);
                    free(process);
                    return;
                } else {
                    struct Process* prev = process->prev;
                    struct Process* next = process->next;
                    prev->next = next;
                    next->prev = prev;
                    struct Process* tmp = process;
                    free_tape(tmp->tape);
                    free(tmp);
                    process = next;
                    break;
                }
            }
            // applica la transizione
            write_and_move_tape(&process->tape, first_matching_transition->write_char, first_matching_transition->direction);
            process->state = first_matching_transition->goto_state;
            process->steps++;
        }
        // ritorna un timeout per raggiungimento MAX_STEPS_PROCESS
        process = process->next;
    }
}

/*
Libera la memoria usata per definire transizioni, stati finali etc
*/
void free_static_data() {
    // libera transizioni
    for (int i = 0; i < (max_state - min_state + 1); i++) {
        for (int j = 0; j < (max_char - min_char + 1); j++) {
            while (transitions[i][j] != NULL) {
                struct Transition* tmp_tr = transitions[i][j];
                transitions[i][j] = transitions[i][j]->next_transition;
                free(tmp_tr);
            }
        }
        free(transitions[i]);
    }
    free(transitions);
    // libera stati terminali
    free(terminal_states->states);
    free(terminal_states);
}

/*
Carica transizione da file, data la linea
*/
void load_transitions() {
    int start_state, goto_state, read_status;
    char match_char, write_char, direction;
    struct Transition* all_transitions = NULL;
    struct Transition* new_transition;
    read_status = scanf("%d %c %c %c %d\n", &start_state, &match_char, &write_char, &direction, &goto_state);
    min_state = start_state;
    max_state = start_state;
    min_char = match_char;
    max_char = match_char;
    while (read_status) {
        if (start_state < min_state) min_state = start_state;
        if (start_state > max_state) max_state = start_state;
        if (match_char < min_char) min_char = match_char;
        if (match_char > max_char) max_char = match_char;
        new_transition = (struct Transition*)malloc(sizeof(struct Transition));
        new_transition->start_state = start_state;
        new_transition->match_char = match_char;
        new_transition->write_char = write_char;
        new_transition->direction = direction;
        new_transition->goto_state = goto_state;
        new_transition->next_transition = all_transitions;
        all_transitions = new_transition;
        read_status = scanf("%d %c %c %c %d\n", &start_state, &match_char, &write_char, &direction, &goto_state);
    }
    transitions = (struct Transition***)malloc((max_state - min_state + 1) * sizeof(struct Transition**));
    for (int i = 0; i < (max_state - min_state + 1); i++) {
        transitions[i] = (struct Transition**)malloc((max_char - min_char + 1) * sizeof(struct Transition*));
        for (int j = 0; j < (max_char - min_char + 1); j++) {
            transitions[i][j] = NULL;
        }
    }
    while (all_transitions != NULL) {
        new_transition = all_transitions;
        all_transitions = all_transitions->next_transition;
        new_transition->next_transition = transitions[new_transition->start_state - min_state][new_transition->match_char - min_char];
        transitions[new_transition->start_state - min_state][new_transition->match_char - min_char] = new_transition;
    }
    return;
}

/*
Carica stato terminale da file, data la linea
*/
void load_terminal_states() {
    int state, read_status;
    terminal_states = (struct TerminalStates*)malloc(sizeof(struct TerminalStates));
    terminal_states->size = 0;
    terminal_states->states = malloc(TERMINAL_ARRAY_SIZE * sizeof(int));
    read_status = scanf("%d\n", &state);
    while (read_status) {
        if (terminal_states->size != 0 && terminal_states->size % TERMINAL_ARRAY_SIZE == 0) {
            terminal_states->states = realloc(terminal_states->states, (terminal_states->size + TERMINAL_ARRAY_SIZE) * sizeof(int));
        }
        terminal_states->states[terminal_states->size] = state;
        terminal_states->size++;
        read_status = scanf("%d\n", &state);
    }
}

/*
Carica la stringa da verificare nel nastro, data la linea
*/
struct TapeCell* preload_tape() {
    struct TapeCell* main_cell = (struct TapeCell*)malloc(sizeof(struct TapeCell));
    main_cell->position = TAPE_BLOCK_SIZE / 2;
    main_cell->left = NULL;
    struct TapeCell* curr_cell = main_cell;
    int position = TAPE_BLOCK_SIZE / 2;
    char current_char;
    for (int i = 0; i < TAPE_BLOCK_SIZE / 2; i++) curr_cell->data[i] = '_';
    while ((current_char = fgetc(stdin)) != '\n' && current_char != EOF) {
        if (position >= TAPE_BLOCK_SIZE) {
            struct TapeCell* new_cell = (struct TapeCell*)malloc(sizeof(struct TapeCell));
            new_cell->position = 0;
            new_cell->left = curr_cell;
            curr_cell->right = new_cell;
            curr_cell = new_cell;
            position = 0;
        }
        curr_cell->data[position] = current_char;
        position++;
    }
    for (int i = position; i < TAPE_BLOCK_SIZE; i++) curr_cell->data[i] = '_';
    curr_cell->right = NULL;
    return main_cell;
}

/*
Crea il processo
*/
void create_executions() {
    char current_char;
    while (!feof(stdin) && (current_char = fgetc(stdin)) != EOF && ungetc(current_char, stdin)) {
        struct TapeCell* tape = preload_tape();
        struct Process* processes = (struct Process*)malloc(sizeof(struct Process));
        processes->state = 0;
        processes->steps = 0;
        processes->tape = tape;
        processes->prev = processes;
        processes->next = processes;
        run_process(processes);
    }
}

/*
Crea e carica il nastro con i valori da file
*/
void load_stdin() {
    // leggi file
    char mode[5];
    fgets(mode,5,stdin);
    load_transitions();
    fgets(mode,5,stdin);
    load_terminal_states();
    fgets(mode,5,stdin);
    scanf("%d\n", &steps_limit);
    fgets(mode,5,stdin);
    create_executions();
}

int main(int argc, char* argv[]) {
    load_stdin();
    free_static_data();
    return 0;
}
