#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define TAPE_BLOCK_SIZE 100
#define TERMINAL_ARRAY_SIZE 10
#define TRANSITION_HASH_SIZE 20
#define MAX_STEPS_PROCESS 5

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

struct Transition* transitions[TRANSITION_HASH_SIZE];
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
void write_and_move_tape(struct TapeCell* tape, char write_char, char direction) {
    tape->data[tape->position] = write_char;
    switch (direction) {
        case 'L':
            if (tape->position <= 0) {
                if (tape->left == NULL) {
                    // alloca in memoria la nuova cella del nastro
                    struct TapeCell* new_cell = (struct TapeCell*)malloc(sizeof(struct TapeCell));
                    // inizializza la nuova cella con tutti blank
                    for (int i = 0; i < TAPE_BLOCK_SIZE; i++) new_cell->data[i] = '_';
                    // setta il pointer non usato a NULL e la posizione corretta per un futuro uso
                    new_cell->left = NULL;
                    new_cell->position = TAPE_BLOCK_SIZE - 1;
                    // aggancia la nuova cella alle precendenti
                    tape->left = new_cell;
                    new_cell->right = tape;
                    // rendi la nuova cella, la cella di default
                    tape = new_cell;
                } else {
                    (tape) = tape->left;
                }
            } else tape->position--;
        break;
        case 'S':
            // non fare nulla
        break;
        case 'R':
            if (tape->position >= TAPE_BLOCK_SIZE - 1) {
                if (tape->right == NULL) {
                    // uguale al caso sinistro
                    struct TapeCell* new_cell = (struct TapeCell*)malloc(sizeof(struct TapeCell));
                    for (int i = 0; i < TAPE_BLOCK_SIZE; i++) new_cell->data[i] = '_';
                    new_cell->right = NULL;
                    new_cell->position = 0;
                    tape->right = new_cell;
                    new_cell->left = tape;
                    tape = new_cell;
                } else {
                    tape = tape->right;
                }
            } else tape->position++;
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
	unsigned long int hashval = 0;
	int i = 0;
	while (hashval < ULONG_MAX && i < strlen(key)) {
        hashval = hashval << 8;
		hashval += key[i];
		i++;
	}
	return hashval % TRANSITION_HASH_SIZE;
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
void print_tape(struct TapeCell* tape) {
    struct TapeCell* tape_copy = tape;
    while (tape_copy->left != NULL) tape_copy = tape_copy->left;
    while (tape_copy != NULL) {
        printf("|");
        for (int i = 0; i < TAPE_BLOCK_SIZE; i++) {
            if (tape_copy == tape && i == tape_copy->position) printf("\033[31;1m%c\033[0m", tape_copy->data[i]);
            else {
                if (i == tape_copy->position) printf("\033[32;1m%c\033[0m", tape_copy->data[i]);
                else printf("%c", tape_copy->data[i]);
            }
        }
        tape_copy = tape_copy->right;
    }
    printf("|\n");
}

/*
Esegui n mosse del processo
*/
char run_process(struct Process* process) {
    int start_steps = process->steps;
    while (process->steps - start_steps < MAX_STEPS_PROCESS || MAX_STEPS_PROCESS == 0) {
        // se i passi sono al limite o oltre, esci
        if (process->steps >= steps_limit) {
            return 'U';
        }
        // trova hash della transizione che ci serve
        char match_char = read_tape(process->tape);
        int start_state = process->state;
        char key[5];
        snprintf(key, 5,"%c%d", match_char, start_state);
        int index = generate_hash(key);
        // naviga le transizioni
        struct Transition* tmp_tr = transitions[index];
        struct Transition* first_matching_transition = NULL;
        while (tmp_tr != NULL) {
            if (tmp_tr->start_state == start_state && tmp_tr->match_char == match_char) {
                // se trova una transizione verso uno stato finale, esci
                if (is_state_terminal(tmp_tr->goto_state)) {
                    return '1';
                }
                if (first_matching_transition == NULL) {
                    //printf("first valid move %d->%d, %c->%c, %c\n", tmp_tr->start_state, tmp_tr->goto_state, tmp_tr->match_char, tmp_tr->write_char, tmp_tr->direction);
                    // conservo la prima transizione così da poterla applicare dopo aver copiato i nastri
                    first_matching_transition = tmp_tr;
                } else {
                    // qui eseguo il "fork", creando un nuovo processo
                    struct Process* new_process = (struct Process*)malloc(sizeof(struct Process));
                    new_process->tape = make_tape_copy(process->tape);
                    write_and_move_tape(new_process->tape, tmp_tr->write_char, tmp_tr->direction);
                    new_process->state = tmp_tr->goto_state;
                    new_process->steps = process->steps + 1;
                    // linka a precendenti processi
                    struct Process* next = process->next;
                    new_process->prev = process;
                    new_process->next = next;
                    next->prev = new_process;
                    process->next = new_process;
                }
            }
            tmp_tr = tmp_tr->next_transition;
        }
        // se non trova nessuna transizione valida, esci
        if (first_matching_transition == NULL) {
            return '0';
        }
        // applica la transizione
        write_and_move_tape(process->tape, first_matching_transition->write_char, first_matching_transition->direction);
        process->state = first_matching_transition->goto_state;
        process->steps++;
    }
    // ritorna un timeout per raggiungimento MAX_STEPS_PROCESS
    return 'T';
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
void launch_execution(struct Process* processes) {
    int one_terminated = 0;
    while (1) {
        char status = run_process(processes);
        switch (status) {
            case '1':
                printf("1\n");
                struct Process* curr = processes->next;
                while (curr->next != processes) {
                    struct Process* tmp = curr;
                    free_tape(tmp->tape);
                    free(tmp);
                    curr = curr->next;
                }
                free_tape(processes->tape);
                free(processes);
                return;
            break;
            case '0':
                if (processes == processes->next) {
                    printf("0\n");
                    free_tape(processes->tape);
                    free(processes);
                    return;
                } else {
                    one_terminated = 1;
                    struct Process* prev = processes->prev;
                    struct Process* next = processes->next;
                    prev->next = next;
                    next->prev = prev;
                    free_tape(processes->tape);
                    free(processes);
                    processes = next;
                }
            break;
            case 'U':
                if (processes == processes->next) {
                    if (one_terminated) printf("U\n");
                    else printf("0\n");
                    free_tape(processes->tape);
                    free(processes);
                    return;
                } else {
                    struct Process* prev = processes->prev;
                    struct Process* next = processes->next;
                    prev->next = next;
                    next->prev = prev;
                    struct Process* tmp = processes;
                    processes = next;
                    free_tape(tmp->tape);
                    free(tmp);
                }
            break;
            case 'T':
                // non fare nulla, semplicemente cambia processo
                processes = processes->next;
            break;
        }
    }
}

/*
Libera la memoria usata per definire transizioni, stati finali etc
*/
void free_static_data() {
    // libera hash table transizioni, non uso puntatori temporanei tanto non mi importa di perdere la testa
    for (int i = 0; i < TRANSITION_HASH_SIZE; i++) {
        while (transitions[i] != NULL) {
            struct Transition* tmp_tr = transitions[i];
            transitions[i] = transitions[i]->next_transition;
            free(tmp_tr);
        }
    }
    // libera stati terminali
    free(terminal_states->states);
    free(terminal_states);
}

/*
Carica transizione da file, data la linea
*/
int load_transitions() {
    int start_state, goto_state, read_status;
    char match_char, write_char, direction;
    read_status = scanf("%d %c %c %c %d\n", &start_state, &match_char, &write_char, &direction, &goto_state);
    while (read_status) {
        struct Transition* new_transition = (struct Transition*)malloc(sizeof(struct Transition));
        new_transition->start_state = start_state;
        new_transition->match_char = match_char;
        new_transition->write_char = write_char;
        new_transition->direction = direction;
        new_transition->goto_state = goto_state;
        char key[5];
        snprintf(key, 5,"%c%d", new_transition->match_char, new_transition->start_state);
        int index = generate_hash(key);
        new_transition->next_transition = transitions[index];
        transitions[index] = new_transition;
        read_status = scanf("%d %c %c %c %d\n", &start_state, &match_char, &write_char, &direction, &goto_state);
    }
    return 0;
}

/*
Carica stato terminale da file, data la linea
*/
void load_terminal_states() {
    int state, read_status;
    struct TerminalStates* new_ts = (struct TerminalStates*)malloc(sizeof(struct TerminalStates));
    new_ts->size = 0;
    new_ts->states = malloc(TERMINAL_ARRAY_SIZE * sizeof(int));
    terminal_states = new_ts;
    read_status = scanf("%d\n", &state);
    while (read_status) {
        if (terminal_states->size > ((terminal_states->size / TERMINAL_ARRAY_SIZE) + 1) * TERMINAL_ARRAY_SIZE) {
            terminal_states->states = realloc(terminal_states->states, ((terminal_states->size / TERMINAL_ARRAY_SIZE) + 1) * TERMINAL_ARRAY_SIZE * sizeof(int));
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
    main_cell->position = 0;
    main_cell->left = NULL;
    struct TapeCell* curr_cell = main_cell;
    int position = 0;
    char current_char;
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
    while (!feof(stdin)) {
        struct TapeCell* tape = preload_tape();
        struct Process* processes = (struct Process*)malloc(sizeof(struct Process));
        processes->state = 0;
        processes->steps = 0;
        processes->tape = tape;
        processes->prev = processes;
        processes->next = processes;
        launch_execution(processes);
    }
}

/*
Crea e carica il nastro con i valori da file
*/
int load_stdin() {
    // inizializza le strutture dati
    for (int i = 0; i < TRANSITION_HASH_SIZE; i++) transitions[i] = NULL;
    // leggi file
    char mode[4];
    fgets(mode,4,stdin);
    load_transitions();
    fgets(mode,4,stdin);
    load_terminal_states();
    fgets(mode,4,stdin);
    scanf("%d\n", &steps_limit);
    fgets(mode,4,stdin);
    create_executions();
    return 0;
}

int main(int argc, char* argv[]) {
    load_stdin();
    free_static_data();
    return 0;
}
