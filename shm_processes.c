#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define SHM_SIZE 2  // Two integers: BankAccount and Turn

int main() {
    int shm_id;
    int *ShmPTR;

    // Seed random number generator
    srand(time(NULL));

    // Create shared memory segment
    shm_id = shmget(IPC_PRIVATE, SHM_SIZE * sizeof(int), IPC_CREAT | 0666);
    if (shm_id < 0) {
        printf("shmget error\n");
        exit(1);
    }

    // Attach shared memory to process
    ShmPTR = (int *) shmat(shm_id, NULL, 0);
    if ((long) ShmPTR == -1) {
        printf("shmat error\n");
        exit(1);
    }

    // Initialize shared variables
    ShmPTR[0] = 0;  // BankAccount
    ShmPTR[1] = 0;  // Turn

    pid_t pid = fork();

    if (pid < 0) {
        printf("Fork failed\n");
        exit(1);
    }

    // Parent process (Dear Old Dad)
    if (pid > 0) {
        for (int i = 0; i < 25; i++) {
            sleep(rand() % 6);  // Sleep 0-5 seconds
            
            int account = ShmPTR[0];  // Copy BankAccount to local account
            while (ShmPTR[1] != 0) {
                // Wait until it's the parent's turn
            }

            if (account <= 100) {
                int balance = rand() % 101;  // Generate random deposit
                if (balance % 2 == 0) {
                    account += balance;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
                } else {
                    printf("Dear old Dad: Doesn't have any money to give\n");
                }
            } else {
                printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
            }

            ShmPTR[0] = account;  // Write updated balance to BankAccount
            ShmPTR[1] = 1;  // Set Turn to 1 (Child's turn)
        }

        // Wait for the child to finish
        wait(NULL);

        // Detach shared memory
        shmdt((void *) ShmPTR);

        // Remove shared memory segment
        shmctl(shm_id, IPC_RMID, NULL);

    } else {  // Child process (Poor Student)
        for (int i = 0; i < 25; i++) {
            sleep(rand() % 6);  // Sleep 0-5 seconds

            int account = ShmPTR[0];  // Copy BankAccount to local account
            while (ShmPTR[1] != 1) {
                // Wait until it's the child's turn
            }

            int balance = rand() % 51;  // Generate random withdrawal amount
            printf("Poor Student needs $%d\n", balance);

            if (balance <= account) {
                account -= balance;
                printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Poor Student: Not Enough Cash ($%d)\n", account);
            }

            ShmPTR[0] = account;  // Write updated balance to BankAccount
            ShmPTR[1] = 0;  // Set Turn to 0 (Parent's turn)
        }

        // Child process exits
        exit(0);
    }

    return 0;
}