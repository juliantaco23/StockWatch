// user_management.c
#include "user_management.h"
#include <string.h>
#include <stdio.h>

#define MAX_USERS 10
#define ID_LENGTH 7
#define PASSWORD_LENGTH 5
#define MAX_FAILED_ATTEMPTS 3

typedef struct {
    char id[ID_LENGTH];
    char password[PASSWORD_LENGTH];
    int failed_attempts;
} User;

User users[MAX_USERS];

void user_init() {
    // Initialize users with predefined IDs and passwords
    const char* predefined_ids[] = {"123456", "567891", "987654", "432198", "111111", 
                                    "555555", "999999", "777777", "333333", "444444"};
    const char* predefined_passwords[] = {"1234", "6543", "9876", "1231", "2222", 
                                          "5555", "9999", "7777", "3333", "4444"};

    for (int i = 0; i < MAX_USERS; i++) {
        strncpy(users[i].id, predefined_ids[i], ID_LENGTH - 1);
        users[i].id[ID_LENGTH - 1] = '\0';
        strncpy(users[i].password, predefined_passwords[i], PASSWORD_LENGTH - 1);
        users[i].password[PASSWORD_LENGTH - 1] = '\0';
        users[i].failed_attempts = 0;
    }
}

bool verify_user(const char* id, const char* password) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(id, users[i].id) == 0) {
            if (strcmp(password, users[i].password) == 0) {
                users[i].failed_attempts = 0;
                return true;
            } else {
                users[i].failed_attempts++;
                return false;
            }
        }
    }
    return false;
}

bool change_password(const char* id, const char* new_password) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(id, users[i].id) == 0) {
            strncpy(users[i].password, new_password, PASSWORD_LENGTH - 1);
            users[i].password[PASSWORD_LENGTH - 1] = '\0';
            return true;
        }
    }
    return false;
}

void reset_failed_attempts(const char* id) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(id, users[i].id) == 0) {
            users[i].failed_attempts = 0;
            break;
        }
    }
}

bool is_user_blocked(const char* id) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(id, users[i].id) == 0) {
            return users[i].failed_attempts >= MAX_FAILED_ATTEMPTS;
        }
    }
    return false;
}