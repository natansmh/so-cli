#include "../../include/scheduler.h"
#include "../../include/globals.h"
#include "../../include/metrics_utils.h"
#include "../../include/memory.h"
#include <stdlib.h>

bool has_executing_process(int running_process) {
    return running_process != -1;
}

void initialize_default_processes() {
    // Exemplo de processo 1 (dados padrão para demonstração)
    processes[0].id = 1;
    processes[0].arrival_time = 0;
    processes[0].execution_time = 5;
    processes[0].remaining_time = processes[0].execution_time;
    processes[0].priority = 1;
    processes[0].deadline = 10;

    // Exemplo de processo 2
    processes[1].id = 2;
    processes[1].arrival_time = 2;
    processes[1].execution_time = 3;
    processes[1].remaining_time = processes[1].execution_time;
    processes[1].priority = 2;
    processes[1].deadline = 8;

    // Exemplo de processo 3
    processes[2].id = 3;
    processes[2].arrival_time = 4;
    processes[2].execution_time = 4;
    processes[2].remaining_time = processes[2].execution_time;
    processes[2].priority = 3;
    processes[2].deadline = 15;

    // Exemplo de processo 4
    processes[3].id = 4;
    processes[3].arrival_time = 6;
    processes[3].execution_time = 6;
    processes[3].remaining_time = processes[3].execution_time;
    processes[3].priority = 5;
    processes[3].deadline = 21;

    // Exemplo de processo 5
    processes[4].id = 5;
    processes[4].arrival_time = 10;
    processes[4].execution_time = 2;
    processes[4].remaining_time = processes[4].execution_time;
    processes[4].priority = 4;
    processes[4].deadline = 18;

    // Exemplo de processo 6
    processes[5].id = 6;
    processes[5].arrival_time = 12;
    processes[5].execution_time = 12;
    processes[5].remaining_time = processes[5].execution_time;
    processes[5].priority = 6;
    processes[5].deadline = 35;

    num_processes = 6;

    // Aloca e inicializa os vetores de timeline
    for (int i = 0; i < num_processes; i++) {
        processes[i].timeline = malloc(TOTAL_TIME * sizeof(ProcessState));
        processes[i].page_fault_occurred = malloc(TOTAL_TIME * sizeof(bool));
        for (int t = 0; t < TOTAL_TIME; t++) {
            processes[i].timeline[t] = NOT_ARRIVED;
            processes[i].page_fault_occurred[t] = false;
        }
    }
}

void execute_fifo() {
    // FIFO (não-preemptivo): escolhe o processo que chegou primeiro
    // e o executa até terminar; só então passa para o próximo.
    int running_process = -1;        // Índice do processo na CPU; -1 = CPU livre
    int process_completed = 0;       // Contador de processos finalizados

    for (int t = 0; t < TOTAL_TIME && process_completed < num_processes; t++) {
        // Se CPU está livre, seleciona o próximo processo por menor arrival_time
        if (running_process == -1) {
            // ========== PECULIARIDADE FIFO: seleção por menor arrival_time (primeiro que chega) ==========
            // Escolha: menor tempo de chegada entre os processos prontos e não concluídos
            int earliest_arrival = TOTAL_TIME + 1;  // Valor sentinela para comparação
            running_process = -1;

            for (int i = 0; i < num_processes; i++) {
                if (processes[i].arrival_time <= t &&
                    processes[i].remaining_time > 0 &&
                    processes[i].arrival_time < earliest_arrival) {
                    earliest_arrival = processes[i].arrival_time;
                    running_process = i;
                }
            }
        }

        // Atualiza estados da timeline para todos os processos neste tique
        for (int i = 0; i < num_processes; i++) {
            if (i == running_process && running_process != -1) {
                // Memória: verifica e marca falha de página (não bloqueia a CPU)
                if (memory_enabled && processes[i].page_fault_occurred) {
                    bool page_fault = check_page_fault(i);
                    processes[i].page_fault_occurred[t] = page_fault;
                }

                // Execução normal do processo ativo
                processes[i].timeline[t] = EXECUTING;
                processes[i].remaining_time--;
                current_time_global = t;

                // Conclusão: se terminou, libera a CPU
                if (processes[i].remaining_time <= 0) {
                    process_completed++;
                    running_process = -1;  // Free CPU for next process
                }
            } else if (processes[i].arrival_time <= t && processes[i].remaining_time > 0) {
                // Processo já chegou e ainda tem trabalho: permanece em espera
                processes[i].timeline[t] = WAITING;
            } else if (processes[i].arrival_time > t) {
                // Processo ainda não chegou
                processes[i].timeline[t] = NOT_ARRIVED;
            } else {
                // Processo já foi concluído
                processes[i].timeline[t] = COMPLETED;
            }
        }

        // Persistir estado da memória para animação/visualização
        if (memory_enabled) {
            save_memory_state(t);
        }
    }
}

void execute_sjf() {
    // SJF (não-preemptivo): escolhe o processo com menor tempo total de execução
    // entre os que já chegaram. Executa até terminar.
    int running_process = -1;        // Índice do processo na CPU; -1 = CPU livre
    int process_completed = 0;       // Contador de processos finalizados

    for (int t = 0; t < TOTAL_TIME && process_completed < num_processes; t++) {

        if (running_process == -1) {
            // ========== PECULIARIDADE SJF: seleção por menor execution_time total ==========
            // Seleção pelo menor execution_time entre os prontos
            int shortest_time = TOTAL_TIME + 1;  // Valor sentinela para comparação
            running_process = -1;

            // Find process with least total execution time
            for (int i = 0; i < num_processes; i++) {
                if (processes[i].arrival_time <= t &&
                    processes[i].remaining_time > 0 &&
                    processes[i].execution_time < shortest_time) {
                    shortest_time = processes[i].execution_time;
                    running_process = i;
                }
            }
        }

        // Atualiza estados da timeline
        for (int i = 0; i < num_processes; i++) {
            if (i == running_process && running_process != -1) {
                // Memória: marca falha de página, se ocorrer
                if (memory_enabled && processes[i].page_fault_occurred) {
                    bool page_fault = check_page_fault(i);
                    processes[i].page_fault_occurred[t] = page_fault;
                }

                // Execução do processo ativo
                processes[i].timeline[t] = EXECUTING;
                processes[i].remaining_time--;
                current_time_global = t;

                // Se terminou, libera CPU
                if (processes[i].remaining_time <= 0) {
                    process_completed++;
                    running_process = -1;
                }

            } else if (processes[i].arrival_time <= t && processes[i].remaining_time > 0) {
                processes[i].timeline[t] = WAITING;
            } else if (processes[i].arrival_time > t) {
                processes[i].timeline[t] = NOT_ARRIVED;
            } else {
                processes[i].timeline[t] = COMPLETED;
            }
        }

        // Persistir estado da memória
        if (memory_enabled) {
            save_memory_state(t);
        }
    }
}

void execute_edf() {
    // EDF (preemptivo com quantum e overhead): escolhe a menor deadline.
    // Ao expirar quantum ou trocar de processo, aplica OVERHEAD.
    int current_quantum = 0;         // Contador de tiques do quantum atual
    int running_process = -1;        // Índice do processo na CPU; -1 = CPU livre
    int overhead_remaining = 0;      // Tiques restantes de overhead de contexto

    // Inicialização: zera overhead e preenche timeline com NOT_ARRIVED
    for (int i = 0; i < num_processes; i++) {
        processes[i].remaining_time = processes[i].execution_time;
        processes[i].overhead = false;
        for (int t = 0; t < TOTAL_TIME; t++) {
            processes[i].timeline[t] = NOT_ARRIVED;
        }
    }

    for (int t = 0; t < TOTAL_TIME; t++) {
        // Parte 1 (EDF): controla quantum e término do processo atual
        if (running_process != -1) {
            bool needs_preemption = (current_quantum >= quantum);
            bool has_finished = (processes[running_process].remaining_time <= 0);

            if (needs_preemption || has_finished) {
                if (needs_preemption && processes[running_process].remaining_time > 0) {
                    processes[running_process].overhead = true;
                    overhead_remaining = overhead_time;
                }
                running_process = -1;
                current_quantum = 0;
            }
        }

        // Parte 2 (EDF): escolhe o pronto com menor deadline (CPU livre e sem OVERHEAD)
        if (!has_executing_process(running_process) && overhead_remaining == 0) {
            int earliest_deadline = TOTAL_TIME + 1;
            running_process = -1;

            for (int i = 0; i < num_processes; i++) {
                if (processes[i].arrival_time <= t &&
                    processes[i].remaining_time > 0 &&
                    processes[i].deadline < earliest_deadline) {
                    earliest_deadline = processes[i].deadline;
                    running_process = i;
                }
            }
        }

        // Parte 3 (EDF): escreve estados no tempo t (overhead, execução, deadline estourado)
        for (int i = 0; i < num_processes; i++) {
            if (processes[i].overhead) {
                processes[i].timeline[t] = OVERHEAD;
                overhead_remaining--;
                if (overhead_remaining == 0) {
                    processes[i].overhead = false;
                }
            } else if (i == running_process && has_executing_process(running_process)) {
                if (memory_enabled && processes[i].page_fault_occurred) {
                    bool page_fault = check_page_fault(i);
                    processes[i].page_fault_occurred[t] = page_fault;
                }

                if (t - processes[i].arrival_time >= processes[i].deadline) {
                    processes[i].timeline[t] = DEADLINE_MISSED;
                } else {
                    processes[i].timeline[t] = EXECUTING;
                }
                processes[i].remaining_time--;
                current_quantum++;
                current_time_global = t;
            } else if (processes[i].arrival_time <= t && processes[i].remaining_time > 0) {
                processes[i].timeline[t] = WAITING;
            } else if (processes[i].arrival_time > t) {
                processes[i].timeline[t] = NOT_ARRIVED;
            } else {
                processes[i].timeline[t] = COMPLETED;
            }
        }

        // Persistir estado da memória
        if (memory_enabled) {
            save_memory_state(t);
        }
    }
}

void execute_rr() {
    // Round Robin (preemptivo): fila por ordem de chegada; cada processo
    // roda por um quantum. Ao expirar sem terminar, aplica OVERHEAD e reentra na fila.
    int current_quantum = 0;                  // Contador de tiques do quantum atual
    int running_process = -1;                 // Índice do processo na CPU; -1 = CPU livre
    int process_queue[MAX_PROCESSES];         // Fila de processos prontos (índices)
    int queue_size = 0;                       // Tamanho atual da fila
    int overhead_remaining = 0;               // Tiques restantes de overhead de contexto

    // Loop principal no tempo: constrói e utiliza a fila de prontos
    for (int t = 0; t < TOTAL_TIME; t++) {
        // ========== PECULIARIDADE RR parte 1: fila circular (FIFO) de processos prontos ==========
        // Entrada na fila: processos que chegam neste tique
        for (int i = 0; i < num_processes; i++) {              // Varre todos os processos
            if (processes[i].arrival_time == t) {              // Chegou exatamente neste tique?
                process_queue[queue_size++] = i;               // Adiciona no fim da fila (empilha)
            }
        }

        // Verifica expiração de quantum ou término do processo atual
        if (has_executing_process(running_process)) {          // Há processo rodando?
            bool needs_preemption = (current_quantum >= quantum);                   // Quantum expirou?
            bool has_finished = (processes[running_process].remaining_time <= 0);  // Terminou todo trabalho?

            if (needs_preemption || has_finished) {            // Preempção ou término?
                // ========== PECULIARIDADE RR parte 2: reencadeia processo na fila após quantum ==========
                // Quantum expirou sem terminar: aplica OVERHEAD e reencadeia na fila
                if (needs_preemption && processes[running_process].remaining_time > 0) { // Quantum acabou mas ainda tem trabalho?
                    processes[running_process].overhead = true;     // Marca que vai sofrer OVERHEAD
                    process_queue[queue_size++] = running_process;  // Reinsere no fim da fila
                    overhead_remaining = overhead_time;             // Define duração do OVERHEAD
                }
                running_process = -1;                               // Libera CPU
                current_quantum = 0;                                // Zera contador de quantum
            }
        }

        // Seleção do próximo da fila (se não há OVERHEAD em curso)
        // Executado quando: CPU livre E há processos na fila E não há OVERHEAD
        if (!has_executing_process(running_process) && queue_size > 0 && overhead_remaining == 0) { // Condições atendidas?
            running_process = process_queue[0];                     // Escolhe o primeiro da fila (FIFO)
            // Remove da fila deslocando todos os elementos para frente
            for (int i = 0; i < queue_size - 1; i++) {             // Para cada elemento (exceto último)
                process_queue[i] = process_queue[i + 1];            // Desloca para posição anterior
            }
            queue_size--;                                          // Diminui tamanho da fila
        }

        // Atualiza estados para todos os processos
        for (int i = 0; i < num_processes; i++) {              // Para cada processo
            if (processes[i].overhead) {                           // Está em OVERHEAD?
                processes[i].timeline[t] = OVERHEAD;               // Marca OVERHEAD na timeline

                overhead_remaining--;                              // Decrementa duração de OVERHEAD
                if (overhead_remaining == 0) {                     // OVERHEAD acabou?
                    processes[i].overhead = false;                 // Limpa flag de OVERHEAD
                }
            }
            else if (i == running_process && has_executing_process(running_process)) { // É o processo ativo?
                // Memória: marca falha de página, se ocorrer
                if (memory_enabled && processes[i].page_fault_occurred) {
                    bool page_fault = check_page_fault(i);         // Verifica page fault
                    processes[i].page_fault_occurred[t] = page_fault; // Registra ocorrência
                }

                // Execução do processo atual por quantum
                processes[i].timeline[t] = EXECUTING;              // Marca EXECUTING na timeline
                processes[i].remaining_time--;                     // Consome 1 tique de execução
                current_quantum++;                                 // Avança contador de quantum
                current_time_global = t;                           // Atualiza tempo global p/ memória
            } else if (processes[i].arrival_time <= t && processes[i].remaining_time > 0) { // Chegou e tem trabalho?
                processes[i].timeline[t] = WAITING;
            } else if (processes[i].arrival_time > t) {
                processes[i].timeline[t] = NOT_ARRIVED;
            } else {
                processes[i].timeline[t] = COMPLETED;
            }
        }
    }
}

void execute_cfs() {
    // CFS simplificado (preemptivo com OVERHEAD): escolhe o processo com menor vruntime
    // atualiza vruntime ponderado por prioridade; troca com custo quando necessário.
    int running_process = -1;          // Índice do processo na CPU; -1 = CPU livre
    int process_completed = 0;         // Contador de processos finalizados
    int overhead_remaining = 0;        // Tiques restantes de overhead de contexto
    int preempted_process = -1;        // Processo preemptado (usado para marcar OVERHEAD)
    const double EPSILON = 1e-9;       // Tolerância para comparação de vruntime (doubles)

    // Inicialização: prepara todos os processos antes de começar a simulação
    for (int i = 0; i < num_processes; i++) {                   // Para cada processo
        processes[i].remaining_time = processes[i].execution_time; // Tempo restante inicial
        processes[i].vruntime = -1.0;                               // -1 sinaliza que ainda não chegou
        for (int t = 0; t < TOTAL_TIME; t++) {                     // Inicializa toda a timeline
            processes[i].timeline[t] = NOT_ARRIVED;                 // Preenche com NOT_ARRIVED
        }
    }

    // Loop principal de tempo
    for (int t = 0; t < TOTAL_TIME && process_completed < num_processes; t++) {
        // ========== PECULIARIDADE CFS parte 1: inicializa vruntime = tempo atual na chegada ==========
        // Inicialização de vruntime no momento da chegada
        for (int i = 0; i < num_processes; i++) {                   // Varre processos
            if (processes[i].arrival_time == t && processes[i].vruntime < 0) { // Chegou agora e ainda não foi inicializado?
                processes[i].vruntime = (double)t;                  // Define vruntime = tempo atual
            }
        }

        if (overhead_remaining > 0) {                               // Há OVERHEAD em curso?
            // Período de OVERHEAD: marca timeline e aguarda terminar
            for (int i = 0; i < num_processes; i++) {               // Para cada processo
                if (i == preempted_process) {                       // É o processo que foi preemptado?
                    processes[i].timeline[t] = OVERHEAD;            // Marca OVERHEAD na timeline
                } else if (processes[i].arrival_time <= t && processes[i].remaining_time > 0) { // Pronto?
                    processes[i].timeline[t] = WAITING;             // Marca WAITING
                } else if (processes[i].remaining_time <= 0) {      // Já completou?
                    processes[i].timeline[t] = COMPLETED;           // Marca COMPLETED
                }
            }
            overhead_remaining--;                                   // Decrementa tempo de OVERHEAD
            running_process = -1;                                   // CPU livre durante OVERHEAD
            if (overhead_remaining == 0) {                          // Acabou o OVERHEAD?
                preempted_process = -1;                             // Limpa marcador de preempção
            }
            continue;                                               // Pula para próximo tique
        }

        // Verifica se o processo atual completou todo seu trabalho (não por quantum, mas por término total)
        // Executado quando: não há OVERHEAD em curso E há processo rodando E esse processo zerou remaining_time
        if (has_executing_process(running_process) && processes[running_process].remaining_time <= 0) { // Processo atual terminou?
            process_completed++;                                    // Incrementa contador de concluídos
            running_process = -1;                                   // Libera CPU para próxima seleção
        }

        // ========== PECULIARIDADE CFS parte 2: seleciona menor vruntime (empate: maior índice) ==========
        // Seleciona o menor vruntime; em empate, escolhe maior índice para estabilidade
        int selected_process = -1;           // Processo escolhido neste tique
        double min_vruntime_prontos = -1.0;  // Menor vruntime encontrado entre prontos

        for (int i = 0; i < num_processes; i++) {                   // Varre todos os processos
            if (processes[i].arrival_time <= t && processes[i].remaining_time > 0 && processes[i].vruntime >= 0.0) { // Está pronto?

                // Critério de seleção: menor vruntime OU empate + maior índice
                if (selected_process == -1 ||                        // Primeiro candidato? OU
                    processes[i].vruntime < min_vruntime_prontos - EPSILON || // vruntime menor? OU
                    (fabs(processes[i].vruntime - min_vruntime_prontos) < EPSILON && i > selected_process)) // Empate e índice maior?
                {
                    min_vruntime_prontos = processes[i].vruntime;   // Atualiza menor vruntime
                    selected_process = i;                            // Atualiza processo escolhido
                }
            }
        }

        int current_rp = running_process;  // Salva o processo que estava rodando antes da seleção
        running_process = selected_process; // Define novo processo escolhido

        // Se havia processo rodando e o selecionado é diferente, avalia preempção/empate
        if (current_rp != -1 && selected_process != current_rp) {    // Troca de processo?
            // Preempção estrita: novo processo tem vruntime significativamente menor
            int is_strict_preemption = (processes[current_rp].vruntime > processes[selected_process].vruntime + EPSILON); // vruntime atual > novo + tolerância
            // Empate: vruntime praticamente igual (troca por índice maior)
            int is_tie_switch = (fabs(processes[current_rp].vruntime - processes[selected_process].vruntime) < EPSILON); // Diferença < tolerância

            if (is_strict_preemption || is_tie_switch)
            {
                overhead_remaining = overhead_time;
                preempted_process = current_rp;
                running_process = -1;
                t--; // Decrement time to make selected process run on next tick (after overhead)
                continue;
            }
        }

        if (has_executing_process(running_process)) {
            int i = running_process;

            // Memória: marca falha de página, se ocorrer
            if (memory_enabled && processes[i].page_fault_occurred) {
                bool page_fault = check_page_fault(i);
                processes[i].page_fault_occurred[t] = page_fault;
            }

            // ========== PECULIARIDADE CFS parte 3: atualiza vruntime com peso de prioridade ==========
            // Execução e atualização de vruntime ponderado pela prioridade
            processes[i].timeline[t] = EXECUTING;

            int delta_t = 1;  // Incremento de tempo real (1 tique)
            // vruntime_i = vruntime_i + Delta_t * w(prioridade_i)
            double priority_weight = pow(1.25, (double)processes[i].priority - 1.0);  // Peso: 1.25^(prioridade-1)
            processes[i].vruntime += delta_t * priority_weight;
            processes[i].remaining_time--;

            current_time_global = t;  // Atualiza tempo global para LRU
        }

        for (int i = 0; i < num_processes; i++) {
            if (processes[i].timeline[t] != EXECUTING && processes[i].timeline[t] != OVERHEAD) {
                if (processes[i].arrival_time <= t && processes[i].remaining_time > 0)
                    processes[i].timeline[t] = WAITING;
                else if (processes[i].remaining_time <= 0)
                    processes[i].timeline[t] = COMPLETED;
                else if (processes[i].arrival_time > t)
                    processes[i].timeline[t] = NOT_ARRIVED;
            }
        }

        // Persistir estado da memória
        if (memory_enabled) {
            save_memory_state(t);
        }
    }
}

void run_current_algorithm() {
    // Inicializa o sistema de memória, se habilitado
    if (memory_enabled) {
        init_memory_system();
    }

    switch (current_algorithm) {
        case 0: execute_fifo(); break;
        case 1: execute_sjf(); break;
        case 2: execute_edf(); break;
        case 3: execute_rr(); break;
        case 4: execute_cfs(); break;
    }

    // Após executar o algoritmo escolhido, computa as métricas de resumo
    compute_metrics_for_all();
    compute_summary_stats();
}

void reset_simulation() {
    current_time = 0;
    for (int i = 0; i < num_processes; i++) {
        processes[i].remaining_time = processes[i].execution_time;
        for (int t = 0; t < TOTAL_TIME; t++) {
            processes[i].timeline[t] = NOT_ARRIVED;
            if (processes[i].page_fault_occurred) {
                processes[i].page_fault_occurred[t] = false;
            }
        }
    }
}
