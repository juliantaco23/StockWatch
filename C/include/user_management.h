// user_management.h
#ifndef USER_MANAGEMENT_H
#define USER_MANAGEMENT_H

#include <stdbool.h>

void user_init();
bool verify_user(const char* id, const char* password);
bool change_password(const char* id, const char* new_password);
void reset_failed_attempts(const char* id);
bool is_user_blocked(const char* id);

#endif // USER_MANAGEMENT_H

